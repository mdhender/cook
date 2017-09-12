/*
 *      cook - file construction tool
 *      Copyright (C) 1995-2001, 2004, 2006-2009 Peter Miller
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 3 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program. If not, see
 *      <http://www.gnu.org/licenses/>.
 */

#include <common/ac/errno.h>
#include <common/ac/fcntl.h>
#include <common/ac/limits.h>
#include <common/ac/signal.h>
#include <common/ac/stddef.h>
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>
#include <common/ac/time.h>
#include <common/ac/unistd.h>

#include <cook/archive.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <common/ac/utime.h>

#include <common/error_intl.h>
#include <common/exeext.h>
#include <common/home_directo.h>
#include <common/mem.h>
#include <cook/option.h>
#include <cook/os_interface.h>
#include <cook/os/wait.h>
#include <cook/stat.cache.h>
#include <common/str_list.h>
#include <cook/tempfilename.h>
#include <common/trace.h>

#ifdef HAVE_GETRUSAGE
#include <sys/resource.h>
#else
#include <sys/times.h>
#endif


/*
 * NAME
 *      os_mtime - return the last-modified time of a file
 *
 * SYNOPSIS
 *      time_t os_mtime(string_ty *path);
 *
 * DESCRIPTION
 *      Os_mtime returns the time the named file was last modified.
 *      It returns 0 if the file does not exist.
 *
 * CAVEAT
 *      Assumes time will be the UNIX format.
 */

time_t
os_mtime_oldest(string_ty *path)
{
    time_t          result;

    trace(("os_mtime_oldest(path = \"%s\")\n{\n", path->str_text));
    result = stat_cache_oldest(path, 1);
    trace(("return (%ld) %s", (long)result, ctime(&result)));
    trace(("}\n"));
    return result;
}

time_t
os_mtime_newest(string_ty *path)
{
    time_t          result;

    trace(("os_mtime_newest(path = \"%s\")\n{\n", path->str_text));
    result = stat_cache_newest(path, 1);
    trace(("return (%ld) %s", (long)result, ctime(&result)));
    trace(("}\n"));
    return result;
}


static void
adjust_message(string_ty *path, long nsec)
{
    int             direction;
    string_ty       *buffer;
    sub_context_ty  *scp;

    /*
     * work out the direction of adjustment
     */
    trace(("adjust_message(nsec = %ld)\n{\n", nsec));
    if (nsec >= 0)
        direction = 1;
    else
    {
        nsec = -nsec;
        direction = -1;
    }

    /*
     * work out the units of adjustment
     */
    scp = sub_context_new();
    if (nsec >= (365L * 24 * 60 * 60))
    {
        sub_var_set
        (
            scp,
            "Number",
            "%4.2f",
            nsec / (double)(365L * 24 * 60 * 60)
        );
        buffer = subst_intl(scp, i18n("$number years"));
    }
    else if (nsec >= (24L * 60 * 60))
    {
        sub_var_set_long(scp, "Number1", nsec / (24L * 60 * 60));
        sub_var_set_long(scp, "Number2", (nsec / (60 * 60)) % 24);
        buffer = subst_intl(scp, i18n("$number1 days $number2 hours"));
    }
    else if (nsec >= (60 * 60))
    {
        sub_var_set_long(scp, "Number1", nsec / (60 * 60));
        sub_var_set_long(scp, "Number2", (nsec / 60) % 60);
        buffer = subst_intl(scp, i18n("$number1 hours $number2 minutes"));
    }
    else if (nsec >= 60)
    {
        sub_var_set_long(scp, "Number1", nsec / 60);
        sub_var_set_long(scp, "Number2", nsec % 60);
        buffer = subst_intl(scp, i18n("$number1 minutes $number2 seconds"));
    }
    else
    {
        sub_var_set_long(scp, "Number", nsec);
        buffer = subst_intl(scp, i18n("$number seconds"));
    }

    /*
     * print the message
     * (re-use substitution context)
     */
    trace(("mark\n"));
    sub_var_set_string(scp, "File_Name", path);
    sub_var_set_string(scp, "Number", buffer);
    error_intl
    (
        scp,
        (
            direction < 0
        ?
            i18n("adjusting \"$filename\" back $number")
        :
            i18n("adjusting \"$filename\" forward $number")
        )
    );
    str_free(buffer);
    sub_context_delete(scp);
    trace(("}\n"));
}

