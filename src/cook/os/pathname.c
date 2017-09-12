/*
 *      cook - file construction tool
 *      Copyright (C) 1999, 2006-2009 Peter Miller
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

#include <common/ac/stdlib.h>
#include <common/ac/errno.h>
#include <common/ac/string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <common/ac/unistd.h>
#include <common/ac/mntent.h>

#include <common/error.h>
#include <common/error_intl.h>
#include <common/mem.h>
#include <common/os_path_cat.h>
#include <common/str.h>
#include <common/str_list.h>
#include <common/sub.h>
#include <common/trace.h>
#include <cook/os_interface.h>


/*
 * NAME
 *      os_curdir_sub - get current directory
 *
 * SYNOPSIS
 *      string_ty *os_curdir_sub(void);
 *
 * DESCRIPTION
 *      The os_curdir_sub function is used to determine the system's idea
 *      of the current directory.
 *
 * RETURNS
 *      A pointer to a string in dynamic memory is returned.
 *      A null pointer is returned on error.
 *
 * CAVEAT
 *      DO NOT use str_free() on the value returned.
 */

static string_ty *
os_curdir_sub(void)
{
    static string_ty *s;

    if (!s)
    {
        char            buffer[2000];

        if (!getcwd(buffer, sizeof(buffer)))
        {
            sub_context_ty  *scp;

            scp = sub_context_new();
            sub_errno_set(scp);
            fatal_intl(scp, i18n("getcwd: $errno"));
            /* NOTREACHED */
        }
        assert(buffer[0] == '/');
        s = str_from_c(buffer);
    }
    return s;
}


/*
 * NAME
 *      os_curdir - full current directory path
 *
 * SYNOPSIS
 *      string_ty *os_curdir(void);
 *
 * DESCRIPTION
 *      Os_curdir is used to determine the pathname
 *      of the current directory.  Automounter vaguaries will be elided.
 *
 * RETURNS
 *      A pointer to a string in dynamic memory is returned.
 *      A null pointer is returned on error.
 *
 * CAVEAT
 *      Use str_free() when you are done with the value returned.
 */

string_ty *
os_curdir(void)
{
    static string_ty *result;

    if (!result)
    {
        string_ty       *dot;

        dot = str_from_c(".");
        result = os_pathname(dot);
        str_free(dot);
    }
    return str_copy(result);
}


/*
 * NAME
 *      has_prefix
 *
 * SYNOPSIS
 *      string_ty *has_prefix(string_ty *prefix, string_ty *candidate);
 *
 * DESCRIPTION
 *      The has_prefix function is used to test if the ``prefix'' string
 *      is a prefix of the ``candidate'' string.
 *
 * RETURNS
 *      On success: the remainder of the string with the prefix removed.
 *      On failure: NULL.
 */

static string_ty *
has_prefix(string_ty *pfx, string_ty *path)
{
    if (str_equal(pfx, path))
        return str_copy(path);
    if
    (
        pfx->str_length < path->str_length
    &&
        path->str_text[pfx->str_length] == '/'
    &&
        0 == memcmp(pfx->str_text, path->str_text, pfx->str_length)
    )
    {
        return
            str_n_from_c
            (
                path->str_text + pfx->str_length,
                path->str_length - pfx->str_length
            );
    }
    return 0;
}


/*
 * NAME
 *      has_a_prefix
 *
 * SYNOPSIS
 *      string_ty *has-a_prefix(string_list_ty *prefix, string_ty *candidate);
 *
 * DESCRIPTION
 *      The has_a_prefix function is used to test if any one of the
 *      ``prefix'' strings is a prefix of the ``candidate'' string.
 *
 * RETURNS
 *      On success: the remainder of the string with the prefix removed.
 *      On failure: NULL.
 */

static string_ty *
has_a_prefix(string_list_ty *pfx, string_ty *path)
{
    size_t          j;
    string_ty       *result;

    for (j = 0; j < pfx->nstrings; ++j)
    {
        result = has_prefix(pfx->string[j], path);
        if (result)
            return result;
    }
    return 0;
}


