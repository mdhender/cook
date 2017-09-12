/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 1998, 2006-2009 Peter Miller
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

#include <common/ac/time.h>

#include <common/language.h>
#include <common/str.h>
#include <common/sub/date.h>
#include <common/sub/private.h>
#include <common/trace.h>
#include <common/wstr_list.h>


/*
 * NAME
 *      sub_date - the date substitution
 *
 * SYNOPSIS
 *      wstring_ty *sub_date(wstring_list_ty *arg);
 *
 * DESCRIPTION
 *      The sub_date function implements the date substitution.
 *      The date substitution is replaced by the current date.
 *      The optional arguments may construct format string,
 *      similar to the date(1) command.
 *
 * ARGUMENTS
 *      arg     - list of arguments, including the function name as [0]
 *
 * RETURNS
 *      a pointer to a string in dynamic memory;
 *      or NULL on error, setting suberr appropriately.
 */

wstring_ty *
sub_date(sub_context_ty *scp, wstring_list_ty *arg)
{
    wstring_ty      *result;
    time_t          now;

    trace(("sub_date()\n{\n"));
    time(&now);
    if (arg->nitems < 2)
    {
        char            *time_string;

        time_string = ctime(&now);
        result = wstr_n_from_c(time_string, 24);
    }
    else
    {
        struct tm       *tm;
        char            buf[1000];
        size_t          nbytes;
        wstring_ty      *wfmt;
        string_ty       *fmt;

        wfmt = wstring_list_to_wstring(arg, 1, 32767, (char *)0);
        fmt = wstr_to_str(wfmt);
        wstr_free(wfmt);
        tm = localtime(&now);

        /*
         * The strftime is locale dependent.
         */
        language_human();
        nbytes = strftime(buf, sizeof(buf) - 1, fmt->str_text, tm);
        language_C();

        if (!nbytes && fmt->str_length)
        {
            sub_context_error_set(scp, i18n("strftime output too large"));
            result = 0;
        }
        else
            result = wstr_n_from_c(buf, nbytes);
        str_free(fmt);
    }
    trace(("return %p;\n", result));
    trace(("}\n"));
    return result;
}
