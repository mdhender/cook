/*
 *      cook - file construction tool
 *      Copyright (C) 1997-1999, 2001, 2003, 2004, 2006, 2007 Peter Miller;
 *      All rights reserved.
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
 *
 * See also: cook/os.c::execute()
 */

#include <common/ac/errno.h>
#include <common/ac/fcntl.h>
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>
#include <common/ac/time.h>
#include <common/ac/unistd.h>
#include <sys/types.h>
#include <sys/stat.h>   /* for chmod */

#include <common/error.h>
#include <common/error_intl.h>
#include <cook/expr/position.h>
#include <cook/flag.h>
#include <cook/id.h>
#include <cook/id/variable.h>
#include <common/mem.h>
#include <cook/meter.h>
#include <cook/opcode/context.h>
#include <cook/opcode/command.h>
#include <cook/opcode/private.h>
#include <cook/option.h>
#include <cook/os_interface.h>
#include <common/star.h>
#include <common/str_list.h>
#include <cook/tempfilename.h>
#include <common/trace.h>


typedef struct opcode_command_ty opcode_command_ty;
struct opcode_command_ty
{
    opcode_ty       inherited;
    int             input;
    expr_position_ty pos;
};


static void
destructor(opcode_ty *that)
{
    opcode_command_ty *this;

    this = (opcode_command_ty *)that;
    expr_position_destructor(&this->pos);
}


/*
 * NAME
 *    echo_command
 *
 * SYNOPSIS
 *    void echo_command(void);
 *
 * DESCRIPTION
 *    The echo_command function is used to determine whether the
 *    command will probably produce output.  Used to see of a
 *    star_eoln needs to be issued.
 *
 * RETURNS
 *    int; 1 if prints, 0 if not
 */

static int
echo_command(string_list_ty *wlp)
{
    static string_ty *echo;
    string_ty       *tmp;
    char            *cp;

    if (!echo)
        echo = str_from_c("echo");
    if (wlp->nstrings < 1 || !str_equal(wlp->string[0], echo))
        return 0;
    tmp = wl2str(wlp, 0, wlp->nstrings - 1, (char *)0);
    for (cp = tmp->str_text; *cp; ++cp)
    {
        if (strchr("^|>", *cp))
        {
            str_free(tmp);
            return 0;
        }
    }
    str_free(tmp);
    return 1;
}


static string_list_ty *
id_var_search(opcode_context_ty *ocp, string_ty *key)
{
    id_ty           *idp;
    string_list_ty  *result;

    idp = opcode_context_id_search(ocp, key);
    if (!idp)
        return 0;
    result = id_variable_query2(idp);
    if (!result || !result->nstrings)
        return 0;
    return result;
}


static void
chmod_and_check(const char *fn, int mode)
{
    if (chmod(fn, mode))
    {
        sub_context_ty    *scp;

        scp = sub_context_new();
        sub_errno_set(scp);
        sub_var_set_charstar(scp, "File_Name", fn);
        sub_var_set(scp, "Number", "0%o", mode);
        fatal_intl(scp, i18n("chmod $filename $mode: $errno"));
        /* NOTREACHED */
    }
}


/*
 * NAME
 *    spawn - execute a command
 *
 * SYNOPSIS
 *    opcode_status_ty spawn(string_list_ty *args, string_ty *input);
 *
 * DESCRIPTION
 *    The spawn function is used to launch the given command.
 *    The command is not waited for.
 *
 * RETURNS
 *    opcode_status_ty...
 *        opcode_status_error    if something went wrong
 *        opcode_status_wait    if something all went well
 */

