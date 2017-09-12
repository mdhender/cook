/*
 *      cook - file construction tool
 *      Copyright (C) 1990-1994, 1997, 2006, 2007 Peter Miller;
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
 *
 * This file contains the functions for manipulating statement trees,
 * allocating, interpreting and releasing.
 */

#include <cook/desist.h>
#include <cook/match.h>
#include <common/mem.h>
#include <cook/opcode/context.h>
#include <cook/opcode/list.h>
#include <cook/option.h>
#include <common/star.h>
#include <cook/stmt.h>
#include <common/trace.h>


/*
 * NAME
 *      stmt_private_new - allocate a statement structure
 *
 * SYNOPSIS
 *      stmt_ty *stmt_private_new(stmt_method_ty *);
 *
 * DESCRIPTION
 *      Allocates a new statement instance.
 *
 * RETURNS
 *      A pointer to the dynamically allocated space is returned.
 *
 * CAVEAT
 *      It is the responsibility of the caller to ensure that the space is
 *      freed when finished with, by a call to stmt_delete().
 */

stmt_ty *
stmt_private_new(stmt_method_ty *mp)
{
    stmt_ty         *sp;

    trace(("stmt_private_new()\n{\n"));
    sp = mem_alloc(mp->size);
    sp->method = mp;
    sp->s_references = 1;
    trace(("return %08lX;\n", sp));
    trace(("}\n"));
    return sp;
}


/*
 * NAME
 *      stmt_copy - copy statement tree
 *
 * SYNOPSIS
 *      stmt_ty *stmt_copy(stmt_ty *);
 *
 * DESCRIPTION
 *      The stmt_copy function is used to make a copy of a statement tree.
 *
 * RETURNS
 *      stmt_ty* - pointer to the root of the copied statement tree in dynamic
 *      memory.
 *
 * CAVEAT
 *      Use the stmt_delete function to release the tree when finished with.
 */

stmt_ty *
stmt_copy(stmt_ty *sp)
{
    trace(("stmt_copy(sp = %08X)\n{\n", sp));
    assert(sp);
    assert(sp->s_references > 0);
    sp->s_references++;
    trace(("return %08X;\n", sp));
    trace(("}\n"));
    return sp;
}


/*
 * NAME
 *      stmt_delete - delete a statement instance
 *
 * SYNOPSIS
 *      void stmt_delete(stmt_ty *sp);
 *
 * DESCRIPTION
 *      Frees a statement structure after it has been executed.
 *
 * CAVEAT
 *      It is assumed that the statement tree is in dynamic memory.
 */

void
stmt_delete(stmt_ty *sp)
{
    trace(("stmt_delete(sp = %08X)\n{\n", sp));
    assert(sp);
    assert(sp->s_references > 0);
    sp->s_references--;
    if (sp->s_references <= 0)
    {
        if (sp->method->destructor)
            sp->method->destructor(sp);
        sp->method = 0; /* paranoia */
        mem_free(sp);
    }
    trace(("}\n"));
}


/*
 * NAME
 *      stmt_evaluate - evaluate a statement node
 *
 * SYNOPSIS
 *      stmt_result_ty stmt_evaluate(stmt_ty *sp);
 *
 * DESCRIPTION
 *      Stmt_eval is used to evaluate a statement tree.
 *      It performs the actions so implied.
 *
 * RETURNS
 *      The value returned indicates why the statement evaluation terminated.
 *          STMT_OK     normal termination, success
 *          STMT_LSTOP  a loopstop statement was encountered
 *          STMT_ERROR  an execution error in a command was encountered
 *      There is also the posibility of internal subroutines;
 *      If an when this happens, an additional STMT_RET value could be returned.
 */

stmt_result_ty
stmt_evaluate(stmt_ty *sp, const match_ty *mp)
{
    stmt_result_ty  status;
    opcode_list_ty  *olp;

    trace(("stmt_evaluate(sp = %08X)\n{\n", sp));
    assert(sp);
    olp = stmt_compile(sp);
    if (olp)
    {
        opcode_context_ty *ocp;
        opcode_status_ty istatus;

        ocp = opcode_context_new(olp, mp);
        istatus = opcode_context_execute_nowait(ocp);
        opcode_context_delete(ocp);
        opcode_list_delete(olp);
        if (istatus == opcode_status_success)
            status = STMT_OK;
        else
            status = STMT_ERROR;
    }
    else
        status = STMT_ERROR;
    trace(("return %d;\n", status));
    trace(("}\n"));
    return status;
}


/*
 * NAME
 *      stmt_compile
 *
 * SYNOPSIS
 *      opcode_list_ty *stmt_compile(stmt_ty *);
 *
 * DESCRIPTION
 *      The stmt_compile function is used to compile the given statement
 *      into an opcode list.
 *
 * RETURNS
 *      opcode_list_ty *; the null pointer on error
 */

opcode_list_ty *
stmt_compile(stmt_ty *sp)
{
    opcode_list_ty  *olp;
    stmt_result_ty  status;

    trace(("stmt_compile(sp = %08X)\n{\n", sp));
    assert(sp);
    olp = opcode_list_new();
    status = stmt_code_generate(sp, olp);
    if (status != STMT_OK)
    {
        opcode_list_delete(olp);
        olp = 0;
    }
    else
    {
        /*
         * This option is not documented.
         * It is only for debugging purposes.
         */
        if (option_test(OPTION_DISASSEMBLE))
            opcode_list_disassemble(olp);
    }
    trace(("return %08lX;\n", (long)olp));
    trace(("}\n"));
    return olp;
}


/*
 * NAME
 *      stmt_code_generate
 *
 * SYNOPSIS
 *      stmt_result_ty stmt_code_generate(stmt_ty *, struct opcode_list_ty *);
 *
 * DESCRIPTION
 *      The stmt_code_generate function is used to generate the opcdes
 *      for the given statement.
 */

stmt_result_ty
stmt_code_generate(stmt_ty *sp, struct opcode_list_ty *olp)
{
    stmt_result_ty  status;

    trace(("stmt_code_generate(sp = %08lX, olp = %08lX)\n{\n",
            (long)sp, (long)olp));
    assert(sp);
    assert(sp->method);
    assert(sp->method->code_generate);
    status = sp->method->code_generate(sp, olp);
    trace(("return %d;\n", status));
    trace(("}\n"));
    return status;
}