#ifdef __CYGWIN32__

/*
 * NAME
 *      utimes4cygwin
 *
 * DESCRIPTION
 *      On Cygwin, some files magically get an .exe ending to the
 *      filename.  Therefore, we insert an extra check after all other
 *      handling has been done to check for that case.  This fix needs to
 *      be here because os_exists(filename) returns true if "filename"
 *      or "filename.exe" exist, but utime(filename) only works with
 *      "filename". So we do this here, after we know that there is no
 *      file called "filename".
 *
 * RETURNS
 *      int; 0 on success, -1 and errno on failure.
 *
 * AUTHOR
 *      This code was contributed by Adam Schlegel <aceschle@thinkage.ca>
 */

static int
utimes4cygwin(string_ty *path, struct utimbuf *ut)
{
    string_ty       *fullpath;
    int             j;
    const char      *suffix;
    int             err;

    for (j = 0;; j++)
    {
        suffix = exeext_nth(j);
        if (!suffix || !*suffix)
        {
            errno = ENOENT;
            return -1;
        }
        fullpath = str_catenate(path, str_from_c(suffix));
        err = utime(fullpath->str_text, ut);
        if (!err || errno != ENOENT)
        {
            str_free(fullpath);
            return err;
        }
        str_free(fullpath);
    }
}

#endif

/*
 * NAME
 *      os_mtime_adjust - indicate change
 *
 * SYNOPSIS
 *      int os_mtime_adjust(string_ty *path);
 *
 * DESCRIPTION
 *      The os_mtime_adjust function is used to adjust the value in the stat
 *      cache to indicate that a recipe has constructed a file, and thus
 *      changed it last-modified time.  No change to the actual file system
 *      occurs.
 *
 * RETURNS
 *      int; -1 on error, 0 on success
 */

int
os_mtime_adjust(string_ty *path, time_t min_age)
{
    time_t          mtime;
    int             err;

    trace(("os_mtime_adjust(path = \"%s\")\n{\n", path->str_text));
    if (option_test(OPTION_UPDATE) && option_test(OPTION_ACTION))
    {
        stat_cache_clear(path);
        mtime = os_mtime_newest(path);
        if (mtime < 0)
        {
            trace(("return -1;\n"));
            trace(("}\n"));
            return -1;
        }
        if (mtime)
        {
            if
            (
                option_test(OPTION_UPDATE_MAX)
            ?
                mtime != min_age
            :
                mtime < min_age
            )
            {
                struct utimbuf  ut;

                if (!option_test(OPTION_SILENT))
                {
                    adjust_message(path, (long)min_age - (long)mtime);
                }
                ut.modtime = min_age;
                ut.actime = min_age;
                err = utime(path->str_text, &ut);
                if (err && errno == ENOENT)
                {
                    errno = 0;
                    err = archive_utime(path, &ut);
                }
#ifdef __CYGWIN32__
                if (err && errno == ENOENT)
                {
                    errno = 0;
                    err = utimes4cygwin(path, &ut);
                }
#endif /* __CYGWIN32__ */
                if (err && errno == EPERM)
                {
                    sub_context_ty  *scp;

                    /*
                     * The only way to get this
                     * error message is if the
                     * recipe body invoked sudo (or
                     * similar) to change the owner.
                     *
                     * They did it to themselves, so
                     * just give them a warning.
                     */
                    scp = sub_context_new();
                    sub_errno_set(scp);
                    sub_var_set_string(scp, "File_Name", path);
                    error_intl
                    (
                        scp,
                        i18n("warning: when adjusting \"$filename\": $errno")
                    );
                    trace(("return 0;\n"));
                    trace(("}\n"));
                    return 0;
                }
                if (err)
                {
                    sub_context_ty  *scp;

                    scp = sub_context_new();
                    sub_errno_set(scp);
                    sub_var_set_string(scp, "File_Name", path);
                    error_intl(scp, i18n("utime $filename: $errno"));
                    sub_context_delete(scp);
                    option_set_errors();
                    trace(("return -1;\n"));
                    trace(("}\n"));
                    return -1;
                }
#if defined(sun) || defined(__sun__)
                else
                {
                    /*
                     * Solaris is brain dead.
                     * The inode times are not
                     * updated until the next sync.
                     * So we'll help it along.
                     */
                    sync();
                }
#endif
                stat_cache_set(path, min_age, 1);
            }
        }
        else
        {
            /*
             * file was deleted (or was a dummy)
             * so pretend it was changed "now"
             */
            time(&mtime);
            if (mtime > min_age)
                min_age = mtime;
            stat_cache_set(path, min_age, 0);
        }
    }
    else
    {
        time(&mtime);
        if (mtime > min_age)
            min_age = mtime;
        stat_cache_set(path, min_age, 0);
    }
    trace(("return 0;\n"));
    trace(("}\n"));
    return 0;
}


