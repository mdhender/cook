/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 2001, 2003, 2006-2008 Peter Miller
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

#ifndef COMMON_ITAB_H
#define COMMON_ITAB_H

#include <common/main.h>

typedef long itab_key_ty;

typedef struct itab_row_ty itab_row_ty;
struct itab_row_ty
{
        itab_key_ty     key;
        void            *data;
        itab_row_ty     *overflow;
};

typedef struct itab_ty itab_ty;
struct itab_ty
{
        void            (*reap)(void *);
        itab_row_ty     **hash_table;
        itab_key_ty     hash_modulus;
        itab_key_ty     hash_mask;
        itab_key_ty     load;
};

itab_ty *itab_alloc(int);
void itab_free(itab_ty *);
void *itab_query(itab_ty *, itab_key_ty);
void itab_assign(itab_ty *, itab_key_ty, void *);
void itab_delete(itab_ty *, itab_key_ty);
void itab_walk(itab_ty *,
        void (*)(itab_ty *, itab_key_ty, void *, void *),
        void *);

#endif /* COMMON_ITAB_H */