/*
 * NAME
 *      get_prefix_list
 *
 * SYNOPSIS
 *      string_list_ty *get_prefix_list(void);
 *
 * DESCRIPTION
 *      The get_prefix_list function is used to get the list of possible
 *      auto mount points from the COOK_AUTOMOUNT_POINTS environment
 *      variable.
 *
 *      The value is colon separated.  values not starting with slash,
 *      and the root directory, are silently ignored.
 *
 * RETURNS
 *      string list of prefixes
 *      DO NOT string_list_delete is *ever* because it is cached.
 */

static string_list_ty *
get_prefix_list(void)
{
    static string_list_ty *prefix;
    char            *cp;
    string_list_ty  tmp;
    string_ty       *s;
    size_t          j;

    if (prefix)
        return prefix;

    /*
     * Pull the value out of the relevant environment
     * variable, and break it into pieces (it's colon
     * separated).
     */
    prefix = string_list_new();
    cp = getenv("COOK_AUTOMOUNT_POINTS");
    if (!cp)
        cp = "/tmp_mnt:/a:/.automount";
    s = str_from_c(cp);
    string_list_constructor(&tmp);
    str2wl(&tmp, s, ":", 0);
    str_free(s);

    /*
     * Rip off any trailing slashes.
     */
    for (j = 0; j < tmp.nstrings; ++j)
    {
        size_t          max;

        s = tmp.string[j];
        if (s->str_text[0] != '/')
            continue;
        max = s->str_length;
        while (max > 0 && s->str_text[max - 1] == '/')
            --max;
        if (max != s->str_length)
        {
            if (max > 0)
            {
                s = str_n_from_c(s->str_text, max);
                string_list_append_unique(prefix, s);
                str_free(s);
            }
        }
        else
            string_list_append_unique(prefix, s);
    }
    string_list_destructor(&tmp);

    return prefix;
}


/*
 * NAME
 *      get_auto_mount_dirs
 *
 * SYNOPSIS
 *      string_list_ty *get_auto_mount_dirs(void);
 *
 * DESCRIPTION
 *      The get_auto_mount_dirs function is used to grab all the active
 *      mount points with an automount prefix.
 *
 *      Because we are called (indirectly) by os_pathname, all of
 *      the relevant auto mount activities have taken place, and thus
 *      the mount table (obtained through the getmntent api) will be
 *      up-to-date, and contain mount entries with auto mount point prefixes.
 *
 *      We cache, and re-read if this the MOUNTED file changes.
 *
 * RETURNS
 *      String list of actual automounted mount points.
 *      DO NOT string_list_delete is *ever* because it is cached.
 */

static string_list_ty *
get_auto_mount_dirs(string_list_ty *prefix)
{
    FILE            *fp;
    static string_list_ty *dirs;
    static struct stat mntent_st;
    static string_ty *slash;
    int             err;
    struct stat     st;
    string_ty       *p1;
    struct stat     st1;
    string_ty       *p2;
    struct stat     st2;

    fp = setmntent(MOUNTED, "r");
    if (!fp)
    {
        if (dirs && dirs->nstrings)
        {
            string_list_delete(dirs);
            dirs = string_list_new();
        }
        return dirs;
    }

    /*
     * The ``fp'' varuiable is probably not a real FILE*, so don't
     * use it like one!  E.g. Cygwin cast it to FILE*, but there's
     * actually a whole 'nother data structure behind there, and it
     * GPFs if you try to access it as if it were a FILE*.
     */
    err = stat(MOUNTED, &st);
    if (err)
        memset(&st, 0, sizeof(st));
    if (dirs && mntent_st.st_mtime == st.st_mtime)
    {
        endmntent(fp);
        return dirs;
    }
    mntent_st = st;

    if (!slash)
        slash = str_from_c("/");
    if (dirs)
        string_list_delete(dirs);
    dirs = string_list_new();
    for (;;)
    {
        struct mntent   *mep;
        string_ty       *dir;
        string_ty       *tmp;

        mep = getmntent(fp);
        if (!mep)
            break;
        dir = str_from_c(mep->mnt_dir);

        /*
         * Ignore ``/'' because everything is below it.
         */
        if (str_equal(dir, slash))
        {
          the_next_one:
            str_free(dir);
            continue;
        }

        /*
         * If the mount point doesn't have an automount
         * point prefix, skip this mount point.
         *
         * We are called by os_pathname, which has just
         * exersized all of the symbolic links and automount
         * thingies.  So, they will all be in the mount table.
         * (Except for really weird cases of symlinks between
         * NFS file systems, with some servers fast and some
         * really really slow, but don't worry about that.)
         */
        tmp = has_a_prefix(prefix, dir);
        if (!tmp)
            goto the_next_one;

        /*
         * Simply fiddling with the path is a security
         * hole.  We must make sure that the auto-mounted
         * and un-auto-mounted paths give the same answer.
         */
        p1 = str_format("%s/.", dir->str_text);
        err = lstat(p1->str_text, &st1);
        str_free(p1);
        if (err)
            goto the_next_one;
        p2 = str_format("%s/.", tmp->str_text);
        str_free(tmp);
        err = lstat(p2->str_text, &st2);
        str_free(p2);
        if (err)
            goto the_next_one;
        if (st1.st_ino != st2.st_ino || st1.st_dev != st2.st_dev)
            goto the_next_one;

        /*
         * Everything checks out,
         * remember this one.
         */
        string_list_append(dirs, dir);
        str_free(dir);
    }
    endmntent(fp);
    return dirs;
}


