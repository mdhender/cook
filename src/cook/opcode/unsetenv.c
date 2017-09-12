/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 1998, 2001, 2006, 2007 Peter Miller;
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

#include <common/ac/stdio.h>

#include <common/env.h>
#include <common/error_intl.h>
#include <cook/expr/position.h>
#include <cook/opcode/context.h>
#include <cook/opcode/private.h>
#include <cook/opcode/unsetenv.h>
#include <common/str_list.h>
#include <common/trace.h>


typedef struct opcode_unsetenv_ty opcode_unsetenv_ty;
struct opcode_unsetenv_ty
{
    opcode_ty       inherited;
    expr_position_ty pos;
};


/*
 * NAME
 *      destructor
 *
 * SYNOPSIS
 *      void destructor(opcode_ty *);
 *
 * DESCRIPTION
 *      The destructor function is used to release resources held by
 *      this opcode.  Do not free the opcode itself, this is done by the
 *      base class.
 */

static void
destructor(opcode_ty *op)
{
    opcode_unsetenv_ty *this;

    trace(("opcode_unsetenv::destructor()\n{\n"));
    this = (opcode_unsetenv_ty *) op;
    expr_position_destructor(&this->pos);
    trace(("}\n"));
}


/*
 * NAME
 *      execute
 *
 * SYNOPSIS
 *      opcode_status_ty execute(opcode_ty *, opcode_context_ty *);
 *
 * DESCRIPTION
 *      The execute function is used to execute the given opcode within
 *      the given interpretation context.
 *
 * RETURNS
 *      opcode_status_ty to indicate the result of the execution
 */

static opcode_status_ty
execute(const opcode_ty *op, opcode_context_ty *icp)
{
    const opcode_unsetenv_ty *this;
    opcode_status_ty status;
    string_list_ty  *slp;
    size_t          j;

    trace(("opcode_unsetenv::execute()\n{\n"));
    this = (const opcode_unsetenv_ty *)op;
    status = opcode_status_success;
    slp = opcode_context_string_list_pop(icp);
    if (!slp->nstrings)
    {
        error_with_position(&this->pos, 0, i18n("unsetenv was given no words"));
        status = opcode_status_error;
    }
    for (j = 0; j < slp->nstrings; ++j)
        env_unset(slp->string[j]->str_text);
    string_list_delete(slp);
    trace(("return %s;\n", opcode_status_name(status)));
    trace(("}\n"));
    return status;
}


/*
 * NAME
 *      script
 *
 * SYNOPSIS
 *      opcode_status_ty script(opcode_ty *, opcode_context_ty *);
 *
 * DESCRIPTION
 *      The script function is used to script the given opcode within
 *      the given interpretation context.
 *
 * RETURNS
 *      opcode_status_ty to indicate the result
 */

static opcode_status_ty
script(const opcode_ty *op, opcode_context_ty *icp)
{
    const opcode_unsetenv_ty *this;
    opcode_status_ty status;
    string_list_ty  *slp;
    size_t          j;

    trace(("opcode_unsetenv::script()\n{\n"));
    this = (const opcode_unsetenv_ty *)op;
    status = opcode_status_success;
    slp = opcode_context_string_list_pop(icp);
    if (!slp->nstrings)
    {
        error_with_position(&this->pos, 0, i18n("unsetenv was given no words"));
        status = opcode_status_error;
    }
    for (j = 0; j < slp->nstrings; ++j)
    {
        string_ty       *s;

        s = str_quote_shell(slp->string[j]);
        printf("unset %s\n", s->str_text);
        str_free(s);
    }
    string_list_delete(slp);
    trace(("return %s;\n", opcode_status_name(status)));
    trace(("}\n"));
    return status;
}


/*
 * NAME
 *      disassemble
 *
 * SYNOPSIS
 *      void disassemble(opcode_ty *);
 *
 * DESCRIPTION
 *      The disassemble function is used to disassemble the copdode and
 *      its arguments onto the standard output.  Don't worry about the
 *      location or a trailing newline.
 */

static void
disassemble(const opcode_ty *op)
{
    const opcode_unsetenv_ty *this;

    trace(("opcode_unsetenv::disassemle()\n{\n"));
    this = (const opcode_unsetenv_ty *)op;
    if (this->pos.pos_name)
    {
        printf("# %s:%d", this->pos.pos_name->str_text, this->pos.pos_line);
    }
    trace(("}\n"));
}


/*
 * NAME
 *      method
 *
 * DESCRIPTION
 *      The method variable describes this class.
 *
 * CAVEAT
 *      This symbol is not exported from this file.
 */

static opcode_method_ty method =
{
    "unsetenv",
    sizeof(opcode_unsetenv_ty),
    destructor,
    execute,
    script,
    disassemble,
};


/*
 * NAME
 *      opcode_unsetenv_new
 *
 * SYNOPSIS
 *      opcode_ty *opcode_unsetenv_new(void);
 *
 * DESCRIPTION
 *      The opcode_unsetenv_new function is used to allocate a new
 *      instance of an unsetenv opcode.
 *
 * RETURNS
 *      opcode_ty *; use opcode_delete when you are finished with it.
 */

opcode_ty *
opcode_unsetenv_new(expr_position_ty *pp)
{
    opcode_ty       *op;
    opcode_unsetenv_ty *this;

    trace(("opcode_unsetenv_new()\n{\n"));
    op = opcode_new(&method);
    this = (opcode_unsetenv_ty *)op;
    expr_position_copy_constructor(&this->pos, pp);
    trace(("return %08lX;\n", (long)op));
    trace(("}\n"));
    return op;
}
