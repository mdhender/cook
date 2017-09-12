/*
 *	cook - file construction tool
 *	Copyright (C) 1994, 1997, 1998 Peter Miller;
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
 * MANIFEST: functions to manipulate if statements
 */

#include <ac/ctype.h>
#include <ac/string.h>

#include <emit.h>
#include <stmt/if.h>
#include <trace.h>
#include <variable.h>

typedef struct stmt_if_ty stmt_if_ty;
struct stmt_if_ty
{
	STMT
	blob_list_ty	*condition;
	stmt_ty		*then_clause;
	stmt_ty		*else_clause;
};


static void destructor _((stmt_ty *));

static void
destructor(that)
	stmt_ty		*that;
{
	stmt_if_ty	*this;

	trace(("if::destructor()\n{\n"/*}*/));
	this = (stmt_if_ty *)that;
	blob_list_free(this->condition);
	stmt_free(this->then_clause);
	if (this->else_clause)
		stmt_free(this->else_clause);
	trace((/*{*/"}\n"));
}


static void emit _((stmt_ty *));

static void
emit(that)
	stmt_ty		*that;
{
	stmt_if_ty	*this;
	size_t		j;

	trace(("if::emit()\n{\n"/*}*/));
	this = (stmt_if_ty *)that;
	emit_line_number
	(
		this->condition->list[0]->line_number,
		this->condition->list[0]->file_name
	);
	emit_str("#if");
	for (j = 0; j < this->condition->length; ++j)
	{
		emit_char(' ');
		emit_string(this->condition->list[j]->text);
	}
	emit_char('\n');

	stmt_emit(this->then_clause);
	emit_bol();

	if (this->else_clause)
	{
		emit_str("#else\n");
		stmt_emit(this->else_clause);
		emit_bol();
	}
	emit_str("#endif\n");
	trace((/*{*/"}\n"));
}


static void regroup _((stmt_ty *));

static void
regroup(that)
	stmt_ty		*that;
{
	stmt_if_ty	*this;

	trace(("if::regroup()\n{\n"/*}*/));
	this = (stmt_if_ty *)that;
	stmt_regroup(this->then_clause);
	if (this->else_clause)
		stmt_regroup(this->else_clause);
	trace((/*{*/"}\n"));
}


static void sort _((stmt_ty *));

static void
sort(that)
	stmt_ty		*that;
{
	stmt_if_ty	*this;

	trace(("if::sort()\n{\n"/*}*/));
	this = (stmt_if_ty *)that;
	stmt_sort(this->then_clause);
	if (this->else_clause)
		stmt_sort(this->else_clause);
	trace((/*{*/"}\n"));
}


static stmt_method_ty method =
{
	sizeof(stmt_if_ty),
	"if",
	0, /* constructor */
	destructor,
	emit,
	regroup,
	sort,
};


static blob_list_ty *ifeq _((blob_list_ty *, string_list_ty *));

static blob_list_ty *
ifeq(blp, ref)
	blob_list_ty	*blp;
	string_list_ty	*ref;
{
	blob_ty		*arg;
	blob_ty		*bp;
	blob_list_ty	*result;
	string_ty	*s;
	string_ty	*s2;
	char		*cp;
	size_t		j;

	/*
	 * allocate the result list
	 */
	trace(("ifeq()\n{\n"/*}*/));
	result = blob_list_alloc();

	/*
	 * make sure we were given enough arguments
	 */
	if (blp->length < 2)
	{
		arg = blp->list[0];
		blob_list_append
		(
			result,
			blob_alloc
			(
				str_from_c("0"),
				arg->file_name,
				arg->line_number
			)
		);
		trace((/*{*/"}\n"));
		return result;
	}

	/*
	 * turn the list of arguments into a single string
	 */
	arg = blp->list[1];
	s = str_copy(blp->list[1]->text);
	for (j = 2; j < blp->length; ++j)
	{
		s2 = str_format("%S %S", s, blp->list[j]->text);
		str_free(s);
		s = s2;
	}
	bp = blob_alloc(s, arg->file_name, arg->line_number);

	/*
	 * rename the variables
	 * and reform to be a single string, again.
	 */
	variable_rename(bp, result, ref, VAREN_NO_QUOQUO);
	blob_free(bp);
	s = result->length ? str_copy(result->list[0]->text) : str_from_c("0");
	for (j = 1; j < result->length; ++j)
	{
		s2 = str_format("%S %S", s, result->list[j]->text);
		str_free(s);
		s = s2;
	}
	blob_list_free(result);

	/*
	 * construct the result
	 */
	result = blob_list_alloc();
	switch (s->str_text[0])
	{
	case '(':
		/*
		 * ifeq (xxx,yyy)
		 */
		if (s->str_length < 3)
			goto useless;
		cp = strchr(s->str_text, ',');
		if
		(
			cp == 0
		||
			s->str_text[s->str_length - 1] != ')'
		)
			goto useless;

		blob_list_append
		(
			result,
			blob_alloc
			(
				str_from_c("[in"),
				arg->file_name,
				arg->line_number
			)
		);

		s2 = str_n_from_c(s->str_text + 1, cp - s->str_text - 1);
		if (s2->str_length == 0)
			s2 = str_from_c("\"\"");
		bp = blob_alloc(s2, arg->file_name, arg->line_number);
		blob_list_append(result, bp);

		s2 = str_n_from_c(cp + 1, s->str_text + s->str_length - cp - 2);
		if (s2->str_length == 0)
			s2 = str_from_c("\"\"");
		bp = blob_alloc(s2, arg->file_name, arg->line_number);
		blob_list_append(result, bp);

		blob_list_append
		(
			result,
			blob_alloc
			(
				str_from_c("]"),
				arg->file_name,
				arg->line_number
			)
		);
		break;

	case '\'':
	case '"':
		/*
		 * ifeq "xxx" "yyy"
		 */
		if (s->str_length < 5)
			goto useless;
		cp = strchr(s->str_text + 1, s->str_text[0]);
		if
		(
			!cp
		||
			cp[1] != ' '
		||
			cp[2] != s->str_text[0]
		||
			s->str_text[s->str_length - 1] != s->str_text[0]
		)
			goto useless;

		blob_list_append
		(
			result,
			blob_alloc
			(
				str_from_c("[in"),
				arg->file_name,
				arg->line_number
			)
		);

		s2 = str_n_from_c(s->str_text + 1, cp - s->str_text - 1);
		if (s2->str_length == 0)
			s2 = str_from_c("\"\"");
		bp = blob_alloc(s2, arg->file_name, arg->line_number);
		blob_list_append(result, bp);

		s2 = str_n_from_c(cp + 3, s->str_text + s->str_length - cp - 4);
		if (s2->str_length == 0)
			s2 = str_from_c("\"\"");
		bp = blob_alloc(s2, arg->file_name, arg->line_number);
		blob_list_append(result, bp);

		blob_list_append
		(
			result,
			blob_alloc
			(
				str_from_c("]"),
				arg->file_name,
				arg->line_number
			)
		);
		break;

	default:
		/*
		 * We were given some useless thing, just rename the
		 * variables and copy it through.
		 */
		useless:
		bp = blob_alloc(str_copy(s), arg->file_name, arg->line_number);
		blob_list_append(result, bp);
		break;
	}
	str_free(s);
	trace((/*{*/"}\n"));
	return result;
}


