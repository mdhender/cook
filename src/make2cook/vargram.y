/*
 *      cook - file construction tool
 *      Copyright (C) 1994, 1997, 2002, 2006-2008 Peter Miller
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

%{

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/trace.h>
#include <make2cook/vargram.h>
#include <make2cook/variable.h>

#ifdef  DEBUG
#define YYDEBUG 1
#ifdef YYBISON
#define fprintf yytrace2
#else
#define printf trace_where(__FILE__, __LINE__), yytrace
#endif
extern int      yydebug;
#endif


static string_ty *
patvar(string_ty *name, string_ty *from, string_ty *to)
{
    string_ty       *tmp;
    string_ty       *result;
    string_ty       *s_from;
    string_ty       *s_to;

    if (!strchr(from->str_text, '%'))
    {
        tmp = from;
        if (tmp->str_length == 0)
            from = str_from_c("%0%");
        else if (tmp->str_text[0] == '.')
            from = str_format("%%0%%%s", tmp->str_text);
        else
            from = str_format("%%0%%.%s", tmp->str_text);
        str_free(tmp);
    }
    else
    {
        tmp = from;
        s_from = str_from_c("%");
        s_to = str_from_c("%0%");
        from = str_substitute(s_from, s_to, tmp);
        str_free(tmp);
    }

    if (!strchr(to->str_text, '%'))
    {
        tmp = to;
        if (tmp->str_length == 0)
            to = str_from_c("%0%");
        else if (tmp->str_text[0] == '.')
            to = str_format("%%0%%%s", tmp->str_text);
        else
            to = str_format("%%0%%.%s", tmp->str_text);
        str_free(tmp);
    }
    else
    {
        tmp = to;
        s_from = str_from_c("%");
        s_to = str_from_c("%0%");
        to = str_substitute(s_from, s_to, tmp);
        str_free(tmp);
    }

    tmp = variable_mangle_lookup(name);
    str_free(name);
    result =
        str_format
        (
            "[patsubst %s %s %s]",
            from->str_text,
            to->str_text,
            tmp->str_text
        );
    str_free(tmp);
    str_free(from);
    str_free(to);
    return result;
}


static string_ty *
function(string_ty *name, string_list_ty *args)
{
    string_ty       *s;
    string_ty       *result;
    static string_ty *foreach;

    if (!foreach)
        foreach = str_from_c("foreach");
    if (str_equal(name, foreach) && args->nstrings == 3)
    {
        string_ty       *s_from;
        string_ty       *s_to;

        /*
         * The foreach function is treated specially.  This is
         * not an exact semantic mapping, but it is better than
         * nothing.
         */
        variable_mangle_forget(args->string[0]);
        s_from = str_format("[%s]", args->string[0]->str_text);
        s_to = str_from_c("%");
        s = str_substitute(s_from, s_to, args->string[2]);
        result =
            str_format
            (
                "[fromto %% %s %s]",
                s->str_text,
                args->string[1]->str_text
            );
        str_free(s);
        str_free(s_from);
        str_free(s_to);
    }
    else
    {
        /*
         * Construct the function invokation.  There are
         * make-equivalents for all the function names built
         * into cook, so there is no need to translate the
         * function name.
         */
        s = wl2str(args, 0, args->nstrings - 1, (char *)0);
        string_list_destructor(args);
        result = str_format("[%s %s]", name->str_text, s->str_text);
        str_free(s);
    }
    str_free(name);
    string_list_destructor(args);
    trace(("result = \"%s\";\n", result->str_text));
    return result;
}

%}

%token  COLON
%token  COMMA
%token  DOLLAR
%token  EQU
%token  LB
%token  LP
%token  PLAIN
%token  RB
%token  RP
%token  SPACE

%union
{
    string_ty       *lv_string;
    string_list_ty  lv_list;
}

%type <lv_string> arg
%type <lv_string> argc
%type <lv_list>   csl
%type <lv_string> gizzards
%type <lv_string> name
%type <lv_string> namec
%type <lv_string> oname
%type <lv_string> ossl
%type <lv_string> parens
%type <lv_string> PLAIN
%type <lv_list>   ssl
%type <lv_string> var

%%

