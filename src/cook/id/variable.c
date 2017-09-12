/*
 *      cook - file construction tool
 *      Copyright (C) 1997-1999, 2006-2009 Peter Miller
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

#include <common/error_intl.h>
#include <cook/expr/position.h>
#include <cook/id/variable.h>
#include <cook/id/private.h>
#include <cook/opcode/context.h>
#include <common/trace.h>


typedef struct id_variable_ty id_variable_ty;
struct id_variable_ty
{
    id_ty           inherited;
    string_list_ty  value;
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
    id_variable_ty  *this;

    trace(("id_variable::destructor(idp = %p)\n{\n", idp));
    this = (id_variable_ty *) idp;
    string_list_destructor(&this->value);
    trace(("}\n"));
}


/*
 * NAME
 *      evaluate
 *
 * SYNOPSIS
 *      int evaluate(id_ty *, const string_list_ty *, string_list_ty *);
 *
 * DESCRIPTION
 *      The evaluate function is used to evaluate an ID instance (there
 *      are several types).  The arguments to the evaluation are not to
 *      be changed, the results are only to be appended (not
 *      constructor'ed first).
 *
 * RETURNS
 *      int; 0 on success, -1 on error.
 */

static int
interpret(id_ty *idp, opcode_context_ty *ocp, const expr_position_ty *pp)
{
    id_variable_ty  *this;
    int             status;
    string_list_ty  *arg;

    trace(("id_variable::interpret(idp = %p)\n{\n", idp));
    this = (id_variable_ty *)idp;
    status = 0;
    arg = opcode_context_string_list_pop(ocp);
    assert(arg->nstrings >= 1);
    if (arg->nstrings >= 2)
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        sub_var_set_string(scp, "Name", arg->string[0]);
        error_with_position
        (
            pp,
            scp,
            i18n("$name: variable references no arguments")
        );
        sub_context_delete(scp);
        status = -1;
    }
    string_list_delete(arg);
    opcode_context_string_push_list(ocp, &this->value);
    trace(("}\n"));
    return status;
}


/*
 * NAME
 *      method
 *
 * DESCRIPTION
 *      The method variable describes this ID class.
 *
 * CAVEAT
 *      This symbol is not to be exported from this file (its name is
 *      not unique).
 */

static id_method_ty method =
{
    "variable",
    sizeof(id_variable_ty),
    destructor,
    interpret,
    interpret,                  /* script */
};


/*
 * NAME
 *      id_variable_new
 *
 * SYNOPSIS
 *      void id_variable_new(void);
 *
 * DESCRIPTION
 *      The id_variable_new function is used to create a new instance of
 *      a variable ID's value.  The given value is copied.
 *
 * RETURNS
 *      id_ty *; a pointer to a ID instance is dynamic memory.
 *
 * CAVEAT
 *      Use id_instance_delete when you are done with it.
 */

id_ty *
id_variable_new(string_list_ty *slp)
{
    id_ty           *idp;
    id_variable_ty  *this;

    trace(("id_variable::new()\n{\n"));
    idp = id_instance_new(&method);
    this = (id_variable_ty *)idp;
    string_list_copy_constructor(&this->value, slp);
    trace(("return %p;\n", idp));
    trace(("}\n"));
    return idp;
}


/*
 * NAME
 *      id_variable_query
 *
 * SYNOPSIS
 *      void id_variable_query(void);
 *
 * DESCRIPTION
 *      The id_variable_query function is used to extract the string
 *      list value from the variable.
 */

void
id_variable_query(id_ty *idp, string_list_ty *slp)
{
    id_variable_ty  *this;

    trace(("id_variable::query()\n{\n"));
    if (idp->method != &method)
        string_list_constructor(slp);
    else
    {
        this = (id_variable_ty *) idp;
        string_list_copy_constructor(slp, &this->value);
    }
    trace(("}\n"));
}


string_list_ty *
id_variable_query2(id_ty *idp)
{
    id_variable_ty  *this;
    string_list_ty  *slp;

    trace(("id_variable::query()\n{\n"));
    if (idp->method != &method)
        slp = 0;
    else
    {
        this = (id_variable_ty *) idp;
        slp = &this->value;
    }
    trace(("}\n"));
    return slp;
}
