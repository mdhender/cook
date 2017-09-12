/*
 *      cook - file construction tool
 *      Copyright (C) 1993-1995, 1997-1999, 2001, 2003, 2006-2009 Peter Miller
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

%token  CATENATE
%token  COLON
%token  COLON2  /* used within lexer */
%token  DATA
%token  DATAEND
%token  ELSE
%token  EQUALS
%token  FAIL
%token  FILE_BOUNDARY /* happens at the end of include files */
%token  FUNCTION
%token  HOST_BINDING
%token  IF
%token  JUNK
%token  LBRACE
%token  LBRAK
%token  LOOP
%token  LOOPSTOP
%token  PLUS_EQUALS
%token  RBRACE
%token  RBRAK
%token  RETURN
%token  SEMICOLON
%token  SET
%token  SINGLE_THREAD
%token  THEN
%token  UNSETENV
%token  WORD

%right  ELSE
%left   CATENATE

%{
#include <common/ac/stddef.h>
#include <common/ac/stdlib.h>
#include <common/ac/stdio.h>

#include <cook/expr.h>
#include <cook/expr/catenate.h>
#include <cook/expr/constant.h>
#include <cook/expr/function.h>
#include <cook/expr/list.h>
#include <cook/function.h>
#include <cook/lex.h>
#include <common/mem.h>
#include <cook/option.h>
#include <cook/parse.h>
#include <cook/stmt.h>
#include <cook/stmt/append.h>
#include <cook/stmt/assign.h>
#include <cook/stmt/command.h>
#include <cook/stmt/compound.h>
#include <cook/stmt/fail.h>
#include <cook/stmt/gosub.h>
#include <cook/stmt/if.h>
#include <cook/stmt/list.h>
#include <cook/stmt/loop.h>
#include <cook/stmt/loopvar.h>
#include <cook/stmt/nop.h>
#include <cook/stmt/recipe.h>
#include <cook/stmt/return.h>
#include <cook/stmt/set.h>
#include <cook/stmt/unsetenv.h>
#include <common/sub.h>
#include <common/symtab.h>
#include <common/trace.h>
#include <common/star.h>
#include <common/str_list.h>

#ifdef  DEBUG
#define YYDEBUG 1
#define printf trace_where(__FILE__, __LINE__), lex_trace
extern int      yydebug;
#endif


/*
 *  NAME
 *      parse - read and process a cookbook
 *
 *  SYNOPSIS
 *      void parse(string_ty *filename);
 *
 *  DESCRIPTION
 *      Parse reads and processes a cookbook.
 *
 *  CAVEAT
 *      If any errors are found, the user will be told,
 *      and this routine will not return.
 */

void
parse(string_ty *filename)
{
    int yyparse(void); /* forward */

    trace(("parse(filename = %p)\n{\n", filename));
    trace_string(filename->str_text);
    lex_open(filename, filename);
#if YYDEBUG
    yydebug = trace_pretest_;
#endif
    yyparse();
    lex_close();
    trace(("}\n"));
}

%}

%union
{
    expr_ty         *lv_expr;
    expr_list_ty    lv_elist;
    stmt_ty         *lv_stmt;
    stmt_list_ty    lv_slist;
    string_ty       *lv_word;
    int             lv_number;
    expr_position_ty lv_position;
}

%type <lv_position> COLON
%type <lv_stmt>     command
%type <lv_stmt>     compound_statement
%type <lv_number>   data
%type <lv_elist>    elist
%type <lv_expr>     expr
%type <lv_elist>    exprs
%type <lv_elist>    host_binding_clause
%type <lv_expr>     if_clause
%type <lv_number>   lbrak
%type <lv_position> SEMICOLON
%type <lv_elist>    set_clause
%type <lv_elist>    single_thread_clause
%type <lv_stmt>     statement
%type <lv_slist>    statements
%type <lv_stmt>     use_clause
%type <lv_word>     WORD

%%

cook
    : /* empty */
    | cook statement
        {
            star_as_specified('+');
            if (stmt_evaluate($2, 0))
            {
                lex_error(0, i18n("statement failed"));
                option_set_errors();

                /*
                 * Halt the parse immediately.
                 */
                return 1;
            }
            stmt_delete($2);
        }
    | cook function_declaration
    | cook error
        {
            lex_mode(LM_NORMAL);
        }
    ;

