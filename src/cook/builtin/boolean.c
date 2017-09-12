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
 *
 * The builtin function all append their results to the supplied
 * `result' word list.  The first word of the `args' word list
 * is the name of the function.
 *
 * all of the functions return 0 in success, or -1 on error.
 */

#include <cook/builtin/boolean.h>
#include <common/error_intl.h>
#include <cook/expr/position.h>
#include <common/str_list.h>
#include <common/trace.h>


/*
 * NAME
 *      builtin_if - conditional evaluation
 *
 * SYNOPSIS
 *      int builtin_if(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *      Defined is a built-in function of if, described as follows:
 *      This function requires one or more arguments.
 *      The condition before the 'then' keyword is evaluated,
 *      if true, the words between the 'then' and the 'else' are the result,
 *      otherwise the words between the 'else' and the end are the value.
 *      The else is optional.
 *
 * RETURNS
 *      Appropriate things, see above.
 *
 * CAVEAT
 *      The returned result is in dynamic memory.
 *      It is the responsibility of the caller to dispose of
 *      the result when it is finished, with a string_list_destructor() call.
 *
 *      'then' and 'else' cant be escaped, sorry.
 */

static int
if_interpret(string_list_ty *result, const string_list_ty *args,
    const expr_position_ty *pp, const struct opcode_context_ty *ocp)
{
    size_t          j;
    int             cond;
    static string_ty *str_then;
    static string_ty *str_else;

    trace(("if\n"));
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
    cond = 0;
    if (!str_then)
        str_then = str_from_c("then");
    if (!str_else)
        str_else = str_from_c("else");
    for
    (
        j = 1;
        j < args->nstrings && !str_equal(str_then, args->string[j]);
        j++
    )
        cond |= str_bool(args->string[j]);
    if (j >= args->nstrings)
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        sub_var_set_string(scp, "Name", args->string[0]);
        error_with_position(pp, scp, i18n("$name: no 'then' word"));
        sub_context_delete(scp);
        return -1;
    }
    j++;
    if (cond)
    {
        while (j < args->nstrings && !str_equal(str_else, args->string[j]))
        {
            string_list_append(result, args->string[j]);
            j++;
        }
    }
    else
    {
        while (j < args->nstrings && !str_equal(str_else, args->string[j]))
            j++;
        if (j < args->nstrings)
        {
            j++;
            while (j < args->nstrings)
            {
                string_list_append(result, args->string[j]);
                ++j;
            }
        }
    }
    return 0;
}


builtin_ty builtin_if =
{
    "if",
    if_interpret,
    if_interpret,               /* script */
};


/*
 * NAME
 *      builtin_not - logical negation
 *
 * SYNOPSIS
 *      int builtin_not(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *      Not is a built-in function of cook, described as follows:
 *      This function requires zero or more arguments,
 *      the value to be logically negated.
 *
 * RETURNS
 *      It returns "1" (true) if all of the arguments are "" (false), or there
 *      are no arguments; and returns "" (false) otherwise.
 *
 * CAVEAT
 *      The returned result is in dynamic memory.
 *      It is the responsibility of the caller to dispose of
 *      the result when it is finished, with a string_list_destructor() call.
 */

static int
not_interpret(string_list_ty *result, const string_list_ty *args,
    const expr_position_ty *pp, const struct opcode_context_ty *ocp)
{
    size_t          j;

    trace(("not\n"));
    (void)pp;
    (void)ocp;
    assert(result);
    assert(args);
    assert(args->nstrings);
    for (j = 1; j < args->nstrings; j++)
    {
        if (str_bool(args->string[j]))
        {
            string_list_append(result, str_false);
            return 0;
        }
    }
    string_list_append(result, str_true);
    return 0;
}


builtin_ty builtin_not =
{
    "not",
    not_interpret,
    not_interpret,              /* script */
};


/*
 * NAME
 *      builtin_and - logical conjunction
 *
 * SYNOPSIS
 *      int builtin_and(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *      And is a built-in function of cook, described as follows:
 *      This function requires at least two arguments,
 *      upon which it forms a logical conjunction.
 *
 * RETURNS
 *      The value returned is "1" (true) if none of the arguments
 *      are "" (false), otherwise "" (false) is returned.
 *
 * CAVEAT
 *      The returned result is in dynamic memory.
 *      It is the responsibility of the caller to dispose of
 *      the result when it is finished, with a string_list_destructor() call.
 */

static int
and_interpret(string_list_ty *result, const string_list_ty *args,
    const expr_position_ty *pp, const struct opcode_context_ty *ocp)
{
    size_t          j;

    trace(("and\n"));
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
    for (j = 1; j < args->nstrings; j++)
    {
        if (!str_bool(args->string[j]))
        {
            string_list_append(result, str_false);
            return 0;
        }
    }
    string_list_append(result, str_true);
    return 0;
}


builtin_ty builtin_and =
{
    "and",
    and_interpret,
    and_interpret,              /* script */
};


/*
 * NAME
 *      builtin_or - logical disjunction
 *
 * SYNOPSIS
 *      int builtin_or(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *      Or is a built-in function of cook, described as follows:
 *      This function requires at least two arguments,
 *      upon which it forms a logical disjunction.
 *
 * RETURNS
 *      The value returned is "1" (true) if any one of the arguments is
 *      not "" (false), otherwise "" (false) is returned.
 *
 * CAVEAT
 *      The returned result is in dynamic memory.
 *      It is the responsibility of the caller to dispose of
 *      the result when it is finished, with a string_list_destructor() call.
 */

static int
or_interpret(string_list_ty *result, const string_list_ty *args,
    const expr_position_ty *pp, const struct opcode_context_ty *ocp)
{
    size_t          j;

    trace(("or\n"));
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
    for (j = 1; j < args->nstrings; j++)
    {
        if (str_bool(args->string[j]))
        {
            string_list_append(result, str_true);
            return 0;
        }
    }
    string_list_append(result, str_false);
    return 0;
}


builtin_ty builtin_or =
{
    "or",
    or_interpret,
    or_interpret,               /* script */
};


/*
 * NAME
 *      builtin_in - test for set membership
 *
 * SYNOPSIS
 *      int builtin_in(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *      In is a built-in function of cook, described as follows:
 *      This function requires one or more arguments.
 *
 * RETURNS
 *      A word list containg a single word: "1" (true) if the first argument
 *      is the same as any of the later ones; "" (false) if not.
 *
 * CAVEAT
 *      The returned result is in dynamic memory.
 *      It is the responsibility of the caller to dispose of
 *      the result when it is finished, with a string_list_destructor() call.
 */

static int
in_interpret(string_list_ty *result, const string_list_ty *args,
    const expr_position_ty *pp, const struct opcode_context_ty *ocp)
{
    size_t          j;

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
    for (j = 2; j < args->nstrings; j++)
    {
        if (str_equal(args->string[1], args->string[j]))
        {
            string_ty       *s;

            s = str_format("%ld", (long)j - 1);
            string_list_append(result, s);
            str_free(s);
            return 0;
        }
    }
    string_list_append(result, str_false);
    return 0;
}


builtin_ty builtin_in =
{
    "in",
    in_interpret,
    in_interpret,               /* script */
};