/*
 * NAME
 *      remove_automounter_prefix
 *
 * SYNOPSIS
 *      string_ty *remove_automounter_prefix(string_ty *path);
 *
 * DESCRIPTION
 *      The remove_automounter_prefix function is used to remove
 *      any automounter prefix that may be present on an absolute
 *      path name.  The prefixes to check for are obtained from the
 *      COOK_AUTOMOUNT_POINTS environment variable.
 *
 * RETURNS
 *      string_ty * - pointer dynamically allocated string.  Use str_free
 *      when done with it.
 *
 * CAVEAT
 *      This function is dangerous.  Use with extreme care.
 */

static string_ty *
remove_automounter_prefix(string_ty *path)
{
    string_list_ty  *prefix;
    string_list_ty  *amdl;
    string_ty       *result;

    /*
     * Get the list of possible automount prefixes.
     */
    prefix = get_prefix_list();

    /*
     * Get the list of automounted mount points.
     * It's cached, so it usually doesn't take long.
     */
    amdl = get_auto_mount_dirs(prefix);

    /*
     * Look for leading path prefixes.
     */
    result = has_a_prefix(prefix, path);
    if (result)
    {
        string_ty       *tmp;

        /*
         * Now see if the path (known to have an auto mount
         * prefix) is below an automounted mount point (also
         * known to have an auto mount prefix).
         */
        tmp = has_a_prefix(amdl, path);
        if (tmp)
        {
            str_free(tmp);
            return result;
        }

        /*
         * The path is below an auto mount directory, but
         * not in the mount table, so it's bogus in some way.
         * Ignore it, in case it is an attempt to subvert Aegis
         * into a security breach.
         */
        str_free(result);
    }

    /*
     * No match, return the original.
     */
    return str_copy(path);
}


/*
 * NAME
 *      os_pathname - determine full file name
 *
 * SYNOPSIS
 *      string_ty *os_pathname(string_ty *path);
 *
 * DESCRIPTION
 *      Os_pathname is used to determine the full path name
 *      of a partial path given.
 *
 * ARGUMENTS
 *      path    - path to canonicalize
 *
 * RETURNS
 *      pointer to dynamically allocated string.
 *
 * CAVEAT
 *      Use str_free() when you are done with the value returned.
 */

