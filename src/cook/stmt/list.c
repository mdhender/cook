/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 1999, 2001, 2006-2009 Peter Miller
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

#include <common/mem.h>
#include <cook/stmt.h>
#include <cook/stmt/list.h>
#include <common/trace.h>


/*
 *  NAME
 *      stmt_list_append - append to a statement list
 *
 *  SYNOPSIS
 *      void stmt_list_append(stmt_list_ty *slp, stmt *sp);
 *
 *  DESCRIPTION
 *      stmt_list_append is used to append a statement to a statement list.
 */

void
stmt_list_append(stmt_list_ty *sl, stmt_ty *s)
{
    trace(("stmt_list_append(sl = %p, s = %p)\n{\n", sl, s));
    if (sl->sl_nstmts >= sl->sl_nstmts_max)
    {
        size_t          nbytes;

        sl->sl_nstmts_max = sl->sl_nstmts_max * 2 + 4;
        nbytes = sl->sl_nstmts_max * sizeof(sl->sl_stmt[0]);
        sl->sl_stmt = mem_change_size(sl->sl_stmt, nbytes);
    }
    sl->sl_stmt[sl->sl_nstmts++] = stmt_copy(s);
    trace(("}\n"));
}


/*
 *  NAME
 *      stmt_list_destructor - free statement list
 *
 *  SYNOPSIS
 *      void stmt_list_destructor(stmt_list_ty *);
 *
 *  DESCRIPTION
 *      The stmt_list_destructor function is used to free the list of
 *      statement trees.
 *
 *  RETURNS
 *      void
 */

void
stmt_list_destructor(stmt_list_ty *sl)
{
    size_t          j;

    trace(("stmt_list_destructor(sl = %p)\n{\n", sl));
    for (j = 0; j < sl->sl_nstmts; ++j)
        stmt_delete(sl->sl_stmt[j]);
    if (sl->sl_stmt)
        mem_free(sl->sl_stmt);
    sl->sl_nstmts = 0;
    sl->sl_nstmts_max = 0;
    sl->sl_stmt = 0;
    trace(("}\n"));
}


void
stmt_list_constructor(stmt_list_ty *slp)
{
    slp->sl_nstmts = 0;
    slp->sl_nstmts_max = 0;
    slp->sl_stmt = 0;
}


void
stmt_list_copy_constructor(stmt_list_ty *slp, const stmt_list_ty *other)
{
    size_t          j;

    slp->sl_nstmts = 0;
    slp->sl_nstmts_max = 0;
    slp->sl_stmt = 0;

    for (j = 0; j < other->sl_nstmts; ++j)
        stmt_list_append(slp, other->sl_stmt[j]);
}


stmt_list_ty *
stmt_list_new(void)
{
    stmt_list_ty    *slp;

    slp = mem_alloc(sizeof(stmt_list_ty));
    stmt_list_constructor(slp);
    return slp;
}


void
stmt_list_delete(stmt_list_ty *slp)
{
    stmt_list_destructor(slp);
    mem_free(slp);
}
