/*
 *	cook - file construction tool
 *	Copyright (C) 1994, 1997, 1998, 1999, 2001 Peter Miller;
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
 * MANIFEST: functions to do lexical analysis on the fingerprint cache file
 */

#include <ac/ctype.h>
#include <ac/errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <error_intl.h>
#include <fingerprint/lex.h>
#include <input/file_text.h>
#include <input/null.h>
#include <str.h>
#include <fingerprint/gram.gen.h> /* after str.h */

static input_ty	*fp;
static int	nerr;
static long	linum;


/*
 * NAME
 *	fingerprint_lex_open
 *
 * SYNOPSIS
 *	void fingerprint_lex_open(string_ty *filename);
 *
 * DESCRIPTION
 *	The fingerprint_lex_open function is used to commence lexical
 *	analysis of the given file.  If an error occurs, it will be
 *	reported and the function will NOT return.
 */

void
fingerprint_lex_open(fn)
	string_ty	*fn;
{
	struct stat	st;

	/* don't use os_exists, you get a loop */
	if
	(
		stat(fn->str_text, &st) < 0
	&&
		(errno == ENOENT || errno == ENOTDIR)
	)
		fp = input_null();
	else
		fp = input_file_text_open(fn);
	linum = 1;
	nerr = 0;
}


/*
 * NAME
 *	fingerprint_lex_close
 *
 * SYNOPSIS
 *	void fingerprint_lex_close(void);
 *
 * DESCRIPTION
 *	The fingerprint_lex_close function is used to complete lexical
 *	analysis of a file and release resources consumed in doing so.
 *	If an erro roccurred during the parse, the total will be reported
 *	and this function will NOT return.
 */

void
fingerprint_lex_close()
{
	if (nerr)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%S", input_filename(fp));
		sub_var_set(scp, "Number", "%d", nerr);
		sub_var_optional(scp, "Number");
		fatal_intl(scp, i18n("$filename: found $number fatal errors"));
		/* NOTREACHED */
	}
	input_delete(fp);
	nerr = 0;
	fp = 0;
	linum = 0;
}


/*
 * NAME
 *	lex_getc
 *
 * SYNOPSIS
 *	int lex_getc(void);
 *
 * DESCRIPTION
 *	The lex_getc function is used to fetch another character from
 *	the input stream.  If tere are errors reading the file, they
 *	will be reported and this function will NOT return.
 *
 * RETURNS
 *	int; zero or positive numbers for input characters, or INPUT_EOF
 *	on end of file.
 */

static int lex_getc _((void));

static int
lex_getc()
{
	int		c;

	c = input_getc(fp);
	if (c == '\n')
		linum++;
	return c;
}


/*
 * NAME
 *	lex_getc_undo
 *
 * SYNOPSIS
 *	void lex_getc_undo(int);
 *
 * DESCRIPTION
 *	The lex_getc_undo function is used to return the given character
 *	to the input stream.  The push back stack is arbitrarily deep.
 */

static void lex_getc_undo _((int));

static void
lex_getc_undo(c)
	int		c;
{
	switch (c)
	{
	case INPUT_EOF:
		break;

	case '\n':
		--linum;
		/* fall through... */

	default:
		input_ungetc(fp, c);
		break;
	}
}


/*
 * NAME
 *	fingerprint_error
 *
 * SYNOPSIS
 *	void fingerprint_error(sub_context_ty *cntxt, char *text);
 *
 * DESCRIPTION
 *	The fingerprint_error function is used to report errors
 *	while parsing fingerprint files.  The substitution context
 *	given (if any) will be used.  The error message text will be
 *	internationalized and substituted before being reported.
 *
 *	The function does return, however the fingerprint_lex_close
 *	function will, when eventually called, not return.
 */

static void fingerprint_error _((sub_context_ty *, char *));

static void
fingerprint_error(scp, s)
	sub_context_ty	*scp;
	char		*s;
{
	string_ty	*buffer;
	int		len;
	int		need_to_delete;

	if (scp)
		need_to_delete = 0;
	else
	{
		scp = sub_context_new();
		need_to_delete = 1;
	}

	buffer = subst_intl(scp, s);
	len = buffer->str_length;
	while (len > 0 && isspace(buffer->str_text[len - 1]))
		--len;

	/* re-use substitution context */
	sub_var_set(scp, "File_Name", "%S", input_filename(fp));
	sub_var_set(scp, "Number", "%ld", linum);
	sub_var_set(scp, "MeSsaGe", "%.*S", len, buffer);
	error_intl(scp, i18n("$filename: $number: $message"));
	str_free(buffer);
	if (++nerr >= 20)
	{
		/* re-use substitution context */
		sub_var_set(scp, "File_Name", "%S", input_filename(fp));
		fatal_intl(scp, i18n("$filename: too many fatal errors"));
	}

	if (need_to_delete)
		sub_context_delete(scp);
}


/*
 * NAME
 *	fingerprint_gram_lex
 *
 * SYNOPSIS
 *	int fingerprint_gram_lex(void);
 *
 * DESCRIPTION
 *	The fingerprint_gram_lex function is called by the yacc-generated
 *	grammar to partition the input into discrete tokens.  The return
 *	values are defined by yacc.
 *
 * SIDE EFFECTS
 *	Tokens which need more to describe them than just the return
 *	value have that additional information stored into the
 *	fingerprint_gram_lval variable, provided to us by yacc.
 */

int
fingerprint_gram_lex()
{
	int		c;
	char		buffer[2000];
	char		*cp;
	long		n;

	if (!fp)
		return 0;
	for (;;)
	{
		c = lex_getc();
		switch (c)
		{
		case INPUT_EOF:
			return 0;

		case '"':
			cp = buffer;
			for (;;)
			{
				c = lex_getc();
				if (c == INPUT_EOF || c == '\n')
				{
					unterm:
					fingerprint_gram_error
					(
						i18n("unterminated string")
					);
					break;
				}
				if (c == '"')
					break;
				if (c == '\\')
				{
					c = lex_getc();
					if (c == INPUT_EOF || c == '\n')
						goto unterm;
					if (c != '"' && c != '\\')
					{
						sub_context_ty	*scp;

						scp = sub_context_new();
						sub_var_set
						(
							scp,
							"Name",
							"\\%c",
							c
						);
						fingerprint_error
						(
							scp,
						  i18n("unknown '$name' escape")
						);
						sub_context_delete(scp);
					}
				}
				if (cp < ENDOF(buffer))
					*cp++ = c;
			}
			fingerprint_gram_lval.lv_string =
				str_n_from_c(buffer, cp - buffer);
			return STRING;

		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			n = 0;
			for (;;)
			{
				n = n * 10 + c - '0';
				c = lex_getc();
				switch (c)
				{
				case '0': case '1': case '2': case '3':
				case '4': case '5': case '6': case '7':
				case '8': case '9':
					continue;

				default:
					break;
				}
				break;
			}
			lex_getc_undo(c);
			fingerprint_gram_lval.lv_number = n;
			return NUMBER;

		case ' ':
		case '\t':
		case '\n':
			break;

		case '=':
			return EQ;

		case '{':
			return LB;

		case '}':
			return RB;

		default:
			return JUNK;
		}
	}
}


/*
 * NAME
 *	fingerprint_gram_error
 *
 * SYNOPSIS
 *	void fingerprint_gram_error(char *text);
 *
 * DESCRIPTION
 *	The fingerprint_gram_error function is called by yacc to report
 *	parse errors.
 */

void
fingerprint_gram_error(s)
	char		*s;
{
	fingerprint_error(0, s);
}
