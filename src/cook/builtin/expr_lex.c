/*
 *      cook - file construction tool
 *      Copyright (C) 1998, 1999, 2006-2008, 2010 Peter Miller
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

#include <common/ac/stdlib.h>
#include <common/ac/errno.h>

#include <common/sub.h>
#include <common/symtab.h>
#include <common/str_list.h>
#include <cook/builtin/expr_lex.h>
#include <cook/builtin/expr_parse.yacc.h>
#include <cook/expr/position.h>


static const string_list_ty *arg;
static size_t   arg_pos;
static int      error_count;
static const expr_position_ty *pp;


void
builtin_expr_lex_open(const string_list_ty *the_arg,
    const expr_position_ty *the_pp)
{
    arg = the_arg;
    pp = the_pp;
    arg_pos = 1;
    error_count = 0;
}


void
builtin_expr_lex_close(void)
{
    arg = 0;
    pp = 0;
    arg_pos = 0;
    error_count = 0;
}


void
builtin_expr_parse_error(char *s)
{
    sub_context_ty  *scp;

    scp = sub_context_new();
    sub_var_set(scp, "MeSsaGe", "%s", s);
    error_with_position(pp, scp, i18n("$message"));
    sub_context_delete(scp);
    error_count++;
}


int
builtin_expr_lex_error_count(void)
{
    return error_count;
}


typedef struct table_ty table_ty;
struct table_ty
{
    char            *name;
    int             value;
};

static table_ty table[] =
{
    { "!", LOGIC_NOT },
    { "!=", NE },
    { "%", MOD },
    { "&", BIT_AND },
    { "&&", LOGIC_AND },
    { "(", LP },
    { ")", RP },
    { "*", MUL },
    { "+", PLUS },
    { "-", MINUS },
    { "/", DIV },
    { ":", COLON },
    { "<", LT },
    { "<<", SHIFT_L },
    { "<=", LE },
    { "==", EQ },
    { ">", GT },
    { ">=", GE },
    { ">>", SHIFT_R },
    { "?", QUEST },
    { "^", BIT_XOR },
    { "|", BIT_OR },
    { "||", LOGIC_OR },
    { "~", BIT_NOT },
};


int
builtin_expr_parse_lex(void)
{
    string_ty       *s;
    char            *end;
    long            n;
    int             *data;
    static symtab_ty *stp;

    /*
     * falling off the end gives end-of-file
     */
    if (!arg || arg_pos >= arg->nstrings)
        return 0;
    s = arg->string[arg_pos++];

    /*
     * As a special case, to match the other boolean conventions
     * elsewhere in cook, the empty string means ``zero''.
     */
    if (s->str_length == 0)
    {
        builtin_expr_parse_lval.lv_integer = 0;
        return NUMBER;
    }

    /*
     * first try for a number
     */
    errno = 0;
    end = 0;
    n = strtol(s->str_text, &end, 0);
    if (end && !*end && errno != ERANGE)
    {
        builtin_expr_parse_lval.lv_integer = n;
        return NUMBER;
    }

    /*
     * ckeck for symbols
     */
    if (!stp)
    {
        table_ty        *tp;

        stp = symtab_alloc(SIZEOF(table));
        for (tp = table; tp < ENDOF(table); ++tp)
        {
            string_ty       *name;

            name = str_from_c(tp->name);
            symtab_assign(stp, name, &tp->value);
            str_free(name);
        }
    }
    data = symtab_query(stp, s);
    if (data)
        return *data;

    /*
     * anything else is junk
     */
    return JUNK;
}
