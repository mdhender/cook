/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 1998, 2006, 2007 Peter Miller;
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

#include <common/sub/ident.h>
#include <common/sub/private.h>
#include <common/trace.h>
#include <common/wstr_list.h>


/*
 * NAME
 *      sub_identifier - the identifier substitution
 *
 * SYNOPSIS
 *      string_ty *sub_identifier(wstring_list_ty *arg);
 *
 * DESCRIPTION
 *      The sub_identifier function implements the identifier substitution.
 *      The identifier substitution is replaced by the single argument
 *      mapped to a C identifier.  All characters which are not legal in
 *      a C identifier are replaced by an underscore.
 *
 *      Requires exactly one argument.
 *
 * ARGUMENTS
 *      arg     - list of arguments, including the function name as [0]
 *
 * RETURNS
 *      a pointer to a string in dynamic memory;
 *      or NULL on error, setting suberr appropriately.
 */

wstring_ty *
sub_identifier(sub_context_ty *scp, wstring_list_ty *arg)
{
    wstring_ty      *result;

    trace(("sub_identifier()\n{\n"));
    if (arg->nitems != 2)
    {
        sub_context_error_set(scp, i18n("requires one argument"));
        result = 0;
    }
    else
        result = wstr_to_ident(arg->item[1]);
    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
