/*
 *      cook - file construction tool
 *      Copyright (C) 1999, 2001, 2006-2010 Peter Miller
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
 */

#include <common/ac/errno.h>
#include <common/ac/dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <common/ac/unistd.h>

#include <common/error_intl.h>
#include <common/mem.h>
#include <common/os_path_cat.h>
#include <common/star.h>
#include <common/str.h>
#include <common/symtab.h>
#include <common/trace.h>
#include <cook/fingerprint.h>
#include <cook/fingerprint/filename.h>
#include <cook/fingerprint/find.h>
#include <cook/fingerprint/gram.h>
#include <cook/fingerprint/record.h>
#include <cook/fingerprint/subdir.h>
#include <cook/os_interface.h>


/*
 * NAME
 *      reap
 *
 * SYNOPSIS
 *      void reap(void *);
 *
 * DESCRIPTION
 *      The reap function is used to delete fp_record_ty symbol table
 *      entries.  Called exclusively by symtab_free().
 */

static void
reap(void *p)
{
    fp_record_ty    *frp;

    trace(("reap\n"));
    frp = p;
    fp_record_delete(frp);
}


/*
 * NAME
 *      fp_subdir_new
 *
 * SYNOPSIS
 *      void fp_subdir_new(string_ty *path);
 *
 * DESCRIPTION
 *      The fp_subdir_new function is used to allocate a fp_subdir_ty
 *      structure in dynamic memory and initialize it to empty.  The path
 *      is remembered for later reading and writing of the cache file.
 */

fp_subdir_ty *
fp_subdir_new(string_ty *path)
{
    fp_subdir_ty    *this;

    trace(("fp_subdir_new(path = \"%s\")\n{\n", path->str_text));
    this = mem_alloc(sizeof(fp_subdir_ty));
    this->stp = symtab_alloc(5);
    this->stp->reap = reap;
    this->path = str_copy(path);
    this->dirty = 0;
    this->cache_in_dot = 0;
    this->need_to_read = 1;
    trace(("return %p;\n", this));
    trace(("}\n"));
    return this;
}


/*
 * NAME
 *      fp_subdir_delete
 *
 * SYNOPSIS
 *      void fp_subdir_delete(void);
 *
 * DESCRIPTION
 *      The fp_subdir_delete function is used to release the resources
 *      held by a fp_subdir_ty structure.
 */

void
fp_subdir_delete(fp_subdir_ty *this)
{
    trace(("fp_subdir_delete(this = %p)\n{\n", this));
    str_free(this->path);
    this->path = 0;
    symtab_free(this->stp);
    this->stp = 0;
    mem_free(this);
    trace(("}\n"));
}


/*
 * NAME
 *      fp_subdir_read
 *
 * SYNOPSIS
 *      void fp_subdir_read(fp_subdir_ty *);
 *
 * DESCRIPTION
 *      The fp_subdir_read function is used to read the cache file
 *      assoictaed with a sub-directory, if it exists.
 */

void
fp_subdir_read(fp_subdir_ty *this)
{
    string_ty       *fn;

    if (!this->need_to_read)
        return;
    this->need_to_read = 0;

    trace(("fp_subdir_read(this = %p)\n{\n", this));
    fn = os_path_cat(this->path, fp_filename());
    fp_gram(this, fn);
    str_free(fn);
    trace(("}\n"));
}


/*
 * NAME
 *      walk
 *
 * SYNOPSIS
 *      void walk(symtab_ty *stp, string_ty *key, fp_record_ty *value,
 *              FILE *ofile);
 *
 * DESCRIPTION
 *      The walk function is used to emit entries in a fp_record_ty
 *      symbol table to a file.  Called exclusively  via symtab_walk
 *      by fp_subdir_write.
 */

static void
walk(symtab_ty *sp, string_ty *key, void *p, void *arg)
{
    fp_record_ty    *data;
    FILE            *fp;

    trace(("walk(sp = %p, key = \"%s\", p = %p, arg = %p)\n{\n", sp,
        key->str_text, p, arg));
    (void)sp;
    data = p;
    fp = arg;
    fp_record_write(data, key, fp);
    trace(("}\n"));
}


/*
 * NAME
 *      fp_subdir_write
 *
 * SYNOPSIS
 *      void fp_subdir_write(void);
 *
 * DESCRIPTION
 *      The fp_subdir_write function is used to write the contents of
 *      a fp_subdir_ty structure out to the corresponding disk cache file.
 *
 *      It will not be written out if it isn't dirty (hasn't changed)
 *      or if cahce updating has been displabed.
 */

