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

#include <common/sub/dirname.h>
#include <common/sub/private.h>
#include <common/trace.h>
#include <common/wstr_list.h>


static wstring_ty *
wdirname(wstring_ty *s)
{
    wchar_t         *the_end;

    /*
     * strip trailing slashes
     */
    the_end = s->wstr_text + s->wstr_length;
    while (the_end > s->wstr_text && the_end[-1] == '/')
        --the_end;

    /*
     * strip trailing filename
     */
    while (the_end > s->wstr_text && the_end[-1] != '/')
        --the_end;

    /*
     * strip trailing slashes
     */
    while (the_end > s->wstr_text && the_end[-1] == '/')
        --the_end;

    /*
     * If there is anything left, that's the answer.
     */
    if (the_end > s->wstr_text)
        return wstr_n_from_wc(s->wstr_text, the_end - s->wstr_text);

    /*
     * If there is nothing left, but we were given an absolute path,
     * root is the answer.
     */
    if (s->wstr_text[0] == '/')
        return wstr_from_c("/");

    /*
     * Otherwise, the current dirtectory is the answer.
     */
    return wstr_from_c(".");
}


/*
 * NAME
 *      sub_dirname - the dirname substitution
 *
 * SYNOPSIS
 *      wstring_ty *sub_dirname(wstring_list_ty *arg);
 *
 * DESCRIPTION
 *      The sub_dirname function implements the dirname substitution.
 *      The dirname substitution is replaced by the dirname of
 *      the argument path, similar to the dirname(1) command.
 *
 * ARGUMENTS
 *      arg     - list of arguments, including the function name as [0]
 *
 * RETURNS
 *      a pointer to a string in dynamic memory;
 *      or NULL on error, setting suberr appropriately.
 */

wstring_ty *
sub_dirname(sub_context_ty *scp, wstring_list_ty *arg)
{
    wstring_ty      *result;

    trace(("sub_dirname()\n{\n"));
    if (arg->nitems != 2)
    {
        sub_context_error_set(scp, i18n("requires one argument"));
        result = 0;
    }
    else
        result = wdirname(arg->item[1]);
    trace(("return %p;\n", result));
    trace(("}\n"));
    return result;
}
