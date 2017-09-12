/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 1998, 2001, 2006-2009 Peter Miller
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
#include <common/str.h>
#include <common/trace.h>


/*
 * NAME
 *      expr_position_constructor
 *
 * SYNOPSIS
 *      void expr_position_constructor(expr_position_ty *, string_ty *, int);
 *
 * DESCRIPTION
 *      The expr_position_constructor function is used to initialize an
 *      expr_position_ty structure for use, given a filename and a line
 *      number.
 */

void
expr_position_constructor(expr_position_ty *to, string_ty *fn, int ln)
{
    trace(("expr_position_constructor(to = %p)\n{\n", to));
    to->pos_name = fn ? str_copy(fn) : str_from_c("");
    to->pos_line = ln;
    trace(("%s:%d (ref cnt %ld)\n",
        (to->pos_name ? to->pos_name->str_text : ""), to->pos_line,
        (to->pos_name ? to->pos_name->str_references : 0)));
    trace(("}\n"));
}


/*
 * NAME
 *      expr_position_constructorC
 *
 * SYNOPSIS
 *      void expr_position_constructorC(expr_position_ty *, char *, int);
 *
 * DESCRIPTION
 *      The expr_position_constructorC function is used to initialize an
 *      expr_position_ty structure for use, given a filename and a line
 *      number.
 */

void
expr_position_constructorC(expr_position_ty *to, char *fn, int ln)
{
    trace(("expr_position_constructorC(to = %p)\n{\n", to));
    to->pos_name = str_from_c(fn ? fn : "");
    to->pos_line = ln;
    trace(("%s:%d (ref cnt %ld)\n",
        (to->pos_name ? to->pos_name->str_text : ""), to->pos_line,
        (to->pos_name ? to->pos_name->str_references : 0)));
    trace(("}\n"));
}


/*
 * NAME
 *      expr_position_copy_constructor
 *
 * SYNOPSIS
 *      void expr_position_copy_constructor(expr_position_ty *,
 *              const expr_position_ty *);
 *
 * DESCRIPTION
 *      The expr_position_copy_constructor function is used to
 *      initialize an expr_position_ty structure for use, given a
 *      expr_position_ty instance to copy.
 */

void
expr_position_copy_constructor(expr_position_ty *to,
    const expr_position_ty *from)
{
    trace(("expr_position_copy_constructor(to = %p)\n{\n", to));
    if (from)
        expr_position_constructor(to, from->pos_name, from->pos_line);
    else
    {
        to->pos_name = str_from_c("");
        to->pos_line = 0;
    }
    trace(("%s:%d (ref cnt %ld)\n",
        (to->pos_name ? to->pos_name->str_text : ""), to->pos_line,
        (to->pos_name ? to->pos_name->str_references : 0)));
    trace(("}\n"));
}


/*
 * NAME
 *      expr_position_destructor
 *
 * SYNOPSIS
 *      void expr_position_destructor(expr_position_ty *);
 *
 * DESCRIPTION
 *      The expr_position_destructor function is used to release the
 *      resources held by a expr_position_ty structure.
 */

void
expr_position_destructor(expr_position_ty *pp)
{
    trace(("expr_position_destructor(pp = %p)\n{\n", pp));
    trace(("%s:%d (ref cnt %ld)\n",
        (pp->pos_name ? pp->pos_name->str_text : ""), pp->pos_line,
        (pp->pos_name ? pp->pos_name->str_references : 0)));
    if (pp->pos_name)
        str_free(pp->pos_name);
    pp->pos_name = 0;
    pp->pos_line = 0;
    trace(("}\n"));
}


/*
 * NAME
 *      error_with_position
 *
 * SYNOPSIS
 *      void error_with_position(expr_position_ty *, char *);
 *
 * DESCRIPTION
 *      The error_with_position function is used to report an error at a
 *      given location.  The arguments should be set using sub_var_set,
 *      as the string will be passed through the internationalized error
 *      functions.
 */

void
error_with_position(const expr_position_ty *pp, sub_context_ty *scp,
    char *fmt)
{
    string_ty       *s;
    int             need_to_delete;

    if (scp)
        need_to_delete = 0;
    else
    {
        scp = sub_context_new();
        need_to_delete = 1;
    }

    s = subst_intl(scp, fmt);

    /* re-use the substitution context */
    if (pp && pp->pos_name && pp->pos_line)
    {
        sub_var_set_string(scp, "File_Name", pp->pos_name);
        sub_var_set_long(scp, "Number", pp->pos_line);
        sub_var_set_string(scp, "MeSsaGe", s);
        error_intl(scp, i18n("$filename: $number: $message"));
        str_free(s);
    }
    else
    {
        sub_var_set_string(scp, "MeSsaGe", s);
        error_intl(scp, i18n("$message"));
        str_free(s);
    }

    if (need_to_delete)
        sub_context_delete(scp);
}