/*
 * NAME
 *      os_clear_stat - invalidate cache
 *
 * SYNOPSIS
 *      int os_clear_stat(string_ty *path);
 *
 * DESCRIPTION
 *      The os_clear_stat function is used to invalidate the the stat
 *      cache to indicate that a recipe has constructed a file, and thus
 *      changed it last-modified time.  No change to the actual file system
 *      occurs.
 *
 * RETURNS
 *      int; 0 on success, -1 on error
 *
 * CAVEAT
 *      This is used in situations where the recipe changes a file not named
 *      in the targets of the recipe.  This usually occurs around mkdir, rm
 *      and mv commands, used in conjunction with the [exists] builtin function.
 */

int
os_clear_stat(string_ty *path)
{
    stat_cache_clear(path);
    return 0;
}


/*
 * NAME
 *      os_touch - update the modify time of the file
 *
 * SYNOPSIS
 *      int os_touch(string_ty *path);
 *
 * DESCRIPTION
 *      Os_touch updates the last-modified time of the file to the present.
 *      If the named file does not exist, then nothing is done.
 *
 * RETURNS
 *      int; 0 on success, -1 on error
 */

int
os_touch(string_ty *path)
{
    struct utimbuf  ut;
    int             err;

    time(&ut.modtime);
    if (ut.modtime < 0)
        ut.modtime = 0;
    ut.actime = ut.modtime;
    err = utime(path->str_text, &ut);
    if (err && errno == ENOENT)
        err = archive_utime(path, &ut);
    if (err)
    {
        sub_context_ty  *scp;

        if (errno == ENOENT)
        {
            stat_cache_clear(path);
            return 0;
        }
        scp = sub_context_new();
        sub_errno_set(scp);
        sub_var_set_string(scp, "File_Name", path);
        error_intl(scp, i18n("utime $filename: $errno"));
        sub_context_delete(scp);
        option_set_errors();
        return -1;
    }
#if defined(sun) || defined(__sun__)
    else
    {
        /*
         * Solaris is brain dead.  The inode times are not
         * updated until the next sync.  So we'll help it along.
         */
        sync();
    }
#endif
    stat_cache_set(path, ut.modtime, 1);
    return 0;
}


/*
 * NAME
 *      os_delete - delete a file
 *
 * SYNOPSIS
 *      int os_delete(string_ty *path);
 *
 * DESCRIPTION
 *      Os_delete deletes the named file.
 *      If it does not exist, no error is given.
 *
 * RETURNS
 *      int; -1 on error, 0 on success
 */

int
os_delete(string_ty *path, int echo)
{
    if (unlink(path->str_text))
    {
        if (errno != ENOENT)
        {
            error_intl_unlink(path->str_text);
            option_set_errors();
            return -1;
        }
    }
    else if (echo)
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        sub_var_set_string(scp, "File_Name", path);
        error_intl(scp, i18n("rm $filename"));
        sub_context_delete(scp);
    }

    /*
     * if the knew about the existence of the file before we deleted
     * it, then we will have to adjust the stat cache.
     */
    stat_cache_clear(path);
    return 0;
}


/*
 * NAME
 *      exit_status - pretty print
 *
 * SYNOPSIS
 *      int exit_status(char *cmd, int status, int errok);
 *
 * DESCRIPTION
 *      The exit_status function is used to pretty print the meaning of the
 *      exit status of the given command.  No output is generated for normal
 *      (0) termination.
 *
 * RETURNS
 *      int: zero if the command succeeded, 1 if it failed.
 *
 * CAVEAT
 *      This function should be static, but func_collect (builtin.c) uses it.
 */

