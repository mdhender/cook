/*
 *	cook - file construction tool
 *	Copyright (C) 1997-1999, 2001, 2003 Peter Miller;
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
 * MANIFEST: functions to perform $ substitutions
 */

#include <ac/errno.h>
#include <ac/libintl.h>
#include <ac/limits.h>
#include <ac/string.h>
#include <ac/wctype.h>

#include <arglex.h>
#include <error.h>
#include <error_intl.h>
#include <language.h>
#include <mem.h>
#include <str.h>
#include <sub.h>
#include <sub/basename.h>
#include <sub/date.h>
#include <sub/dirname.h>
#include <sub/downcase.h>
#include <sub/errno.h>
#include <sub/expr.h>
#include <sub/ident.h>
#include <sub/length.h>
#include <sub/left.h>
#include <sub/plural.h>
#include <sub/private.h>
#include <sub/progname.h>
#include <sub/right.h>
#include <sub/upcase.h>
#include <sub/zero_pad.h>
#include <trace.h>
#include <wstr_list.h>


typedef wstring_ty *(*fp)_((sub_context_ty *, wstring_list_ty *));

typedef struct table_ty table_ty;
struct table_ty
{
	const char	*name;
	fp		func;
	int		resubstitute;
	wstring_ty	*value;
	int		must_be_used	:1;
	int		append_if_unused :1;
	int		override	:1;
};

typedef struct diversion_ty diversion_ty;
struct diversion_ty
{
	long		pos;
	wstring_ty	*text;
	diversion_ty	*prev;
	int		resubstitute;
};


typedef struct collect_ty collect_ty;
struct collect_ty
{
	size_t	pos;
	size_t	size;
	wchar_t	*buf;
};

/* typedef struct sub_context_ty sub_context_ty; */
struct sub_context_ty
{
	diversion_ty	*diversion;
	table_ty	*sub_var_list;
	size_t		sub_var_size;
	size_t		sub_var_pos;
	char		*suberr;
	int		errno_sequester;
};


static void collect_constructor _((collect_ty *));

static void
collect_constructor(cp)
	collect_ty	*cp;
{
	cp->buf = 0;
	cp->size = 0;
	cp->pos = 0;
}


static void collect_destructor _((collect_ty *));

static void
collect_destructor(cp)
	collect_ty	*cp;
{
	if (cp->buf)
		mem_free(cp->buf);
	cp->buf = 0;
	cp->size = 0;
	cp->pos = 0;
}


/*
 * NAME
 *	collect
 *
 * SYNOPSIS
 *	void collect(collect_ty *, int c);
 *
 * DESCRIPTION
 *	The collect function is used to accumulate a string
 *	one character at a time.  No size limit.
 *
 * ARGUMENTS
 *	c - the character being collected
 */

static void collect _((collect_ty *, wchar_t));

static void
collect(cp, c)
	collect_ty	*cp;
	wchar_t		c;
{
	if (cp->pos >= cp->size)
	{
		size_t	nbytes;

		cp->size += (1L << 10);
		nbytes = cp->size * sizeof(wchar_t);
		cp->buf = mem_change_size(cp->buf, nbytes);
	}
	cp->buf[cp->pos++] = c;
}


static void collect_n _((collect_ty *, wchar_t *, size_t));

static void
collect_n(cp, s, n)
	collect_ty	*cp;
	wchar_t		*s;
	size_t		n;
{
	while (n > 0)
	{
		collect(cp, *s++);
		--n;
	}
}


/*
 * NAME
 *	collect_end
 *
 * SYNOPSIS
 *	wstring_ty *collect_end(collect_ty *);
 *
 * DESCRIPTION
 *	The collect_end function is used to fetch the string
 *	accumulated with the collect function.
 *	The bufferer for the collect function is cleared.
 *
 * RETURNS
 *	wstring_ty *; pointer to the string in dynamic memory.
 */

static wstring_ty *collect_end _((collect_ty *));

