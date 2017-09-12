/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 1998, 2003, 2006, 2007 Peter Miller;
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

#ifndef COOK_GRAPH_FILE_PAIR_H
#define COOK_GRAPH_FILE_PAIR_H

#include <common/main.h>

struct expr_position_ty; /* existence */
struct string_list_ty; /* existence */
struct string_ty; /* existence */

typedef struct graph_file_pair_ty graph_file_pair_ty;
struct graph_file_pair_ty
{
        struct symtab_ty *stp;
        struct symtab_ty *foreign_derived;
};

graph_file_pair_ty *graph_file_pair_new(struct string_list_ty *);
void graph_file_pair_delete(graph_file_pair_ty *);
void graph_file_pair_remember(graph_file_pair_ty *, struct string_ty *,
        struct string_ty *, const struct expr_position_ty *);
void graph_file_pair_remember_tlist(graph_file_pair_ty *,
        struct string_list_ty *, struct string_ty *,
        const struct expr_position_ty *);
void graph_file_pair_remember_lists(graph_file_pair_ty *,
        struct string_list_ty *, struct string_list_ty *,
        const struct expr_position_ty *);
void graph_file_pair_check(graph_file_pair_ty *, struct string_ty *,
        struct string_ty *, struct graph_ty *);
void graph_file_pair_foreign_derived(graph_file_pair_ty *,
        const struct string_list_ty *);
int graph_file_pair_exists(graph_file_pair_ty *, struct string_ty *,
        struct string_ty *);

#endif /* COOK_GRAPH_FILE_PAIR_H */
