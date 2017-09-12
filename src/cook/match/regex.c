/*
 *	cook - file construction tool
 *	Copyright (C) 1999, 2001 Peter Miller;
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
 * MANIFEST: functions to manipulate regex pattern matching
 */

#include <ac/stddef.h>
#include <ac/string.h>
#include <ac/regex.h>

#include <error_intl.h>
#include <expr/position.h>
#include <match/private.h>
#include <match/regex.h>
#include <str.h>
#include <stracc.h>
#include <trace.h>


typedef struct match_regex_ty match_regex_ty;
struct match_regex_ty
{
	match_ty	inherited;

	/*
	 * This holds the compiled regular expression.
	 * It is internal to the regex implementation.
	 */
	regex_t		preg;

	/*
	 * The ``actual'' string points to the string which was matched.
	 * We have to keep it around, because the next field points into it.
	 */
	string_ty	*actual;

	/*
	 * The match field contains pointers into the ``actual'' string of
	 * the various matching items (well, indexes actually).
	 */
	regmatch_t	match[10];
};


static void report_regex_error _((const expr_position_ty *, int, regex_t *,
	string_ty *));

static void
report_regex_error(pp, err, preg, formal)
	const expr_position_ty *pp;
	int		err;
	regex_t		*preg;
	string_ty	*formal;
{
	sub_context_ty	*scp;
	char		buffer[200];

	regerror(err, preg, buffer, sizeof(buffer));

	scp = sub_context_new();
	sub_var_set(scp, "Pattern", "%S", formal);
	sub_var_set(scp, "MeSsaGe", "%s", buffer);
	error_with_position
	(
		pp,
		scp,
		i18n("pattern \"$pattern\" error: $message")
	);
	sub_context_delete(scp);
}


static void destructor _((match_ty *));

static void
destructor(mp)
	match_ty	*mp;
{
	match_regex_ty	*this;

	trace(("match_regex::destructor(mp = %08X)\n{\n"/*}*/, mp));
	this = (match_regex_ty *)mp;
	if (this->actual)
		str_free(this->actual);
	regfree(&this->preg);
	trace((/*{*/"}\n"));
}


static void constructor _((match_ty *));

static void
constructor(mp)
	match_ty	*mp;
{
	match_regex_ty	*this;

	trace(("match_regex::constructor(mp = %08X)\n{\n"/*}*/, mp));
	this = (match_regex_ty *)mp;
	this->actual = 0;
	/* this is not 100% portable */
	memset(&this->preg, 0, sizeof(this->preg));
	trace((/*{*/"}\n"));
}


static int compile _((match_ty *, string_ty *, const expr_position_ty *));

static int
compile(mp, formal, pp)
	match_ty	*mp;
	string_ty	*formal;
	const expr_position_ty *pp;
{
	match_regex_ty	*this;
	int		result;
	int		err;
	long		formal_start;
	long		formal_end;

	trace(("match_regex::complie(mp = %08lX, formal = %08lX)\n{\n",
		(long)mp, (long)formal));
	trace(("formal = \"%s\";\n", formal->str_text));
	this = (match_regex_ty *)mp;
	result = -1;

	/*
	 * Release resources held by previous compiles.
	 */
	regfree(&this->preg);

	/*
	 * Work formal over so that it has ^ at the beginning, and $
	 * at the end.	This ensures that the whole `actual' is matches,
	 * not just the bits the pattern likes.  I would much prefer a
	 * flag to regcomp (so that patterns with ``|'' work prperly,
	 * for instance) , but there isn't one.  Sigh.
	 */
	for
	(
		formal_start = 0;
		(
			formal_start < formal->str_length
		&&
			formal->str_text[formal_start] == '^'
		);
		++formal_start
	)
		;
	for
	(
		formal_end = formal->str_length;
		(
			formal_end >= 0
		&&
			formal->str_text[formal_end - 1] == '$'
		);
		--formal_end
	)
		;
	formal =
		str_format
		(
			"^%.*s$",
			(int)(formal_end - formal_start),
			formal->str_text + formal_start
		);

	/*
	 * compile the regular expression
	 */
	err = regcomp(&this->preg, formal->str_text, REG_BASIC);
	if (err != 0)
	{
		report_regex_error(pp, err, &this->preg, formal);
		result = -1;
	}
	else
		result = 0;

	/*
	 * release our worked-over formal expression
	 */
	str_free(formal);

	/*
	 * return result
	 */
	trace(("return %d;\n", result));
	trace(("}\n"));
	return result;
}


