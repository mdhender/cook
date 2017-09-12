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

#ifndef COOK_OPCODE_LABEL_H
#define COOK_OPCODE_LABEL_H

#include <common/ac/stddef.h>

#include <common/main.h>

typedef struct opcode_label_ty opcode_label_ty;
struct opcode_label_ty
{
        size_t          pc;
        size_t          npending;
        size_t          npending_max;
        size_t          **pending;
};

opcode_label_ty *opcode_label_new(void);
void opcode_label_delete(opcode_label_ty *);
void opcode_label_define(opcode_label_ty *, size_t);
void opcode_label_refer(opcode_label_ty *, size_t *);

#endif /* COOK_OPCODE_LABEL_H */