int
exit_status(char *cmd, int status, int errok)
{
    int             a;
    int             b;
    int             c;
    sub_context_ty  *scp;

    a = (status >> 8) & 0xFF;
    b = (status >> 7) & 1;
    c = status & 0x7F;
    switch (c)
    {
    case 0x7F:
        /*
         * process was stopped,
         * since we didn't do it, treat it as an error
         */
        scp = sub_context_new();
        sub_var_set_charstar(scp, "File_Name", cmd);
        error_intl(scp, i18n("command $filename: stopped"));
        sub_context_delete(scp);
        return 1;

    case 0:
        /* normal termination */
        if (a)
        {
            scp = sub_context_new();
            sub_var_set_charstar(scp, "File_Name", cmd);
            sub_var_set_long(scp, "Number", a);
            if (errok)
            {
                int             silent;

                silent = option_test(OPTION_SILENT);
                if (!silent)
                {
                    error_intl
                    (
                        scp,
                        i18n("command $filename: exit status $number (ignored)")
                    );
                }
            }
            else
            {
                error_intl(scp, i18n("command $filename: exit status $number"));
            }
            sub_context_delete(scp);
            if (errok)
                return 0;
            return 1;
        }
        return 0;

    default:
        /*
         * process dies from unhandled condition
         */
        scp = sub_context_new();
        sub_var_set_charstar(scp, "File_Name", cmd);
        sub_var_set_charstar(scp, "Name", safe_strsignal(c));
        if (b)
        {
            error_intl
            (
                scp,
                i18n("command $filename: terminated by $name (core dumped)")
            );
        }
        else
        {
            error_intl(scp, i18n("command $filename: terminated by $name"));
        }
        return 1;
    }
}


/*
 * NAME
 *      execute - do a command
 *
 * SYNOPSIS
 *      int execute(string_list_ty *cmd, int fd, int errok);
 *
 * DESCRIPTION
 *      The execute function is used to execute the command in the word list.
 *      If the file descriptor is >= 0, it indicates a file to use as stdin to
 *      the command.
 *
 * RETURNS
 *      int: zero if the commands succeeds, nonzero if it fails.
 *
 * SEE ALSO
 *      cook/opcode/command.c
 */

static int
execute(string_list_ty *cmd, int fd, int errok)
{
    int             child;
    int             pid;
    int             status;
    static char     **argv;
    static size_t   argvlen;
    size_t          j;
    sub_context_ty  *scp;

    if (!argv)
    {
        argvlen = cmd->nstrings + 1;
        argv = mem_alloc(argvlen * sizeof(char *));
    }
    else
    {
        if (argvlen < cmd->nstrings + 1)
        {
            argvlen = cmd->nstrings + 1;
            argv = mem_change_size(argv, argvlen * sizeof(char *));
        }
    }
    for (j = 0; j < cmd->nstrings; ++j)
        argv[j] = cmd->string[j]->str_text;
    argv[cmd->nstrings] = 0;
    switch (child = fork())
    {
    case -1:
        scp = sub_context_new();
        sub_errno_set(scp);
        error_intl(scp, i18n("fork(): $errno"));
        sub_context_delete(scp);
        return -1;

    case 0:
        if (fd >= 0)
        {
            if (close(0) && errno != EBADF)
            {
                string_ty       *fn0;
                int             err;

                err = errno;
                scp = sub_context_new();
                fn0 = subst_intl(scp, "standard input");
                /* re-use substitution context */
                sub_errno_setx(scp, err);
                sub_var_set_string(scp, "File_Name", fn0);
                fatal_intl(scp, i18n("close $filename: $errno"));
                sub_context_delete(scp);
                str_free(fn0);
            }
            if (dup(fd) < 0)
            {
                scp = sub_context_new();
                sub_errno_set(scp);
                fatal_intl(scp, i18n("dup(): $errno"));
                /* NOTREACHED */
            }
        }
        if (argv[0][0] == '/')
            execv(argv[0], argv);
        else
            execvp(argv[0], argv);
        scp = sub_context_new();
        sub_errno_set(scp);
        sub_var_set_charstar(scp, "File_Name", argv[0]);
        fatal_intl(scp, i18n("exec $filename: $errno"));
        /* NOTREACHED */

    default:
        for (;;)
        {
            pid = os_waitpid(child, &status);
            assert(pid == child || pid == -1);
            if (pid == child)
                return exit_status(argv[0], status, errok);
            if (pid < 0 && errno != EINTR)
            {
                scp = sub_context_new();
                sub_errno_set(scp);
                error_intl(scp, i18n("wait(): $errno"));
                sub_context_delete(scp);
                option_set_errors();
                return -1;
            }
        }
    }
}