static wstring_ty *
collect_end(cp)
	collect_ty	*cp;
{
	wstring_ty	*result;

	result = wstr_n_from_wc(cp->buf, cp->pos);
	cp->pos = 0;
	return result;
}


static void sub_context_constructor _((sub_context_ty *));

static void
sub_context_constructor(scp)
	sub_context_ty	*scp;
{
	trace(("sub_context_constructor()\n{\n"));
	scp->diversion = 0;
	scp->sub_var_list = 0;
	scp->sub_var_size = 0;
	scp->sub_var_pos = 0;
	scp->suberr = 0;
	scp->errno_sequester = 0;
	trace(("}\n"));
}


sub_context_ty *
sub_context_new()
{
	sub_context_ty	*scp;
	int		hang_on_a_second;

	hang_on_a_second = errno;
	scp = mem_alloc(sizeof(sub_context_ty));
	errno = hang_on_a_second;
	sub_context_constructor(scp);
	return scp;
}


static void sub_context_destructor _((sub_context_ty *));

static void
sub_context_destructor(scp)
	sub_context_ty	*scp;
{
	size_t		j;

	for (j = 0; j < scp->sub_var_pos; ++j)
		wstr_free(scp->sub_var_list[j].value);
	if (scp->sub_var_list)
		mem_free(scp->sub_var_list);

	scp->diversion = 0;
	scp->sub_var_list = 0;
	scp->sub_var_size = 0;
	scp->sub_var_pos = 0;
	scp->suberr = 0;
	scp->errno_sequester = 0;
}


void
sub_context_delete(scp)
	sub_context_ty	*scp;
{
	sub_context_destructor(scp);
	mem_free(scp);
}


void
sub_context_error_set(scp, s)
	sub_context_ty	*scp;
	char		*s;
{
	scp->suberr = s;
}



static table_ty	table[] =
{
	{ "Basename",			sub_basename,			},
	{ "DAte",			sub_date,			},
	{ "Dirname",			sub_dirname,			},
	{ "DownCase",			sub_downcase,			},
	{ "ERrno",			sub_errno,			},
	{ "EXpression",			sub_expression,			},
	/* File_Name							*/
	/* Guess							*/
	{ "IDentifier",			sub_identifier,			},
	{ "LEFt",			sub_left,			},
	{ "LENgth",			sub_length,			},
	/* Name								*/
	/* Number							*/
	{ "PLural",			sub_plural,			},
	{ "PROGname",			sub_progname,			},
	{ "RIght",			sub_right,			},
	{ "UpCase",			sub_upcase,			},
	/* Value							*/
	{ "Zero_Pad",			sub_zero_pad,			},
};


/*
 * NAME
 *	sub_diversion
 *
 * SYNOPSIS
 *	void sub_diversion(wstring_ty *s, int resub);
 *
 * DESCRIPTION
 *	The sub_diversion function is used to divert input
 *	to a string, until that string is exhausted.
 *	When the string is exhausted, input will resume
 *	from the previous string.
 *
 * ARGUMENTS
 *	s - string to take as input
 */

static void sub_diversion _((sub_context_ty *, wstring_ty *, int));

