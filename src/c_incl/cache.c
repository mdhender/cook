/*
 *      cook - file construction tool
 *      Copyright (C) 1992-1994, 1997-1999, 2001, 2006, 2007 Peter Miller;
 *      All rights reserved.
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

#include <common/ac/fcntl.h>
#include <common/ac/stdio.h>
#include <common/ac/string.h>

#include <c_incl/cache.h>
#include <common/error_intl.h>
#include <common/mem.h>
#include <c_incl/os_interface.h>
#include <common/progname.h>
#include <common/symtab.h>
#include <common/trace.h>


static int      need_to_write;
static symtab_ty *symtab;


static void
reap(void *p)
{
    cache_ty        *cp;

    cp = p;
    string_list_destructor(&cp->ingredients);
    mem_free(cp);
}


/*
 * NAME
 *      cache_initialize - start up cache
 *
 * SYNOPSIS
 *      void cache_initialize(void);
 *
 * DESCRIPTION
 *      The cache_initialize function is used to create the hash table.
 *
 * RETURNS
 *      void
 *
 * CAVEAT
 *      Assumes the str_initialize function has been called already.
 */

void
cache_initialize(void)
{
    trace(("init\n"));
    symtab = symtab_alloc(100);
    symtab->reap = reap;
}


/*
 * NAME
 *      cache_search - search for a variable
 *
 * SYNOPSIS
 *      int cache_search(string_ty *filename);
 *
 * DESCRIPTION
 *      The cache_search function is used to search for
 *      a filename in the cache.
 *
 * RETURNS
 *      If the variable has been defined, the function returns a non-zero value
 *      and the value is returned through the 'value' pointer.
 *      If the variable has not been defined, it returns zero,
 *      and 'value' is unaltered.
 *
 * CAVEAT
 *      The value returned from this function, when returned, is allocated
 *      in dynamic memory (it is a copy of the value remembered by this module).
 *      It is the responsibility of the caller to free it when finished with,
 *      by a string_list_destructor() call.
 */

cache_ty *
cache_search(string_ty *filename)
{
    cache_ty        *cp;

    assert(symtab);
    cp = symtab_query(symtab, filename);
    if (!cp)
    {
        cp = mem_alloc(sizeof(cache_ty));
        memset(&cp->st, 0, sizeof(cp->st));
        string_list_constructor(&cp->ingredients);
        symtab_assign(symtab, filename, cp);
    }
    return cp;
}


/*
 * NAME
 *      build_filename - for cache file
 *
 * SYNOPSIS
 *      void build_filename(char *buffer);
 *
 * DESCRIPTION
 *      The build_filename function is used to build
 *      the name of the cache file.
 *
 * ARGUMENTS
 *      buffer  - where to put the file name
 *
 * CAVEATS
 *      The cache file is in the current directory.
 */

static char *
build_filename(void)
{
    static string_ty *s;

    if (!s)
        s = str_format(".%.11src", progname_get());
    return s->str_text;
}


/*
 * NAME
 *      fread_sane - a saner version of fread
 *
 * SYNOPSIS
 *      int fread_sane(FILE *fp, void *buf, size_t buflen);
 *
 * DESCRIPTION
 *      The fread_sane function is used to read from a standard stream.
 *
 * ARGUMENTS
 *      fp      - the stream to read from
 *      buf     - where to place the bytes read
 *      buflen  - number of bytes to read
 *
 * RETURNS
 *      0 on no error, -1 on any error
 *
 * CAVEATS
 *      This version considers it to be an error if end-of-file is reached.
 */

static int
fread_sane(FILE *fp, void *buf, size_t buflen)
{
    if (fread(buf, 1, buflen, fp) != buflen)
        return -1;
    return 0;
}


/*
 * NAME
 *      cache_read_string - read a string from a file
 *
 * SYNOPSIS
 *      string_ty *cache_read_string(FILE *fp));
 *
 * DESCRIPTION
 *      The cache_read_string function is used to read a string
 *      from a file.
 *
 * ARGUMENTS
 *      fp      - file to read string from
 *
 * RETURNS
 *      pointer to string if successful, 0 if not.
 *
 * CAVEATS
 *      Must be symmetric with cache_write string below.
 */

static string_ty *
cache_read_string(FILE *fp)
{
    static size_t   buflen;
    static char     *buf;
    size_t          len;

    if (fread_sane(fp, &len, sizeof(len)))
        return 0;
    if (len > buflen)
    {
        buflen = (len + 0xFF) & ~0xFF;
        buf = mem_change_size(buf, buflen);
    }
    if (fread_sane(fp, buf, len))
        return 0;
    return str_n_from_c(buf, len);
}