int
os_execute_magic_characters(string_ty * s)
{
    const char      *cp;

    cp = s->str_text;
    while (*cp)
    {
        if (strchr("\t\n !\"#$&'()*:;<=>?[\\]^`|", *cp))
            return 1;
        ++cp;
    }
    return 0;
}


int
os_execute_magic_characters_list(string_list_ty * slp)
{
    size_t          j;

    for (j = 0; j < slp->nstrings; ++j)
        if (os_execute_magic_characters(slp->string[j]))
            return 1;
    return 0;
}


/*
 * NAME
 *      os_execute - execute a command
 *
 * SYNOPSIS
 *      int os_execute(string_list_ty *args, string_ty *input, int errok);
 *
 * DESCRIPTION
 *      Os_execute performs the given command.
 *      If the command succeeds 0 is returned.
 *      If the command fails a diagnostic message is given
 *      and 1 is returned.
 *
 * SEE ALSO
 *      cook/opcode/command.c
 */

int
os_execute(string_list_ty *args, string_ty *input, int errok)
{
    int             fd;
    int             retval;

    assert(args);
    assert(args->nstrings > 0);

    fd = -1;
    if (input)
    {
        string_ty       *tfn;

        /*
         * He has given a string to be used as input to the command,
         * so write it out to a file, and then redirect the input.
         */
        tfn = temporary_filename();
        fd = open(tfn->str_text, O_RDWR | O_CREAT | O_TRUNC, 0666);
        if (fd < 0)
        {
            error_intl_open(tfn->str_text);
            str_free(tfn);
          exec_fails:
            option_set_errors();
            retval = -1;
            goto ret;
        }
        if (unlink(tfn->str_text))
        {
            error_intl_unlink(tfn->str_text);
            str_free(tfn);
            goto exec_fails;
        }
        if
        (
            write(fd, input->str_text, input->str_length) < 0
        ||
            lseek(fd, 0L, SEEK_SET)
        )
        {
            error_intl_write(tfn->str_text);
            str_free(tfn);
            goto exec_fails;
        }
        str_free(tfn);
    }

    if (os_execute_magic_characters_list(args))
    {
        string_ty       *str;
        string_list_ty  newcmd;
        char            *cp;

        cp = getenv("SHELL");
        if (!cp || !*cp)
            cp = CONF_SHELL;
        string_list_constructor(&newcmd);
        str = str_from_c(cp);
        string_list_append(&newcmd, str);
        str_free(str);
        if (option_test(OPTION_ERROK))
            str = str_from_c("-c");
        else
            str = str_from_c("-ce");
        string_list_append(&newcmd, str);
        str_free(str);
        str = wl2str(args, 0, args->nstrings - 1, (char *)0);
        string_list_append(&newcmd, str);
        str_free(str);
        retval = execute(&newcmd, fd, errok);
        string_list_destructor(&newcmd);
    }
    else
    {
        retval = execute(args, fd, errok);
    }
    if (input)
        close(fd);
  ret:
    return retval;
}


/*
 * NAME
 *      os_exists - tests for the existence of a file
 *
 * SYNOPSIS
 *      int os_exists(string_ty *path);
 *
 * DESCRIPTION
 *      The os_exists function is used to determine the existence of a file.
 *
 * RETURNS
 *      int; 1 if the file exists, 0 if it does not.  -1 on error
 */

int
os_exists(string_ty *path)
{
    time_t          mtime;
    int             result;

    trace(("os_exists(path = \"%s\")\n{\n", path->str_text));
    mtime = stat_cache_newest(path, 1);
    if (mtime < 0)
        result = -1;
    else
        result = (mtime != 0);
    trace(("return %d\n", result));
    trace(("}\n"));
    return result;
}


int
os_exists_symlink(string_ty *path)
{
    time_t          mtime;
    int             result;

    trace(("os_exists_symlink(path = \"%s\")\n{\n", path->str_text));
    mtime = stat_cache_newest(path, 0);
    if (mtime < 0)
        result = -1;
    else
        result = (mtime != 0);
    trace(("return %d\n", result));
    trace(("}\n"));
    return result;
}