statement
    : compound_statement
        {
            $$ = $1;
        }
    | UNSETENV exprs SEMICOLON
        {
            $$ = stmt_unsetenv_new(&$2);
            expr_list_destructor(&$2);
            expr_position_destructor(&$3);
        }
    | elist EQUALS exprs SEMICOLON
        {
            $$ = stmt_assign_new(&$1, &$3);
            expr_list_destructor(&$1);
            expr_list_destructor(&$3);
            expr_position_destructor(&$4);
        }
    | elist PLUS_EQUALS exprs SEMICOLON
        {
            $$ = stmt_append_new(&$1, &$3);
            expr_list_destructor(&$1);
            expr_list_destructor(&$3);
            expr_position_destructor(&$4);
        }
    | elist COLON exprs set_clause if_clause single_thread_clause
                    host_binding_clause compound_statement use_clause
        {
            $$ =
                stmt_recipe_new
                (
                    &$1,               /* target */
                    &$3,               /* need */
                    (expr_list_ty *)0, /* need2 */
                    &$4,               /* flags */
                    $2.multi,          /* multiple */
                    $5,                /* precondition */
                    &$6,               /* single thread */
                    &$7,               /* host binding */
                    $8,                /* action */
                    $9,                /* use_action */
                    &$2                /* position */
                );
            expr_list_destructor(&$1);
            expr_position_destructor(&$2);
            expr_list_destructor(&$3);
            expr_list_destructor(&$4);
            if ($5)
                expr_delete($5);
            expr_list_destructor(&$6);
            expr_list_destructor(&$7);
            stmt_delete($8);
            if ($9)
                stmt_delete($9);
        }
    | elist COLON exprs COLON exprs set_clause if_clause
                    single_thread_clause host_binding_clause
                    compound_statement use_clause
        {
            $$ =
                stmt_recipe_new
                (
                    &$1,    /* target */
                    &$3,    /* need */
                    &$5,    /* need2 */
                    &$6,    /* flags */
                    ($2.multi || $4.multi),
                    $7,     /* precondition */
                    &$8,    /* single thread */
                    &$9,    /* host binding */
                    $10,    /* action */
                    $11,    /* use_action */
                    &$2     /* position */
                );
            expr_list_destructor(&$1);
            expr_position_destructor(&$2);
            expr_list_destructor(&$3);
            expr_position_destructor(&$4);
            expr_list_destructor(&$5);
            expr_list_destructor(&$6);
            if ($7)
                expr_delete($7);
            expr_list_destructor(&$8);
            expr_list_destructor(&$9);
            stmt_delete($10);
            if ($11)
                stmt_delete($11);
        }
    | elist COLON exprs set_clause if_clause SEMICOLON
        {
            $$ =
                stmt_recipe_new
                (
                    &$1,            /* target */
                    &$3,            /* need */
                    (expr_list_ty *)0, /* need2 */
                    &$4,            /* flags */
                    $2.multi,       /* multiple */
                    $5,             /* precondition */
                    (expr_list_ty *)0, /* single thread */
                    (expr_list_ty *)0, /* host binding */
                    (stmt_ty *)0,   /* action */
                    (stmt_ty *)0,   /* use_action */
                    &$2             /* position */
                );
            expr_list_destructor(&$1);
            expr_position_destructor(&$2);
            expr_list_destructor(&$3);
            expr_list_destructor(&$4);
            if ($5)
                expr_delete($5);
            expr_position_destructor(&$6);
        }
    | FILE_BOUNDARY
        {
            /*
             * This is a magic zero-length token generated
             * at the end of include files.  The idea is
             * that statements may not span include file
             * boundaries, which will catch a variety of
             * errors when include files are generated by
             * programs.  E.g. include dependencies.
             *
             * If there are problems with a generated
             * include file, the insertion of the
             * FILE_BOUNDARY will give a syntax error in the
             * OFFENDING include file, not in the next
             * statement, which is inevitably be the Wrong
             * file.
             */
            $$ = stmt_nop_new();
        }
    ;

set_clause
    : /* empty */
        {
            expr_list_constructor(&$$);
        }
    | SET exprs
        {
            $$ = $2;
        }
    ;

if_clause
    : /* empty */
        {
            $$ = 0;
        }
    | IF expr
        {
            $$ = $2;
        }
    ;

use_clause
    : /* empty */
        {
            $$ = 0;
        }
    | THEN compound_statement
        {
            $$ = $2;
        }
    ;

single_thread_clause
    : /* empty */
        {
            expr_list_constructor(&$$);
        }
    | SINGLE_THREAD elist
        {
            $$ = $2;
        }
    ;

host_binding_clause
    : /* empty */
        {
            expr_list_constructor(&$$);
        }
    | HOST_BINDING elist
        {
            $$ = $2;
        }
    ;

