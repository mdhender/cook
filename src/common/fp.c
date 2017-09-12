/*
 *      cook - file construction tool
 *      Copyright (C) 1994, 1997, 1999, 2005-2008 Peter Miller
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
#include <common/ac/fcntl.h>
#include <common/ac/unistd.h>
#include <common/ac/dirent.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <common/fp.h>
#include <common/str_list.h>
#include <common/mem.h>


fingerprint_ty *
fingerprint_new(fingerprint_methods_ty *mp)
{
    fingerprint_ty  *fp;

    fp = mem_alloc(mp->size);
    fp->method = mp;
    mp->constructor(fp);
    return fp;
}


void
fingerprint_delete(fingerprint_ty *fp)
{
    fp->method->destructor(fp);
    mem_free(fp);
}


static int
fingerprint_scan_dir(fingerprint_ty *fp, const char *fn)
{
    DIR             *dp;
    string_list_ty  wl;
    string_ty       *s;
    size_t          j;

    /*
     * Read the directory contents, and remember the names.
     * Don't hash it just yet.
     */
    dp = opendir(fn);
    if (!dp)
        return -1;
    string_list_constructor(&wl);
    for (;;)
    {
        struct dirent   *dep;

        dep = readdir(dp);
        if (!dep)
            break;
        s = str_from_c(dep->d_name);
        string_list_append(&wl, s);
        str_free(s);
    }
    closedir(dp);

    /*
     * Sort the names before we hash anything.  That way, even if
     * the files in the directory get re-arranged, the hash stays
     * the same.  This is common when a file is deleted, and then
     * re-created in identical form.
     */
    string_list_sort(&wl);

    /*
     * Now hash the strings in the list.  This takes advantage of
     * the fact that all the strings are NUL terminated to introduce
     * a unique separator.
     */
    for (j = 0; j < wl.nstrings; ++j)
    {
        s = wl.string[j];
        fingerprint_addn(fp, (unsigned char *)s->str_text, s->str_length + 1);
    }
    string_list_destructor(&wl);
    return 0;
}

static int
fingerprint_scan(fingerprint_ty *fp, const char *fn)
{
    int             fd;
    unsigned char   ibuf[1024];
    long            nbytes;
    int             err;

    if (fn)
    {
        struct stat st;

        /*
         * The most common case is to obtain a fingerprint for
         * a file.  In the rare case that it is a directory,
         * we need to do things differently.
         */
        if (stat(fn, &st) == 0 && S_ISDIR(st.st_mode))
        {
            return fingerprint_scan_dir(fp, fn);
        }

        /*
         * Assume it's a normal file.
         * (Slow things will happen if, say, it's a tape device.)
         */
        fd = open(fn, O_RDONLY|O_BINARY, 0666);
        if (fd < 0)
            return -1;
    }
    else
    {
        fd = 0;
    }
    for (;;)
    {
        nbytes = read(fd, ibuf, sizeof(ibuf));
        if (nbytes < 0)
        {
            if (fn)
            {
                err = errno;
                close(fd);
                errno = err;
            }
            return -1;
        }
        if (nbytes == 0)
            break;
        fingerprint_addn(fp, ibuf, nbytes);
    }
    if (fn && close(fd) < 0)
        return -1;
    return 0;
}


int
fingerprint_file_hash(fingerprint_ty *fp, const char *fn, unsigned char *obuf,
    size_t obuf_len)
{
    if (fingerprint_scan(fp, fn))
        return -1;
    return fingerprint_hash(fp, obuf, obuf_len);
}


int
fingerprint_file_sum(fingerprint_ty *fp, const char *fn, char *obuf,
    size_t obuf_len)
{
    if (fingerprint_scan(fp, fn))
        return -1;
    fingerprint_sum(fp, obuf, obuf_len);
    return 0;
}


void
fingerprint_add(fingerprint_ty *p, int c)
{
    unsigned char   buf;

    buf = c;
    fingerprint_addn(p, &buf, 1);
}
