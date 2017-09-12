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

#ifndef COOK_RECIPE_LIST_H
#define COOK_RECIPE_LIST_H

#include <common/ac/stddef.h>
#include <common/main.h>

typedef struct recipe_list_ty recipe_list_ty;
struct recipe_list_ty
{
        size_t          nrecipes;
        size_t          nrecipes_max;
        struct recipe_ty **recipe;
};

void recipe_list_constructor(recipe_list_ty *);
void recipe_list_destructor(recipe_list_ty *);
void recipe_list_append(recipe_list_ty *, struct recipe_ty *);

recipe_list_ty *recipe_list_new(void);
void recipe_list_delete(recipe_list_ty *);

#endif /* COOK_RECIPE_LIST_H */
