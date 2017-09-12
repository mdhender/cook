/*
 *      cook - file construction tool
 *      Copyright (C) 1994, 1997, 2003, 2006, 2007 Peter Miller;
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

#ifndef COMMON_SYMTAB_H
#define COMMON_SYMTAB_H

#include <common/str.h>

typedef struct symtab_row_ty symtab_row_ty;
struct symtab_row_ty
{
        string_ty       *key;
        void            *data;
        symtab_row_ty   *overflow;
};

typedef struct symtab_ty symtab_ty;
struct symtab_ty
{
        void            (*reap)(void *);
        symtab_row_ty   **hash_table;
        str_hash_ty     hash_modulus;
        str_hash_ty     hash_mask;
        str_hash_ty     hash_load;
};

symtab_ty *symtab_alloc(unsigned);
void symtab_free(symtab_ty *);
void *symtab_query(symtab_ty *, string_ty *);
void *symtab_query_fuzzy(symtab_ty *, string_ty *, string_ty **);
void *symtab_query_fuzzyN(symtab_ty **, size_t, string_ty *, string_ty **);
void symtab_assign(symtab_ty *, string_ty *, void *);
void symtab_assign_push(symtab_ty *, string_ty *, void *);
void symtab_delete(symtab_ty *, string_ty *);
void symtab_dump(symtab_ty *, char *);
void symtab_walk(symtab_ty *stp, void (*func)(symtab_ty *stp,
        string_ty *key, void *data, void *arg), void *arg);

#endif /* COMMON_SYMTAB_H */