static int execute _((match_ty *, string_ty *, const expr_position_ty *));

static int
execute(mp, actual, pp)
	match_ty	*mp;
	string_ty	*actual;
	const expr_position_ty *pp;
{
	match_regex_ty	*this;
	int		result;
	int		err;

	trace(("match_regex::execute(mp = %08lX, actual = %08lX)\n{\n",
		(long)mp, (long)actual));
	trace(("actual = \"%s\";\n", actual->str_text));
	this = (match_regex_ty *)mp;
	result = -1;

	/*
	 * Release resources held by previous executes.
	 */
	if (this->actual)
	{
		str_free(this->actual);
		this->actual = 0;
	}

	/*
	 * execute the regular expression
	 */
	err =
		regexec
		(
			&this->preg,
			actual->str_text,
			SIZEOF(this->match),
			this->match,
			0
		);
	switch (err)
	{
	case 0:
		this->actual = str_copy(actual);
		result = 1;
		break;

	case REG_NOMATCH:
		result = 0;
		break;

	default:
		report_regex_error(pp, err, &this->preg, actual);
		result = -1;
		break;
	}

	trace(("return %d;\n", result));
	trace(("}\n"));
	return result;
}


static void illegal_sub_expr _((const expr_position_ty *, string_ty *, int));

static void
illegal_sub_expr(pp, s, why)
	const expr_position_ty *pp;
	string_ty	*s;
	int		why;
{
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set(scp, "Name", "\\%d", why);
	sub_var_set(scp, "Pattern", "%S", s);
	error_with_position
	(
		pp,
		scp,
		i18n("illegal use of '$name' in \"$pattern\" pattern")
	);
	sub_context_delete(scp);
}


static string_ty *reconstruct_lhs _((const match_ty *, string_ty *,
	const expr_position_ty *));

static string_ty *
reconstruct_lhs(mp, lhs, pp)
	const match_ty	*mp;
	string_ty	*lhs;
	const expr_position_ty *pp;
{
	const match_regex_ty *this;
	static stracc	buffer;
	int		depth;
	int		n;
	char		*cp;
	char		*ip;
	string_ty	*s;
	int		j;

	trace(("match_regex::reconstruct_lhs(mp = %08lX, lhs = %08X)\n{\n",
		(long)mp, (long)lhs));
	this = (const match_regex_ty *)mp;
	trace_string(lhs->str_text);

	assert(this->actual);
	if (!this->actual)
	{
		/* this_is_a_bug(); */
		trace(("}\n"));
		return 0;
	}
	ip = this->actual->str_text;

	sa_open(&buffer);

	depth = 0;
	n = 0;
	for (cp = lhs->str_text; *cp; ++cp)
	{
		const regmatch_t *rm;

		switch (*cp)
		{
		default:
			if (depth == 0)
				sa_char(&buffer, *cp);
			break;

		case '\\':
			switch (*++cp)
			{
			default:
				if (depth == 0)
					sa_char(&buffer, *cp);
				break;

			case '(':
				++depth;
				++n;
				if (depth == 1)
				{
					rm = &this->match[n];
					if (rm->rm_so < 0)
					{
						illegal_sub_expr(pp, lhs, n);
						trace(("return NULL;\n"));
						trace(("}\n"));
						return 0;
					}
					sa_chars
					(
						&buffer,
						ip + rm->rm_so,
						rm->rm_eo - rm->rm_so
					);
				}
				break;

			case ')':
				if (depth > 0)
					--depth;
				break;

			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				if (depth == 0)
				{
					j = *cp - '0';
					rm = &this->match[j];
					if (rm->rm_so < 0)
					{
						illegal_sub_expr(pp, lhs, j);
						trace(("return NULL;\n"));
						trace(("}\n"));
						return 0;
					}
					sa_chars
					(
						&buffer,
						ip + rm->rm_so,
						rm->rm_eo - rm->rm_so
					);
				}
				break;

			case 0:
				--cp;
				break;
			}
			break;
		}
	}

	s = sa_close(&buffer);
	trace_string(s->str_text);
	trace(("return %08lX;\n", s));
	trace(("}\n"));
	return s;
}


