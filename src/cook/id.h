/*
 *      cook - file construction tool
 *      Copyright (C) 1990-1994, 1997, 1999, 2006-2008 Peter Miller
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

#ifndef COOK_ID_H
#define COOK_ID_H

#include <common/str_list.h>

struct expr_position_ty; /* existence */
struct opcode_context_ty; /* existence */

extern string_ty *id_need;
extern string_ty *id_younger;
extern string_ty *id_targets;
extern string_ty *id_target;
extern string_ty *id_search_list;

typedef struct id_ty id_ty;
struct id_ty
{
        struct id_method_ty *method;
};

void id_initialize(void);
void id_reset(void);
void id_dump(char *);

int id_interpret(id_ty *, struct opcode_context_ty *,
        const struct expr_position_ty *);
int id_interpret_script(id_ty *, struct opcode_context_ty *,
        const struct expr_position_ty *);

#endif /* COOK_ID_H */
