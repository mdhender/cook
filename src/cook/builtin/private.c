/*
 *      cook - file construction tool
 *      Copyright (C) 1999, 2006-2008 Peter Miller
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

#include <cook/builtin/private.h>
#include <cook/expr/position.h>
#include <cook/opcode/context.h>
#include <common/str_list.h>
#include <common/trace.h>


int
builtin_interpret(builtin_ty *bp, string_list_ty *result,
    const string_list_ty *args, const expr_position_ty *pp,
    const opcode_context_ty *ocp)
{
    trace(("builtin_interpret\n"));
    assert(bp);
    assert(bp->interpret);
    return bp->interpret(result, args, pp, ocp);
}


int
builtin_script(builtin_ty *bp, string_list_ty *result,
    const string_list_ty *args, const expr_position_ty *pp,
    const opcode_context_ty *ocp)
{
    trace(("builtin_script\n"));
    assert(bp);
    assert(bp->script);
    return bp->script(result, args, pp, ocp);
}
