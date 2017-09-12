/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 2006-2008 Peter Miller
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

#ifndef COOK_GRAPH_RECIPE_LIST_H
#define COOK_GRAPH_RECIPE_LIST_H

#include <common/ac/stddef.h>
#include <common/main.h>

typedef struct graph_recipe_list_ty graph_recipe_list_ty;
struct graph_recipe_list_ty
{
        size_t          nrecipes;
        size_t          nrecipes_max;
        struct graph_recipe_ty **recipe;
};

void graph_recipe_list_constructor(graph_recipe_list_ty *);
void graph_recipe_list_destructor(graph_recipe_list_ty *);

void graph_recipe_list_append(graph_recipe_list_ty *,
        struct graph_recipe_ty *);
void graph_recipe_list_append_list(graph_recipe_list_ty *,
        struct graph_recipe_list_ty *);

graph_recipe_list_ty *graph_recipe_list_new(void);
void graph_recipe_list_delete(graph_recipe_list_ty *);

/*
 * again, this time ignoring reference counts
 */
typedef struct graph_recipe_list_nrc_ty graph_recipe_list_nrc_ty;
struct graph_recipe_list_nrc_ty
{
        size_t          nrecipes;
        size_t          nrecipes_max;
        struct graph_recipe_ty **recipe;
};

void graph_recipe_list_nrc_constructor(graph_recipe_list_nrc_ty *);
void graph_recipe_list_nrc_destructor(graph_recipe_list_nrc_ty *);

void graph_recipe_list_nrc_append(graph_recipe_list_nrc_ty *,
        struct graph_recipe_ty *);
void graph_recipe_list_nrc_append_list(graph_recipe_list_nrc_ty *,
        struct graph_recipe_list_nrc_ty *);

graph_recipe_list_nrc_ty *graph_recipe_list_nrc_new(void);
void graph_recipe_list_nrc_delete(graph_recipe_list_nrc_ty *);

#endif /* COOK_GRAPH_RECIPE_LIST_H */