static opcode_status_ty
spawn(string_list_ty *args, string_ty *input, int *pid_p,
    string_ty *host_binding, opcode_context_ty *ocp)
{
    size_t          j;
    int             fd;
    string_ty       *iname;
    int             pid;
    opcode_status_ty status;
    static char     **argv;
    static size_t   argvlen;
    string_list_ty  cmd;
    sub_context_ty  *scp;
    char            *shell;

    trace(("spawn()\n{\n"));
    assert(args);

    /*
     * build the input file, if required
     */
    status = opcode_status_error;
    fd = -1;
    iname = 0;
    if (input)
    {
        /*
         * He has given a string to be used as input to the command,
         * so write it out to a file, and then redirect the input.
         */
        iname = temporary_filename();
        fd = open(iname->str_text, O_RDWR | O_CREAT | O_TRUNC, 0666);
        if (fd < 0)
            fatal_intl_open(iname->str_text);
        if (unlink(iname->str_text))
        {
            error_intl_unlink(iname->str_text);
            goto done;
        }
        if
        (
            write(fd, input->str_text, input->str_length) < 0
        ||
            lseek(fd, 0L, SEEK_SET)
        )
        {
            error_intl_write(iname->str_text);
            goto done;
        }
    }

    /*
     * See if there is a host_binding in effect.
     */
    if (host_binding)
    {
        static string_ty *key;
        string_list_ty  *slp;
        string_ty       *s;
        string_ty       *rcmd;
        string_ty       *rcmdq;
        string_ty       *result_fn;
        FILE            *result_fp;
        string_ty       *cwd;
        string_ty       *script_fn;
        FILE            *script_fp;

        /*
         * Work out the name of the remote shell command.
         */
        if (!key)
            key = str_from_c("parallel_rsh");
        slp = id_var_search(ocp, key);
        if (slp)
            string_list_copy_constructor(&cmd, slp);
        else
        {
            static string_ty *rsh;

            if (!rsh)
                rsh = str_from_c(CONF_REMOTE_SHELL);
            string_list_constructor(&cmd);
            string_list_append(&cmd, rsh);
        }

        /*
         * Add the name of the host.
         */
        string_list_append(&cmd, host_binding);

        /*
         * The remote end will need to change directory to where
         * we are now, since it is *highly* unlikely that we are
         * in our home directory.
         */
        cwd = os_curdir(); /* do not str_free this */

        /*
         * We are going to create a small shell script contining
         * the command to be executed.  This deals with various
         * weird and obscure $SHELL settings, and it also deals
         * with command lines so long that rsh barfs.
         *
         * Yes, there is a potential race condition here.
         * Between writing the script and executing it, we
         * could be tricked into executing something else.
         * Especially if there is a symlink lying in wait.
         */
        script_fn = dot_temporary_filename();
        script_fp = fopen_and_check(script_fn->str_text, "w");

        /*
         * what shell are we using
         */
        shell = getenv("SHELL");
        if (!shell || !*shell)
            shell = CONF_SHELL;
        fprintf(script_fp, "#!%s\n", shell);

        /*
         * Now write the actual command to the script file.
         */
        rcmd = wl2str(args, 0, args->nstrings, (char *)0);
        fprintf(script_fp, "%s\n", rcmd->str_text);
        str_free(rcmd);

        fflush_and_check(script_fp, script_fn->str_text);
        fclose_and_check(script_fp, script_fn->str_text);
        chmod_and_check(script_fn->str_text, 0755);

        /*
         * This is the result file.  It is where the remote
         * command will stash its exit status.    We seed it with
         * failure in case the rsh fails altogether.
         *
         * And, yes, there is a race conditon and we could be
         * lied to if they are quick.
         */
        result_fn = dot_temporary_filename();
        result_fp = fopen_and_check(result_fn->str_text, "w");
        fprintf(result_fp, "42\n");
        fflush_and_check(result_fp, result_fn->str_text);
        fclose_and_check(result_fp, result_fn->str_text);

        /*
         * Because rsh(1) always returns an exit status of zero,
         * we have to make other arrangements to obtain the exit
         * status.
         *
         * We actually need to be a more devious, so that we can
         * get the exit status back.  Write it to a temporary
         * file server side, then read and remove it client side.
         *
         * This is the server side command.  It gets quoted
         * again to protect it from the client side shell.
         *
         * We use sh explicitly, because the shell at the other
         * end could be csh, tcsh, or something even stranger,
         * and we need to guarantee the command will execute
         * exactly the way we need.
         */
        rcmd =
            str_format
            (
                "sh -c 'cd %s && sh %s %s/%s; echo $? > %s/%s'",
                cwd->str_text,
                (option_test(OPTION_ERROK) ? "-c" : "-ce"),
                cwd->str_text,
                script_fn->str_text,
                cwd->str_text,
                result_fn->str_text
            );
        rcmdq = str_quote_shell(rcmd);
        str_free(rcmd);
        string_list_append(&cmd, rcmdq);
        str_free(rcmdq);

        /*
         * This is the rest of the server side command.
         *
         * On some systems this gives ``Text file busy'' errors,
         * as if the script file has yet to be released by
         * the kernel.    Since we *aren't* executing it when the
         * RM command runs, I have no idea how to avoid this
         * (other than ignoring it with -f).  Maybe this is some
         * kind of NFS latency?
         */
        s =
            str_format
            (
                "&& exit `cat %s;rm -f %s %s`",
                result_fn->str_text,
                result_fn->str_text,
                script_fn->str_text
            );
        string_list_append(&cmd, s);
        str_free(s);
        str_free(result_fn);
        str_free(script_fn);

        /*
         * Because the command has a semicolon and back quotes,
         * we need to hand it to a shell (as alluded to above).
         * Assemble into a single string.
         */
        rcmd = wl2str(&cmd, 0, cmd.nstrings, (char *)0);
        string_list_destructor(&cmd);

        /*
         * Build the final command to be executed.
         * It even cleans up after itself.
         */
        s = str_from_c("sh");
        string_list_append(&cmd, s);
        str_free(s);
        s = str_from_c("-c");
        string_list_append(&cmd, s);
        str_free(s);
        string_list_append(&cmd, rcmd);
        str_free(rcmd);
    }
    else
    {
        /*
         * build the command
         */
        if (os_execute_magic_characters_list(args))
        {
            string_ty       *str;

            /*
             * what shell are we using
             */
            shell = getenv("SHELL");
            if (!shell || !*shell)
                shell = CONF_SHELL;

            string_list_constructor(&cmd);
            str = str_from_c(shell);
            string_list_append(&cmd, str);
            str_free(str);
            if (option_test(OPTION_ERROK))
                str = str_from_c("-c");
            else
                str = str_from_c("-ce");
            string_list_append(&cmd, str);
            str_free(str);
            str = wl2str(args, 0, args->nstrings - 1, (char *)0);
            string_list_append(&cmd, str);
            str_free(str);
        }
        else
            string_list_copy_constructor(&cmd, args);
    }

    /*
     * build the argv array
     */
    if (!argv)
    {
        argvlen = cmd.nstrings + 1;
        argv = mem_alloc(argvlen * sizeof(char *));
    }
    else if (argvlen < cmd.nstrings + 1)
    {
        argvlen = cmd.nstrings + 1;
        argv = mem_change_size(argv, argvlen * sizeof(char *));
    }
    if (cmd.nstrings == 0)
    {
        string_list_destructor(&cmd);
        status = opcode_status_success;
        goto done;
    }
    for (j = 0; j < cmd.nstrings; ++j)
        argv[j] = cmd.string[j]->str_text;
    argv[cmd.nstrings] = 0;

    /*
     * spawn the child process
     */
    switch (pid = fork())
    {
    case -1:
        scp = sub_context_new();
        sub_errno_set(scp);
        error_intl(scp, i18n("fork(): $errno"));
        sub_context_delete(scp);
        break;

    case 0:
        /*
         * child
         */
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
                fatal_intl
                (
                    scp,
                    i18n("close $filename: $errno")
                );
                /* NOTREACHED */
                sub_context_delete(scp);
                str_free(fn0);
            }
            if (dup(fd) < 0)
            {
                scp = sub_context_new();
                sub_errno_set(scp);
                fatal_intl(scp, i18n("dup(): $errno"));
                /* NOTREACHED */
                sub_context_delete(scp);
            }
            close(fd);
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
        sub_context_delete(scp);

    default:
        /*
         * parent
         */
        if (fd >= 0)
        {
            close(fd);
            fd = -1;
        }
        string_list_destructor(&cmd);
        status = opcode_status_wait;
        trace(("pid = %d;\n", pid));
        *pid_p = pid;
        break;
    }
    done:
    if (fd >= 0)
        close(fd);
    if (iname)
        str_free(iname);
    trace(("return %s;\n", opcode_status_name(status)));
    trace(("}\n"));
    return status;
}


