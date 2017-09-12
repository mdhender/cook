/*
 *      cook - file construction tool
 *      Copyright (C) 1991-1994, 1997, 1999, 2000, 2006, 2007 Peter Miller;
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
 * The builtin functions all append their results to the supplied
 * `result' word list.  The first word of the `args' word list
 * is the name of the function.
 *
 * all of the functions return 0 in success, or -1 on error.
 */

#include <cook/builtin/stringset.h>
#include <common/str_list.h>


#define STRINGSET_EOLN  1
#define STRINGSET_WORD  2
#define STRINGSET_PLUS  3
#define STRINGSET_MINUS 4
#define STRINGSET_STAR  5

static const string_list_ty *stringset_args;
static size_t   stringset_pos;
static int      stringset_token;
static string_ty *stringset_token_value;


static void
stringset_lex(void)
{
    static string_ty *plus;
    static string_ty *minus;
    static string_ty *star;

    if (!plus)
        plus = str_from_c("+");
    if (!minus)
        minus = str_from_c("-");
    if (!star)
        star = str_from_c("*");
    if (stringset_pos >= stringset_args->nstrings)
    {
        stringset_token_value = str_false;
        stringset_token = STRINGSET_EOLN;
        return;
    }
    stringset_token_value = stringset_args->string[stringset_pos++];
    if (str_equal(plus, stringset_token_value))
    {
        stringset_token = STRINGSET_PLUS;
        return;
    }
    if (str_equal(minus, stringset_token_value))
    {
        stringset_token = STRINGSET_MINUS;
        return;
    }
    if (str_equal(star, stringset_token_value))
    {
        stringset_token = STRINGSET_STAR;
        return;
    }
    stringset_token = STRINGSET_WORD;
}


static void
stringset_three(string_list_ty *result)
{
    while (stringset_token == STRINGSET_WORD)
    {
        string_list_append_unique(result, stringset_token_value);
        stringset_lex();
    }
}


static void
stringset_two(string_list_ty *result)
{
    stringset_three(result);
    while (stringset_token == STRINGSET_STAR)
    {
        string_list_ty  lhs;
        string_list_ty  rhs;
        size_t          j;
        string_ty       *s;

        stringset_lex();
        string_list_constructor(&rhs);
        stringset_three(&rhs);
        lhs = *result;
        string_list_constructor(result);
        for (j = 0; j < rhs.nstrings; ++j)
        {
            s = rhs.string[j];
            if (string_list_member(&lhs, s))
                string_list_append_unique(result, s);
        }
        string_list_destructor(&lhs);
        string_list_destructor(&rhs);
    }
}


static void
stringset_one(string_list_ty *result)
{
    string_list_ty  wl;
    size_t          j;

    stringset_two(result);
    for (;;)
    {
        switch (stringset_token)
        {
        default:
            return;

        case STRINGSET_MINUS:
            string_list_constructor(&wl);
            stringset_lex();
            stringset_two(&wl);
            for (j = 0; j < wl.nstrings; ++j)
                string_list_remove(result, wl.string[j]);
            string_list_destructor(&wl);
            break;

        case STRINGSET_PLUS:
            string_list_constructor(&wl);
            stringset_lex();
            stringset_two(&wl);
            string_list_append_list_unique(result, &wl);
            string_list_destructor(&wl);
            break;
        }
    }
}


static int
interpret(string_list_ty *result, const string_list_ty *args,
    const struct expr_position_ty *pp, const struct opcode_context_ty *ocp)
{
    string_list_ty  wl;

    (void)pp;
    (void)ocp;
    stringset_args = args;
    stringset_pos = 1;
    stringset_lex();

    string_list_constructor(&wl);
    stringset_one(&wl);
    string_list_append_list(result, &wl);
    string_list_destructor(&wl);
    stringset_args = 0;
    return 0;
}


builtin_ty builtin_stringset =
{
    "stringset",
    interpret,
    interpret,                  /* script */
};
