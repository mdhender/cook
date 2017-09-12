/*
 *	cook - file construction tool
 *	Copyright (C) 1992, 1993, 1994, 1997, 1998, 2001 Peter Miller;
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
 * MANIFEST: functions to scan *roff source files
 */

#include <ac/ctype.h>
#include <ac/string.h>

#include <input.h>
#include <lang_roff.h>
#include <mem.h>
#include <str_list.h>
#include <trace.h>


/*
 * NAME
 *	directive
 *
 * SYNOPSIS
 *	void directive(char *line, string_list_ty *type1,
 *		string_list_ty *type2);
 *
 * DESCRIPTION
 *	The directive function is used to scan a . line for an
 *	include directive.  If one is found, the filename
 *	is resolved, and the path appended to the appropriate list.
 *
 * ARGUMENTS
 *	line	- the line of text from the program
 *	type1	- list of <filenames>
 *	type2	- list of "filenames"
 *
 * CAVEATS
 *	Just ignore anything we don't understand.
 */

static void directive _((char *s, string_list_ty *type1,
	string_list_ty *type2));

static void
directive(s, type1, type2)
	char		*s;
	string_list_ty	*type1;
	string_list_ty	*type2;
{
	char		*filename;
	string_ty	*path;

	/*
	 * see if it is a .so directive
	 */
	trace(("directive(s = \"%s\", type1 = %08lX, type2 = %08lX)\n{\n"/*}*/,
		s, type1, type2));
	assert(*s == '.');
	s++;
	while (isspace(*s))
		++s;
	if (*s++ != 's')
		goto done;
	if (*s++ != 'o')
		goto done;
	while (isspace(*s))
		++s;
	if (!*s)
		goto done;

	/*
	 * find the end of the filename
	 *	(ignore anything on the end of the line)
	 */
	filename = s;
	while (*s && !isspace(*s))
		++s;

	/*
	 * extract the path
	 */
	path = str_n_from_c(filename, s - filename);

	/*
	 * dispatch the path to the appropriate list
	 */
	string_list_append_unique(type1, path);
	str_free(path);

	/*
	 * here for all exits
	 */
	done:
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	lang_roff_scan
 *
 * SYNOPSIS
 *	int lang_roff_scan(input_ty *fp, string_list_ty *type1,
 *		string_list_ty *type2);
 *
 * DESCRIPTION
 *	The lang_roff_scan function is used to scan a file looking
 *	for nclude files.  It does not walk the children.
 *	The names of any include files encountered are appended
 *	to the appropriate list.
 *
 * ARGUMENTS
 *	fp	- file stream to scan
 *	type1	- list of <filenames>
 *	type2	- list of "filenames"
 *
 * RETURNS
 *	int;	0 on success
 *		-1 on file errors
 */

static int lang_roff_scan _((input_ty *, string_list_ty *, string_list_ty *));

static int
lang_roff_scan(fp, type1, type2)
	input_ty	*fp;
	string_list_ty	*type1;
	string_list_ty	*type2;
{
	size_t		pos;
	size_t		max;
	char		*line;
	int		result;
	int		c;

	trace(("lang_roff_scan(fp = %08lX, type1 = %08lX, \
type2 = %08lX)\n{\n"/*}*/, fp, type1, type2));
	pos = 0;
	max = 100;
	line = mem_alloc(max);
	result = 0;
	for (;;)
	{
		if (pos >= max)
		{
			max += 80;
			line = mem_change_size(line, max);
		}
		c = input_getc(fp);
		switch (c)
		{
		case INPUT_EOF:
			if (!pos)
				break;
			/* fall through... */

		case '\n':
			line[pos] = 0;
			pos = 0;

			/*
			 * see if it is a control line
			 */
			if (line[0] == '.')
				directive(line, type1, type2);
			continue;

		default:
			line[pos++] = c;
			continue;
		}
		break;
	}
	mem_free(line);
	trace(("return %d;\n", result));
	trace((/*{*/"}\n"));
	return result;
}


static void lang_roff_prepare _((void));

static void
lang_roff_prepare()
{
	trace(("lang_roff_prepare()\n{\n"/*}*/));
	if (sniff_include_count() == 0)
		sniff_include(".");
	trace((/*{*/"}\n"));
}


sniff_ty lang_roff =
{
	lang_roff_scan,
	lang_roff_prepare,
};
