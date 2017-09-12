/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 2001, 2006, 2007 Peter Miller;
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

#ifndef COOK_OPCODE_PRIVATE_H
#define COOK_OPCODE_PRIVATE_H

#include <cook/opcode.h>

struct opcode_context_ty; /* existence */

typedef struct opcode_method_ty opcode_method_ty;
struct opcode_method_ty
{
        char            *name;
        int             size;
        void            (*destructor)(opcode_ty *);
        opcode_status_ty (*execute)(const opcode_ty *,
                                struct opcode_context_ty *);
        opcode_status_ty (*script)(const opcode_ty *,
                                struct opcode_context_ty *);
        void            (*disassemble)(const opcode_ty *);
};

opcode_ty *opcode_new(opcode_method_ty *);

#endif /* COOK_OPCODE_PRIVATE_H */