/*
 * NAME
 *    execute
 *
 * SYNOPSIS
 *    opcode_status_ty execute(opcode_ty *, opcode_context_ty *);
 *
 * DESCRIPTION
 *    The execute function is used to execute the given opcode within
 *    the given interpretation context.
 *
 * RETURNS
 *    opcode_status_ty to indicate the result of the execution
 */

static opcode_status_ty
execute(const opcode_ty *op, opcode_context_ty *ocp)
{
    const opcode_command_ty *this;
    opcode_status_ty status;
    string_list_ty  *wlp;
    string_list_ty  *flags_words;
    string_ty       *isp;
    char            *cmd;
    flag_ty         *flags;

    /*
     * pop the arguments off the value stack
     */
    trace(("opcode_command::execute()\n{\n"));
    this = (const opcode_command_ty *)op;
    status = opcode_status_success;
    isp = 0;
    flags_words = 0;
    flags = 0;
    wlp = 0;
    if (ocp->pid)
    {
        ocp->pid = 0;
        wlp = ocp->wlp;
        ocp->wlp = 0;
        goto resume;
    }
    if (this->input)
    {
        string_list_ty    *islp;

        islp = opcode_context_string_list_pop(ocp);
        isp = wl2str(islp, 0, islp->nstrings - 1, (char *)0);
        string_list_delete(islp);
    }
    flags_words = opcode_context_string_list_pop(ocp);
    wlp = opcode_context_string_list_pop(ocp);

    /*
     * take care of the options
     */
    flags = flag_recognize(flags_words, &this->pos);
    if (!flags)
    {
        /*
         * Error message already printed.
         */
        status = opcode_status_error;
        goto done;
    }
    string_list_delete(flags_words);
    flags_words = 0;
    flag_set_options(flags, OPTION_LEVEL_EXECUTE);
    flag_delete(flags);
    flags = 0;

    /*
     * echo the command if required
     */
    if (!option_test(OPTION_SILENT))
    {
        string_ty *cp;

        /*
         * If the command has not been silenced,
         * form it into a string and echo it.
         */
        cp = wl2str(wlp, 0, wlp->nstrings - 1, (char *)0);
        if (option_test(OPTION_TELL_POSITION))
        {
            error_raw
            (
                "%s: %d: %s",
                this->pos.pos_name->str_text,
                this->pos.pos_line,
                cp->str_text
            );
        }
        else
        {
            error_raw("%s", cp->str_text);
        }
        str_free(cp);
    }

    /*
     * execute the command if required
     */
    if (option_test(OPTION_ACTION))
    {
        /*
         * emit suitable progress stars
         */
        if (option_test(OPTION_SILENT))
        {
            if (echo_command(wlp))
                star_eoln();
            else
                star_bang();
        }
        else
            star_sync();

        /*
         * invalidate the stat cache
         */
        if (option_test(OPTION_INVALIDATE_STAT_CACHE))
        {
            size_t          j;

            for (j = 0; j < wlp->nstrings; ++j)
                if (os_clear_stat(wlp->string[j]))
                    status = opcode_status_error;
        }

        /*
         * prepare for metering
         */
#ifdef HAVE_WAIT3
        if (option_test(OPTION_METER))
        {
            if (!ocp->meter_p)
                ocp->meter_p = meter_alloc();
            meter_begin(ocp->meter_p);
        }
#endif

        /*
         * run the command
         */
        status = spawn(wlp, isp, &ocp->pid, ocp->host_binding, ocp);
        if (status == opcode_status_wait)
        {
            trace(("...wait\n"));
            ocp->wlp = wlp;
            wlp = 0;
            goto done;
        }
        if (status == opcode_status_error)
            goto done;
resume:
        /*
         * Finish metering.
         */
#ifdef HAVE_WAIT3
        if (option_test(OPTION_METER))
        {
            assert(ocp->meter_p);
            meter_print(ocp->meter_p);
        }
#endif
        /*
         * Echo the exit status of the command, if it was not
         * successful.  The error flag will also be examined (it
         * changes the text of the message, too) to see if we
         * should terminate successfully or with and error.
         */
        cmd = (wlp->nstrings ? wlp->string[0]->str_text : "");
        if
        (
            exit_status
            (
                cmd,
                ocp->exit_status,
                option_test(OPTION_ERROK)
            )
        )
        {
            status = opcode_status_error;
        }
    }

    /*
     * rescind the flag settings
     */
    option_undo_level(OPTION_LEVEL_EXECUTE);

    /*
     * release the arguments and exit
     */
    done:
    if (flags_words)
        string_list_delete(flags_words);
    if (flags)
        flag_delete(flags);
    if (wlp)
        string_list_delete(wlp);
    if (isp)
        str_free(isp);
    trace(("return %s;\n", opcode_status_name(status)));
    trace(("}\n"));
    return status;
}


