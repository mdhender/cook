/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 2006, 2007 Peter Miller;
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

#ifndef COOK_RECIPE_H
#define COOK_RECIPE_H

#include <cook/expr/position.h>

typedef struct recipe_ty recipe_ty;
struct recipe_ty
{
        long            reference_count;
        struct string_list_ty *target;
        struct opcode_list_ty *need1;
        struct opcode_list_ty *need2;
        struct flag_ty  *flags;
        int             multiple;
        struct opcode_list_ty *precondition;
        struct opcode_list_ty *single_thread;
        struct opcode_list_ty *host_binding;
        struct opcode_list_ty *out_of_date;
        struct opcode_list_ty *up_to_date;
        expr_position_ty pos;           /* for tracing and debugging */
        int             implicit;
        int             inhibit;        /* for graph generation */
};

recipe_ty *recipe_new(struct string_list_ty *, struct opcode_list_ty *,
        struct opcode_list_ty *, struct flag_ty *, int,
        struct opcode_list_ty *, struct opcode_list_ty *,
        struct opcode_list_ty *, struct opcode_list_ty *,
        struct opcode_list_ty *, const expr_position_ty *);
void recipe_delete(recipe_ty *);
recipe_ty *recipe_copy(recipe_ty *);

void recipe_flags_set(recipe_ty *);

#endif /* COOK_RECIPE_H */
