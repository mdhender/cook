/*
 *      cook - file construction tool
 *      Copyright (C) 1994, 1997, 2006-2008 Peter Miller
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

#ifndef COMMON_FP_H
#define COMMON_FP_H

#include <common/ac/stddef.h>

#include <common/ac/stddef.h>

struct fingerprint_methods_ty; /* forward */

#define FINGERPRINT_BASE_CLASS \
    struct fingerprint_methods_ty *method;

typedef struct fingerprint_ty fingerprint_ty;
struct fingerprint_ty
{
    FINGERPRINT_BASE_CLASS
};

typedef struct fingerprint_methods_ty fingerprint_methods_ty;
struct fingerprint_methods_ty
{
    long            size;
    char            *name;
    void (*constructor)(fingerprint_ty *);
    void (*destructor)(fingerprint_ty *);
    void (*addn)(fingerprint_ty *, const void *, size_t);
    int (*hash)(fingerprint_ty *, unsigned char *, size_t);
    void (*sum)(fingerprint_ty *, char *, size_t);
};

fingerprint_ty *fingerprint_new(fingerprint_methods_ty *);
void fingerprint_delete(fingerprint_ty *);
void fingerprint_add(fingerprint_ty *, int);
int fingerprint_file_hash(fingerprint_ty *, const char *, unsigned char *,
    size_t);
int fingerprint_file_sum(fingerprint_ty *fp, const char *fn, char *obuf,
    size_t obuf_size);

#define fingerprint_addn(p, s, n) \
    (p)->method->addn((p), (s), (n))
#define fingerprint_hash(p, s, n) \
    (p)->method->hash((p), (s), (n))
#define fingerprint_sum(p, s, n) \
    (p)->method->sum((p), (s), (n))

#endif /* COMMON_FP_H */
