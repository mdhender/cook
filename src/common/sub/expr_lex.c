/*
 *	cook - file construction tool
 *	Copyright (C) 1997 Peter Miller;
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
 * MANIFEST: functions to manipulate expr_lexs
 */

#include <str.h>
#include <sub/expr_lex.h>
#include <sub/expr_gram.gen.h>


static string_ty *text;
static size_t	pos;


void
sub_expr_lex_open(s)
	string_ty	*s;
{
	text = s;
	pos = 0;
}


void
sub_expr_lex_close()
{
	text = 0;
	pos = 0;
}


static int lex_getc _((void));

static int
lex_getc()
{
	int		c;

	if (!text || pos >= text->str_length)
		c = 0;
	else
		c = (unsigned char)text->str_text[pos];
	++pos;
	return c;
}


static void lex_getc_undo _((int));

static void
lex_getc_undo(c)
	int		c;
{
	if (pos > 0)
		--pos;
}



int
sub_expr_gram_lex()
{
	int		c;
	long		n;

	for (;;)
	{
		c = lex_getc();
		switch (c)
		{
		case 0:
			return 0;

		case '(':
			return LP;

		case ')':
			return RP;

		case '-':
			return MINUS;

		case '+':
			return PLUS;

		case '*':
			return MUL;

		case '/':
			return DIV;

		case ' ':
		case '\t':
		case '\n':
		case '\f':
			break;

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
				lex_getc_undo(c);
				break;
			}
			sub_expr_gram_lval.lv_number = n;
			return NUMBER;

		default:
			return JUNK;
		}
	}
}
