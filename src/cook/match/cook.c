/*
 *	cook - file construction tool
 *	Copyright (C) 1999, 2000, 2001 Peter Miller;
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
 * MANIFEST: functions to manipulate cook native matching
 *
 * This is in the inner loop, so it must perform well.
 * A free list of match structures is maintained to avoid malloc calls;
 * malloc is only called when this free list is empty.
 *
 * The tough part about designing a pattern matcher for something like cook is
 * that the patterns must be reversible.  That is, it must be possible to use
 * the same string both as a pattern to be matched against and as a template
 * for building a string once a pattern has matched.  Rather like the
 * difference between the left and right sides of an editor search-and-replace
 * command using the same description for both the search pattern and the
 * replace template.  This is why classic regular expressions have not been
 * used.  They tend to be slow to match, too.
 *
 * This matcher has eleven match "fields", referenced as % and %0 to %9.
 * The % character can be escaped as %%.  The % and %1 to %9 forms match any
 * character except '/'.  The %0 form matches all characters, but must be
 * either empty, or have whole path components, including the trailing '/' on
 * each component.  A few examples will make this clearer:
 *	"%.c" matches "fred.c" with %="fred"
 *	"%.c" failes to match "snot/fred.c"
 *	"%1/%2.c"matches "snot/fred.c" with %1="snot" and %2="fred"
 *	"%1/%2.c" fails to match "etc/boo/fred.c"
 *	"%0%5.c" matches "fred.c" with %0="" and %5="fred"
 *	"%0%6.c" matches "snot/fred.c" with %0="snot/" and %6="fred"
 *	"%0%7.c" matches "etc/boo/fred.c" with %0="etc/boo/" and %7="fred"
 *	"/usr/%1/%1%2/%3.%2%4" matches "/usr/man/man1/fred.1x" with %1="man",
 *		%2="1", %3="fred" and %4="x".
 * The %0 behaviour is designed to allow patterns to range over subtrees in a
 * controlled manner.  Note that the use of this sort of pattern in a recipe
 * will result in deeper seraches than the naive recipe designer would expect.
 */

#include <ac/ctype.h>
#include <ac/string.h>

#include <error.h> /* for assert */
#include <error_intl.h>
#include <expr/position.h>
#include <match/cook.h>
#include <match/private.h>
#include <str.h>
#include <stracc.h>
#include <trace.h>

/*
 * Define this symbol if you want the original strict interpretation of
 * the %0 patter.  I.e. it may only apprae at the start of the pattern,
 * or immedately after a slash (/) synbol.  Relaxing this restruiction
 * does not change the semantics of any cookbooks.  However, it means
 * some errors will not be detected - except that most users didn't think
 * they were errors.  E.g.
 * 	[match_mask -I%0% [CFLAGS]]
 *
#define STRICT_PERCENT_0_USAGE
 */


#ifdef DEBUG
enum magic_t
{
	MAGIC_CONSTRUCTED = 4321,
	MAGIC_COMPILED,
	MAGIC_EXECUTED,
	MAGIC_RECONSTRUCTED,
	MAGIC_DESTROYED
};
typedef enum magic_t magic_t;
#endif /* DEBUG */

typedef struct match_cook_ty match_cook_ty;
struct match_cook_ty
{
	match_ty	inherited;
	string_ty	*formal;
	string_ty	*fill[11];
#ifdef DEBUG
	magic_t		magic;
#endif /* DEBUG */
};


#define MATCH_CHAR '%'


/*
 * NAME
 *	illegal_pattern - complain
 *
 * SYNOPSIS
 *	void illegal_pattern(char *s);
 *
 * DESCRIPTION
 *	The illegal_pattern function is used to complain about errors in
 *	pattern secifications.
 *
 * RETURNS
 *	void
 */

static void illegal_pattern _((const expr_position_ty *, string_ty *, int));

static void
illegal_pattern(pp, s, why)
	const expr_position_ty *pp;
	string_ty	*s;
	int		why;
{
	sub_context_ty	*scp;

	if (why < 0)
	{
		scp = sub_context_new();
		sub_var_set(scp, "Name", "%c0", MATCH_CHAR);
		sub_var_set(scp, "Pattern", "%S", s);
		error_with_position
		(
			pp,
			scp,
		     i18n("illegal position of '$name' in \"$pattern\" pattern")
		);
		sub_context_delete(scp);
		return;
	}
	scp = sub_context_new();
	if (why >= 10)
		sub_var_set(scp, "Name", "%c", MATCH_CHAR);
	else
		sub_var_set(scp, "Name", "%c%d", MATCH_CHAR, why);
	sub_var_set(scp, "Pattern", "%S", s);
	error_with_position
	(
		pp,
		scp,
		i18n("illegal use of '$name' in \"$pattern\" pattern")
	);
	sub_context_delete(scp);
}