void
fp_subdir_write(fp_subdir_ty *this, int *need_to_write_dot)
{
    static string_ty *dot;
    string_ty       *fn;
    FILE            *fp;
    struct stat     st;

    /*
     * If we aren't dirty, don't write us out
     */
    trace(("fp_subdir_write(this = %p)\n{\n", this));
    if (!this->dirty)
    {
        trace(("not dirty\n"));
        trace(("}\n"));
        return;
    }

    /*
     * build the filename to write
     */
    fn = os_path_cat(this->path, fp_filename());

    /*
     * We are about to write the file.  If the file isn't a regular
     * file, unlink it.  (It could be part of a symlink farm, it
     * could be someone being naughty, it could be plain stupid.)
     */
    if
    (
        stat(fn->str_text, &st) == 0
    &&
        !S_ISREG(st.st_mode)
    &&
        unlink(fn->str_text) != 0
    )
    {
        switch (errno)
        {
        case ENOENT:
        case ENOSYS:
            break;

        case EACCES:
            str_free(fn);
            trace(("mark not writable any more\n"));
            this->cache_in_dot = 1;
            *need_to_write_dot = 1;
            trace(("}\n"));
            return;

        default:
            fatal_intl_unlink(fn->str_text);
            /* NOTREACHED */
        }
    }

    /*
     * Open the file for writing.
     * If there is a permissions problem, quietly slink away.
     */
    fp = fopen(fn->str_text, "w");
    if (!fp)
    {
        if (errno == EACCES || errno == ENOENT || errno == ENOSYS)
        {
            str_free(fn);
            trace(("mark not writable any more\n"));
            this->cache_in_dot = 1;
            *need_to_write_dot = 1;
            trace(("}\n"));
            return;
        }
        fatal_intl_open(fn->str_text);
    }

    /*
     * walk the symbol table, bumping as we go.
     */
    star_as_specified('@');
    symtab_walk(this->stp, walk, fp);

    /*
     * NOW we're clean.
     */
    this->dirty = 0;
    if (this->cache_in_dot)
    {
        *need_to_write_dot = 1;
        this->cache_in_dot = 0;
    }

    /*
     * The top-level cache also gets all of the files from deeper,
     * unwritable directories.
     */
    if (!dot)
        dot = str_from_c(".");
    if (str_equal(this->path, dot))
        fp_find_main_write(fp);

    /*
     * close up and go home
     */
    fflush_and_check(fp, fn->str_text);
    fclose_and_check(fp, fn->str_text);
    str_free(fn);
    trace(("}\n"));
}


void
fp_subdir_dirty_notify(fp_subdir_ty *this, string_ty *filename)
{
    /*
     * ignore the fingerprint cache file
     */
    if (str_equal(filename, fp_filename()))
        return;

    /*
     * Update this flag, even if cache_in_dot,
     * because the "." cache will get it.
     */
    this->dirty = 1;
}


void
fp_subdir_tweak(fp_subdir_ty *this)
{
    DIR             *dp;
    struct dirent   *dep;
    string_ty       *filename;
    static string_ty *avoid;
    static string_ty *dot;
    static string_ty *dotdot;
    string_list_ty  sl;
    size_t          j;

    /*
     * read the directory
     */
    trace(("fp_subdir_update(this = %p)\n{\n", this));
    star_as_specified('*');
    dp = opendir(this->path->str_text);
    if (!dp)
        fatal_intl_opendir(this->path->str_text);
    if (!avoid)
        avoid = fp_filename();
    if (!dot)
        dot = str_from_c(".");
    if (!dotdot)
        dotdot = str_from_c("..");
    string_list_constructor(&sl);
    for (;;)
    {
        dep = readdir(dp);
        if (!dep)
            break;
        filename = str_from_c(dep->d_name);
        if
        (
            !str_equal(filename, avoid)
        &&
            !str_equal(filename, dot)
        &&
            !str_equal(filename, dotdot)
        )
            string_list_append(&sl, filename);
        str_free(filename);
    }
    closedir(dp);

    /*
     * visit each entry in the directory
     * - for each file/symlink, update the stat cache
     * - for each directory, recurse
     * - ignore evrything else
     */
    for (j = 0; j < sl.nstrings; ++j)
    {
        struct stat     st;
        string_ty       *s;
        int             err;

        s = os_path_cat(this->path, sl.string[j]);
        trace(("stat(\"%s\")\n", s->str_text));
#ifdef S_IFLNK
        err = lstat(s->str_text, &st);
#else
        err = stat(s->str_text, &st);
#endif
        if (err)
            fatal_intl_stat(s->str_text);

        switch (st.st_mode & S_IFMT)
        {
        case S_IFDIR:
            fp_subdir_tweak(fp_find_subdir(s, 1));
            break;

        case S_IFREG:
#ifdef S_IFLNK
        case S_IFLNK:
#endif
            fp_record_tweak(fp_find_record(s), st.st_mtime, fp_fingerprint(s));
            break;
        }
        str_free(s);
    }
    string_list_destructor(&sl);
    trace(("}\n"));
}