static blob_list_ty *ifneq _((blob_list_ty *arg, string_list_ty *));

static blob_list_ty *
ifneq(blp, ref)
	blob_list_ty	*blp;
	string_list_ty	*ref;
{
	blob_ty		*arg;
	blob_list_ty	*result;
	blob_ty		*bp;

	arg = blp->list[0];
	result = ifeq(blp, ref);
	bp = blob_alloc(str_from_c("[not"), arg->file_name, arg->line_number);
	blob_list_prepend(result, bp);
	bp = blob_alloc(str_from_c("]"), arg->file_name, arg->line_number);
	blob_list_append(result, bp);
	return result;
}


static blob_list_ty *ifdef _((blob_list_ty *, string_list_ty *));

static blob_list_ty *
ifdef(blp, ref)
	blob_list_ty	*blp;
	string_list_ty	*ref;
{
	blob_ty		*bp;
	blob_list_ty	*result;
	size_t		j;

	bp = blp->list[0];
	result = blob_list_alloc();
	blob_list_append
	(
		result,
		blob_alloc
		(
			str_from_c("[defined"),
			bp->file_name,
			bp->line_number
		)
	);
	for (j = 1; j < blp->length; ++j)
		variable_rename(blp->list[j], result, ref, VAREN_QUOTE_SPACES);
	blob_list_append
	(
		result,
		blob_alloc
		(
			str_from_c("]"),
			bp->file_name,
			bp->line_number
		)
	);
	return result;
}


static blob_list_ty *ifndef _((blob_list_ty *, string_list_ty *));

static blob_list_ty *
ifndef(blp, ref)
	blob_list_ty	*blp;
	string_list_ty	*ref;
{
	blob_ty		*arg;
	blob_list_ty	*result;
	blob_ty		*bp;

	arg = blp->list[0];
	result = ifdef(blp, ref);
	bp = blob_alloc(str_from_c("[not"), arg->file_name, arg->line_number);
	blob_list_prepend(result, bp);
	bp = blob_alloc(str_from_c("]"), arg->file_name, arg->line_number);
	blob_list_append(result, bp);
	return result;
}


typedef struct table_ty table_ty;
struct table_ty
{
	char		*name;
	blob_list_ty	*(*rewrite)_((blob_list_ty *, string_list_ty *));
	string_ty	*fast;
};

static table_ty table[] =
{
	{ "ifeq",	ifeq,	},
	{ "ifneq",	ifneq,	},
	{ "ifdef",	ifdef,	},
	{ "ifndef",	ifndef,	},
};


stmt_ty *
stmt_if_alloc(condition, then_clause, else_clause)
	blob_list_ty	*condition;
	stmt_ty		*then_clause;
	stmt_ty		*else_clause;
{
	stmt_if_ty	*result;
	blob_list_ty	*c2;
	table_ty	*tp;

	trace(("stmt_if_alloc()\n{\n"/*}*/));
	result = (stmt_if_ty *)stmt_alloc(&method);

	assert(condition->length >= 1);
	for (tp = table; tp < ENDOF(table); ++tp)
	{
		if (!tp->fast)
			tp->fast = str_from_c(tp->name);
		if (str_equal(condition->list[0]->text, tp->fast))
			break;
	}
	assert(tp < ENDOF(table));
	if (tp >= ENDOF(table))
		tp = &table[0];
	c2 = tp->rewrite(condition, &result->ref);
	blob_list_free(condition);

	result->condition = c2;
	result->then_clause = then_clause;
	result->else_clause = else_clause;

	stmt_variable_merge((stmt_ty *)result, then_clause);
	if (else_clause)
		stmt_variable_merge((stmt_ty *)result, else_clause);
	trace((/*{*/"}\n"));
	return (stmt_ty *)result;
}
