/*
 *      cook - file construction tool
 *      Copyright (C) 1998, 2006-2008 Peter Miller
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

#ifndef COMMON_STRACC_H
#define COMMON_STRACC_H

#include <common/str.h>

typedef struct stracc stracc;
struct  stracc
{
        size_t          sa_max;         /* size of string accum buffer  */
        size_t          sa_len;         /* number of chars accumulated  */
        char            *sa_buf;        /* the string accum buffer      */
        int             sa_inuse;
};

stracc *stracc_new(void);
void stracc_constructor(stracc *);
void stracc_delete(stracc *);
void stracc_destructor(stracc *);
void sa_open(stracc *);
void sa_char(stracc *, int);
void sa_chars(stracc *, const char *, size_t);
string_ty *sa_close(stracc *);
size_t sa_mark(stracc *);
void sa_goto(stracc *, size_t);

#endif /* COMMON_STRACC_H */