static void
sub_diversion(scp, s, resubstitute)
	sub_context_ty	*scp;
	wstring_ty	*s;
	int		resubstitute;
{
	diversion_ty	*dp;

	trace(("sub_diversion(s = %8.8lX, resub = %d)\n{\n"/*}*/,
		(long)s, resubstitute));
	dp = (diversion_ty *)mem_alloc(sizeof(diversion_ty));
	dp->text = wstr_copy(s);
	dp->pos = 0;
	dp->resubstitute = resubstitute;
	dp->prev = scp->diversion;
	scp->diversion = dp;
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	sub_diversion_close
 *
 * SYNOPSIS
 *	void sub_diversion_close(void);
 *
 * DESCRIPTION
 *	The sub_diversion_close function is used to
 *	release a diversion when it has been exhausted.
 */

static void sub_diversion_close _((sub_context_ty *));

static void
sub_diversion_close(scp)
	sub_context_ty	*scp;
{
	diversion_ty	*dp;

	trace(("sub_diversion_close()\n{\n"/*}*/));
	assert(scp->diversion);
	dp = scp->diversion;
	scp->diversion = dp->prev;
	wstr_free(dp->text);
	mem_free((char *)dp);
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	execute
 *
 * SYNOPSIS
 *	void execute(wstring_list_ty *args);
 *
 * DESCRIPTION
 *	The execute function is used to perform the substitution
 *	described by the argument list.
 *
 * ARGUMENTS
 *	args - the name and arguments of the substitution
 */

static void execute _((sub_context_ty *, wstring_list_ty *));

static void
execute(scp, arg)
	sub_context_ty	*scp;
	wstring_list_ty	*arg;
{
	string_ty	*cmd;
	wstring_ty	*s;
	table_ty	*hit[20];
	int		nhits;
	table_ty	*tp;
	long		j;

	trace(("execute(scp = %08lX, arg = %08lX)\n{\n"/*}*/,
		(long)scp, (long)arg));
	if (arg->nitems == 0)
	{
		sub_context_ty	*scp2;

		scp2 = sub_context_new();
		fatal_intl(scp2, i18n("empty $${} substitution"));
		/* NOTREACHED */
		sub_context_delete(scp2);
	}

	/*
	 * scan the variables
	 */
	nhits = 0;
	cmd = wstr_to_str(arg->item[0]);
	trace_string(cmd->str_text);
	for (j = 0; j < scp->sub_var_pos; ++j)
	{
		tp = &scp->sub_var_list[j];
		trace(("tp = %08lX\n", (long)tp));
		trace_string(tp->name);
		if (arglex_compare(tp->name, cmd->str_text))
		{
			if (nhits < SIZEOF(hit))
				hit[nhits++] = tp;
		}
	}

	/*
	 * scan the functions
	 */
	for (tp = table; tp < ENDOF(table); ++tp)
	{
		if (arglex_compare(tp->name, cmd->str_text))
		{
			if (tp->override)
				goto override;
			if (nhits < SIZEOF(hit))
				hit[nhits++] = tp;
		}
	}
	str_free(cmd);

	/*
	 * figure what to do
	 */
	switch (nhits)
	{
	case 0:
		scp->suberr = i18n("unknown substitution name");
		s = 0;
		break;

	case 1:
		tp = hit[0];
		override:
		if (tp->value)
		{
			/*
			 * flag that the variable has been used
			 */
			tp->must_be_used = 0;
			s = wstr_copy(tp->value);
		}
		else
		{
			wstr_free(arg->item[0]);
			arg->item[0] = wstr_from_c(tp->name);
			s = tp->func(scp, arg);
		}
		break;

	default:
		scp->suberr = i18n("ambiguous substitution name");
		s = 0;
		break;
	}

	/*
	 * deal with the result
	 */
	if (s)
	{
		sub_diversion(scp, s, tp->resubstitute);
		wstr_free(s);
	}
	else
	{
		wstring_ty	*s2;
		string_ty	*s3;
		sub_context_ty	*scp2;
		char		*the_error;

		assert(scp->suberr);
		s2 = wstring_list_to_wstring(arg, 0, arg->nitems, (char *)0);
		s3 = wstr_to_str(s2);
		wstr_free(s2);
		the_error = scp->suberr ? scp->suberr : "this is a bug";
		scp2 = sub_context_new();
		sub_var_set(scp2, "Name", "%S", s3);
		sub_var_set(scp2, "Message", "%s", gettext(the_error));
		fatal_intl
		(
			scp2,
			i18n("substitution $${$name} failed: $message")
		);
		/* NOTREACHED */
		sub_context_delete(scp2);
		str_free(s3);
	}
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	sub_getc_meta
 *
 * SYNOPSIS
 *	void sub_getc_meta(void);
 *
 * DESCRIPTION
 *	The sub_getc_meta function is used to get a character from
 *	the current input string.  When the current string is exhaused,
 *	the previous string is resumed.
 *
 * RETURNS
 *	int - the chacater, or NUL to indicate end of input
 */

static wchar_t sub_getc_meta _((sub_context_ty *));

static wchar_t
sub_getc_meta(scp)
	sub_context_ty	*scp;
{
	wchar_t		result;
	diversion_ty	*dp;

	trace(("sub_getc_meta()\n{\n"/*}*/));
	dp = scp->diversion;
	if (!dp)
		result = 0;
	else if (dp->pos >= dp->text->wstr_length)
		result = 0;
	else
		result = dp->text->wstr_text[dp->pos++];
#ifdef DEBUG
	if (iswprint(result) && result >= CHAR_MIN && result <= CHAR_MAX)
		trace(("return '%c';\n", (char)result));
	else
		trace(("return %4.4lX;\n", (long)result));
#endif
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_getc_meta_undo
 *
 * SYNOPSIS
 *	void sub_getc_meta_undo(int c);
 *
 * DESCRIPTION
 *	The sub_getc_meta_undo function is used to give back
 *	a character output by sub_getc_meta.
 *
 * ARGUMENTS
 *	c - character being given back
 *
 * CAVEAT
 *	Only push back what was read.
 */

static void sub_getc_meta_undo _((sub_context_ty *, wchar_t));

static void
sub_getc_meta_undo(scp, c)
	sub_context_ty	*scp;
	wchar_t		c;
{
	diversion_ty	*dp;

	trace(("sub_getc_meta_undo(%ld)\n{\n"/*}*/, (long)c));
#ifdef DEBUG
	if (iswprint(c) && c >= CHAR_MIN && c <= CHAR_MAX)
		trace(("c = '%c'\n", (char)c));
#endif
	dp = scp->diversion;
	assert(dp);
	if (!c)
	{
		assert(dp->pos == dp->text->wstr_length);
	}
	else
	{
		assert(dp->pos >= 1);
		dp->pos--;
		assert(c == dp->text->wstr_text[dp->pos]);
	}
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	dollar
 *
 * SYNOPSIS
 *	wchar_t dollar(void);
 *
 * DESCRIPTION
 *	The dollar function is used to perform dollar ($) substitutions.
 *	On entry, the $ is expected to have been consumed.
 *
 *	The substitution is usually achieved as a side-effect,
 *	by using the sub_diversion function.
 *
 * RETURNS
 *	wchar_t	a character to deliver as output,
 *		or NUL if none.
 */

static wchar_t sub_getc _((sub_context_ty *)); /* forward */

static wchar_t dollar _((sub_context_ty *));

static wchar_t
dollar(scp)
	sub_context_ty	*scp;
{
	wstring_list_ty	arg;
	int		result;
	wchar_t		c;
	wstring_ty	*s;
	wchar_t		quoted;
	collect_ty	tmp;

	trace(("dollar()\n{\n"/*}*/));
	collect_constructor(&tmp);
	wstring_list_zero(&arg);
	result = 0;
	c = sub_getc_meta(scp);
	switch (c)
	{
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		for (;;)
		{
			collect(&tmp, c);
			c = sub_getc_meta(scp);
			switch (c)
			{
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				continue;

			default:
				sub_getc_meta_undo(scp, c);
				break;
			}
			break;
		}
		s = collect_end(&tmp);
		trace(("push arg\n"));
		wstring_list_append(&arg, s);
		wstr_free(s);
		execute(scp, &arg);
		wstring_list_free(&arg);
		break;

	case 'a': case 'b': case 'c': case 'd': case 'e':
	case 'f': case 'g': case 'h': case 'i': case 'j':
	case 'k': case 'l': case 'm': case 'n': case 'o':
	case 'p': case 'q': case 'r': case 's': case 't':
	case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
	case 'A': case 'B': case 'C': case 'D': case 'E':
	case 'F': case 'G': case 'H': case 'I': case 'J':
	case 'K': case 'L': case 'M': case 'N': case 'O':
	case 'P': case 'Q': case 'R': case 'S': case 'T':
	case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
		for (;;)
		{
			collect(&tmp, c);
			c = sub_getc_meta(scp);
			switch (c)
			{
			case 'a': case 'b': case 'c': case 'd': case 'e':
			case 'f': case 'g': case 'h': case 'i': case 'j':
			case 'k': case 'l': case 'm': case 'n': case 'o':
			case 'p': case 'q': case 'r': case 's': case 't':
			case 'u': case 'v': case 'w': case 'x': case 'y':
			case 'z':
			case 'A': case 'B': case 'C': case 'D': case 'E':
			case 'F': case 'G': case 'H': case 'I': case 'J':
			case 'K': case 'L': case 'M': case 'N': case 'O':
			case 'P': case 'Q': case 'R': case 'S': case 'T':
			case 'U': case 'V': case 'W': case 'X': case 'Y':
			case 'Z':
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
			case '_': case '-':
				continue;

			default:
				sub_getc_meta_undo(scp, c);
				break;
			}
			break;
		}
		s = collect_end(&tmp);
		trace(("push arg\n"));
		wstring_list_append(&arg, s);
		wstr_free(s);
		execute(scp, &arg);
		wstring_list_free(&arg);
		break;

	case '{'/*}*/:
		c = sub_getc(scp);
		for (;;)
		{
			/*
			 * look for terminator
			 */
			if (c == /*{*/'}')
				break;

			/*
			 * watch out for unterminated substitutions
			 */
			if (!c)
			{
				sub_context_ty	*scp2;

				scp2 = sub_context_new();
				fatal_intl
				(
					scp2,
					i18n("unterminated $${} substitution")
				);
				/* NOTREACHED */
				sub_context_delete(scp2);
				break;
			}

			/*
			 * skip white space separating the arguments
			 */
			if (iswspace(c))
			{
				c = sub_getc(scp);
				continue;
			}

			/*
			 * collect the argument
			 *	any run of non-white-space characters
			 */
			quoted = 0;
			for (;;)
			{
				if (!c)
				{
					if (quoted)
					{
						sub_context_ty	*scp2;

						scp2 = sub_context_new();
						fatal_intl
						(
							scp2,
						i18n("unterminated $${} quotes")
						);
						/* NOTREACHED */
						sub_context_delete(scp2);
					}
					break;
				}
				if (!quoted && (iswspace(c) || c == /*{*/'}'))
					break;
				if (c == quoted)
				{
					assert(quoted);
					quoted = 0;
				}
				else if (c == '\'')
				{
					assert(!quoted);
					quoted = c;
				}
				else if (c == '\\')
				{
					c = sub_getc(scp);
					if (!c)
					{
						sub_context_ty	*scp2;

						scp2 = sub_context_new();
						fatal_intl
						(
							scp2,
					   i18n("unterminated $${} \\ sequence")
						);
						/* NOTREACHED */
						sub_context_delete(scp2);
					}
					collect(&tmp, c);
				}
				else
					collect(&tmp, c);
				c = sub_getc(scp);
			}
			s = collect_end(&tmp);
			trace(("push arg\n"));
			wstring_list_append(&arg, s);
			wstr_free(s);
		}
		execute(scp, &arg);
		wstring_list_free(&arg);
		break;

	case '$':
		result = '$';
		break;

	default:
		sub_getc_meta_undo(scp, c);
		result = '$';
		break;
	}
	collect_destructor(&tmp);
#ifdef DEBUG
	if (iswprint(result) && result >= CHAR_MIN && result <= CHAR_MAX)
		trace(("return '%c';\n", (char)result));
	else
		trace(("return %4.4lX;\n", (long)result));
#endif
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_getc
 *
 * SYNOPSIS
 *	wchar_t sub_getc(void);
 *
 * DESCRIPTION
 *	The sub_getc function is used to get a character from
 *	the substitution stream.  This is used both for the final output
 *	and for fetching arguments to dollar ($) substitutions.
 *
 * RETURNS
 *	wchar_t	a character from the stream,
 *		or NUL to indicate end of input.
 */

static wchar_t sub_getc _((sub_context_ty *));

static wchar_t
sub_getc(scp)
	sub_context_ty	*scp;
{
	wchar_t		c;

	trace(("sub_getc()\n{\n"/*}*/));
	for (;;)
	{
		c = sub_getc_meta(scp);
		switch (c)
		{
		default:
			break;

		case 0:
			if (!scp->diversion)
				break;
			sub_diversion_close(scp);
			continue;

		case '$':
			if (!scp->diversion->resubstitute)
				break;
			c = dollar(scp);
			if (!c)
				continue;
			break;
		}
		break;
	}
#ifdef DEBUG
	if (iswprint(c) && c >= CHAR_MIN && c <= CHAR_MAX)
		trace(("return '%c';\n", (char)c));
	else
		trace(("return %4.4lX;\n", (long)c));
#endif
	trace((/*{*/"}\n"));
	return c;
}


static wstring_ty *subst _((sub_context_ty *, wstring_ty *));

static wstring_ty *
subst(scp, s)
	sub_context_ty	*scp;
	wstring_ty	*s;
{
	collect_ty	buf;
	wchar_t		c;
	wstring_ty	*result;
	table_ty	*tp;
	table_ty	*the_end;
	int		error_count;

	trace(("subst(scp = %08lX, s = %8.8lX)\n{\n"/*}*/,
		(long)scp, (long)s));
	collect_constructor(&buf);
	sub_diversion(scp, s, 1);
	for (;;)
	{
		/*
		 * get the next character
		 */
		c = sub_getc(scp);
		if (!c)
			break;

		/*
		 * save the character
		 */
		collect(&buf, c);
	}

	/*
	 * find any unused variables marked "append if unused"
	 */
	the_end = scp->sub_var_list + scp->sub_var_pos;
	for (tp = scp->sub_var_list; tp < the_end; ++tp)
	{
		if (!tp->append_if_unused)
			continue;
		if (!tp->must_be_used)
			continue;
		assert(tp->value);

		/*
		 * flag that the variable has been used
		 */
		tp->must_be_used = 0;
		if (!tp->value->wstr_length)
			continue;

		/*
		 * append to the buffer, separated by a space
		 */
		collect(&buf, (wchar_t)' ');
		collect_n(&buf, tp->value->wstr_text, tp->value->wstr_length);
	}

	/*
	 * find any unused variables
	 * and complain about them
	 */
	error_count = 0;
	for (tp = scp->sub_var_list; tp < the_end; ++tp)
	{
		sub_context_ty	*scp2;
		string_ty	*tmp;

		if (!tp->must_be_used)
			continue;

		/*
		 * Make sure the variables of this message are optional,
		 * to avoid infinite loops if there is a mistake in the
		 * translation string.
		 */
		scp2 = sub_context_new();
		tmp = wstr_to_str(s);
		sub_var_set(scp2, "Message", "%S", tmp);
		sub_var_optional(scp2, "Message");
		sub_var_set(scp2, "Name", "$%s", tp->name);
		sub_var_optional(scp2, "Name");
		error_intl
		(
			scp2,
		  i18n("in substitution \"$message\" variable \"$name\" unused")
		);
		sub_context_delete(scp2);
		str_free(tmp);
		++error_count;
	}
	if (error_count > 0)
	{
		string_ty	*tmp;
		sub_context_ty	*scp2;

		/*
		 * Make sure the variables of this message are optional,
		 * to avoid infinite loops if there is a mistake in the
		 * translation string.
		 */
		scp2 = sub_context_new();
		tmp = wstr_to_str(s);
		sub_var_set(scp2, "Message", "%S", tmp);
		sub_var_optional(scp2, "Message");
		sub_var_set(scp2, "Number", "%d", error_count);
		sub_var_optional(scp2, "Number");
		fatal_intl
		(
			scp2,
		     i18n("in substitution \"$message\" found unused variables")
		);
		/* NOTREACHED */
		str_free(tmp);
		sub_context_delete(scp2);
	}

	/*
	 * clear the slate, ready for the next run
	 */
	sub_var_clear(scp);
	result = collect_end(&buf);
	collect_destructor(&buf);
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


wstring_ty *
subst_intl_wide(scp, msg)
	sub_context_ty	*scp;
	const char	*msg;
{
	char		*tmp;
	wstring_ty	*s;
	wstring_ty	*result;

	trace(("subst_intl_wide(scp = %08lX, msg = \"%s\")\n{\n"/*}*/,
		(long)scp, msg));
	language_human();
	tmp = gettext(msg);
	language_C();
#if 0 /* def DEBUG */
#ifdef HAVE_GETTEXT
	if (tmp == msg)
		error_raw("warning: message \"%s\" has no translation", msg);
#endif /* HAVE_GETTEXT */
#endif /* DEBUG */
	s = wstr_from_c(tmp);
	result = subst(scp, s);
	wstr_free(s);
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


string_ty *
subst_intl(scp, s)
	sub_context_ty	*scp;
	const char	*s;
{
	wstring_ty	*result_wide;
	string_ty	*result;

	trace(("subst_intl(scp = %08lX, s = \"%s\")\n{\n"/*}*/,
		(long)scp, s));
	result_wide = subst_intl_wide(scp, s);
	result = wstr_to_str(result_wide);
	wstr_free(result_wide);
	trace(("return \"%s\";\n", result->str_text));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	substitute
 *
 * SYNOPSIS
 *	string_ty *substitute(string_ty *s);
 *
 * DESCRIPTION
 *	The substitute function is used to perform substitutions on
 *	strings.  Usually command strings, but not always.
 *
 *	The format of substitutions, and the commonly available
 *	substitutions, are described in aesub(5).
 *
 * ARGUMENTS
 *	cp	- the aegis change involved with the command
 *		  This may never be NULL.
 *	s	- the string to be substituted.
 *
 * RETURNS
 *	string_ty *; pointer to string in dynamic memory
 */

string_ty *
substitute(scp, s)
	sub_context_ty	*scp;
	string_ty	*s;
{
	wstring_ty	*ws;
	wstring_ty	*result_wide;
	string_ty	*result;

	assert(s);
	trace(("substitute(scp = %08lX, s = \"%s\")\n{\n"/*}*/,
		(long)scp, s->str_text));
	ws = str_to_wstr(s);
	result_wide = subst(scp, ws);
	wstr_free(ws);
	result = wstr_to_str(result_wide);
	wstr_free(result_wide);
	trace(("return \"%s\";\n", result->str_text));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	sub_var_clear
 *
 * SYNOPSIS
 *	void sub_var_clear(void);
 *
 * DESCRIPTION
 *	The sub_var_clear function is used to clear all of
 *	the substitution variables.  Not usually needed manually,
 *	as this is done automatically at the end of every substitute().
 */

void
sub_var_clear(scp)
	sub_context_ty	*scp;
{
	size_t		j;

	trace(("sub_var_clear()\n{\n"/*}*/));
	for (j = 0; j < scp->sub_var_pos; ++j)
		wstr_free(scp->sub_var_list[j].value);
	scp->sub_var_pos = 0;
	scp->errno_sequester = 0;
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	sub_var_set
 *
 * SYNOPSIS
 *	void sub_var_set(char *name, char *fmt, ...);
 *
 * DESCRIPTION
 *	The sub_var_set function is used to set the value of a
 *	substitution variable.  These variables are command specific,
 *	as opposed to the functions which are always present.
 *	The user documentation does NOT make this distinction by
 *	using the names "variable" and "function", they are always referred
 *	to as "substitutions".
 *
 * ARGUMENTS
 *	name	- the name of the variable
 *	fmt,...	- a format string and arguments to construct the value.
 *		  Handed to str_vformat to make a (string_ty *) out of it.
 *
 * CAVEAT
 *	remains in scope until the next invokation of sub_var_clear,
 *	or until the end of the next invokation of substitute.
 */

void
#if defined(__STDC__) && __STDC__
sub_var_set(sub_context_ty *scp, const char *name, const char *fmt, ...)
#else
sub_var_set(scp, name, fmt sva_last)
	sub_context_ty	*scp;
	const char	*name;
	const char	*fmt;
	sva_last_decl
#endif
{
	va_list		ap;
	string_ty	*s;
	table_ty	*svp;

	trace(("sub_var_set(scp = %08lX, name = \"%s\")\n{\n",
		(long)scp, name));
	sva_init(ap, fmt);
	s = str_vformat(fmt, ap);
	va_end(ap);
	trace_string(s->str_text);

	if (scp->sub_var_pos >= scp->sub_var_size)
	{
		size_t		nbytes;

		scp->sub_var_size += 10;
		nbytes = scp->sub_var_size * sizeof(scp->sub_var_list[0]);
		scp->sub_var_list = mem_change_size(scp->sub_var_list, nbytes);
	}
	svp = &scp->sub_var_list[scp->sub_var_pos++];
	svp->name = name;
	svp->func = 0;
	svp->value = str_to_wstr(s);
	str_free(s);
	svp->must_be_used = 1;
	svp->append_if_unused = 0;
	svp->override = 0;
	svp->resubstitute = !svp->must_be_used;
	trace(("}\n"));
}


void
sub_var_resubstitute(scp, name)
	sub_context_ty	*scp;
	const char	*name;
{
	table_ty	*the_end;
	table_ty	*svp;

	the_end = scp->sub_var_list + scp->sub_var_pos;
	for (svp = scp->sub_var_list; svp < the_end; ++svp)
		if (!strcmp(svp->name, name))
			break;
	assert(svp < the_end);
	if (svp < the_end)
		svp->resubstitute = 1;
}


void
sub_var_override(scp, name)
	sub_context_ty	*scp;
	const char	*name;
{
	table_ty	*the_end;
	table_ty	*svp;

	the_end = scp->sub_var_list + scp->sub_var_pos;
	for (svp = scp->sub_var_list; svp < the_end; ++svp)
		if (!strcmp(svp->name, name))
			break;
	assert(svp < the_end);
	if (svp < the_end)
		svp->override = 1;
}


void
sub_var_optional(scp, name)
	sub_context_ty	*scp;
	const char	*name;
{
	table_ty	*the_end;
	table_ty	*svp;

	the_end = scp->sub_var_list + scp->sub_var_pos;
	for (svp = scp->sub_var_list; svp < the_end; ++svp)
		if (!strcmp(svp->name, name))
			break;
	assert(svp < the_end);
	if (svp < the_end)
		svp->must_be_used = 0;
}


void
sub_var_append_if_unused(scp, name)
	sub_context_ty	*scp;
	const char	*name;
{
	table_ty	*the_end;
	table_ty	*svp;

	the_end = scp->sub_var_list + scp->sub_var_pos;
	for (svp = scp->sub_var_list; svp < the_end; ++svp)
		if (!strcmp(svp->name, name))
			break;
	assert(svp < the_end);
	if (svp < the_end)
	{
		svp->must_be_used = 1;
		svp->append_if_unused = 1;
	}
}


void
sub_errno_setx(scp, x)
	sub_context_ty	*scp;
	int		x;
{
	scp->errno_sequester = x;
}


void
sub_errno_set(scp)
	sub_context_ty	*scp;
{
	sub_errno_setx(scp, errno);
}


int
sub_context_errno_get(scp)
	sub_context_ty	*scp;
{
	if (scp->errno_sequester == 0)
		scp->errno_sequester = errno;
	return scp->errno_sequester;
}