/*
 * NAME
 *      cache_read_item - read a cache item from a file
 *
 * SYNOPSIS
 *      int cache_read_item(FILE *fp);
 *
 * DESCRIPTION
 *      The cache_read_item function is used to read an item from
 *      the cache file and installit into the cache.
 *
 * ARGUMENTS
 *      fp      - the file to read the item from
 *
 * RETURNS
 *      0 in success, -1 on any error
 *
 * CAVEATS
 *      Must be symmetric with cache_write_item below.
 */

static int
cache_read_item(FILE *fp)
{
    string_ty       *s;
    cache_ty        *cp;
    size_t          nitems;
    size_t          j;

    s = cache_read_string(fp);
    if (!s)
        return -1;
    cp = cache_search(s);
    assert(cp);
    if (fread_sane(fp, &cp->st, sizeof(cp->st)))
        return -1;
    if (fread_sane(fp, &nitems, sizeof(nitems)))
        return -1;
    for (j = 0; j < nitems; ++j)
    {
        s = cache_read_string(fp);
        if (!s)
            return -1;
        string_list_append_unique(&cp->ingredients, s);
        str_free(s);
    }
    return 0;
}


static void
flock_shared(int fd, char *fn)
{
    struct flock    p;

    memset(&p, 0, sizeof(p));
    p.l_type = F_RDLCK;
    p.l_whence = SEEK_SET;
    p.l_start = 0;
    p.l_len = 1;
    if (fcntl(fd, F_SETLKW, &p))
    {
        sub_context_ty *scp;

        scp = sub_context_new();
        sub_errno_set(scp);
        sub_var_set(scp, "File_Name", "%s", fn);
        fatal_intl(scp, i18n("lock \"$filename\" shared: $errno"));
        /* NOTREACHED */
    }
}


static void
flock_exclusive(int fd, char *fn)
{
    struct flock    p;

    memset(&p, 0, sizeof(p));
    p.l_type = F_WRLCK;
    p.l_whence = SEEK_SET;
    p.l_start = 0;
    p.l_len = 1;
    if (fcntl(fd, F_SETLKW, &p))
    {
        sub_context_ty *scp;

        scp = sub_context_new();
        sub_errno_set(scp);
        sub_var_set(scp, "File_Name", "%s", fn);
        fatal_intl(scp, i18n("lock \"$filename\" exclusive: $errno"));
        /* NOTREACHED */
    }
}


static void
flock_release(int fd, char *fn)
{
    struct flock    p;

    memset(&p, 0, sizeof(p));
    p.l_type = F_UNLCK;
    p.l_whence = SEEK_SET;
    p.l_start = 0;
    p.l_len = 1;
    if (fcntl(fd, F_SETLKW, &p))
    {
        sub_context_ty *scp;

        scp = sub_context_new();
        sub_errno_set(scp);
        sub_var_set(scp, "File_Name", "%s", fn);
        fatal_intl(scp, i18n("unlock \"$filename\": $errno"));
        /* NOTREACHED */
    }
}


/*
 * NAME
 *      cache_read - read the cache file into the cache
 *
 * SYNOPSIS
 *      void cache_read(void);
 *
 * DESCRIPTION
 *      The cache_read function is used to read the cache file into the cache.
 *
 * CAVEATS
 *      If the cache file is not there, it is as iff the cache file
 *      contained an image of an empty cache.  I.e. nothing happens,
 *      but it is not an error.
 */

void
cache_read(void)
{
    str_hash_ty     nitems;
    str_hash_ty     j;
    FILE            *fp;
    char            *filename;

    /*
     * open the cache file.
     * if it's not there, quietly slink away
     */
    filename = build_filename();
    if (!os_exists(filename))
        return;
    fp = fopen_and_check(filename, "rb");

    /*
     * Take a shared lock: many processes may read simultaneously.
     */
    flock_shared(fileno(fp), filename);

    /*
     * get the number of entries in the file
     */
    if (fread_sane(fp, &nitems, sizeof(nitems)))
        fatal_intl_read(filename);

    /*
     * read each entry in the file
     */
    for (j = 0; j < nitems; ++j)
    {
        if (cache_read_item(fp))
            fatal_intl_read(filename);
    }

    /*
     * Release the file lock.
     */
    flock_release(fileno(fp), filename);

    /*
     * all done
     */
    fclose_and_check(fp, filename);
}


