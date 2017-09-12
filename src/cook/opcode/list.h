/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 2001, 2006-2008 Peter Miller
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

#ifndef COOK_OPCODE_LIST_H
#define COOK_OPCODE_LIST_H

#include <common/ac/stddef.h>
#include <common/main.h>

struct match_ty; /* existence */

typedef struct opcode_list_ty opcode_list_ty;
struct opcode_list_ty
{
        long            reference_count;
        size_t          length;
        size_t          maximum;
        struct opcode_ty **list;
        struct opcode_label_ty *break_label;
        struct opcode_label_ty *continue_label;
        struct opcode_label_ty *return_label;
};


opcode_list_ty *opcode_list_new(void);
opcode_list_ty *opcode_list_copy(opcode_list_ty *);
void opcode_list_delete(opcode_list_ty *);
void opcode_list_append(opcode_list_ty *, struct opcode_ty *);
void opcode_list_disassemble(opcode_list_ty *);

struct string_list_ty *opcode_list_run(opcode_list_ty *,
        const struct match_ty *);
int opcode_list_run_bool(opcode_list_ty *, const struct match_ty *);

#endif /* COOK_OPCODE_LIST_H */