static string_ty *reconstruct_rhs _((const match_ty *, string_ty *,
	const expr_position_ty *));

static string_ty *
reconstruct_rhs(mp, rhs, pp)
	const match_ty	*mp;
	string_ty	*rhs;
	const expr_position_ty *pp;
{
	const match_regex_ty *this;
	static stracc	buffer;
	char		*cp;
	char		*ip;
	string_ty	*s;
	int		j;

	trace(("match_regex::reconstruct_rhs(mp = %08lX, rhs = %08X)\n{\n",
		(long)mp, (long)rhs));
	this = (const match_regex_ty *)mp;
	trace_string(rhs->str_text);
	assert(this->actual);
	if (!this->actual)
	{
		/* this_is_a_bug(); */
		trace(("}\n"));
		return 0;
	}
	ip = this->actual->str_text;

	sa_open(&buffer);

	/*
	 * replace the matched portion with the right hand side
	 */
	for (cp = rhs->str_text; *cp; ++cp)
	{
		const regmatch_t *rm;

		switch (*cp)
		{
		default:
			sa_char(&buffer, *cp);
			break;

		case '&':
			rm = &this->match[0];
			sa_chars
			(
				&buffer,
				ip + rm->rm_so,
				rm->rm_eo - rm->rm_so
			);
			break;

		case '\\':
			switch (*++cp)
			{
			default:
				sa_char(&buffer, *cp);
				break;

			case '0': case '1': case '2': case '3':
			case '4': case '5': case '6': case '7':
			case '8': case '9':
				j = *cp - '0';
				rm = &this->match[j];
				if (rm->rm_so < 0)
				{
					illegal_sub_expr(pp, rhs, j);
					trace(("return NULL;\n"));
					trace(("}\n"));
					return 0;
				}
				sa_chars
				(
					&buffer,
					ip + rm->rm_so,
					rm->rm_eo - rm->rm_so
				);
				break;

			case 0:
				--cp;
				break;
			}
			break;
		}
	}

	/*
	 * Build the result and return.
	 */
	s = sa_close(&buffer);
	trace_string(s->str_text);
	trace(("return %08lX;\n", s));
	trace(("}\n"));
	return s;
}


static int usage_mask _((const match_ty *, string_ty *,
	const expr_position_ty *));

static int
usage_mask(mp, s, pp)
	const match_ty	*mp;
	string_ty	*s;
	const expr_position_ty *pp;
{
	int		n;
	const char	*cp;

	/*
	 * This is just plain horrible.  We are trying to see if
	 * we have been given a regex pattern *and* it has one or
	 * more sub-expressions.  (One that has some problems is OK,
	 * because the real regcomp will (hopefully) report them.)
	 * Since there is no nice way to ask POSIX regex this question,
	 * this less-than-perfect method is used.
	 *
	 * Unfortunately, different rx implementations give different
	 * re_nsub results for the same string (literal strings give 0
	 * on some, and 1 on others) so it can't be used.
	 */
	n = 0;
	for (cp = s->str_text; *cp; ++cp)
	{
		if (cp[0] == '\\' && cp[1] == '(')
		{
			++n;
			++cp;
		}
	}
	return (n ? ((1 << n) - 1) : 0);
}


static match_method_ty vtbl =
{
	"regex",
	sizeof(match_regex_ty),
	destructor,
	constructor,
	compile,
	execute,
	reconstruct_lhs,
	reconstruct_rhs,
	usage_mask,
};


match_ty *
match_regex_new()
{
	return match_private_new(&vtbl);
}
