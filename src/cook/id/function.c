/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 1999, 2006, 2007 Peter Miller;
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

#include <cook/id/function.h>
#include <cook/id/private.h>
#include <cook/opcode/context.h>
#include <cook/opcode/list.h>
#include <common/trace.h>


typedef struct id_function_ty id_function_ty;
struct id_function_ty
{
    id_ty           inherited;
    opcode_list_ty  *value;
};


/*
 * NAME
 *      destructor
 *
 * SYNOPSIS
 *      void destructor(id_ty *);
 *
 * DESCRIPTION
 *      The destructor function is used to release the resources held by
 *      an ID instance.
 */

static void
destructor(id_ty *idp)
{
    id_function_ty  *this;

    trace(("id_function::destructor(idp = %08lX)\n{\n", (long)idp));
    this = (id_function_ty *) idp;
    opcode_list_delete(this->value);
    trace(("}\n"));
}


/*
 * NAME
 *      interpret
 *
 * SYNOPSIS
 *      int interpret(id_ty *);
 *
 * DESCRIPTION
 *      The interpret function is used to evaluate an ID instance (there
 *      are several types).  The arguments to the interpretation are not to
 *      be changed, the results are only to be appended (not
 *      constructor'ed first).
 *
 * RETURNS
 *      int; 0 on success, -1 on error.
 */

static int
interpret(id_ty *idp, opcode_context_ty *ocp, const struct expr_position_ty *pp)
{
    id_function_ty  *this;

    trace(("id_function::evaluate(idp = %08lX)\n{\n", (long)idp));
    (void)pp;
    this = (id_function_ty *) idp;
    opcode_context_call(ocp, this->value);
    trace(("}\n"));
    return 0;
}


/*
 * NAME
 *      method
 *
 * DESCRIPTION
 *      The method function describes this ID class.
 *
 * CAVEAT
 *      This symbol is not to be exported from this file (its name is
 *      not unique).
 */

static id_method_ty method =
{
    "function",
    sizeof(id_function_ty),
    destructor,
    interpret,
    interpret,                  /* script */
};


/*
 * NAME
 *      id_function_new
 *
 * SYNOPSIS
 *      void id_function_new(void);
 *
 * DESCRIPTION
 *      The id_function_new function is used to create a new instance of
 *      a function ID's value.  The given value is copied.
 *
 * RETURNS
 *      id_ty *; a pointer to a ID instance is dynamic memory.
 *
 * CAVEAT
 *      Use id_instance_delete when you are done with it.
 */

id_ty *
id_function_new(opcode_list_ty *olp)
{
    id_ty           *idp;
    id_function_ty  *this;

    trace(("id_function::new()\n{\n"));
    idp = id_instance_new(&method);
    this = (id_function_ty *) idp;
    this->value = olp;
    trace(("return %08lX;\n", (long)idp));
    trace(("}\n"));
    return idp;
}
