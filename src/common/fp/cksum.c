/*
 *      cook - file construction tool
 *      Copyright (C) 1994, 1997, 2006, 2007 Peter Miller;
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

#include <common/ac/string.h>

#include <common/fp/cksum.h>
#include <common/fp/crc32.h>
#include <common/fp/len.h>

typedef struct cksum_ty cksum_ty;
struct cksum_ty
{
    FINGERPRINT_BASE_CLASS
    fingerprint_ty  *crc32;
    fingerprint_ty  *len;
};


static void
cksum_constructor(fingerprint_ty *p)
{
    cksum_ty        *f;

    f = (cksum_ty *)p;
    f->crc32 = fingerprint_new(&fp_crc32);
    f->len = fingerprint_new(&fp_len);
}


static void
cksum_destructor(fingerprint_ty *p)
{
    cksum_ty        *f;

    f = (cksum_ty *)p;
    fingerprint_delete(f->crc32);
    fingerprint_delete(f->len);
}


static void
cksum_addn(fingerprint_ty *p, const void *s, size_t n)
{
    cksum_ty        *f;

    f = (cksum_ty *)p;
    fingerprint_addn(f->crc32, s, n);
    fingerprint_addn(f->len, s, n);
}


static int
cksum_hash(fingerprint_ty *p, unsigned char *h, size_t h_len)
{
    cksum_ty        *f;
    int             nbytes;
    unsigned char   *obuf;

    f = (cksum_ty *)p;
    obuf = h;
    nbytes = fingerprint_hash(f->crc32, h, h_len);
    h += nbytes;
    h_len -= nbytes;
    nbytes = fingerprint_hash(f->len, h, h_len);
    h += nbytes;
    h_len -= nbytes;
    return (h - obuf);
}


static void
cksum_sum(fingerprint_ty *p, char *s, size_t len)
{
    cksum_ty        *f;
    size_t          len1;

    f = (cksum_ty *)p;
    fingerprint_sum(f->crc32, s, len);
    len1 = strlen(s);
    s += len1;
    len -= len1;
    *s++ = ' ';
    --len;
    fingerprint_sum(f->len, s, len);
}


fingerprint_methods_ty fp_cksum =
{
    sizeof(cksum_ty),
    "cksum",
    cksum_constructor,
    cksum_destructor,
    cksum_addn,
    cksum_hash,
    cksum_sum
};