int
os_exists_dir(string_ty *path)
{
    struct stat     st;
    int             result;

    trace(("os_exists_symlink(path = \"%s\")\n{\n", path->str_text));
#ifdef S_ISLNK
    result = lstat(path->str_text, &st);
#else
    result = stat(path->str_text, &st);
#endif
    result = (result == 0) && S_ISDIR(st.st_mode);
    trace(("return %d\n", result));
    trace(("}\n"));
    return result;
}


/*
 * NAME
 *      os_accdir - return the directory path of the users account
 *
 * SYNOPSIS
 *      string_ty *os_accdir(void);
 *
 * DESCRIPTION
 *      The full pathname of the user's account is returned.
 *      The string will have been dynamically allocated.
 *
 * RETURNS
 *      A pointer to a string in dynamic memory is returned.
 *      A null pointer is returned on error.
 *
 * CAVEAT
 *      Use str_free() when you are done with the value returned.
 */

string_ty *
os_accdir(void)
{
    static string_ty  *s;

    if (!s)
    {
        const char      *cp;

        cp = home_directory();
        assert(cp);
        s = str_from_c(cp);
    }
    return str_copy(s);
}


/*
 * NAME
 *      os_entryname - take path apart
 *
 * SYNOPSIS
 *      string_ty *os_entryname(string_ty *path);
 *
 * DESCRIPTION
 *      Os_entryname is used to extract the entry part
 *      from a pathname.
 *
 * RETURNS
 *      pointer to dynamically allocated string.
 *
 * CAVEAT
 *      Use str_free() when you are done with the return value.
 */

string_ty *
os_entryname(string_ty *path)
{
    char            *cp;

    trace(("os_entryname(path = %p)\n{\n", path));
    trace_string(path->str_text);
    cp = strrchr(path->str_text, '/');
    if (cp)
        path = str_from_c(cp + 1);
    else
        path = str_copy(path);
    trace_string(path->str_text);
    trace(("return %p;\n", path));
    trace(("}\n"));
    return path;
}


/*
 * NAME
 *      os_dirname - take path apart
 *
 * SYNOPSIS
 *      string_ty *os_dirname(string_ty *path);
 *
 * DESCRIPTION
 *      Os_dirname is used to extract the directory part
 *      of a pathname.
 *
 * RETURNS
 *      pointer to dynamically allocated string.
 *      A null pointer is returned on error.
 *
 * CAVEAT
 *      Use str_free() when you are done with the value returned.
 */

string_ty *
os_dirname(string_ty *path)
{
    char            *cp;

    trace(("os_dirname(path = %p)\n{\n", path));
    trace_string(path->str_text);
    cp = strrchr(path->str_text, '/');
    if (cp)
    {
        if (cp > path->str_text)
        {
            path = str_n_from_c(path->str_text, cp - path->str_text);
        }
        else
            path = str_from_c("/");
    }
    else
        path = os_curdir();
    trace_string(path->str_text);
    trace(("return %p;\n", path));
    trace(("}\n"));
    return path;
}


#ifdef HAVE_PATHCONF

static long
pathconf_inner(const char *path, int arg)
{
    long            result;

    errno = EINVAL; /* IRIX 5.2 fails to set on errors */
    result = pathconf(path, arg);
    if (result < 0)
    {
        switch (errno)
        {
        case ENOSYS: /* lotsa systems say this for EINVAL */
#ifdef EOPNOTSUPP
        case EOPNOTSUPP: /* HPUX says this for EINVAL */
#endif
            errno = EINVAL;
            break;
        }
    }
    return result;
}


static long
pathconf_wrapper(const char *path, int arg, long default_value)
{
    long            result;

    result = pathconf_inner(path, arg);
    if (result < 0 && errno == EINVAL)
    {
        /*
         * probably NFSv2 mounted
         * assume is same as root
         */
        result = pathconf_inner("/", arg);
        if (result < 0 && errno == EINVAL)
            result = default_value;
    }
    return result;
}

#endif


static int
os_pathconf_path_max(char *path)
{
    long            result;

    result = 1024;
#ifdef HAVE_PATHCONF
    result = pathconf_wrapper(path, _PC_PATH_MAX, result);
    if (result < 0)
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        sub_errno_set(scp);
        sub_var_set_charstar(scp, "File_Name", path);
        fatal_intl(scp, i18n("pathconf(\"$filename\", {PATH_MAX}): $errno"));
        /* NOTREACHED */
    }
#endif
    return result;
}