/*
 * NAME
 *    script
 *
 * SYNOPSIS
 *    opcode_status_ty script(opcode_ty *, opcode_context_ty *);
 *
 * DESCRIPTION
 *    The script function is used to script the given opcode within
 *    the given interpretation context.
 *
 * RETURNS
 *    opcode_status_ty to indicate the result
 */

static opcode_status_ty
script(const opcode_ty *op, opcode_context_ty *ocp)
{
    const opcode_command_ty *this;
    opcode_status_ty status;
    string_list_ty  *wlp;
    string_ty       *isp;
    string_list_ty  *flags_words;
    flag_ty         *flags;

    /*
     * pop the arguments off the value stack
     */
    trace(("opcode_command::script()\n{\n"));
    this = (const opcode_command_ty *)op;
    status = opcode_status_success;
    if (this->input)
    {
        string_list_ty  *islp;

        islp = opcode_context_string_list_pop(ocp);
        isp = wl2str(islp, 0, islp->nstrings - 1, (char *)0);
        string_list_delete(islp);
    }
    else
        isp = 0;
    flags_words = opcode_context_string_list_pop(ocp);
    wlp = opcode_context_string_list_pop(ocp);

    /*
     * set the flags
     */
    flags = flag_recognize(flags_words, &this->pos);
    string_list_delete(flags_words);
    if (!flags)
    {
        status = opcode_status_error;
        goto done;
    }
    flag_set_options(flags, OPTION_LEVEL_EXECUTE);
    flag_delete(flags);

    /*
     * If the command is empty (e.g. [print ...]; or [write ...];)
     * do nothing at all.
     */
    if (wlp->nstrings == 0)
        goto done;

    /*
     * echo the command if required
     */
    if (!option_test(OPTION_SILENT))
    {
        string_ty       *s1;
        string_ty       *s2;

        s1 = wl2str(wlp, 0, wlp->nstrings, " ");
        s2 = str_quote_shell(s1);
        str_free(s1);
        if (option_test(OPTION_TELL_POSITION))
        {
            string_ty       *s3;

            s3 = str_quote_shell(this->pos.pos_name);
            printf
            (
                "echo %s: %d: %s\n",
                s3->str_text,
                this->pos.pos_line,
                s2->str_text
            );
            str_free(s3);
        }
        else
        {
            printf("echo %s\n", s2->str_text);
        }
        str_free(s2);
    }

    /*
     * script the command if required
     */
    if (option_test(OPTION_ACTION))
    {
        size_t          j;

        printf("(");
        for (j = 0; j < wlp->nstrings; ++j)
            printf(" %s", wlp->string[j]->str_text);
        printf(" )");

        if (isp)
        {
            char            fubar[50];
            time_t          t;
            int             nl;

            time(&t);
            if (t < 0)
                t = 0;
            snprintf
            (
                fubar,
                sizeof(fubar),
                "fubar%ldfubar%ldfubar",
                (long)t,
                (long)t
            );

            nl =
                (
                    isp->str_length
                &&
                    isp->str_text[isp->str_length - 1] != '\n'
                );
            printf
            (
                " << '%s'\n%s%s%s",
                fubar,
                isp->str_text,
                (nl ? "\n" : ""),
                fubar
            );
        }
        printf("\n");

        if (!option_test(OPTION_ERROK))
            printf("test $? -eq 0 || exit 1\n");
    }

    /*
     * rescind the flag settings
     */
    option_undo_level(OPTION_LEVEL_EXECUTE);

    /*
     * release the arguments and exit
     */
    done:
    string_list_delete(wlp);
    if (isp)
        str_free(isp);
    trace(("return %s;\n", opcode_status_name(status)));
    trace(("}\n"));
    return status;
}