main
    : dbg
        { variable_mangle_result(str_from_c("")); }
    | dbg strings
    | error
        { variable_mangle_result(str_from_c("")); }
    ;

dbg
    : /* empty */
        {
#if YYDEBUG
            yydebug = trace_pretest_;
#endif
        }
    ;

strings
    : string
    | strings string
    | strings SPACE
    ;

string
    : gizzards
        { variable_mangle_result($1); }
    | gizzards LP ssl RP
        {
            size_t          j;

            for (j = 0; j < $3.nstrings; ++j)
            {
                variable_mangle_result
                (
                    str_format("%s(%s)", $1->str_text, $3.string[j]->str_text)
                );
            }
            str_free($1);
            string_list_destructor(&$3);
        }
    ;

gizzards
    : var
        { $$ = $1; }
    | gizzards var
        {
            $$ = str_catenate($1, $2);
            str_free($1);
            str_free($2);
        }
    ;

var
    : DOLLAR DOLLAR
        { $$ = str_from_c("$"); }
    | DOLLAR PLAIN
        {
            $$ = variable_mangle_lookup($2);
            str_free($2);
        }
    | DOLLAR LP name RP
        {
            $$ = variable_mangle_lookup($3);
            str_free($3);
        }
    | DOLLAR LB name RB
        {
            $$ = variable_mangle_lookup($3);
            str_free($3);
        }
    | DOLLAR LP name COLON oname EQU oname RP
        { $$ = patvar($3, $5, $7); }
    | DOLLAR LB name COLON oname EQU oname RB
        { $$ = patvar($3, $5, $7); }
    | DOLLAR LP name SPACE csl RP
        { $$ = function($3, &$5); }
    | DOLLAR LB name SPACE csl RB
        { $$ = function($3, &$5); }
    | PLAIN
        { $$ = $1; }
    ;

name
    : namec
        { $$ = $1; }
    | name namec
        {
            $$ = str_catenate($1, $2);
            str_free($1);
            str_free($2);
        }
    ;

oname
    : /* empty */
        { $$ = str_from_c(""); }
    | oname namec
        {
            $$ = str_catenate($1, $2);
            str_free($1);
            str_free($2);
        }
    ;

namec
    : var
        { $$ = $1; }
    | COMMA
        { $$ = str_from_c(","); }
    ;

csl
    : ossl
        {
            string_list_constructor(&$$);
            string_list_append(&$$, $1);
            str_free($1);
        }
    | csl comma ossl
        {
            $$ = $1;
            string_list_append(&$$, $3);
            str_free($3);
        }
    ;

ossl
    : /* empty */
        {
            /*
             * Guess that empty space separated lists were
             * really a single space for substitution.
             * E.g. $(subst $(\n), ,$(list))
             */
            $$ = str_from_c("\" \"");
        }
    | ssl
        {
            $$ = wl2str(&$1, 0, $1.nstrings - 1, (char *)0);
            string_list_destructor(&$1);
        }
    | ssl SPACE
        {
            $$ = wl2str(&$1, 0, $1.nstrings - 1, (char *)0);
            string_list_destructor(&$1);
        }
    ;

ssl
    : arg
        {
            string_list_constructor(&$$);
            string_list_append(&$$, $1);
            str_free($1);
        }
    | ssl SPACE arg
        {
            $$ = $1;
            string_list_append(&$$, $3);
            str_free($3);
        }
    ;

arg
    : argc
        { $$ = $1; }
    | arg argc
        {
            $$ = str_catenate($1, $2);
            str_free($1);
            str_free($2);
        }
    ;

argc
    : PLAIN
        { $$ = $1; }
    | EQU
        { $$ = str_from_c("\\="); }
    | COLON
        { $$ = str_from_c("\\:"); }
    | var
        { $$ = $1; }
    | parens
        { $$ = $1; }
    | error
        { $$ = str_from_c(""); }
    ;

parens
    : LP csl RP
        {
            string_ty       *s;

            s = wl2str(&$2, 0, $2.nstrings, ",");
            string_list_destructor(&$2);
            $$ = str_format("(%s)", s->str_text);
            str_free(s);
        }
    ;

comma
    : COMMA
    | comma SPACE
    ;