static int
os_pathconf_name_max(const char *path)
{
    long            result;

#ifdef HAVE_LONG_FILE_NAMES
    result = 255;
#else
    result = 14;
#endif
#ifdef HAVE_PATHCONF
    result = pathconf_wrapper(path, _PC_NAME_MAX, result);
    if (result < 0)
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        sub_errno_set(scp);
        sub_var_set_charstar(scp, "File_Name", path);
        fatal_intl(scp, i18n("pathconf(\"$filename\", {NAME_MAX}): $errno"));
        /* NOTREACHED */
    }
#endif
    return result;
}


/*
 * NAME
 *      os_legal_path - test if path is legal
 *
 * SYNOPSIS
 *      int os_legal_path(string_ty *path);
 *
 * DESCRIPTION
 *      The os_legal_path function is used to test if each of the components of
 *      the given path are legal to the underlying operating system.
 *
 * RETURNS
 *      int: zero if it is an illegal path, nonzero it is a legal path.
 */

int
os_legal_path(string_ty *str)
{
    char            *s;
    char            *ep;
    size_t          max;

    max = os_pathconf_path_max(".");
    if (str->str_length < 1 || str->str_length > max)
        return 0;
    max = os_pathconf_name_max(".");
    s = str->str_text;
    for (;;)
    {
        ep = strchr(s, '/');
        if (!ep)
            return (strlen(s) <= max);
        if ((size_t) (ep - s) > max)
            return 0;
        s = ep + 1;
    }
}


int
os_mkdir(string_ty *p, int echo)
{
    char            *cp;

    for (cp = p->str_text;; ++cp)
    {
        if (cp > p->str_text && (*cp == '/' || *cp == 0))
        {
            string_ty      *tmp;

            tmp = str_n_from_c(p->str_text, cp - p->str_text);
            if (os_exists_dir(tmp))
            {
                /*
                 * Do nothing.
                 *
                 * SGI's IRIX 5.2 has a bug: it tests
                 * for permissions before it checks
                 * for existence.  Thus, you can get a
                 * permission denied error for a directory
                 * you couldn't possibly have tried to
                 * create, because it already existed.
                 *
                 * The Solaris automounter pulls a similar
                 * stunt: it says ENOSYS for indirect
                 * top-level directories without looking
                 * to see if the directory exists first.
                 * Amd does not seem to suffer from this.
                 */
            }
            else if (mkdir(tmp->str_text, 0777) < 0)
            {
                if (errno != EEXIST)
                {
                    sub_context_ty  *scp;

                    scp = sub_context_new();
                    sub_errno_set(scp);
                    sub_var_set_string(scp, "File_Name", tmp);
                    error_intl(scp, i18n("mkdir $filename: $errno"));
                    sub_context_delete(scp);
                    str_free(tmp);
                    option_set_errors();
                    return -1;
                }
            }
            else if (echo)
            {
                sub_context_ty  *scp;

                scp = sub_context_new();
                sub_var_set_string(scp, "File_Name", tmp);
                error_intl(scp, i18n("mkdir $filename"));
                sub_context_delete(scp);
            }

            /*
             * if the knew about the existence of the file
             * before we deleted it, then we will have to
             * adjust the stat cache.
             */
            stat_cache_clear(tmp);
            str_free(tmp);
        }
        if (*cp == 0)
            return 0;
    }
}


int
os_executable(string_ty *path)
{
    int             err;
    sub_context_ty  *scp;

    err = access(path->str_text, X_OK);
    if (err != 0)
    {
        switch (errno)
        {
        case EACCES:
            /*
             * The requested access would be denied to the file or
             * search permission is denied to one of the directories
             * in pathname.
             */
            return 0;

        case ENAMETOOLONG:
            /*
             * pathname is too long, therefore the file can't exist
             */
            return 0;

        case ENOENT:
            /*
             * A directory component in pathname would have been
             * accessible but does not exist or was a dangling symbolic
             * link.
             */
            return 0;

        case ENOTDIR:
            /*
             * A component used as a directory in pathname is not, in
             * fact, a directory.
             */
            return 0;

        default:
            scp = sub_context_new();
            sub_errno_set(scp);
            sub_var_set_string(scp, "File_Name", path);
            error_intl(scp, i18n("access(\"$filename\", X_OK)"));
            sub_context_delete(scp);
            option_set_errors();
            return -1;
        }
    }
    return 1;
}
