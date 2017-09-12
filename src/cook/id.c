/*
 *      cook - file construction tool
 *      Copyright (C) 1991-1994, 1997, 1999, 2001, 2006, 2007 Peter Miller;
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

#include <common/ac/stddef.h>
#include <common/ac/stdlib.h>

#include <cook/id/global.h>
#include <cook/id/private.h>
#include <cook/id/variable.h>
#include <common/mem.h>
#include <cook/option.h>
#include <common/progname.h>
#include <common/version-stmp.h>
#include <common/str_list.h>
#include <common/symtab.h>
#include <common/trace.h>

string_ty      *id_need;
string_ty      *id_younger;
string_ty      *id_target;
string_ty      *id_targets;
string_ty      *id_search_list;


/*
 * NAME
 *      id_initialize - start up symbol table
 *
 * SYNOPSIS
 *      void id_initialize(void);
 *
 * DESCRIPTION
 *      The id_initialize function is used to create the hash table.
 *
 * RETURNS
 *      void
 *
 * CAVEAT
 *      Assumes the str_initialize function has been called already.
 */

void
id_initialize(void)
{
    trace(("init\n"));
    id_need = str_from_c("need");
    id_younger = str_from_c("younger");
    id_target = str_from_c("target");
    id_targets = str_from_c("targets");
    id_search_list = str_from_c("search_list");

    id_reset();
}


void
id_reset(void)
{
    string_list_ty  wl;
    string_ty      *s;

    id_global_reset();

    /*
     * set the "version" predefined variable
     */
    string_list_constructor(&wl);
    s = str_from_c(version_stamp());
    string_list_append(&wl, s);
    str_free(s);
    s = str_from_c("version");
    symtab_assign(id_global_stp(), s, id_variable_new(&wl));
    str_free(s);
    string_list_destructor(&wl);

    /*
     * set the "self" predefined variable
     */
    s = str_from_c(progname_get());
    string_list_append(&wl, s);
    str_free(s);
    s = str_from_c("self");
    symtab_assign(id_global_stp(), s, id_variable_new(&wl));
    str_free(s);
    string_list_destructor(&wl);

#ifdef __STDC__
    /*
     * This symbol is only defined if we have a conforming C
     * compiler.  This is support for the C recipes.
     */
    string_list_append(&wl, str_true);
    s = str_from_c("__STDC__");
    symtab_assign(id_global_stp(), s, id_variable_new(&wl));
    str_free(s);
    string_list_destructor(&wl);
#endif

    /*
     * set the default search list
     * in the "search_list" predefined variable
     */
    s = str_from_c(".");
    string_list_append(&wl, s);
    str_free(s);
    symtab_assign(id_global_stp(), id_search_list, id_variable_new(&wl));
    string_list_destructor(&wl);
}


int
id_interpret(id_ty *idp, struct opcode_context_ty *ocp,
    const struct expr_position_ty *pp)
{
    assert(idp);
    assert(idp->method);
    assert(idp->method->interpret);
    return idp->method->interpret(idp, ocp, pp);
}


int
id_interpret_script(id_ty *idp, struct opcode_context_ty *ocp,
    const struct expr_position_ty *pp)
{
    assert(idp);
    assert(idp->method);
    assert(idp->method->script);
    return idp->method->script(idp, ocp, pp);
}
