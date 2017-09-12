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

#include <common/ac/stdlib.h>

#include <common/str.h>
#include <common/sub/plural.h>
#include <common/sub/private.h>
#include <common/trace.h>
#include <common/wstr_list.h>


wstring_ty *
sub_plural(sub_context_ty *scp, wstring_list_ty *arg)
{
    string_ty       *s;
    wstring_ty      *result;
    long            n;

    trace(("sub_plural()\n{\n"));
    switch (arg->nitems)
    {
    default:
        sub_context_error_set(scp, i18n("requires two or three arguments"));
        result = 0;
        break;

    case 3:
        wstring_list_append(arg, wstr_from_c(""));
        /* fall through... */

    case 4:
        s = wstr_to_str(arg->item[1]);
        n = atol(s->str_text);
        str_free(s);
        if (n != 1)
            result = wstr_copy(arg->item[2]);
        else
            result = wstr_copy(arg->item[3]);
        break;
    }
    trace(("return %p;\n", result));
    trace(("}\n"));
    return result;
}
