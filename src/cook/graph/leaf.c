/*
 *      cook - file construction tool
 *      Copyright (C) 1998, 1999, 2006, 2007 Peter Miller;
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

#include <cook/cook.h>
#include <cook/graph/leaf.h>
#include <cook/id.h>
#include <cook/id/variable.h>
#include <cook/match/new_by_recip.h>
#include <cook/id/global.h>
#include <cook/opcode/context.h>
#include <common/symtab.h>
#include <common/str_list.h>


static symtab_ty *stp;
static string_list_ty *leaf_file;       /* leaf of graph */
static string_list_ty *leaf_pattern;
static string_list_ty *exterior_file;   /* exterior of graph */
static string_list_ty *exterior_pattern;
static string_list_ty *interior_file;   /* interior of graph */
static string_list_ty *interior_pattern;
static leaf_ness_ty leaf_exists = leaf_ness_leaf_exists;
static leaf_ness_ty leaf_explicit = leaf_ness_leaf_explicit;
static leaf_ness_ty exterior_explicit = leaf_ness_exterior_explicit;
static leaf_ness_ty interior_exists = leaf_ness_interior_exists;
static leaf_ness_ty interior_explicit = leaf_ness_interior_explicit;


void
leaf_reset(void)
{
    if (stp)
        symtab_free(stp);
    if (leaf_pattern)
        string_list_delete(leaf_pattern);
    if (interior_pattern)
        string_list_delete(interior_pattern);
    if (exterior_pattern)
        string_list_delete(exterior_pattern);
    if (leaf_file)
        string_list_delete(leaf_file);
    if (interior_file)
        string_list_delete(interior_file);
    if (exterior_file)
        string_list_delete(exterior_file);
    stp = 0;
    leaf_file = 0;
    leaf_pattern = 0;
    interior_file = 0;
    interior_pattern = 0;
    exterior_file = 0;
    exterior_pattern = 0;
}


static string_list_ty *
find_variable(char *name)
{
    string_ty       *key;
    id_ty           *idp;
    string_list_ty  *result;

    key = str_from_c(name);
    idp = symtab_query(id_global_stp(), key);
    if (!idp)
        return 0;
    result = id_variable_query2(idp);
    if (!result)
        return 0;
    if (result->nstrings == 0)
        return 0;
    return string_list_new_copy(result);
}


static int
matches_list(string_ty *actual, string_list_ty *formal)
{
    size_t          j;
    match_ty        *mp;
    int             result;

    if (!formal)
        return 0;

    /*
     * Do we need to *clear* the recipe-level flags before evaluating
     * this.  Otherwise the pattern match mode is ambiguous.
     */
    mp = match_new_by_recipe(0);

    result = 0;
    for (j = 0; j < formal->nstrings; ++j)
    {
        /* result can be -1 (error), 0 (false), or 1 (true) */
        result = match_attempt(mp, formal->string[j], actual, 0);
        if (result)
            break;
    }
    match_delete(mp);
    return result;
}


static int
initialize(void)
{
    int             ok;
    size_t          j;
    string_ty       *s;

    if (stp)
        return 0;
    stp = symtab_alloc(5);
    leaf_file = find_variable("graph_leaf_file");
    leaf_pattern = find_variable("graph_leaf_pattern");
    interior_file = find_variable("graph_interior_file");
    interior_pattern = find_variable("graph_interior_pattern");
    exterior_file = find_variable("graph_exterior_file");
    exterior_pattern = find_variable("graph_exterior_pattern");

    if (leaf_file)
    {
        for (j = 0; j < leaf_file->nstrings; ++j)
        {
            s = leaf_file->string[j];
            ok = matches_list(s, interior_pattern);
            if (ok < 0)
                return -1;
            if (!ok)
                symtab_assign(stp, s, &leaf_explicit);
        }
    }

    if (interior_file)
    {
        for (j = 0; j < interior_file->nstrings; ++j)
        {
            s = interior_file->string[j];
            symtab_assign(stp, s, &interior_explicit);
        }
    }

    if (exterior_file)
    {
        for (j = 0; j < exterior_file->nstrings; ++j)
        {
            s = exterior_file->string[j];
            symtab_assign(stp, s, &exterior_explicit);
        }
    }

    return 0;
}


leaf_ness_ty
leaf_query(string_ty *filename, int probe)
{
    leaf_ness_ty    *d;
    time_t          t;
    int             ok;
    opcode_context_ty *ocp;

    /*
     * Initialize if we need to.
     */
    if (!stp)
    {
        ok = initialize();
        if (ok < 0)
            return leaf_ness_error;
    }

    /*
     * See if the results are already cached.
     */
    d = symtab_query(stp, filename);
    if (d)
        return *d;

    /*
     * Check rejection first.
     *
     * The initialization put the exact gnore and reject names into
     * the cache.  We only need to look for patterns.
     */
    ok = matches_list(filename, exterior_pattern);
    if (ok < 0)
        return leaf_ness_error;
    if (ok)
    {
        symtab_assign(stp, filename, &exterior_explicit);
        return leaf_ness_exterior_explicit;
    }
    ok = matches_list(filename, interior_pattern);
    if (ok < 0)
        return leaf_ness_error;
    if (ok)
    {
        symtab_assign(stp, filename, &interior_explicit);
        return leaf_ness_interior_explicit;
    }

    /*
     * Check acceptance second.
     */
    if (leaf_file || leaf_pattern)
    {
        /*
         * The initializtion put the exact accept names
         * into the cache.  We only need to look for
         * patterns.
         */
        ok = matches_list(filename, leaf_pattern);
        if (ok < 0)
            return leaf_ness_error;
        if (ok)
        {
            symtab_assign(stp, filename, &leaf_explicit);
            return leaf_ness_leaf_explicit;
        }

        /*
         * If either accept form was given, then only names
         * in those forms are acceptable.  If we get to here,
         * it isn't acceptable.
         */
        symtab_assign(stp, filename, &interior_explicit);
        return leaf_ness_interior_explicit;
    }

    /*
     * There are times when an ambiguous answer is appropriate.
     * In this case, it isn't definitely a leaf, so return false.
     * Do Not cache the result.
     */
    if (!probe)
        return leaf_ness_indeterminate;

    /*
     * No accept form was given, so go and look in the file system.
     * Thus, existence implies leaf-ness.
     */
    ocp = opcode_context_new(0, 0);
    t = cook_mtime_oldest(ocp, filename, (long *)0, 32767L);
    opcode_context_delete(ocp);
    if (t < 0)
        return leaf_ness_error;
    if (t != 0)
    {
        symtab_assign(stp, filename, &leaf_exists);
        return leaf_ness_leaf_exists;
    }
    symtab_assign(stp, filename, &interior_exists);
    return leaf_ness_interior_exists;
}


const char *
leaf_ness_name(leaf_ness_ty n)
{
    switch (n)
    {
    case leaf_ness_error:
        return "error";

    case leaf_ness_indeterminate:
        return "indeterminate";

    case leaf_ness_interior_exists:
        return "interior_exists";

    case leaf_ness_interior_explicit:
        return "interior_explicit";

    case leaf_ness_leaf_exists:
        return "leaf_exists";

    case leaf_ness_leaf_explicit:
        return "leaf_explicit";

    case leaf_ness_exterior_explicit:
        return "exterior_explicit";
    }
    return "unknown";
}