/*
 * NAME
 *      fwrite_sane - a saner version of fwrite
 *
 * SYNOPSIS
 *      int fwrite_sane(FILE *fp, void *buf, size_t buflen);
 *
 * DESCRIPTION
 *      The fwrite_sane function is used to write data to a file.
 *
 * ARGUMENTS
 *      fp      - file to write to
 *      buf     - pointer to data to write
 *      buflen  - number of bytes in data
 *
 * RETURNS
 *      0 on success, -1 on any error
 */

static int
fwrite_sane(FILE *fp, void *buf, size_t buflen)
{
    if (fwrite(buf, 1, buflen, fp) != buflen)
        return -1;
    return 0;
}


/*
 * NAME
 *      cache_write_string - write a string to a file
 *
 * SYNOPSIS
 *      int cache_write_string(FILE *fp, string_ty *s);
 *
 * DESCRIPTION
 *      The cache_write_string function is used to write a string to a file.
 *
 * ARGUMENTS
 *      fp      - file to write
 *      s       - string to be written
 *
 * RETURNS
 *      0 on success, -1 on any error
 *
 * CAVEATS
 *      Must be symmetric with cache_read_string above.
 */

static int
cache_write_string(FILE *fp, string_ty *s)
{
    if (fwrite_sane(fp, &s->str_length, sizeof(s->str_length)))
        return -1;
    if (fwrite_sane(fp, s->str_text, s->str_length))
        return -1;
    return 0;
}


/*
 * NAME
 *      cache_write_item - write cache item to cache file
 *
 * SYNOPSIS
 *      int cache_write_item(FILE *fp, cache_ty *cp);
 *
 * DESCRIPTION
 *      The cache_write_item function is used to write a cache
 *      item to a cache  file.
 *
 * ARGUMENTS
 *      fp      - file to write
 *      cp      - pointer to cache item to write
 *
 * RETURNS
 *      0 on success, -1 on any error
 *
 * CAVEATS
 *      Must be symmetric with cache_read_item above.
 */

static int
cache_write_item(FILE *fp, string_ty *key, cache_ty *cp)
{
    size_t          j;

    if (cache_write_string(fp, key))
        return -1;
    if (fwrite_sane(fp, &cp->st, sizeof(cp->st)))
        return -1;
    if
    (
        fwrite_sane
        (
            fp,
            &cp->ingredients.nstrings,
            sizeof(cp->ingredients.nstrings)
        )
    )
        return -1;
    for (j = 0; j < cp->ingredients.nstrings; ++j)
        if (cache_write_string(fp, cp->ingredients.string[j]))
            return -1;
    return 0;
}


static void
walk(symtab_ty *stp, string_ty *key, void *data, void *arg)
{
    cache_ty        *cp;
    FILE            *fp;

    (void)stp;
    cp = data;
    fp = arg;
    cache_write_item(fp, key, cp);
}


/*
 * NAME
 *      cache_write - write cache to file
 *
 * SYNOPSIS
 *      void cache_write(void);
 *
 * DESCRIPTION
 *      The cache_write function is used to write the memory image
 *      of the cache into a disk file.
 *
 * CAVEATS
 *      The cache file is in the current directory.
 */

void
cache_write(void)
{
    FILE            *fp;
    char            *filename;

    /*
     * don't change the file if we don't have to
     */
    if (!need_to_write)
        return;
    need_to_write = 0;

    /*
     * open the cache file
     */
    filename = build_filename();
    fp = fopen_and_check(filename, "wb");

    /*
     * Take an exclusive lock: only one process may take the lock at
     * a time.  (Potentially, we can miss updates if there are
     * several parallel updates.  We will simply do them again, next
     * time.)
     */
    flock_exclusive(fileno(fp), filename);

    /*
     * write the number of entries to the file
     */
    if (fwrite_sane(fp, &symtab->hash_load, sizeof(symtab->hash_load)))
        fatal_intl_write(filename);

    /*
     * write each cache entry to the file
     */
    symtab_walk(symtab, walk, fp);
    fflush_and_check(fp, filename);

    /*
     * Release the file lock.
     */
    flock_release(fileno(fp), filename);

    /*
     * close the cache file
     */
    fclose_and_check(fp, filename);
}


/*
 * NAME
 *      cache_update_notify - cache has changed
 *
 * SYNOPSIS
 *      void cache_update_nitify(void);
 *
 * DESCRIPTION
 *      The cache_update_notify function is called whenever the contents
 *      of the cache is changed.  This notifies the cache_write function
 *      that it needs to rewrite the cache file.
 */

void
cache_update_notify(void)
{
    need_to_write = 1;
}
