/*
 *      cook - file construction tool
 *      Copyright (C) 1992-1994, 1997-1999, 2001, 2006-2008 Peter Miller
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
 * The builtin function all append their results to the supplied
 * `result' word list.  The first word of the `args' word list
 * is the name of the function.
 *
 * all of the functions return 0 in success, or -1 on error.
 *
 * Only a limited set of this are candidates for builtin functions,
 * these are
 *      - string manipulation [dirname, stringset, ect ]
 *      - environment manipulation [getenv(3), etc]
 *      - stat(3) related functions [exists, mtime, pathname, etc]
 *      - launching OS commands [execute, collect]
 * The above list is though to be exhaustive.
 *
 * This explicitly and forever excluded from being a builtin function
 * is anything which known or understands the format of some secific
 * class of files.
 *
 * Access to stdio(3) has been thought of, and explicitly avoided.
 * Mostly because a specialist program used through [collect]
 * will almost always be far faster.
 */

#include <cook/builtin/match.h>
#include <common/error_intl.h>
#include <cook/expr/position.h>
#include <cook/match.h>
#include <common/str_list.h>
#include <common/trace.h>


/*
 * NAME
 *      builtin_match - wildcard mapping
 *
 * SYNOPSIS
 *      int builtin_match(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *      Fromto is a built-in function of cook, described as follows:
 *      This function requires at least two arguments.
 *      Fromto gives the user access to the wildcard transformations
 *      available to cook.
 *      The first argument is the "from" form,
 *      the second argument is the "to" form.
 *
 * RETURNS
 *      All other arguments are mapped from one to the other.
 *
 * CAVEAT
 *      The returned result is in dynamic memory.
 *      It is the responsibility of the caller to dispose of
 *      the result when it is finished, with a string_list_destructor() call.
 */

static int
match_interpret(string_list_ty *result, const string_list_ty *args,
    const expr_position_ty *pp, const struct opcode_context_ty *ocp)
{
    size_t          j;
    match_ty        *mp;
    int             retval;
    int             retval2;

    trace(("match\n"));
    (void)ocp;
    assert(result);
    assert(args);
    assert(args->nstrings);
    if (args->nstrings < 2)
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        sub_var_set_string(scp, "Name", args->string[0]);
        error_with_position
        (
            pp,
            scp,
            i18n("$name: requires one or more arguments")
        );
        sub_context_delete(scp);
        return -1;
    }
    retval = 0;
    mp = match_new();
    if (match_compile(mp, args->string[1], pp) < 0)
    {
        retval = -1;
        goto error_ret;
    }
    for (j = 2; j < args->nstrings; j++)
    {
        retval2 = match_execute(mp, args->string[j], pp);
        if (retval2 < 0)
        {
            retval = -1;
            break;
        }
        if (retval2)
        {
            string_ty       *s;

            s = str_format("%ld", (long)j - 1);
            string_list_append(result, s);
            str_free(s);
        }
        else
            string_list_append(result, str_false);
    }
    error_ret:
    match_delete(mp);
    return retval;
}


builtin_ty builtin_match =
{
    "match",
    match_interpret,
    match_interpret,            /* script */
};


builtin_ty builtin_matches =
{
    "matches",
    match_interpret,
    match_interpret,            /* script */
};


/*
 * NAME
 *      builtin_match - wildcard mapping
 *
 * SYNOPSIS
 *      int builtin_match(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *      Fromto is a built-in function of cook, described as follows:
 *      This function requires at least two arguments.
 *      Fromto gives the user access to the wildcard transformations
 *      available to cook.
 *      The first argument is the "from" form,
 *      the second argument is the "to" form.
 *
 * RETURNS
 *      All other arguments are mapped from one to the other.
 *
 * CAVEAT
 *      The returned result is in dynamic memory.
 *      It is the responsibility of the caller to dispose of
 *      the result when it is finished, with a string_list_destructor() call.
 */

static int
match_mask_interpret(string_list_ty *result, const string_list_ty *args,
    const expr_position_ty *pp, const struct opcode_context_ty *ocp)
{
    size_t          j;
    match_ty        *mp;
    int             retval;
    int             retval2;

    trace(("match_mask\n"));
    (void)ocp;
    assert(result);
    assert(args);
    assert(args->nstrings);
    if (args->nstrings < 2)
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        sub_var_set_string(scp, "Name", args->string[0]);
        error_with_position
        (
            pp,
            scp,
            i18n("$name: requires one or more arguments")
        );
        sub_context_delete(scp);
        return -1;
    }
    retval = 0;
    mp = match_new();
    if (match_compile(mp, args->string[1], pp) < 0)
    {
        retval = -1;
        goto error_ret;
    }
    for (j = 2; j < args->nstrings; j++)
    {
        retval2 = match_execute(mp, args->string[j], pp);
        if (retval2 < 0)
        {
            retval = -1;
            break;
        }
        if (retval2)
            string_list_append(result, args->string[j]);
    }
    error_ret:
    match_delete(mp);
    return retval;
}


builtin_ty builtin_match_mask =
{
    "match_mask",
    match_mask_interpret,
    match_mask_interpret,       /* script */
};


builtin_ty builtin_filter =
{
    "filter",
    match_mask_interpret,
    match_mask_interpret,       /* script */
};


/*
 * NAME
 *      builtin_fromto - wildcard mapping
 *
 * SYNOPSIS
 *      int builtin_fromto(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *      Fromto is a built-in function of cook, described as follows:
 *      This function requires at least two arguments.
 *      Fromto gives the user access to the wildcard transformations
 *      available to cook.
 *      The first argument is the "from" form,
 *      the second argument is the "to" form.
 *
 * RETURNS
 *      All other arguments are mapped from one to the other.
 *
 * CAVEAT
 *      The returned result is in dynamic memory.
 *      It is the responsibility of the caller to dispose of
 *      the result when it is finished, with a string_list_destructor() call.
 */

static int
fromto_interpret(string_list_ty *result, const string_list_ty *args,
    const expr_position_ty *pp, const struct opcode_context_ty *ocp)
{
    size_t          j;
    match_ty        *mp;
    int             retval;
    int             retval2;

    trace(("fromto\n"));
    (void)ocp;
    assert(result);
    assert(args);
    assert(args->nstrings);
    if (args->nstrings < 3)
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        sub_var_set_string(scp, "Name", args->string[0]);
        error_with_position
        (
            pp,
            scp,
            i18n("$name: requires two or more arguments")
        );
        sub_context_delete(scp);
        return -1;
    }
    retval = 0;
    mp = match_new();
    if (match_compile(mp, args->string[1], pp) < 0)
    {
        retval = -1;
        goto error_ret;
    }
    for (j = 3; j < args->nstrings; j++)
    {
        retval2 = match_execute(mp, args->string[j], pp);
        if (retval2 < 0)
        {
            retval = -1;
            break;
        }
        if (retval2)
        {
            string_ty       *s;

            s = match_reconstruct_rhs(mp, args->string[2], pp);
            if (!s)
            {
                retval = -1;
                break;
            }
            string_list_append(result, s);
            str_free(s);
        }
        else
            string_list_append(result, args->string[j]);
    }
    error_ret:
    match_delete(mp);
    return retval;
}


builtin_ty builtin_fromto =
{
    "fromto",
    fromto_interpret,
    fromto_interpret,           /* script */
};


builtin_ty builtin_patsubst =
{
    "patsubst",
    fromto_interpret,
    fromto_interpret,           /* script */
};
