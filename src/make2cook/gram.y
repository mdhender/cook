/*
 *      cook - file construction tool
 *      Copyright (C) 1994, 1997, 1998, 2001, 2006-2009 Peter Miller
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

#include <common/error_intl.h>
#include <common/trace.h>
#include <make2cook/gram.h>
#include <make2cook/lex.h>
#include <make2cook/stmt/assign.h>
#include <make2cook/stmt/blank.h>
#include <make2cook/stmt/command.h>
#include <make2cook/stmt/comment.h>
#include <make2cook/stmt/compound.h>
#include <make2cook/stmt/define.h>
#include <make2cook/stmt/export.h>
#include <make2cook/stmt/if.h>
#include <make2cook/stmt/include.h>
#include <make2cook/stmt/rule.h>
#include <make2cook/stmt/unexport.h>
#include <make2cook/stmt/vpath.h>

#ifdef  DEBUG
#define YYDEBUG 1
#ifdef YYBISON
#define fprintf yytrace2
#else
#define printf trace_where(__FILE__, __LINE__), yytrace
#endif
extern int yydebug;
#endif

static stmt_ty *rule_context;
int no_internal_rules;


void
gram(char *filename)
{
    int yyparse(void);

    trace(("gram(filename = %p)\n{\n", filename));
    lex_open(filename);
#if YYDEBUG
    yydebug = trace_pretest_;
#endif
    yyparse();
    lex_close();
    trace(("}\n"));
}

%}

%token  COLON
%token  COLON_COLON
%token  COLON_EQUALS
%token  COMMAND
%token  COMMAND_COMMENT
%token  COMMENT
%token  DEFINE
%token  ELSE
%token  EMPTY
%token  ENDDEF
%token  ENDIF
%token  EOLN
%token  EQUALS
%token  EXPORT
%token  IF
%token  INCLUDE
%token  INCLUDE2
%token  INCLUDE3
%token  OVERRIDE
%token  PLUS_EQUALS
%token  UNEXPORT
%token  VPATH
%token  VPATH2
%token  WORD


%union
{
    blob_ty         *lv_line;
    blob_list_ty    *lv_list;
    stmt_ty         *lv_stmt;
    int             lv_int;
}

%type <lv_line> COMMAND COMMENT IF WORD define_head define_word
%type <lv_line> COMMAND_COMMENT
%type <lv_stmt> stmt conditional stmts comment assignment
%type <lv_stmt> rule rule_inner rule_lhs rule_lhs_inner
%type <lv_stmt> include vpath define
%type <lv_stmt> command commands optional_commands conditional_commands
%type <lv_int> assign_op rule_op
%type <lv_list> word_list word_list_optional if
%type <lv_list> define_list define_list_optional

%right ELSE

%%

makefile
    : stmts
        {
            int             j;
            stmt_ty         *s;

            stmt_regroup($1);

            s = stmt_vpath_default();
            if (s)
                stmt_compound_append($1, s);

            if (!no_internal_rules)
            {
                for (j = 0; ; ++j)
                {
                    s = stmt_rule_default(j);
                    if (!s)
                        break;
                    stmt_compound_append($1, s);
                }
            }

            for (j = 0; ; ++j)
            {
                s = stmt_assign_default($1);
                if (!s)
                    break;
                stmt_compound_prepend($1, s);
            }

            stmt_sort($1);
            stmt_emit($1);
            stmt_free($1);
        }
    ;

stmts
    : /* empty */
        {
            $$ = stmt_compound_alloc();
        }
    | stmts stmt
        {
            $$ = $1;
            stmt_compound_append($$, $2);
        }
    ;

stmt
    : assignment
        { $$ = $1; }
    | comment
        { $$ = $1; }
    | rule
        { $$ = $1; }
    | conditional
        { $$ = $1; }
    | include
        { $$ = $1; }
    | vpath
        { $$ = $1; }
    | define
        { $$ = $1; }
    | EOLN
        { $$ = stmt_blank_alloc(); }
    | error EOLN
        { $$ = stmt_blank_alloc(); }
    ;

assignment
    : WORD assign_op word_list_optional EOLN
        {
            $$ = stmt_assign_alloc(0, $1, $2, $3);
        }
    | OVERRIDE WORD assign_op word_list_optional EOLN
        {
            $$ = stmt_assign_alloc(1, $2, $3, $4);
        }
    | EXPORT WORD assign_op word_list_optional EOLN
        {
            $$ = stmt_compound_alloc();
            stmt_compound_append
            (
                $$,
                stmt_assign_alloc(1, blob_copy($2), $3, $4)
            );
            stmt_compound_append($$, stmt_export_alloc($2));
        }
    | EXPORT WORD
        {
            $$ = stmt_export_alloc($2);
        }
    | UNEXPORT WORD
        {
            $$ = stmt_unexport_alloc($2);
        }
    ;

assign_op
    : EQUALS
        { $$ = stmt_assign_op_normal; }
    | PLUS_EQUALS
        { $$ = stmt_assign_op_plus; }
    | COLON_EQUALS
        { $$ = stmt_assign_op_colon; }
    ;

