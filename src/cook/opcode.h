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

#ifndef COOK_OPCODE_H
#define COOK_OPCODE_H

#include <cook/opcode/status.h>

struct opcode_context_ty; /* existence */

typedef struct opcode_ty opcode_ty;
struct opcode_ty
{
        struct opcode_method_ty *method;
};

void opcode_delete(opcode_ty *);
opcode_status_ty opcode_execute(const opcode_ty *,
        struct opcode_context_ty *);
opcode_status_ty opcode_script(const opcode_ty *,
        struct opcode_context_ty *);
void opcode_disassemble(const opcode_ty *);

#endif /* COOK_OPCODE_H */
