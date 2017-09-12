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

#ifndef MAKE2COOK_STMT_ASSIGN_H
#define MAKE2COOK_STMT_ASSIGN_H

#include <make2cook/blob.h>
#include <make2cook/stmt.h>

enum
{
        stmt_assign_op_normal,
        stmt_assign_op_colon,
        stmt_assign_op_default,
        stmt_assign_op_plus
};

extern int stmt_assign_environment_variables;

stmt_ty *stmt_assign_alloc(int override, blob_ty *lhs, int op,
        blob_list_ty *rhs);
stmt_ty *stmt_assign_default(stmt_ty *);

#endif /* MAKE2COOK_STMT_ASSIGN_H */
