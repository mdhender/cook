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

#ifndef COOK_OPCODE_RECIPE_H
#define COOK_OPCODE_RECIPE_H

#include <common/main.h>

struct opcode_list_ty; /* existence */
struct expr_position_ty; /* existence */

struct opcode_ty *opcode_recipe_new(
        struct opcode_list_ty *need1,
        struct opcode_list_ty *need2,
        struct opcode_list_ty *precondition,
        int multiple,
        struct opcode_list_ty *single_thread,
        struct opcode_list_ty *host_binding,
        struct opcode_list_ty *out_of_date,
        struct opcode_list_ty *up_to_date,
        struct expr_position_ty *pos);

#endif /* COOK_OPCODE_RECIPE_H */
