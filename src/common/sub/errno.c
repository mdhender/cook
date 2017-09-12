/*
 *      cook - file construction tool
 *      Copyright (C) 1997-1999, 2001, 2006, 2007 Peter Miller;
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
 */

#include <common/ac/errno.h>
#include <common/ac/stdio.h>    /* for sprintf */
#include <common/ac/string.h>

#include <pwd.h>
#include <grp.h>
#include <common/ac/unistd.h>

#include <common/str.h>
#include <common/sub/errno.h>
#include <common/sub/private.h>
#include <common/trace.h>
#include <common/wstr_list.h>


/*
 * NAME
 *      sub_errno - the errno substitution
 *
 * SYNOPSIS
 *      wstring_ty *sub_errno(wstring_list_ty *arg);
 *
 * DESCRIPTION
 *      The sub_errno function implements the errno substitution.  The
 *      errno substitution is replaced by the value if th errno variable
 *      provided by the system, as mapped through the strerror function.
 *
 *      Requires exactly zero arguments.
 *
 *      The sub_errno_set() function may be used to remember errno,
 *      and thus isolate the error from subsequest system calls.
 *
 * ARGUMENTS
 *      arg     - list of arguments, including the function name as [0]
 *
 * RETURNS
 *      a pointer to a string in dynamic memory;
 *      or NULL on error, setting suberr appropriately.
 */

wstring_ty *
sub_errno(sub_context_ty *scp, wstring_list_ty *arg)
{
    wstring_ty      *result;

    trace(("sub_errno()\n{\n"));
    if (arg->nitems != 1)
    {
        sub_context_error_set(scp, i18n("requires zero arguments"));
        result = 0;
    }
    else
    {
        int             n;

        n = sub_context_errno_get(scp);
        if (n == EPERM || n == EACCES)
        {
            int             uid;
            struct passwd   *pw;
            char            uidn[20];
            int             gid;
            struct group    *gr;
            char            gidn[20];
            string_ty       *s;

            uid = geteuid();
            pw = getpwuid(uid);
            if (pw)
                snprintf(uidn, sizeof(uidn), "user \"%.8s\"", pw->pw_name);
            else
                snprintf(uidn, sizeof(uidn), "uid %d", uid);

            gid = getegid();
            gr = getgrgid(gid);
            if (gr)
                snprintf(gidn, sizeof(gidn), "group \"%.8s\"", gr->gr_name);
            else
                snprintf(gidn, sizeof(gidn), "gid %d", gid);

            s = str_format("%s [%s, %s]", strerror(n), uidn, gidn);
            result = str_to_wstr(s);
            str_free(s);
        }
        else
            result = wstr_from_c(strerror(n));
    }
    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
