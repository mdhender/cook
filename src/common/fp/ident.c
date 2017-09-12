/*
 *      cook - file construction tool
 *      Copyright (C) 1995, 1997, 1999, 2006, 2007 Peter Miller;
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

#include <common/ac/stdio.h>
#include <common/ac/string.h>

#include <common/fp/ident.h>
#include <common/fp/combined.h>
#include <common/fp/crc32.h>
#include <common/trace.h>


typedef struct ident_ty ident_ty;
struct ident_ty
{
    FINGERPRINT_BASE_CLASS
    fingerprint_ty  *combined;
    fingerprint_ty  *crc32;
};


static void
ident_constructor(fingerprint_ty *p)
{
    ident_ty        *f;

    trace(("constructor\n"));
    f = (ident_ty *)p;
    f->combined = fingerprint_new(&fp_combined);
    f->crc32 = fingerprint_new(&fp_crc32);
}


static void
ident_destructor(fingerprint_ty *p)
{
    ident_ty        *f;

    f = (ident_ty *)p;
    fingerprint_delete(f->combined);
    fingerprint_delete(f->crc32);
}


static void
ident_addn(fingerprint_ty *p, const void *s, size_t n)
{
    ident_ty        *f;

    f = (ident_ty *)p;
    fingerprint_addn(f->combined, s, n);
}


static int
ident_hash(fingerprint_ty *p, unsigned char *h, size_t h_len)
{
    ident_ty        *f;
    int             nbytes;
    unsigned char   t[1024];

    f = (ident_ty *)p;
    nbytes = fingerprint_hash(f->combined, t, sizeof(t));
    fingerprint_addn(f->crc32, t, nbytes);
    nbytes = fingerprint_hash(f->crc32, h, h_len);
    return nbytes;
}


static void
ident_sum(fingerprint_ty *p, char *s, size_t s_len)
{
    unsigned char   h[1024];
    unsigned long   x;
    int             nbytes;
    static char     digits[] = "0123456789";
    char            *cp;

    nbytes = ident_hash(p, h, sizeof(h));
    assert(nbytes == 4);

    /*
     * watch out for 64-bit longs vs 32-bit ints
     */
    x =
        (
            (unsigned long)h[0]
        |
            ((unsigned long)h[1] << 8)
        |
            ((unsigned long)h[2] << 16)
        |
            ((unsigned long)h[3] << 24)
        );
    snprintf(s, s_len, "%8.8lx", x);

    /*
     * some older stdio implementations don't grok the above format
     * string, so hunt down and kill and spaces.
     */
    for (cp = s; *cp; ++cp)
        if (*cp == ' ')
            *cp = '0';

    /*
     * This forces the first character to be a letter, so
     * the result is a valid identifier in most computer
     * languages. The strchr makes it not ASCII specific.
     */
    cp = strchr(digits, *s);
    if (cp)
        *s = "ghijklmnop"[cp - digits];
}


fingerprint_methods_ty fp_ident =
{
    sizeof(ident_ty),
    "identifier",
    ident_constructor,
    ident_destructor,
    ident_addn,
    ident_hash,
    ident_sum
};
