/*
 *      cook - file construction tool
 *      Copyright (C) 1991-1994, 1997-1999, 2006, 2007 Peter Miller;
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

#include <cook/expr/position.h>
#include <cook/match/private.h>
#include <common/mem.h>
#include <common/str.h>
#include <common/trace.h>


/*
 * NAME
 *      match_delete - dispose of match structure
 *
 * SYNOPSIS
 *      void match_delete(match_ty *);
 *
 * DESCRIPTION
 *      The match_delete function is used to dispose of a match structure
 *      allocated by the match_alloc function.
 *
 * RETURNS
 *      void
 */

void
match_delete(match_ty *this)
{
    trace(("match_delete(this = %08X)\n{\n", this));
    if (this->vptr->destructor)
        this->vptr->destructor(this);
    mem_free(this);
    trace(("}\n"));
}


/*
 * NAME
 *      match - attempt to
 *
 * SYNOPSIS
 *      match_ty *match(string_ty *pattern, string_ty *string);
 *
 * DESCRIPTION
 *      The match function is used to match a pattern with a string.
 *      The matching fields are filled in in the returned structure.
 *
 * RETURNS
 *      match_ty *: a pointer to a match structure in dynamic memory with the
 *      match fields set as appropriate.
 *
 *      A NULL pointer is returned if the string does not match the
 *      pattern.
 *
 *      The value MATCH_ERROR will be returned if it was not a valid
 *      pattern; the error message will have been printed already.
 *
 * CAVEAT
 *      The match structure should be released by calling match_delete.,
 */

int
match_attempt(match_ty *mp, string_ty *formal, string_ty *actual,
    const expr_position_ty *pp)
{
    if (match_compile(mp, formal, pp) < 0)
        return -1;
    return match_execute(mp, actual, pp);
}


int
match_compile(match_ty *this, string_ty *formal, const expr_position_ty *pp)
{
    int             result;

    trace(("match_compile(this = %08lX, formal = %08lX)\n{\n", (long)this,
        (long)formal));
    trace_string(formal->str_text);
    result = this->vptr->compile(this, formal, pp);
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}


int
match_execute(match_ty *this, string_ty *actual, const expr_position_ty *pp)
{
    int             result;

    trace(("match_execute(this = %08lX, actual = %08lX)\n{\n", (long)this,
        (long)actual));
    trace_string(actual->str_text);
    result = this->vptr->execute(this, actual, pp);
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}


/*
 * NAME
 *      reconstruct - make string from pattern
 *
 * SYNOPSIS
 *      string_ty *reconstruct(string_ty *pattern, match_ty *field);
 *
 * DESCRIPTION
 *      The reconstruct function is used to rebuild a string from a replacement
 *      pattern and the match field values.
 *
 * RETURNS
 *      string_ty *; pointer to the reconstructed string
 *              or NULL on error (the error will already have been rinted)
 */

string_ty *
match_reconstruct_lhs(const match_ty *this, string_ty *pattern,
    const expr_position_ty *pp)
{
    string_ty       *result;

    trace(("reconstruct(this = %08lX, pattern = %08X)\n{\n", this, pattern));
    result = this->vptr->reconstruct_lhs(this, pattern, pp);
    trace(("return %08lX;\n", result));
    trace(("}\n"));
    return result;
}


string_ty *
match_reconstruct_rhs(const match_ty *this, string_ty *pattern,
    const expr_position_ty *pp)
{
    string_ty       *result;

    trace(("reconstruct(this = %08lX, pattern = %08X)\n{\n", this, pattern));
    result = this->vptr->reconstruct_rhs(this, pattern, pp);
    trace(("return %08lX;\n", result));
    trace(("}\n"));
    return result;
}


int
match_usage_mask(const match_ty *this, string_ty *s, const expr_position_ty *pp)
{
    return this->vptr->usage_mask(this, s, pp);
}
