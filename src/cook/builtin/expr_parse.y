/*
 *	cook - file construction tool
 *	Copyright (C) 1998 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate expr_parse.ys
 */

%token BIT_AND
%token BIT_NOT
%token BIT_OR
%token BIT_XOR
%token COLON
%token DIV
%token EQ
%token GE
%token GT
%token JUNK
%token LE
%token LOGIC_AND
%token LOGIC_NOT
%token LOGIC_OR
%token LP
%token LT
%token MINUS
%token MOD
%token MUL
%token NE
%token NUMBER
%token PLUS
%token QUEST
%token RP
%token SHIFT_L
%token SHIFT_R

%union {
	long	lv_integer;
}

%type	<lv_integer>	NUMBER
%type	<lv_integer>	expr

%right QUEST COLON
%left LOGIC_OR
%left LOGIC_AND
%left BIT_OR
%left BIT_XOR
%left BIT_AND
%left EQ NE
%left LT LE GT GE
%left SHIFT_L SHIFT_R
%left PLUS MINUS
%left MUL DIV MOD
%right LOGIC_NOT BIT_NOT

%{

#include <ac/stdlib.h>

#include <builtin/expr_lex.h>
#include <builtin/expr_parse.h>
#include <expr/position.h>
#include <str.h>
#include <str_list.h>
#include <sub.h>

static string_ty *result;


void
builtin_expr_parse_begin(args, pp)
	const string_list_ty *args;
	const expr_position_ty *pp;
{
	builtin_expr_lex_open(args, pp);
	if (result)
	{
		str_free(result);
		result = 0;
	}
}


string_ty *
builtin_expr_parse_end()
{
	string_ty	*s;

	builtin_expr_lex_close();
	s = result;
	result = 0;
	if (!s)
		s = str_from_c("0");
	return s;
}

%}


%%

main
	: expr
		{
			if (builtin_expr_lex_error_count() != 0)
				YYABORT;
			result = str_format("%ld", $1);
		}
	;

expr
	: NUMBER
		{ $$ = $1; }
	| LP expr RP
		{ $$ = $2; }
	| PLUS expr
		%prec BIT_NOT
		{ $$ = $2; }
	| MINUS expr
		%prec BIT_NOT
		{ $$ = -$2; }
	| LOGIC_NOT expr
		{ $$ = !$2; }
	| BIT_NOT expr
		{ $$ = ~$2; }
	| expr MUL expr
		{ $$ = $1 * $3; }
	| expr DIV expr
		{
			if ($3 == 0)
			{
				yyerror(i18n("division by zero"));
				$$ = 0;
			}
			else
				$$ = $1 / $3;
		}
	| expr MOD expr
		{
			if ($3 == 0)
			{
				yyerror(i18n("modulo by zero"));
				$$ = 0;
			}
			else
				$$ = $1 % $3;
		}
	| expr PLUS expr
		{ $$ = $1 + $3; }
	| expr MINUS expr
		{ $$ = $1 - $3; }
	| expr SHIFT_L expr
		{ $$ = $1 << $3; }
	| expr SHIFT_R expr
		{ $$ = $1 >> $3; }
	| expr LT expr
		{ $$ = ($1 < $3); }
	| expr LE expr
		{ $$ = ($1 <= $3); }
	| expr GT expr
		{ $$ = ($1 > $3); }
	| expr GE expr
		{ $$ = ($1 >= $3); }
	| expr EQ expr
		{ $$ = ($1 == $3); }
	| expr NE expr
		{ $$ = ($1 != $3); }
	| expr BIT_AND expr
		{ $$ = ($1 & $3); }
	| expr BIT_XOR expr
		{ $$ = ($1 ^ $3); }
	| expr BIT_OR expr
		{ $$ = ($1 | $3); }
	| expr LOGIC_AND expr
		{ $$ = ($1 && $3); }
	| expr LOGIC_OR expr
		{ $$ = ($1 || $3); }
	| expr QUEST expr COLON expr
		{ $$ = ($1 ? $3 : $5); }
	;