#ifdef DEBUG

static void check_magic _((match_cook_ty *, int, magic_t));

static void
check_magic(this, line, new_state)
	match_cook_ty	*this;
	int		line;
	magic_t		new_state;
{
	switch (this->magic)
	{
	default:
		fatal_raw("%s: %d: operating on wrong kind of object",
			__FILE__, line);

	case MAGIC_DESTROYED:
		fatal_raw("%s: %d: operating on destroyed object",
			__FILE__, line);

	case MAGIC_CONSTRUCTED:
	case MAGIC_COMPILED:
	case MAGIC_EXECUTED:
	case MAGIC_RECONSTRUCTED:
		break;
	}
	switch (new_state)
	{
	case MAGIC_CONSTRUCTED:
		fatal_raw("%s: %d: can't happen", __FILE__, line);
		break;

	case MAGIC_DESTROYED:
		break;

	case MAGIC_COMPILED:
#if 0
		if (this->magic != MAGIC_CONSTRUCTED)
			fatal_raw("%s: %d: may only compile once",
				__FILE__, line);
		}
#endif
		break;

	case MAGIC_EXECUTED:
		if (this->magic < MAGIC_COMPILED)
		{
			fatal_raw("%s: %d: executing when uncompiled",
				__FILE__, line);
		}
		break;

	case MAGIC_RECONSTRUCTED:
		if (this->magic < MAGIC_EXECUTED)
		{
			fatal_raw("%s: %d: reconstructing when unexecuted",
				__FILE__, line);
		}
		break;
	}
	this->magic = new_state;
}

#endif /* DEBUG */



/*
 * NAME
 *	match_free - dispose of match structure
 *
 * SYNOPSIS
 *	void match_free(match_ty *);
 *
 * DESCRIPTION
 *	The match_free function is used to dispose of a match structure
 *	allocated by the match_alloc function.
 *
 * RETURNS
 *	void
 */

static void destructor _((match_ty *));

static void
destructor(mp)
	match_ty	*mp;
{
	match_cook_ty	*this;
	int		j;

	trace(("match_cook::destructor(mp = %08X)\n{\n"/*}*/, mp));
	this = (match_cook_ty *)mp;
#ifdef DEBUG
	check_magic(this, __LINE__, MAGIC_DESTROYED);
#endif
	if (this->formal)
	{
		str_free(this->formal);
		this->formal = 0;
	}
	for (j = 0; j < SIZEOF(this->fill); ++j)
	{
		if (this->fill[j])
		{
			str_free(this->fill[j]);
			this->fill[j] = 0;
		}
	}
	trace((/*{*/"}\n"));
}


static void constructor _((match_ty *));

