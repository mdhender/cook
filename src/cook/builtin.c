/*
 *      cook - file construction tool
 *      Copyright (C) 1993-2008 Peter Miller
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 3 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program. If not, see
 *      <http://www.gnu.org/licenses/>.
 *
 * The builtin functions all append their results to the supplied
 * `result' word list.  The first word of the `args' word list
 * is the name of the function.
 *
 * all of the functions return 0 in success, or -1 on error.
 *
 * Only a limited set of functionality is candidate for builtin functions,
 * these are
 *      - string manipulation [dirname, stringset, ect ]
 *      - environment manipulation [getenv(3), etc]
 *      - stat(3) related functions [exists, mtime, pathname, etc]
 *      - launching OS commands [execute, collect]
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

#include <cook/builtin.h>
#include <cook/builtin/addprefix.h>
#include <cook/builtin/addsuffix.h>
#include <cook/builtin/basename.h>
#include <cook/builtin/boolean.h>
#include <cook/builtin/cando.h>
#include <cook/builtin/collect.h>
#include <cook/builtin/cook.h>
#include <cook/builtin/defined.h>
#include <cook/builtin/dos.h>
#include <cook/builtin/execute.h>
#include <cook/builtin/exists.h>
#include <cook/builtin/expr.h>
#include <cook/builtin/filter_out.h>
#include <cook/builtin/find_command.h>
#include <cook/builtin/findstring.h>
#include <cook/builtin/getenv.h>
#include <cook/builtin/glob.h>
#include <cook/builtin/home.h>
#include <cook/builtin/interi_files.h>
#include <cook/builtin/join.h>
#include <cook/builtin/match.h>
#include <cook/builtin/mtime.h>
#include <cook/builtin/opsys.h>
#include <cook/builtin/options.h>
#include <cook/builtin/pathname.h>
#include <cook/builtin/positional.h>
#include <cook/builtin/print.h>
#include <cook/builtin/read.h>
#include <cook/builtin/readlink.h>
#include <cook/builtin/relati_dirna.h>
#include <cook/builtin/resolve.h>
#include <cook/builtin/sort_newest.h>
#include <cook/builtin/split.h>
#include <cook/builtin/stringset.h>
#include <cook/builtin/strip.h>
#include <cook/builtin/stripdot.h>
#include <cook/builtin/strlen.h>
#include <cook/builtin/subst.h>
#include <cook/builtin/substr.h>
#include <cook/builtin/suffix.h>
#include <cook/builtin/text.h>
#include <cook/builtin/thread-id.h>
#include <cook/builtin/unsplit.h>
#include <cook/builtin/uptodate.h>
#include <cook/builtin/word.h>
#include <cook/builtin/wordlist.h>
#include <cook/builtin/write.h>
#include <cook/id/builtin.h>
#include <cook/id/global.h>
#include <common/symtab.h>


/*
 * NAME
 *      func - table of built-in functions
 *
 * SYNOPSIS
 *      func_ty func[];
 *
 * DESCRIPTION
 *      Func is a table of function names and pointers
 *      for the built-in functions of cook.
 */

static builtin_ty *func[] = {
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
    &builtin_strlen,
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
 *      builtin_initialize - start up builtins
 *
 * SYNOPSIS
 *      void builtin_initialize(void);
 *
 * DESCRIPTION
 *      The builtin_initialize function is used to initialize the symbol table
 *      with the names and pointers to the builtin functions.
 *
 * CAVEAT
 *      This function must be called after the id_initialize function.
 */

void
builtin_initialize(void)
{
    builtin_ty      **fpp;
    string_ty       *s;

    for (fpp = func; fpp < ENDOF(func); ++fpp)
    {
        s = str_from_c((*fpp)->name);
        symtab_assign(id_global_stp(), s, id_builtin_new(*fpp));
        str_free(s);
    }
}