string_ty *
os_pathname(string_ty *path)
{
    static char     *tmp;
    static size_t   tmplen;
    static size_t   ipos;
    static size_t   opos;
    int             c;
    int             found;
#if defined(S_IFLNK) || defined(S_ISLNK)
    string_list_ty  loop;
    char            pointer[2000];
    int             nbytes;
    string_ty       *s;
#endif
    string_ty       *result;

    /*
     * Change relative pathnames to absolute
     */
    trace(("os_pathname(path = %p)\n{\n", path));
    if (!path)
        path = os_curdir();
    trace_string(path->str_text);
    if (path->str_text[0] != '/')
        path = os_path_cat(os_curdir_sub(), path);
    else
        path = str_copy(path);

    /*
     * Take kinks out of the pathname
     */
    ipos = 0;
    opos = 0;
    found = 0;
#if defined(S_IFLNK) || defined(S_ISLNK)
    string_list_constructor(&loop);
#endif
    while (!found)
    {
        /*
         * get the next character
         */
        c = path->str_text[ipos];
        if (c)
            ipos++;
        else
        {
            found = 1;
            c = '/';
        }

        /*
         * remember the normal characters
         * until get to slash
         */
        if (c != '/')
            goto remember;

        /*
         * leave root alone
         */
        if (!opos)
            goto remember;

        /*
         * "/.." -> "/"
         */
        if (opos == 3 && tmp[1] == '.' && tmp[2] == '.')
        {
            opos = 1;
            continue;
        }

        /*
         * "a//" -> "a/"
         */
        if (tmp[opos - 1] == '/')
            continue;

        /*
         * "a/./" -> "a/"
         */
        if (opos >= 2 && tmp[opos - 1] == '.' && tmp[opos - 2] == '/')
        {
            opos--;
            continue;
        }

        /*
         * "a/b/../" -> "a/"
         */
        if
        (
            opos > 3
        &&
            tmp[opos - 1] == '.'
        &&
            tmp[opos - 2] == '.'
        &&
            tmp[opos - 3] == '/'
        )
        {
            opos -= 4;
            assert(opos > 0);
            while (tmp[opos - 1] != '/')
                opos--;
            continue;
        }

        /*
         * see if the path so far is a symbolic link
         */
#if defined(S_IFLNK) || defined(S_ISLNK)
        s = str_n_from_c(tmp, opos);
        nbytes = readlink(s->str_text, pointer, sizeof(pointer) - 1);
        if (nbytes == 0)
        {
            sub_context_ty  *scp;

            scp = sub_context_new();
            sub_errno_set(scp);
            sub_var_set_string(scp, "File_Name", s);
            fatal_intl(scp, i18n("readlink \"$filename\" returned \"\""));
            /* NOTREACHED */
        }
        else if (nbytes < 0)
        {
            /*
             * probably not a symbolic link
             */
            if
            (
                errno != ENXIO
            &&
                errno != EINVAL
            &&
                errno != ENOENT
            &&
                errno != ENOTDIR
            )
            {
                sub_context_ty  *scp;

                scp = sub_context_new();
                sub_errno_set(scp);
                sub_var_set_string(scp, "File_Name", s);
                fatal_intl(scp, i18n("readlink $filename: $errno"));
                /* NOTREACHED */
            }
            str_free(s);
        }
        else
        {
            string_ty       *newpath;

            pointer[nbytes] = 0;

            str_free(s);
            if (pointer[0] == '/')
                tmp[1] = 0;
            else
            {
                while (tmp[opos - 1] != '/')
                    opos--;
                tmp[opos] = 0;
            }
            newpath =
                str_format("%s/%s/%s", tmp, pointer, path->str_text + ipos);
            str_free(path);
            path = newpath;
            if (string_list_member(&loop, path))
            {
                sub_context_ty  *scp;

                scp = sub_context_new();
                sub_var_set_string(scp, "File_Name", s);
                fatal_intl
                (
                    scp,
                    i18n("symbolic link loop \"$filename\" detected")
                );
                /* NOTREACHED */
            }
            string_list_append(&loop, path);
            path = newpath;
            ipos = 0;
            opos = 0;
            found = 0;
            continue;
        }
#endif

        /*
         * keep the slash
         */
        remember:
        if (opos >= tmplen)
        {
            tmplen = tmplen * 2 + 8;
            tmp = mem_change_size(tmp, tmplen);
        }
        tmp[opos++] = c;
    }
#if defined(S_IFLNK) || defined(S_ISLNK)
    string_list_destructor(&loop);
#endif
    str_free(path);
    assert(opos >= 1);
    assert(tmp[0] == '/');
    assert(tmp[opos - 1] == '/');
    if (opos >= 2)
        opos--;
    path = str_n_from_c(tmp, opos);
    trace_string(path->str_text);

    /*
     * Check for automounter prefixes, and remove them if you
     * find them.  The user needs to use this sparingly, because
     * extreme chaos can result.
     */
    result = remove_automounter_prefix(path);
    str_free(path);
    trace_string(result->str_text);
    trace(("}\n"));
    return result;
}