static void
constructor(mp)
	match_ty	*mp;
{
	match_cook_ty	*this;
	int		j;

	trace(("match_cook::constructor(mp = %08X)\n{\n"/*}*/, mp));
	this = (match_cook_ty *)mp;
	this->formal = 0;
	for (j = 0; j < SIZEOF(this->fill); ++j)
		this->fill[j] = 0;
#ifdef DEBUG
	this->magic = MAGIC_CONSTRUCTED;
#endif /* DEBUG */
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	attempt - match pattern to string
 *
 * SYNOPSIS
 *	int attempt(char *original_patn, char *patn, char *str,
 *		match_ty *field);
 *
 * DESCRIPTION
 *	The attempt function is used to match up a pattern with a string,
 *	filling in the fields as it goes.
 *
 * RETURNS
 *	int: zero if does not match, nonzero if does match.
 *		-1 on error
 *
 * CAVEAT
 *	The field structure is not allocated here.
 */

static int attempt_inner _((match_cook_ty *, char *, char *, char *, char *,
	const expr_position_ty *));

static int
attempt_inner(this, formal_begin, formal_end, actual_begin, actual_end, pp)
	match_cook_ty	*this;
	char		*formal_begin;
	char		*formal_end;
	char		*actual_begin;
	char		*actual_end;
	const expr_position_ty *pp;
{
	size_t		idx;
	string_ty	*sp;
	int		result;
	char		*q;
	int		sub_result;

	trace(("attempt_inner(this = %08lX, formal_begin = %08lX, \
actual_begin = %08lX)\n{\n",
		(long)this, (long)formal_begin, (long)actual_begin));
	trace(("formal = \"%.*s\";\n", (int)(formal_end - formal_begin),
		formal_begin));
	trace(("actual = \"%.*s\";\n", (int)(actual_end - actual_begin),
		actual_begin));

	/*
	 * Rip any matching constant string off the end of the formal
	 * and actual strings.  Nice, easy rejections here, and they lay
	 * the foundations for an optimization inside the main loop,
	 * avoiding a recursion.
	 */
	for (;;)
	{
		/*
		 * If we have run out of formal string, we must also run
		 * out of actual string.
		 */
		assert(formal_begin <= formal_end);
		if (formal_begin >= formal_end)
		{
			result = (actual_begin >= actual_end);
			goto ret;
		}

		/*
		 * If the last character could possibly be part of a
		 * matching sequence, stop.  We can't actually tell,
		 * because the matching sequence is only meaningful
		 * left-to-right.
		 *
		 * Examples of right-to-left broken-ness: %%0, %%%
		 */
		if (formal_end[-1] == MATCH_CHAR)
			break;
		if
		(
			formal_begin + 1 < formal_end
		&&
			formal_end[-2] == MATCH_CHAR
		&&
			isdigit((unsigned char)formal_end[-1])
		)
			break;

		/*
		 * We are looking at a constant.
		 * See if it matches.
		 */
		if
		(
			actual_begin >= actual_end
		||
			actual_end[-1] != formal_end[-1]
		)
		{
			result = 0;
			goto ret;
		}
		--formal_end;
		--actual_end;
	}
	trace(("formal = \"%.*s\";\n",
		(int)(formal_end - formal_begin), formal_begin));
	trace(("actual = \"%.*s\";\n",
		(int)(actual_end - actual_begin), actual_begin));

	for (;;)
	{
		/*
		 * Take care of the end of the string
		 */
		if (formal_begin >= formal_end)
		{
			result = (actual_begin >= actual_end);
			goto ret;
		}

		/*
		 * Take care of literal characters
		 */
		if (*formal_begin != MATCH_CHAR)
		{
			if
			(
				actual_begin >= actual_end
			||
				*formal_begin++ != *actual_begin++
			)
			{
				result = 0;
				goto ret;
			}
			continue;
		}

		/*
		 * take care of quoted match character
		 */
		if
		(
			formal_begin + 1 <= formal_end
		&&
			formal_begin[1] == MATCH_CHAR
		)
		{
			if
			(
				actual_begin >= actual_end
			||
				*actual_begin++ != MATCH_CHAR
			)
			{
				result = 0;
				goto ret;
			}
			formal_begin += 2;
			continue;
		}

		/*
		 * The %0 pattern element matches zero or more directory
		 * pieces, including the trailing slashes.
		 */
		if
		(
			formal_begin + 1 <= formal_end
		&&
			formal_begin[1] == '0'
		)
		{
			char	*midpoint;

			formal_begin += 2;

			/*
			 * It must appear at the beginning of the
			 * pattern, or immediately following slashes.
			 * It may not appear before a slash.
			 */
#ifdef STRICT_PERCENT_0_USAGE
			if
			(
				(
					(
						formal_begin
					>
						this->formal->str_text + 2
					)
				&&
					formal_begin[-3] != '/'
				)
			||
				(
					formal_begin < formal_end
				&&
					*formal_begin == '/'
				)
			)
			{
				illegal_pattern(pp, this->formal, -1);
				result = -1;
				goto ret;
			}
#endif /* STRICT_PERCENT_0_USAGE */

			/*
			 * It could have been seen earlier, must be
			 * identical if so.
			 */
			/* this->mask |= 1; */
			sp = this->fill[0];
			if (sp)
			{
				if
				(
					(
						actual_begin + sp->str_length
					>
						actual_end
					)
				||
					(
						0
					!=
						memcmp
						(
							actual_begin,
							sp->str_text,
							sp->str_length
						)
					)
				)
				{
					result = 0;
					goto ret;
				}
				actual_begin += sp->str_length;
				continue;
			}

			/*
			 * Match the largest number of whole directory chunks.
			 */
			midpoint = actual_end;
			for (;;)
			{
				while (midpoint > actual_begin)
				{
					if (midpoint[-1] == '/')
						break;
					--midpoint;
				}

				this->fill[0] =
					str_n_from_c
					(
						actual_begin,
						midpoint - actual_begin
					);
				trace_string(this->fill[0]->str_text);
				sub_result =
					attempt_inner
					(
						this,
						formal_begin,
						formal_end,
						midpoint,
						actual_end,
						pp
					);
				if (sub_result < 0)
				{
					result = -1;
					goto ret;
				}
				if (sub_result)
				{
					result = 1;
					goto ret;
				}
				str_free(this->fill[0]);
				this->fill[0] = 0;
				--midpoint;
				if (midpoint <= actual_begin)
				{
					result = 0;
					goto ret;
				}
			}
		}

		/*
		 * figure idx
		 */
		if (isdigit((unsigned char)formal_begin[1]))
		{
			idx = formal_begin[1] - '0';
			formal_begin += 2;
		}
		else
		{
			idx = 10;
			++formal_begin;
		}
		/* this->mask |= 1 << idx; */

		/*
		 * see if the field is already set
		 * must be identical if so
		 */
		sp = this->fill[idx];
		if (sp)
		{
			if
			(
				actual_begin + sp->str_length > actual_end
			||
				(
					0
				!=
					memcmp
					(
						actual_begin,
						sp->str_text,
						sp->str_length
					)
				)
			)
			{
				result = 0;
				break;
			}
			actual_begin += sp->str_length;
			continue;
		}

		/*
		 * Fast special case.  This is very common, because we
		 * stripped the constants off the end.
		 */
		if (formal_begin >= formal_end)
		{
			if
			(
				memchr
				(
					actual_begin,
					'/',
					actual_end - actual_begin
				)
			)
			{
				result = 0;
				goto ret;
			}
			this->fill[idx] =
				str_n_from_c
				(
					actual_begin,
					actual_end - actual_begin
				);
			trace(("idx = %ld;\n", (long)idx));
			result = 1;
			break;
		}

		/*
		 * The normal % and %N sequences can't match a slash at
		 * all.  This allows for a quick reject, and short
		 * circuits some of the recursion alternatives.
		 */
		q = memchr(actual_begin, '/', actual_end - actual_begin);
		if (q)
		{
			if
			(
				formal_begin < formal_end
			&&
				!memchr
				(
					formal_begin,
					'/',
					formal_end - formal_begin
				)
			)
			{
				result = 0;
				break;
			}
		}
		else
			q = actual_end;
		/*
                 * Pattern elements are not allowed to be empty at the
                 * start of the pattern because it causes problem with
                 * false matches against absolute paths.
		 * If you change this to a simple ``>='' test 36 will fail,
		 * If you change this to a simple ``>'' test 196 will fail.
		 */
		while
		(
			formal_begin > this->formal->str_text + 2
		?
			q >= actual_begin
		:
			q > actual_begin
		)
		{
			this->fill[idx] =
				str_n_from_c(actual_begin, q - actual_begin);
			trace(("idx = %ld;\n", (long)idx));
			trace_string(this->fill[idx]->str_text);
			sub_result =
				attempt_inner
				(
					this,
					formal_begin,
					formal_end,
					q,
					actual_end,
					pp
				);
			if (sub_result < 0)
			{
				result = -1;
				goto ret;
			}
			if (sub_result)
			{
				result = 1;
				goto ret;
			}
			str_free(this->fill[idx]);
			this->fill[idx] = 0;
			--q;
		}
		result = 0;
		break;
	}
ret:
	trace(("return %d;\n", result));
	trace(("}\n"));
	return result;
}


/*
 * NAME
 *	match - attempt to
 *
 * SYNOPSIS
 *	match_ty *match(string_ty *pattern, string_ty *string);
 *
 * DESCRIPTION
 *	The match function is used to match a pattern with a string.
 *	The matching fields are filled in in the returned structure.
 *
 * RETURNS
 *	match_ty *: a pointer to a match structure in dynamic memory with the
 *	match fields set as appropriate.
 *
 *	A NULL pointer is returned if the string does not match the
 *	pattern.
 *
 *	The value MATCH_ERROR will be returned if it was not a valid
 *	pattern; the error message will have been printed already.
 *
 * CAVEAT
 *	The match structure should be released by calling match_free.,
 */

static int compile _((match_ty *, string_ty *, const expr_position_ty *));

static int
compile(mp, formal, pp)
	match_ty	*mp;
	string_ty	*formal;
	const expr_position_ty *pp;
{
	match_cook_ty	*this;
	int		j;
	int		result;

	trace(("match_cook::compile(mp = %08lX, formal = %08lX)\n{\n",
		(long)mp, (long)formal));
	trace(("formal = \"%s\";\n", formal->str_text));
	this = (match_cook_ty *)mp;
	result = 0;
#ifdef DEBUG
	check_magic(this, __LINE__, MAGIC_COMPILED);
#endif /* DEBUG */

	/*
	 * Reset the match buffers.
	 */
	if (this->formal)
		str_free(this->formal);
	for (j = 0; j < SIZEOF(this->fill); ++j)
	{
		if (this->fill[j])
		{
			str_free(this->fill[j]);
			this->fill[j] = 0;
		}
	}

	/*
	 * remember the pattern
	 */
	this->formal = str_copy(formal);

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
	match_cook_ty	*this;
	int		j;
	int		result;

	trace(("match_cook::execute(mp = %08lX, actual = %08lX)\n{\n",
		(long)mp, (long)actual));
	trace(("actual = \"%s\";\n", actual->str_text));
	this = (match_cook_ty *)mp;
#ifdef DEBUG
	check_magic(this, __LINE__, MAGIC_EXECUTED);
#endif /* DEBUG */

	/*
	 * Reset the match buffers.
	 */
	for (j = 0; j < SIZEOF(this->fill); ++j)
	{
		if (this->fill[j])
		{
			str_free(this->fill[j]);
			this->fill[j] = 0;
		}
	}

	/*
	 * The inner matcher can recurse on itself.
	 * Because it nibbles away at both ends at various times, it
	 * also has a different calling interface.
	 */
	result =
		attempt_inner
		(
			this,
			this->formal->str_text,
			this->formal->str_text + this->formal->str_length,
			actual->str_text,
			actual->str_text + actual->str_length,
			pp
		);
#ifdef DEBUG
	if (result <= 0)
		this->magic = MAGIC_COMPILED;
#endif /* DEBUG */
	trace(("return %d;\n", result));
	trace(("}\n"));
	return result;
}


/*
 * NAME
 *	reconstruct - make string from pattern
 *
 * SYNOPSIS
 *	string_ty *reconstruct(string_ty *pattern, match_ty *field);
 *
 * DESCRIPTION
 *	The reconstruct function is used to rebuild a string from a replacement
 *	pattern and the match field values.
 *
 * RETURNS
 *	string_ty *; pointer to the reconstructed string
 *		or NULL on error (the error will already have been rinted)
 */

static string_ty *reconstruct _((const match_ty *, string_ty *,
	const expr_position_ty *));

static string_ty *
reconstruct(mp, pattern, pp)
	const match_ty	*mp;
	string_ty	*pattern;
	const expr_position_ty *pp;
{
	const match_cook_ty *this;
	static stracc	buffer;
	char		*p;
	string_ty	*s;
	int		idx;

	trace(("match_cook::reconstruct(mp = %08lX, pattern = %08X)\n{\n",
		(long)mp, (long)pattern));
	this = (const match_cook_ty *)mp;
#ifdef DEBUG
	/* magic is mutable */
	check_magic((match_cook_ty *)this, __LINE__, MAGIC_RECONSTRUCTED);
#endif /* DEBUG */
	trace_string(pattern->str_text);

	sa_open(&buffer);
	for (p = pattern->str_text; *p; ++p)
	{
		if (*p == MATCH_CHAR)
		{
			if (p[1] == MATCH_CHAR)
			{
				sa_char(&buffer, MATCH_CHAR);
				++p;
				continue;
			}
			if (p[1] >= '0' && p[1] <= '9')
			{
				idx = p[1] - '0';
				++p;
			}
			else
				idx = 10;
			s = this->fill[idx];
			if (!s)
			{
				illegal_pattern(pp, pattern, idx);
				trace(("return NULL;\n"));
				trace(("}\n"));
				return 0;
			}
			sa_chars(&buffer, s->str_text, s->str_length);
		}
		else
			sa_char(&buffer, *p);
	}

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
	match_cook_ty	*this;
	char		*cp;
	int		result;

	this = (match_cook_ty *)mp;
	result = 0;
	for (cp = s->str_text; *cp; ++cp)
	{
		if (*cp != MATCH_CHAR)
			continue;
		switch (cp[1])
		{
		default:
			result |= (1 << 10);
			break;

		case MATCH_CHAR:
			++cp;
			break;

		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			result |= 1 << (*++cp - '0');
			break;
		}
	}
	return result;
}


static match_method_ty vtbl =
{
	"cook",
	sizeof(match_cook_ty),
	destructor,
	constructor,
	compile,
	execute,
	reconstruct, /* lhs */
	reconstruct, /* rhs */
	usage_mask,
};


/*
 * NAME
 *	match_alloc - allocate match structure
 *
 * SYNOPSIS
 *	match_ty *match_alloc(void);
 *
 * DESCRIPTION
 *	The match_alloc function is used to allocate a match structure.
 *	The returned structure will be all zeros.
 *
 * RETURNS
 *	match_ty * - a pointer to the match structure in dynamic memory
 *
 * CAVEAT
 *	When finished with it should be disposed of by calling the match_free
 *	function.
 */

match_ty *
match_cook_new()
{
	return match_private_new(&vtbl);
}
