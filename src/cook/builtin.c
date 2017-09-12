/*
 *	cook - file construction tool
 *	Copyright (C) 1993, 1994, 1996-1999, 2001, 2002 Peter Miller;
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
 * MANIFEST: functions to access the builtin functions
 *
 * The builtin functions all append their results to the supplied
 * `result' word list.  The first word of the `args' word list
 * is the name of the function.
 *
 * all of the functions return 0 in success, or -1 on error.
 *
 * Only a limited set of functionality is candidate for builtin functions,
 * these are
 *	- string manipulation [dirname, stringset, ect ]
 *	- environment manipulation [getenv(3), etc]
 *	- stat(3) related functions [exists, mtime, pathname, etc]
 *	- launching OS commands [execute, collect]
 * The above list is though to be exhaustive.
 *
 * Explicitly and forever excluded from being a builtin function
 * is anything which knows or understands the format of some secific
 * class of files.
 *
 * Access to stdio(3) has been thought of, and explicitly avoided.
 * Mostly because a specialist program used through [collect]
 * will almost always be far faster.
 */

#include <builtin.h>
#include <builtin/addprefix.h>
#include <builtin/addsuffix.h>
#include <builtin/basename.h>
#include <builtin/boolean.h>
#include <builtin/cando.h>
#include <builtin/collect.h>
#include <builtin/cook.h>
#include <builtin/defined.h>
#include <builtin/dos.h>
#include <builtin/execute.h>
#include <builtin/exists.h>
#include <builtin/expr.h>
#include <builtin/filter_out.h>
#include <builtin/find_command.h>
#include <builtin/findstring.h>
#include <builtin/getenv.h>
#include <builtin/glob.h>
#include <builtin/home.h>
#include <builtin/interi_files.h>
#include <builtin/join.h>
#include <builtin/match.h>
#include <builtin/mtime.h>
#include <builtin/opsys.h>
#include <builtin/options.h>
#include <builtin/pathname.h>
#include <builtin/positional.h>
#include <builtin/print.h>
#include <builtin/read.h>
#include <builtin/readlink.h>
#include <builtin/relati_dirna.h>
#include <builtin/resolve.h>
#include <builtin/sort_newest.h>
#include <builtin/split.h>
#include <builtin/stringset.h>
#include <builtin/strip.h>
#include <builtin/stripdot.h>
#include <builtin/subst.h>
#include <builtin/substr.h>
#include <builtin/suffix.h>
#include <builtin/text.h>
#include <builtin/thread-id.h>
#include <builtin/unsplit.h>
#include <builtin/uptodate.h>
#include <builtin/word.h>
#include <builtin/wordlist.h>
#include <builtin/write.h>
#include <id/builtin.h>
#include <id/global.h>
#include <symtab.h>


/*
 * NAME
 *	func - table of built-in functions
 *
 * SYNOPSIS
 *	func_ty func[];
 *
 * DESCRIPTION
 *	Func is a table of function names and pointers
 *	for the built-in functions of cook.
 */

static builtin_ty *func[] =
{
	&builtin_FILE,
	&builtin_LINE,
	&builtin_addprefix,
	&builtin_addsuffix,
	&builtin_and,
	&builtin_basename,
	&builtin_cando,
	&builtin_catenate,
	&builtin_collect,
	&builtin_collect_lines,
	&builtin_cook,
	&builtin_count,
	&builtin_defined,
	&builtin_dir,
	&builtin_dirname,
	&builtin_dos_path,
	&builtin_dos_path_undo,
	&builtin_downcase,
	&builtin_entryname,
	&builtin_execute,
	&builtin_exists,
	&builtin_exists_symlink,
	&builtin_expr,
	&builtin_filter,
	&builtin_filter_out,
	&builtin_filter_out_,
	&builtin_find_command,
	&builtin_findstring,
	&builtin_firstword,
	&builtin_fromto,
	&builtin_getenv,
	&builtin_glob,
	&builtin_head,
	&builtin_home,
	&builtin_if,
	&builtin_in,
	&builtin_interior_files,
	&builtin_join,
	&builtin_leaf_files,
	&builtin_match,
	&builtin_match_mask,
	&builtin_matches,
	&builtin_mtime,
	&builtin_mtime_seconds,
	&builtin_not,
	&builtin_notdir,
	&builtin_operating_system,
	&builtin_options,
	&builtin_or,
	&builtin_os,
	&builtin_pathname,
	&builtin_patsubst,
	&builtin_prepost,
	&builtin_print,
	&builtin_quote,
	&builtin_read,
	&builtin_read_lines,
	&builtin_readlink,
	&builtin_relative_dirname,
	&builtin_reldir,
	&builtin_resolve,
	&builtin_shell,
	&builtin_sort,
	&builtin_sort_newest,
	&builtin_split,
	&builtin_stringset,
	&builtin_strip,
	&builtin_stripdot,
	&builtin_subst,
	&builtin_substitute,
	&builtin_substr,
	&builtin_substring,
	&builtin_suffix,
	&builtin_tail,
	&builtin_thread_id,
	&builtin_un_dos_path,
	&builtin_unsplit,
	&builtin_upcase,
	&builtin_uptodate,
	&builtin_wildcard,
	&builtin_word,
	&builtin_wordlist,
	&builtin_words,
	&builtin_write,
};


/*
 * NAME
 *	builtin_initialize - start up builtins
 *
 * SYNOPSIS
 *	void builtin_initialize(void);
 *
 * DESCRIPTION
 *	The builtin_initialize function is used to initialize the symbol table
 *	with the names and pointers to the builtin functions.
 *
 * CAVEAT
 *	This function must be called after the id_initialize function.
 */

void
builtin_initialize()
{
	builtin_ty	**fpp;
	string_ty	*s;

	for (fpp = func; fpp < ENDOF(func); ++fpp)
	{
		s = str_from_c((*fpp)->name);
		symtab_assign(id_global_stp(), s, id_builtin_new(*fpp));
		str_free(s);
	}
}
