/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 1998, 2001, 2006-2009 Peter Miller
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

#include <common/ac/string.h>

#include <common/sub/basename.h>
#include <common/sub/private.h>
#include <common/trace.h>
#include <common/wstr_list.h>


static wstring_ty *
entryname(wstring_ty *s)
{
    wchar_t         *start;
    wchar_t         *finish;
    wchar_t         *cp;
    wchar_t         prev;

    start = s->wstr_text;
    finish = s->wstr_text;
    prev = '/';
    cp = s->wstr_text;
    for (;;)
    {
        if (prev == '/')
        {
            if (*cp != '/' && *cp != 0)
                start = cp;
        }
        else
        {
            if (*cp == '/' || *cp == 0)
                finish = cp;
        }
        if (!*cp)
            break;
        prev = *cp++;
    }
    return wstr_n_from_wc(start, finish - start);
}


/*
 * NAME
 *      sub_basename - the basename substitution
 *
 * SYNOPSIS
 *      wstring_ty *sub_basename(wstring_list_ty *arg);
 *
 * DESCRIPTION
 *      The sub_basename function implements the basename substitution.
 *      The basename substitution is replaced by the basename of
 *      the argument path, similar to the basename(1) command.
 *
 * ARGUMENTS
 *      arg     - list of arguments, including the function name as [0]
 *
 * RETURNS
 *      a pointer to a string in dynamic memory;
 *      or NULL on error, setting suberr appropriately.
 */

wstring_ty *
sub_basename(sub_context_ty *scp, wstring_list_ty *arg)
{
    wstring_ty      *result;
    wstring_ty      *suffix;
    wstring_ty      *s1;
    long            len;

    trace(("sub_basename()\n{\n"));
    switch (arg->nitems)
    {
    default:
        sub_context_error_set(scp, i18n("requires one or two arguments"));
        result = 0;
        break;

    case 2:
        result = entryname(arg->item[1]);
        break;

    case 3:
        s1 = entryname(arg->item[1]);
        suffix = arg->item[2];
        len = (long)s1->wstr_length - (long)suffix->wstr_length;
        if
        (
            len > 0
        &&
            !memcmp
            (
                s1->wstr_text + len,
                suffix->wstr_text,
                suffix->wstr_length * sizeof(wchar_t)
            )
        )
        {
            result = wstr_n_from_wc(s1->wstr_text, len);
            wstr_free(s1);
        }
        else
            result = s1;
        break;
    }
    trace(("return %p;\n", result));
    trace(("}\n"));
    return result;
}
