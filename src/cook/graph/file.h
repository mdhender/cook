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

#ifndef COOK_GRAPH_FILE_H
#define COOK_GRAPH_FILE_H

#include <common/ac/stddef.h>
#include <common/main.h>

typedef struct graph_file_ty graph_file_ty;
struct graph_file_ty
{
        long            reference_count;
        struct string_ty *filename;
        struct graph_recipe_list_nrc_ty *input;
        struct graph_recipe_list_nrc_ty *output;
        long            pending;
        int             previous_backtrack;
        int             previous_error;
        long            mtime_oldest;   /* used by graph_recipe_run */
        size_t          input_satisfied; /* used by graph_walk */
        long            done;           /* used by graph_walk */
        size_t          input_uptodate; /* used by graph_walk */
        int             primary_target;
};

graph_file_ty *graph_file_new(struct string_ty *);
void graph_file_delete(graph_file_ty *);
graph_file_ty *graph_file_copy(graph_file_ty *);

#endif /* COOK_GRAPH_FILE_H */
