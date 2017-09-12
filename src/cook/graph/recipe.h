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

#ifndef COOK_GRAPH_RECIPE_H
#define COOK_GRAPH_RECIPE_H

#include <common/ac/stddef.h>
#include <common/main.h>

typedef struct graph_recipe_ty graph_recipe_ty;
struct graph_recipe_ty
{
        long            reference_count;
        int             id;
        struct recipe_ty *rp;
        struct match_ty *mp;
        struct graph_file_list_nrc_ty *input;
        struct graph_file_list_nrc_ty *output;
        size_t          input_satisfied;        /* used by graph_walk */
        long            input_uptodate; /* used by graph_walk */
        struct opcode_context_ty *ocp;  /* used by graph_run */
        struct string_list_ty *single_thread;
        struct string_list_ty *host_binding;
        int             multi_forced; /* used by graph_walk */
};

graph_recipe_ty *graph_recipe_new(struct recipe_ty *);
void graph_recipe_delete(graph_recipe_ty *);
graph_recipe_ty *graph_recipe_copy(graph_recipe_ty *);

int graph_recipe_getpid(graph_recipe_ty *);
void graph_recipe_waited(graph_recipe_ty *, int);

#endif /* COOK_GRAPH_RECIPE_H */
