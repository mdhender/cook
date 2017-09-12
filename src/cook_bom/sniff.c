/*
 *	cook - file construction tool
 *	Copyright (C) 1998, 1999, 2001 Peter Miller;
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
 * MANIFEST: functions to manipulate sniffs
 */

#include <ac/ctype.h>
#include <ac/stdio.h>
#include <ac/string.h>
#include <ac/errno.h>

#include <ac/dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <error_intl.h>
#include <gmatch.h>
#include <sniff.h>
#include <str.h>
#include <stracc.h>
#include <str_list.h>
#include <symtab.h>

static string_list_ty	dir_path;
static string_list_ty	ignore_list;
static char cook_chars[] = "#\"'():;=[\\]{}";
static string_ty *prefix;
static string_ty *suffix;


void
sniff_directory(s)
	char		*s;
{
	string_ty	*dir;

	dir = str_from_c(s);
	string_list_append_unique(&dir_path, dir);
	str_free(dir);
}


void
sniff_ignore(s)
	char		*s;
{
	string_ty	*ss;

	ss = str_from_c(s);
	string_list_append_unique(&ignore_list, ss);
	str_free(ss);
}


static string_ty *path_catenate _((string_ty *, string_ty *));

static string_ty *
path_catenate(dir, filename)
	string_ty	*dir;
	string_ty	*filename;
{
	static string_ty *dot;
	if (!dot)
		dot = str_from_c(".");
	if (str_equal(dir, dot))
		return str_copy(filename);
	if (str_equal(filename, dot))
		return str_copy(dir);
	return str_format("%S/%S", dir, filename);
}


static string_ty * dir_path_nth _((size_t, string_ty *));

static string_ty *
dir_path_nth(n, filename)
	size_t		n;
	string_ty	*filename;
{

	if (!dir_path.nstrings)
		sniff_directory(".");
	if (n >= dir_path.nstrings)
		return 0;
	return path_catenate(dir_path.string[n], filename);
}


static void stat_path _((string_ty *, struct stat *));

static void
stat_path(filename, st)
	string_ty	*filename;
	struct stat	*st;
{
	size_t		j;

	for (j = 0; ; ++j)
	{
		string_ty	*filename2;
		int		err;

		filename2 = dir_path_nth(j, filename);
		if (!filename2)
			break;
#if defined(S_IFLNK) || defined(S_ISLNK)
		err = lstat(filename2->str_text, st);
#else
		err = stat(filename2->str_text, st);
#endif
		str_free(filename2);
		if (!err)
			return;
		if (errno != ENOENT)
			fatal_intl_stat(filename2->str_text);
	}
	errno = ENOENT;
	fatal_intl_stat(filename->str_text);
}


static int ignore _((char *));

static int
ignore(s)
	char		*s;
{
	size_t		j;

	if (s[0] == '.' && (s[1] == 0 || (s[1] == '.' && s[2] == 0)))
		return 1;
	for (j = 0; j < ignore_list.nstrings; ++j)
		if (gmatch(ignore_list.string[j]->str_text, s) > 0)
			return 1;
	return 0;
}


static int scan _((string_ty *, string_list_ty *));

static int
scan(filename, result)
	string_ty	*filename;
	string_list_ty	*result;
{
	DIR		*dp;

	dp = opendir(filename->str_text);
	if (!dp)
		fatal_intl_opendir(filename->str_text);
	for (;;)
	{
		struct dirent	*dep;

		dep = readdir(dp);
		if (!dep)
			break;
		if (!ignore(dep->d_name))
		{
			string_ty	*s;

			s = str_from_c(dep->d_name);
			string_list_append_unique(result, s);
			str_free(s);
		}
	}
	closedir(dp);
	return 1;
}


static void scan_path _((string_ty *, string_list_ty *));

static void
scan_path(filename, result)
	string_ty	*filename;
	string_list_ty	*result;
{
	size_t		j;
	size_t		ndirs;

	ndirs = 0;
	for (j = 0; ; ++j)
	{
		string_ty	*filename2;

		filename2 = dir_path_nth(j, filename);
		if (!filename2)
			break;
		if (scan(filename2, result))
			++ndirs;
	}
	if (ndirs == 0)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_setx(scp, ENOENT);
		sub_var_set(scp, "File_Name", "%S", filename);
		fatal_intl(scp, i18n("open $filename: $errno"));
	}
}


static int quote_cook_char_p _((int));

static int
quote_cook_char_p(c)
	int		c;
{
	c = (unsigned char)c;
	if (isspace(c) || !isprint(c))
		return 1;
	if (strchr(cook_chars, c))
		return 1;
	return 0;
}


static int quote_cook_chars_p _((string_ty *));

