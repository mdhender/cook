/*
 *      cook - file construction tool
 *      Copyright (C) 1997-1999, 2006-2008 Peter Miller
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

#include <cook/builtin/thread-id.h>
#include <common/error_intl.h>
#include <cook/expr/position.h>
#include <cook/opcode/context.h>
#include <common/str_list.h>
#include <common/trace.h>


static int
interpret(string_list_ty *result, const string_list_ty *arg,
    const expr_position_ty *pp, const opcode_context_ty *ocp)
{
    string_ty       *s;

    trace(("thread-id\n"));
    if (arg->nstrings != 1)
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        sub_var_set_string(scp, "Name", arg->string[0]);
        error_with_position(pp, scp, i18n("$name: requires no arguments"));
        sub_context_delete(scp);
        return -1;
    }
    s = str_format("%ld", (long)ocp->thread_id);
    string_list_append(result, s);
    str_free(s);
    return 0;
}


builtin_ty builtin_thread_id =
{
    "thread-id",
    interpret,
    interpret,                  /* script */
};
