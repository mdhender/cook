/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 1999, 2006, 2007 Peter Miller;
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

#ifndef COOK_STMT_LIST_H
#define COOK_STMT_LIST_H

#include <common/ac/stddef.h>
#include <common/main.h>

struct stmt_ty;

typedef struct stmt_list_ty stmt_list_ty;
struct stmt_list_ty
{
        size_t          sl_nstmts;
        size_t          sl_nstmts_max;
        struct stmt_ty  **sl_stmt;
};

void stmt_list_constructor(stmt_list_ty *);
void stmt_list_copy_constructor(stmt_list_ty *, const stmt_list_ty *);
void stmt_list_destructor(stmt_list_ty *);
void stmt_list_append(stmt_list_ty *, struct stmt_ty *);

stmt_list_ty *stmt_list_new(void);
void stmt_list_delete(stmt_list_ty *);

#endif /* COOK_STMT_LIST_H */