static int
quote_cook_chars_p(s)
	string_ty	*s;
{
	char		*cp;
	static symtab_ty *stp;

	/*
	 * check to see if it is a keyword
	 */
	if (!stp)
	{
		static char *keyword[] =
		{
			"data",
			"else",
			"fail",
			"function",
			"host-binding",
			"if",
			"loop",
			"loopstop",
			"return",
			"set",
			"single-thread",
			"then",
			"unsetenv",
		};

		char **cpp;
		stp = symtab_alloc(SIZEOF(keyword));
		for (cpp = keyword; cpp < ENDOF(keyword); ++cpp)
		{
			string_ty *key;
			key = str_from_c(*cpp);
			symtab_assign(stp, key, *cpp);
			str_free(key);
		}
	}
	if (symtab_query(stp, s))
		return 1;
	if (s->str_length == 0)
		return 1;

	/*
	 * look at individual characters
	 */
	cp = s->str_text;
	for (;;)
	{
		int c = *cp++;
		if (!c)
			break;
		if (quote_cook_char_p(c))
			return 1;
		/* watch out for things which look like comment introducers */
		if (c == '/' && *cp == '*')
			return 1;
	}

	/*
	 * Looks like it's safe.
	 */
	return 0;
}


static string_ty * quote_cook_chars _((string_ty *));

static string_ty *
quote_cook_chars(s)
	string_ty	*s;
{
	char		*cp;
	static stracc	sa;

	if (!quote_cook_chars_p(s))
		return str_copy(s);
	sa_open(&sa);
	sa_char(&sa, '"');
	cp = s->str_text;
	for (;;)
	{
		int c = (unsigned char)*cp++;
		if (!c)
			break;
		if (isspace(c) || !isprint(c))
		{
			static char escape[] = "\bb\ff\nn\rr\tt";
			char *e = strchr(escape, c);
			if (e)
			{
				sa_char(&sa, '\\');
				sa_char(&sa, e[1]);
			}
			else
			{
				char tmp[8];
				sprintf(tmp, "\\%o", c);
				sa_chars(&sa, tmp, strlen(tmp));
			}
		}
		else
		{
			if (strchr("\"\\", c))
				sa_char(&sa, '\\');
			sa_char(&sa, c);
		}
	}
	sa_char(&sa, '"');
	return sa_close(&sa);
}


