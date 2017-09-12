/*
 *	cook - file construction tool
 *	Copyright (C) 1993-1995, 1997-1999, 2001, 2003, 2004 Peter Miller;
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
 * MANIFEST: functions to parse #directive lines in cookbooks
 *
 * The hashline.y and parse.y parsers share the same lexer.
 * This means using the classic sed hack for yacc output.
 * Note that the expression grammars must be as similar as possible
 * in the two grammars.
 *
 * The state table in the condition frames is very simple:
 *	state 0: before #if
 *	state 1: after #if (and variants)
 *	state 2: after #elif
 *	state 3: after #else
 *	state 0: after #endif
 */

%{

#include <ac/stdio.h>
#include <ac/stddef.h>
#include <ac/string.h>
#include <ac/time.h>
#include <ac/stdlib.h>

#include <cook.h>
#include <expr.h>
#include <expr/catenate.h>
#include <expr/constant.h>
#include <expr/function.h>
#include <expr/list.h>
#include <hashline.h>
#include <lex.h>
#include <mem.h>
#include <opcode/context.h>
#include <option.h>
#include <os_interface.h>
#include <sub.h>
#include <trace.h>
#include <str_list.h>


static string_list_ty done_once;


typedef struct cond cond;
struct cond
{
	int	pass;
	int	state;
	cond	*next;
};

static	cond	*stack;
static	cond	*cond_free_list;

#ifdef DEBUG
#define YYDEBUG 1
#define printf trace_where(__FILE__, __LINE__), lex_trace
extern int yydebug;
#endif


#define yyerror parse_error


static expr_position_ty *curpos _((void));

static expr_position_ty *
curpos()
{
	static expr_position_ty pos;

	pos.pos_name = lex_cur_file();
	pos.pos_line = lex_cur_line();
	return &pos;
}


/*
 * NAME
 *	open_include - open an include file
 *
 * SYNOPSIS
 *	void open_include(string_ty *filename);
 *
 * DESCRIPTION
 *	The open_include function is used to search for a given file name in
 *	the include path and lex_open it when found.
 *
 * RETURNS
 *	void
 */

static void open_include_once _((string_ty *, string_ty *));

static void
open_include_once(logical, physical)
	string_ty	*logical;
	string_ty	*physical;
{
	if (!string_list_member(&done_once, physical))
		lex_open_include(logical, physical);
}


void
hashline_reset()
{
	string_list_destructor(&done_once);
}


static void open_include _((string_ty *, int));

static void
open_include(filename, local)
	string_ty	*filename;
	int		local;
{
	int		j;
	string_ty	*path;

	trace(("open_include(filename = %08lX, local = %d) entry",
		filename, local));
	trace_string(filename->str_text);
	if (filename->str_text[0] != '/')
	{
		if (local)
		{
			string_ty	*s;

			s = lex_cur_physical_file();
			if (strchr(s->str_text, '/'))
			{
				s = os_dirname(s);
				if (!s)
				{
					bomb:
					yyerror
					(
					 "unable to construct include file name"
					);
					goto ret;
				}
				path = str_format("%S/%S", s, filename);
				str_free(s);
			}
			else
				path = str_copy(filename);
			switch (os_exists(path))
			{
			case -1:
				str_free(path);
				goto bomb;

			case 1:
				open_include_once(filename, path);
				str_free(path);
				goto ret;
			}
			str_free(path);
		}
		for (j = 0; j < option.o_search_path.nstrings; ++j)
		{
			path =
				str_format
				(
					"%S/%S",
					option.o_search_path.string[j],
					filename
				);
			switch (os_exists(path))
			{
			case -1:
				str_free(path);
				goto bomb;

			case 1:
				open_include_once(filename, path);
				str_free(path);
				goto ret;
			}
			str_free(path);
		}
	}
	open_include_once(filename, filename);
	ret:
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	hashline - the # control line processor
 *
 * SYNOPSIS
 *	void hashline(void);
 *
 * DESCRIPTION
 *	The hashline function is used to process # control lines.
 *
 * RETURNS
 *	void
 */

void
hashline()
{
	int yyparse _((void)); /* forward */

	trace(("hashline()\n{\n"/*}*/));
#if YYDEBUG
	yydebug = trace_pretest_;
#endif
	hashline_lex_reset();
	yyparse();
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	cond_alloc - allocate a condition structure
 *
 * SYNOPSIS
 *	cond *cond_alloc(void);
 *
 * DESCRIPTION
 *	The cond_alloc function is used to allocate a condition structure
 *	from dynamic memory.
 *
 * RETURNS
 *	cond * - pointer to condition structure.
 *
 * CAVEAT
 *	A free list is maintained to avoid malloc overheads.
 */

static cond *cond_alloc _((void));

static cond *
cond_alloc()
{
	cond		*c;

	if (cond_free_list)
	{
		c = cond_free_list;
		cond_free_list = c->next;
	}
	else
		c = (cond *)mem_alloc(sizeof(cond));
	return c;
}


/*
 * NAME
 *	cond_free - free condition structure
 *
 * SYNOPSIS
 *	void cond_free(cond*);
 *
 * DESCRIPTION
 *	The cond_free function is used to indicate that a condition structure
 *	is finished with.
 *
 * RETURNS
 *	void
 *
 * CAVEAT
 *	A free list is maintained to avoid malloc overheads.
 */

static void cond_free _((cond *));

static void
cond_free(c)
	cond		*c;
{
	c->next = cond_free_list;
	cond_free_list = c;
}


/*
 * NAME
 *	hash_include - process #include directive
 *
 * SYNOPSIS
 *	void hash_include(expr_list_ty *filename);
 *
 * DESCRIPTION
 *	The hash_include function is used to process #include directives.
 *
 * RETURNS
 *	void
 */

static void hash_include _((expr_list_ty *));

static void
hash_include(elp)
	expr_list_ty	*elp;
{
	string_list_ty	*result;
	string_ty	*s;
	size_t		j;

	/*
	 * if conditional is false, don't do
	 */
	if (stack && !stack->pass)
		return;

	/*
	 * turn the expressions into words
	 */
	result = expr_list_evaluate(elp, 0);
	if (!result)
	{
		hashline_error("expression evaluation failed");
		return;
	}

	/*
	 * include each file
	 */
	for (j = 0; j < result->nstrings; ++j)
	{
		s = result->string[j];
		if
		(
			s->str_length > 2
		&&
			s->str_text[0] == '<'
		&&
			s->str_text[s->str_length - 1] == '>'
		)
		{
			s = str_n_from_c(s->str_text + 1, s->str_length - 2);
			open_include(s, 0);
			str_free(s);
		}
		else
		{
			if (s->str_length)
				open_include(s, 1);
			else
			{
				yyerror
				(
				 "expression produces null file name to include"
				);
			}
		}
	}
	string_list_delete(result);
}


/*
 * NAME
 *	hash_include - process #include-cooked directive
 *
 * SYNOPSIS
 *	void hash_include_cooked(expr_list_ty *filename);
 *
 * DESCRIPTION
 *	The hash_include_cooked function is used to
 *	process #include-cooked directives.
 *
 * RETURNS
 *	void
 */

static void hash_include_cooked _((expr_list_ty *, int));

static void
hash_include_cooked(elp, warn)
	expr_list_ty	*elp;
	int		warn;
{
	string_list_ty	*logical;
	string_ty	*s;
	long		j;
	string_list_ty	physical;
	long		nerr;
	opcode_context_ty *ocp;

	/*
	 * if conditional is false, don't do
	 */
	if (stack && !stack->pass)
		return;

	/*
	 * turn the expressions into words
	 */
	logical = expr_list_evaluate(elp, 0);
	if (!logical)
	{
		hashline_error("expression evaluation failed");
		return;
	}

	/*
	 * make sure we like the words they used
	 */
	nerr = 0;
	for (j = 0; j < logical->nstrings; ++j)
	{
		s = logical->string[j];
		if
		(
			s->str_length > 2
		&&
			s->str_text[0] == '<'
		&&
			s->str_text[s->str_length - 1] == '>'
		)
		{
			yyerror
			(
			       "may not use angle brackets with #include-cooked"
			);
			++nerr;
		}
		else if (!s->str_length)
		{
			yyerror
			(
				"expression produces null file name to include"
			);
			++nerr;
		}
	}
	if (nerr)
	{
		string_list_delete(logical);
		return;
	}

	/*
	 * append to the auto-cook list
	 *
	 * If any of the auto-cook list are out-of-date,
	 * they are recooked, and then cook starts over.
	 */
	cook_auto(logical);

	/*
	 * resolve the words into paths
	 */
	string_list_constructor(&physical);
	ocp = opcode_context_new(0, 0);
	cook_mtime_resolve(ocp, &physical, logical, 0);
	opcode_context_delete(ocp);

	/*
	 * include the resolved paths,
	 * warning if they do not exist
	 * (they will later, hopefully)
	 */
	assert(logical->nstrings == physical.nstrings);
	for (j = 0; j < physical.nstrings; ++j)
	{
		s = physical.string[j];
		if (os_exists(s))
			open_include_once(logical->string[j], s);
		else if (warn)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", s);
			lex_warning
			(
				scp,
			    i18n("include cooked \"$filename\": file not found")
			);
			sub_context_delete(scp);
		}
	}
	string_list_destructor(&physical);
	string_list_delete(logical);
}


/*
 * NAME
 *	hash_if - process #if directive
 *
 * SYNOPSIS
 *	void hash_if(expr_ty *);
 *
 * DESCRIPTION
 *	The hash_if function is used to process #if directives.
 *
 * RETURNS
 *	void
 */

static void hash_if _((expr_ty *));

static void
hash_if(ep)
	expr_ty		*ep;
{
	cond		*c;

	trace(("hash_if(ep = %08lX)\n{\n"/*}*/, ep));
	c = cond_alloc();
	c->next = stack;
	if (stack && !stack->pass)
	{
		c->pass = 0;
		c->state = 1;
		lex_passing(0);
	}
	else
	{
		switch (expr_eval_condition(ep, 0))
		{
		case -1:
			yyerror("expression evaluation failed");
			/* fall through... */

		case 0:
			c->pass = 0;
			c->state = 2;
			lex_passing(0);
			break;

		default:
			c->pass = 1;
			c->state = 1;
			lex_passing(1);
			break;
		}
	}
	stack = c;
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	hash_ifdef - process #ifdef directive
 *
 * SYNOPSIS
 *	void hash_ifdef(expr_ty*);
 *
 * DESCRIPTION
 *	The hash_ifdef function is used to process #ifdef directives.
 *
 * RETURNS
 *	void
 */

static void hash_ifdef _((expr_ty *));

static void
hash_ifdef(ep)
	expr_ty		*ep;
{
	expr_ty		*e1;
	expr_ty		*e2;
	string_ty	*s;

	trace(("hash_ifdef(ep = %08lX)\n{\n"/*}*/, ep));
	s = str_from_c("defined");
	e1 = expr_constant_new(s, curpos());
	str_free(s);
	e2 = expr_function_new2(e1, ep);
	expr_delete(e1);
	hash_if(e2);
	expr_delete(e2);
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	hash_ifndef - process #ifndef directives
 *
 * SYNOPSIS
 *	void hash_ifndef(expr_ty *);
 *
 * DESCRIPTION
 *	The hash_ifndef function is used to process #ifndef directives.
 *
 * RETURNS
 *	void
 */

static void hash_ifndef _((expr_ty *));

static void
hash_ifndef(ep)
	expr_ty		*ep;
{
	expr_ty		*e1;
	expr_ty		*e2;
	expr_ty		*e3;
	string_ty	*s;

	trace(("hash_ifndef(ep = %08lX)\n{\n"/*}*/, ep));
	s = str_from_c("defined");
	e1 = expr_constant_new(s, curpos());
	str_free(s);
	e2 = expr_function_new2(e1, ep);
	expr_delete(e1);

	s = str_from_c("not");
	e1 = expr_constant_new(s, curpos());
	e3 = expr_function_new2(e1, e2);
	expr_delete(e1);
	expr_delete(e2);

	hash_if(e3);
	expr_delete(e3);
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	hash_elif - process #elif directive
 *
 * SYNOPSIS
 *	void hash_elif(expr_ty*);
 *
 * DESCRIPTION
 *	The hash_elif function is used to provess #elif directives.
 *
 * RETURNS
 *	void
 */

static void hash_elif _((expr_ty *));

static void
hash_elif(ep)
	expr_ty		*ep;
{
	trace(("hash_elif(ep = %08lX)\n{\n"/*}*/, ep));
	if (!stack)
		yyerror("#elif without matching #if");
	else
	{
		switch (stack->state)
		{
		case 1:
			stack->pass = 0;
			stack->state = 1;
			lex_passing(0);
			break;

		case 2:
			switch (expr_eval_condition(ep, 0))
			{
			case -1:
				yyerror("expression evaluation failed");
				/* fall through... */

			case 0:
				stack->pass = 0;
				stack->state = 2;
				lex_passing(0);
				break;

			default:
				stack->pass = 1;
				stack->state = 1;
				lex_passing(1);
				break;
			}
			break;

		case 3:
			stack->pass = 0;
			stack->state = 3;
			yyerror("#elif after #else");
			lex_passing(0);
			break;
		}
	}
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	hash_else - process #else directive
 *
 * SYNOPSIS
 *	void hash_else(void);
 *
 * DESCRIPTION
 *	The hash_else function is used to process #else directives.
 *
 * RETURNS
 *	void
 */

static void hash_else _((void));

static void
hash_else()
{
	trace(("hash_else()\n{\n"/*}*/));
	if (!stack)
		yyerror("#else without matching #if");
	else
	{
		switch (stack->state)
		{
		case 1:
			stack->pass = 0;
			stack->state = 3;
			lex_passing(0);
			break;

		case 2:
			stack->pass = 1;
			stack->state = 3;
			lex_passing(1);
			break;

		case 3:
			stack->pass = 0;
			stack->state = 3;
			yyerror("#else after #else");
			lex_passing(0);
			break;
		}
	}
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	hash_endif - process #endif directive
 *
 * SYNOPSIS
 *	void hash_endif(void);
 *
 * DESCRIPTION
 *	The hash_endif function is used to process #endif directives.
 *
 * RETURNS
 *	void
 */

static void hash_endif _((void));

static void
hash_endif()
{
	trace(("hash_endif()\n{\n"/*}*/));
	if (!stack)
		yyerror("#endif without matching #if");
	else
	{
		cond	*c;

		c = stack;
		stack = c->next;
		cond_free(c);
		lex_passing(stack ? stack->pass : 1);
	}
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	hash_line - process #line directive
 *
 * SYNOPSIS
 *	void hash_line(expr_list_ty *elp);
 *
 * DESCRIPTION
 *	The hash_line function is used to process #line directives.
 *
 * RETURNS
 *	void
 */

static void hash_line _((expr_list_ty *));

static void
hash_line(elp)
	expr_list_ty	*elp;
{
	string_list_ty	*result;

	trace(("hash_line(elp = %08lX)\n{\n"/*}*/, elp));
	if (stack && !stack->pass)
		goto ret;

	/*
	 * evaluate the expressions
	 */
	result = expr_list_evaluate(elp, 0);
	if (!result)
	{
		hashline_error("expression evaluation failed");
		goto ret;
	}

	switch (result->nstrings)
	{
	case 1:
		lex_lino_set(result->string[0], (string_ty *)0);
		break;

	case 2:
		lex_lino_set(result->string[0], result->string[1]);
		break;

	default:
		yyerror("#line needs positive decimal line number");
		break;
	}
	string_list_delete(result);
	ret:
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	hash_pragma - process #pragma directive
 *
 * SYNOPSIS
 *	void hash_pragma(expr_list_ty *elp);
 *
 * DESCRIPTION
 *	The hash_pragma function is used to process #pragma directives.
 *
 * RETURNS
 *	void
 */

static void hash_pragma _((expr_list_ty *));

static void
hash_pragma(elp)
	expr_list_ty	*elp;
{
	static expr_ty *once;

	trace(("hash_pragma(elp = %08lX)\n{\n"/*}*/, elp));
	if (stack && !stack->pass)
		goto ret;

	/*
	 * see if it was "#pragma once"
	 */
	if (!once)
	{
		string_ty	*s;

		s = str_from_c("once");
		once = expr_constant_new(s, curpos());
		str_free(s);
	}
	if
	(
		elp->el_nexprs == 1
	&&
		expr_equal(elp->el_expr[0], once)
	)
	{
		string_list_append_unique(&done_once, lex_cur_file());
		goto ret;
	}

	/*
	 * add more pragma's here
	 */

	ret:
	trace((/*{*/"}\n"));
}

%}

%token	HASHLINE_CATENATE
%token	HASHLINE_LBRAK
%token	HASHLINE_RBRAK
%token	HASHLINE_WORD

%token	HASH_ELIF
%token	HASH_ELSE
%token	HASH_ENDIF
%token	HASH_IF
%token	HASH_IFDEF
%token	HASH_IFNDEF
%token	HASH_INCLUDE
%token	HASH_INCLUDE_COOKED
%token	HASH_INCLUDE_COOKED2
%token	HASH_LINE
%token	HASH_PRAGMA
%token	HASH_JUNK

%left	HASHLINE_CATENATE

%union
{
	expr_ty		*lv_expr;
	expr_list_ty	lv_elist;
	string_ty	*lv_word;
}

%type	<lv_elist>	elist
%type	<lv_word>	HASHLINE_WORD
%type	<lv_expr>	expr

%%

/*
 * note that the grammar accepts a single line.
 * this means that 0 (end-of-input) must be sent on end-of-line.
 */

hashline
	: HASH_INCLUDE elist
		{
			hash_include(&$2);
			expr_list_destructor(&$2);
		}
	| HASH_INCLUDE_COOKED elist
		{
			hash_include_cooked(&$2, 1);
			expr_list_destructor(&$2);
		}
	| HASH_INCLUDE_COOKED2 elist
		{
			hash_include_cooked(&$2, 0);
			expr_list_destructor(&$2);
		}
	| HASH_IF expr
		{
			hash_if($2);
			expr_delete($2);
		}
	| HASH_IFDEF expr
		{
			hash_ifdef($2);
			expr_delete($2);
		}
	| HASH_IFNDEF expr
		{
			hash_ifndef($2);
			expr_delete($2);
		}
	| HASH_ELIF expr
		{
			hash_elif($2);
			expr_delete($2);
		}
	| HASH_ELSE
		{
			hash_else();
		}
	| HASH_ENDIF
		{
			hash_endif();
		}
	| HASH_PRAGMA elist
		{
			hash_pragma(&$2);
			expr_list_destructor(&$2);
		}
	| HASH_LINE elist
		{
			hash_line(&$2);
			expr_list_destructor(&$2);
		}
	| error
	;

/*
 * this expression form is the same as in parse.y
 * except that the lbrak processing is not necessary.
 */

expr
	: HASHLINE_WORD
		{
			$$ = expr_constant_new($1, curpos());
			str_free($1);
		}
	| HASHLINE_LBRAK elist HASHLINE_RBRAK
		{
			$$ = expr_function_new(&$2);
			expr_list_destructor(&$2);
		}
	| expr HASHLINE_CATENATE expr
		{
			$$ = expr_catenate_new($1, $3);
			expr_delete($1);
			expr_delete($3);
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
