/*
 *      cook - file construction tool
 *      Copyright (C) 1998, 2002, 2006, 2007 Peter Miller;
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

#ifndef COOK_CASCADE_H
#define COOK_CASCADE_H

#include <cook/expr/position.h>
#include <common/str_list.h>

typedef struct cascade_ty cascade_ty;
struct cascade_ty
{
        string_ty       *ingredient;
        expr_position_ty pos;
};

typedef struct cascade_list_ty cascade_list_ty;
struct cascade_list_ty
{
        size_t          length;
        size_t          maximum;
        cascade_ty      *list;
};

void cascade_list_constructor(cascade_list_ty *);
void cascade_list_destructor(cascade_list_ty *);

void cascade_reset(void);
void cascade_recipe(struct string_list_ty *,
        struct string_list_ty *, const struct expr_position_ty *);
void cascade_find(const string_list_ty *, cascade_list_ty *);
int cascade_used(void);

#endif /* COOK_CASCADE_H */