void
sniff(ifn, ofn)
	char		*ifn;
	char		*ofn;
{
	string_ty	*dirname;
	string_list_ty	contents;
	string_list_ty	files;
	string_list_ty	directories;
	string_list_ty	specials;
	size_t		j;
	FILE		*ofp;
	string_ty	*filename;

	/*
	 * Read the directory contents
	 * (unioned over the directory search path).
	 *
	 * Sort it, so the output is consistent even if files are
	 * deleted and recreated, changing the directory order.
	 */
	if (!ifn)
		ifn = ".";
	dirname = str_from_c(ifn);
	string_list_constructor(&contents);
	scan_path(dirname, &contents);
	string_list_sort(&contents);

	/*
	 * Split the directory contents by type.
	 *
	 * We don't do this during the scan so that files earlier in
	 * the directory search path can "occlude" files later in the
	 * search path, even if they are of different types.
	 */
	string_list_constructor(&files);
	string_list_constructor(&directories);
	string_list_constructor(&specials);
	for (j = 0; j < contents.nstrings; ++j)
	{
		struct stat	st;

		filename = path_catenate(dirname, contents.string[j]);
		stat_path(filename, &st);
		str_free(filename);
		filename = quote_cook_chars(contents.string[j]);
		if (S_ISREG(st.st_mode))
			string_list_append(&files, filename);
		else if (S_ISDIR(st.st_mode))
			string_list_append(&directories, filename);
		else
			string_list_append(&specials, filename);
		str_free(filename);
	}
	string_list_destructor(&contents);

	/*
	 * Open the output file.
	 */
	if (ofn)
	{
		ofp = fopen_and_check(ofn, "w");
	}
	else
	{
		ofn = "standard output";
		ofp = stdout;
	}

	fprintf(ofp, ".cook.bom.dir = [relative_dirname [__FILE__]];\n");
	fprintf(ofp, "if [in [.cook.bom.dir] \".\"] then\n");
	fprintf(ofp, "	.cook.bom/dir = '';\n");
	fprintf(ofp, "else\n");
	fprintf(ofp, "	.cook.bom/dir = [.cook.bom.dir]/;\n");

	/*
	 * Output the normal files.
	 */
	fprintf(ofp, "\n");
	fprintf(ofp, "files_in_[.cook.bom.dir] =\n");
	for (j = 0; j < files.nstrings; ++j)
	{
		filename = files.string[j];
		fprintf(ofp, "\t%s\n", filename->str_text);
	}
	fprintf(ofp, "\t;\n");
	fprintf
	(
		ofp,
		"all_files_in_[.cook.bom.dir] = [files_in_[.cook.bom.dir]];\n"
	);
	fflush_and_check(ofp, ofn);

	/*
	 * Output the special files.
	 */
	fprintf(ofp, "\n");
	fprintf(ofp, "specials_in_[.cook.bom.dir] =\n");
	for (j = 0; j < specials.nstrings; ++j)
	{
		filename = specials.string[j];
		fprintf(ofp, "\t%s\n", filename->str_text);
	}
	fprintf(ofp, "\t;\n");
	fprintf
	(
		ofp,
	    "all_specials_in_[.cook.bom.dir] = [specials_in_[.cook.bom.dir]];\n"
	);
	fflush_and_check(ofp, ofn);

	/*
	 * Output the directories.
	 */
	fprintf(ofp, "\n");
	fprintf(ofp, "directories_in_[.cook.bom.dir] =\n");
	for (j = 0; j < directories.nstrings; ++j)
	{
		filename = directories.string[j];
		fprintf(ofp, "\t%s\n", filename->str_text);
	}
	fprintf(ofp, "\t;\n");
	fprintf
	(
		ofp,
      "all_directories_in_[.cook.bom.dir] = [directories_in_[.cook.bom.dir]];\n"
	);
	fflush_and_check(ofp, ofn);

	/*
	 * Output the reference to the next level of the manifest.
	 */
	if (!prefix)
		prefix = str_from_c("");
	if (!suffix)
		suffix = str_from_c("/manifest.cook");
	if (directories.nstrings > 0)
	{
		fprintf(ofp, "\n");
		fprintf
		(
			ofp,
"#include-cooked-nowarn [prepost %s[.cook.bom/dir] %s \\\n\
\t[directories_in_[.cook.bom.dir]]]\n",
			prefix->str_text,
			suffix->str_text
		);

		fprintf(ofp, "\n\
/*\n\
 * These variables must be calculated again, as the above includes will\n\
 * have over-written them, and they all use the same variables.\n\
 */\n");
		fprintf
		(
			ofp,
			".cook.bom.dir = [relative_dirname [__FILE__]];\n"
		);
		fprintf(ofp, "if [in [.cook.bom.dir] \".\"] then\n");
		fprintf(ofp, "	.cook.bom/dir = '';\n");
		fprintf(ofp, "else\n");
		fprintf(ofp, "	.cook.bom/dir = [.cook.bom.dir]/;\n");
	}

	/*
	 * work over each reference
	 */
	for (j = 0; j < directories.nstrings; ++j)
	{
		filename = directories.string[j];
		fprintf(ofp, "\n");

		fprintf
		(
			ofp,
"if [defined all_files_in_[.cook.bom/dir]%s] then\n\
\tall_files_in_[.cook.bom.dir] +=\n\
\t\t[addprefix %s/ [all_files_in_[.cook.bom/dir]%s]];\n",
			filename->str_text,
			filename->str_text,
			filename->str_text
		);

		fprintf
		(
			ofp,
"if [defined all_specials_in_[.cook.bom/dir]%s] then\n\
\tall_specials_in_[.cook.bom.dir] +=\n\
\t\t[addprefix %s/ [all_specials_in_[.cook.bom/dir]%s]];\n",
			filename->str_text,
			filename->str_text,
			filename->str_text
		);

		fprintf
		(
			ofp,
"if [defined all_directories_in_[.cook.bom/dir]%s] then\n\
\tall_directories_in_[.cook.bom.dir] +=\n\
\t\t[addprefix %s/ [all_directories_in_[.cook.bom/dir]%s]];\n",
			filename->str_text,
			filename->str_text,
			filename->str_text
		);

		fflush_and_check(ofp, ofn);
	}
	fprintf(ofp, "\n.cook.bom.dir = ;\n.cook.bom/dir = ;\n");

	/*
	 * close the output file
	 */
	fflush_and_check(ofp, ofn);
	if (ofp != stdout)
		fclose_and_check(ofp, ofn);
	str_free(dirname);
}


int
sniff_prefix(s)
	char		*s;
{
	string_ty	*tmp;

	if (prefix)
		return -1;
	tmp = str_from_c(s);
	prefix = quote_cook_chars(tmp);
	str_free(tmp);
	return 0;
}


int
sniff_suffix(s)
	char		*s;
{
	string_ty	*tmp;

	if (suffix)
		return -1;
	tmp = str_from_c(s);
	suffix = quote_cook_chars(tmp);
	str_free(tmp);
	return 0;
}
