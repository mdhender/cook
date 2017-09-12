/*
 *	cook - file construction tool
 *	Copyright (C) 1997, 1998, 2001 Peter Miller;
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
 * MANIFEST: functions to scan source files
 */

#include <ac/ctype.h>
#include <ac/string.h>

#include <input.h>
#include <lang_optimis.h>
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

static void directive _((char *, string_list_ty *, string_list_ty *));

static void
directive(s, type1, type2)
	char		*s;
	string_list_ty	*type1;
	string_list_ty	*type2;
{
	char		*filename;
	string_ty	*path;

	/*
	 * Gnaw off any leading white space
	 * and some optional punctuation characters.
	 */
	trace(("directive(s = \"%s\", type1 = %08lX, type2 = %08lX)\n{\n"/*}*/,
		s, type1, type2));
	while (*s && isspace((unsigned char)*s))
		++s;
	if (*s && ispunct((unsigned char)*s))
		++s;
	while (*s && isspace((unsigned char)*s))
		++s;

	/*
	 * See if it is a keyword we like.
	 * It must end in a space or punctuation character.
	 */
	if (*s != 'i' && *s != 'I') goto done; ++s;
	if (*s != 'n' && *s != 'N') goto done; ++s;
	if (*s != 'c' && *s != 'C') goto done; ++s;
	if (*s != 'l' && *s != 'L') goto done; ++s;
	if (*s != 'u' && *s != 'U') goto done; ++s;
	if (*s != 'd' && *s != 'D') goto done; ++s;
	if (*s != 'e' && *s != 'E') goto done; ++s;
	if (!isspace((unsigned char)*s) && !ispunct((unsigned char)*s))
		goto done;

	/*
	 * Skip any dross between the keyword and the filename.  We
	 * assume the filename does not start with any of the junk we
	 * are skipping.  (Optimistic, remember.)
	 */
	while (isspace((unsigned char)*s) || ispunct((unsigned char)*s))
		++s;
	filename = s;

	/*
	 * Hunt for the next white space.  This assumes the the filename
	 * does not contain white space.  (Optimistic, again.)
	 */
	while (*s && !isspace(*s))
		++s;

	/*
	 * Strip any trailing dross from the filename.  We assume the
	 * filename does not end with any of the junk we are skipping.
	 * (Optimistic, aren't we?)
	 */
	while (s > filename && ispunct((unsigned char)s[-1]))
		--s;
	if (s <= filename)
		goto done;

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
 *	lang_optimistic_scan
 *
 * SYNOPSIS
 *	int lang_optimistic_scan(input_ty *fp, string_list_ty *type1,
 *		string_list_ty *type2);
 *
 * DESCRIPTION
 *	The lang_optimistic_scan function is used to scan a file looking
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

static int lang_optimistic_scan _((input_ty *, string_list_ty *,
	string_list_ty *));

static int
lang_optimistic_scan(fp, type1, type2)
	input_ty	*fp;
	string_list_ty	*type1;
	string_list_ty	*type2;
{
	size_t		pos;
	size_t		max;
	char		*line;
	int		result;
	int		c;

	trace(("lang_optimistic_scan(fp = %08lX, type1 = %08lX, \
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
			 * see if it is an include line
			 */
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


static void lang_optimistic_prepare _((void));

static void
lang_optimistic_prepare()
{
	trace(("lang_optimistic_prepare()\n{\n"/*}*/));
	if (sniff_include_count() == 0)
		sniff_include(".");
	trace((/*{*/"}\n"));
}


sniff_ty lang_optimistic =
{
	lang_optimistic_scan,
	lang_optimistic_prepare,
};
