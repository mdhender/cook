/*
 *      cook - file construction tool
 *      Copyright (C) 1994, 1997, 1999, 2006-2008 Peter Miller
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

#include <common/fp/len.h>


typedef struct len_ty len_ty;
struct len_ty
{
    FINGERPRINT_BASE_CLASS
    unsigned long   len;
};


static void
len_constructor(fingerprint_ty *p)
{
    len_ty          *f;

    f = (len_ty *)p;
    f->len = 0;
}


static void
len_destructor(fingerprint_ty *p)
{
    (void)p;
}


static void
len_addn(fingerprint_ty *p, const void *s, size_t n)
{
    len_ty          *f;

    (void)p;
    (void)s;
    f = (len_ty *)p;
    f->len += n;
}


static int
len_hash(fingerprint_ty *p, unsigned char *h, size_t h_len)
{
    len_ty          *f;
    unsigned long   n;
    int             j;

    (void)h_len;

    /*
     * return length as 5 bytes (yes, five)
     */
    f = (len_ty *)p;
    n = f->len;
    f->len = 0;
    for (j = 0; j < 5; ++j)
    {
        h[j] = n & 255;
        n >>= 8;
    }
    return 5;
}


static void
len_sum(fingerprint_ty *p, char *obuf, size_t obuf_len)
{
    len_ty          *f;

    f = (len_ty *)p;
    snprintf(obuf, obuf_len, "%8lu", f->len);
}


fingerprint_methods_ty fp_len =
{
    sizeof(len_ty),
    "length",
    len_constructor,
    len_destructor,
    len_addn,
    len_hash,
    len_sum
};
