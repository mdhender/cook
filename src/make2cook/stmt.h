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

#ifndef MAKE2COOK_STMT_H
#define MAKE2COOK_STMT_H

#include <common/ac/stddef.h>
#include <common/str_list.h>

struct stmt_ty;

typedef struct stmt_method_ty stmt_method_ty;
struct stmt_method_ty
{
        size_t          size;
        char            *name;
        void (*constructor)(struct stmt_ty *);
        void (*destructor)(struct stmt_ty *);
        void (*emit)(struct stmt_ty *);
        void (*regroup)(struct stmt_ty *);
        void (*sort)(struct stmt_ty *);
};

/*
 *      cdef    list of variables defined by a command
 *      mdef    list of variables overide defined by a command
 *      ref     list of variables referenced by a command
 *      rref    list of variables referenced by rule bodies
 */
#define STMT \
        stmt_method_ty  *method;        \
        string_list_ty  mdef;           \
        string_list_ty  cdef;           \
        string_list_ty  ref;            \
        string_list_ty  rref;           \
        int             white_space;

typedef struct stmt_ty stmt_ty;
struct stmt_ty
{
        STMT
};

stmt_ty *stmt_alloc(stmt_method_ty *);
void stmt_free(stmt_ty *);
void stmt_emit(stmt_ty *);
void stmt_variable_merge(stmt_ty *parent, stmt_ty *child);
void stmt_regroup(stmt_ty *);
void stmt_sort(stmt_ty *);

#endif /* MAKE2COOK_STMT_H */