word_list
    : WORD
        {
            $$ = blob_list_alloc();
            blob_list_append($$, $1);
        }
    | word_list WORD
        {
            $$ = $1;
            blob_list_append($$, $2);
        }
    ;

word_list_optional
    : /* empty */
        { $$ = blob_list_alloc(); }
    | word_list
        { $$ = $1; }
    ;

rule
    : rule_inner
        {
            $$ = $1;
            rule_context = 0;
        }
    ;

rule_inner
    : rule_lhs
        { $$ = $1; }
    | rule_lhs commands
        {
            $$ = $1;
            stmt_rule_body($$, $2);
        }
    ;

rule_lhs
    : rule_lhs_inner
        {
            $$ = $1;
            rule_context = $$;
        }
    ;

rule_lhs_inner
    : word_list rule_op word_list_optional EOLN
        {
            $$ = stmt_rule_alloc($1, $2, $3, (blob_list_ty *)0,
                (blob_list_ty *)0, (blob_list_ty *)0);
        }
    | word_list rule_op word_list rule_op word_list_optional EOLN
        {
            $$ = stmt_rule_alloc($3, $4, $5, (blob_list_ty *)0,
                $1, (blob_list_ty *)0);
        }
    ;

commands
    : command
        {
            $$ = stmt_compound_alloc();
            stmt_compound_append($$, $1);
        }
    | commands command
        {
            $$ = $1;
            stmt_compound_append($$, $2);
        }
    ;

command
    : COMMAND
        {
            if (rule_context)
                stmt_rule_context(rule_context);
            $$ = stmt_command_alloc($1);
        }
    | conditional_commands
        { $$ = $1; }
    | COMMAND_COMMENT
        { $$ = stmt_comment_alloc($1); }
    ;

conditional_commands
    : if optional_commands endif
        { $$ = stmt_if_alloc($1, $2, (stmt_ty *)0); }
    | if optional_commands else optional_commands endif
        { $$ = stmt_if_alloc($1, $2, $4); }
    ;

optional_commands
    : /* empty */
        { $$ = stmt_compound_alloc(); }
    | commands
        { $$ = $1; }
    ;

rule_op
    : COLON
        { $$ = 1; }
    | COLON_COLON
        { $$ = 2; }
    ;

conditional
    : if stmts endif
        {
            $$ = stmt_if_alloc($1, $2, (stmt_ty *)0);
        }
    | if stmts else stmts endif
        {
            $$ = stmt_if_alloc($1, $2, $4);
        }
    ;

if
    : IF word_list_optional EOLN
        {
            $$ = $2;
            if (rule_context)
                stmt_rule_context(rule_context);
            blob_list_prepend($$, $1);
        }
    ;

eoln
    : word_list_optional EOLN
        {
            if ($1->length)
            {
                blob_error($1->list[0], 0, i18n("garbage on end of line"));
            }
            blob_list_free($1);
        }
    ;

else
    : ELSE eoln
    ;

endif
    : ENDIF eoln
    ;

comment
    : COMMENT
        { $$ = stmt_comment_alloc($1); }
    ;

include
    : INCLUDE word_list EOLN
        { $$ = stmt_include_alloc($2, 1); }
    | INCLUDE2 word_list EOLN
        { $$ = stmt_include_alloc($2, 2); }
    | INCLUDE3 word_list EOLN
        { $$ = stmt_include_alloc($2, 3); }
    ;

vpath
    : VPATH WORD word_list EOLN
        {
            blob_free($2);
            stmt_vpath_remember1($3);
            $$ = stmt_blank_alloc();
        }
    | VPATH2 assign_op word_list_optional EOLN
        {
            stmt_vpath_remember2($3);
            $$ = stmt_blank_alloc();
        }
    ;

define
    : define_head define_list_optional define_end
        {
            size_t          j;

            /*
             * append newline to all but the last
             */
            for (j = 1; j < $2->length; ++j)
            {
                string_ty       *s;

                s = $2->list[j - 1]->text;
                $2->list[j - 1]->text =
                    str_format("%s\n", s->str_text);
                str_free(s);
            }

            /*
             * Special case the last string if it is empty.
             * The last string will be empty if the user
             * wanted a trailing newline.  Cook can say this
             * more elegantly.
             */
            if ($2->length && $2->list[$2->length - 1]->text->str_length == 0)
            {
                $2->length--;
                blob_free($2->list[$2->length]);
            }

            /*
             * now treat it as a normal assignment
             */
            $$ = stmt_assign_alloc(0, $1, stmt_assign_op_normal, $2);
        }
    ;

define_head
    : DEFINE WORD EOLN
        { $$ = $2; }
    ;

define_end
    : ENDDEF eoln
    ;

define_list_optional
    : /* empty */
        { $$ = blob_list_alloc(); }
    | define_list
        { $$ = $1; }
    ;

define_list
    : define_word
        {
            $$ = blob_list_alloc();
            blob_list_append($$, $1);
        }
    | define_list define_word
        {
            $$ = $1;
            blob_list_append($$, $2);
        }
    ;

define_word
    : WORD
        { $$ = $1; }
    | EOLN
        { $$ = lex_blob(str_from_c("")); }
    ;