/*
 * NAME
 *    disassemble
 *
 * SYNOPSIS
 *    void disassemble(opcode_ty *);
 *
 * DESCRIPTION
 *    The disassemble function is used to disassemble the copdode and
 *    its arguments onto the standard output.  Don't worry about the
 *    location or a trailing newline.
 */

static void
disassemble(const opcode_ty *op)
{
    const opcode_command_ty *this;

    trace(("opcode_command::disassemble()\n{\n"));
    this = (const opcode_command_ty *)op;
    printf
    (
        "%d # %s:%d",
        this->input,
        this->pos.pos_name->str_text,
        this->pos.pos_line
    );
    trace(("}\n"));
}


/*
 * NAME
 *    method
 *
 * DESCRIPTION
 *    The method variable describes this class.
 *
 * CAVEAT
 *    This symbol is not exported from this file.
 */

static opcode_method_ty method =
{
    "command",
    sizeof(opcode_command_ty),
    destructor,
    execute,
    script,
    disassemble,
};


/*
 * NAME
 *    opcode_command_new
 *
 * SYNOPSIS
 *    opcode_ty *opcode_command_new(void);
 *
 * DESCRIPTION
 *    The opcode_command_new function is used to allocate a new instance
 *    of a command opcode.
 *
 * RETURNS
 *    opcode_ty *; use opcode_delete when you are finished with it.
 */

opcode_ty *
opcode_command_new(int input, const expr_position_ty *pp)
{
    opcode_ty       *op;
    opcode_command_ty *this;

    trace(("opcode_command_new()\n{\n"));
    op = opcode_new(&method);
    this = (opcode_command_ty *)op;
    this->input = input;
    expr_position_copy_constructor(&this->pos, pp);
    trace(("return %08lX;\n", (long)op));
    trace(("}\n"));
    return op;
}