statement
    : command
        {
            $$ = $1;
        }
    | IF expr THEN statement
        %prec ELSE
        {
            $$ = stmt_if_new($2, $4, (stmt_ty *)0);
            expr_delete($2);
            stmt_delete($4);
        }
    | IF expr THEN statement ELSE statement
        {
            $$ = stmt_if_new($2, $4, $6);
            expr_delete($2);
            stmt_delete($4);
            stmt_delete($6);
        }
    | LOOP statement
        {
            $$ = stmt_loop_new($2);
            stmt_delete($2);
        }
    | LOOP elist EQUALS exprs compound_statement
        {
            $$ = stmt_loopvar(&$2, &$4, $5);
            expr_list_destructor(&$2);
            expr_list_destructor(&$4);
            stmt_delete($5);
        }
    | LOOPSTOP SEMICOLON
        {
            $$ = stmt_loopstop_new(&$2);
            expr_position_destructor(&$2);
        }
    | SET elist SEMICOLON
        {
            $$ = stmt_set_new(&$2, &$3);
            expr_list_destructor(&$2);
            expr_position_destructor(&$3);
        }
    | FAIL exprs SEMICOLON
        {
            $$ = stmt_fail_new(&$2);
            expr_list_destructor(&$2);
            expr_position_destructor(&$3);
        }
    | SEMICOLON
        {
            $$ = stmt_nop_new();
            expr_position_destructor(&$1);
        }
    ;

compound_statement
    : LBRACE statements RBRACE
        {
            $$ = stmt_compound_new(&$2);
            stmt_list_destructor(&$2);
        }
    ;

statements
    : /* empty */
        {
            stmt_list_constructor(&$$);
        }
    | statements statement
        {
            $$ = $1;
            stmt_list_append(&$$, $2);
            stmt_delete($2);
        }
    | statements error
        {
            $$ = $1;
        }
    ;

elist
    : expr
        {
            expr_list_constructor(&$$);
            expr_list_append(&$$, $1);
            expr_delete($1);
        }
    | elist expr
        {
            $$ = $1;
            expr_list_append(&$$, $2);
            expr_delete($2);
        }
    ;

exprs
    : /* empty */
        {
            expr_list_constructor(&$$);
        }
    | exprs expr
        {
            $$ = $1;
            expr_list_append(&$$, $2);
            expr_delete($2);
        }
    ;

expr
    : WORD
        {
            expr_position_ty pos;

            pos.pos_name = lex_cur_file();
            pos.pos_line = lex_cur_line();

            /*
             * Purify will find a a memory leak at this
             * point, if there are any syntax errors.
             * (E.g. test/01/t0107a.sh)
             * This is acceptable.
             */
            $$ = expr_constant_new($1, &pos);
            str_free($1);
        }
    | lbrak elist RBRAK
        {
            lex_mode($1);
            $$ = expr_function_new(&$2);
            expr_list_destructor(&$2);
        }
    | expr CATENATE expr
        {
            $$ = expr_catenate_new($1, $3);
            expr_delete($1);
            expr_delete($3);
        }
    ;

command
    : elist set_clause SEMICOLON
        {
            $$ = stmt_command_new(&$1, &$2, (expr_ty *)0, &$3);
            expr_list_destructor(&$1);
            expr_list_destructor(&$2);
            expr_position_destructor(&$3);
        }
    | elist set_clause SEMICOLON data expr DATAEND
        {
            $$ = stmt_command_new(&$1, &$2, $5, &$3);
            expr_list_destructor(&$1);
            expr_list_destructor(&$2);
            expr_position_destructor(&$3);
            expr_delete($5);
            lex_mode($4);
        }
    ;

data
    : DATA
        {
            $$ = lex_mode(LM_DATA);
        }
    ;

lbrak
    : LBRAK
        {
            $$ = lex_mode(LM_NORMAL);
        }
    ;

function_declaration
    : FUNCTION WORD EQUALS compound_statement
        {
            if (!function_definition($2, $4))
                lex_error(0, i18n("statement failed"));
            str_free($2);
            stmt_delete($4);
        }
    ;

statement
    : RETURN exprs SEMICOLON
        {
            $$ = stmt_return_new(&$2, &$3);
            expr_list_destructor(&$2);
            expr_position_destructor(&$3);
        }
    ;

statement
    : FUNCTION elist SEMICOLON
        {
            $$ = stmt_gosub_new(&$2, &$3);
            expr_list_destructor(&$2);
            expr_position_destructor(&$3);
        }
    ;
