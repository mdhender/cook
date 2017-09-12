/*
 *	cook - file construction tool
 *	Copyright (C) 1997, 2001 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: interface definition for cook/opcode.c
 */

#ifndef COOK_OPCODE_H
#define COOK_OPCODE_H

#include <opcode/status.h>

struct opcode_context_ty; /* existence */

typedef struct opcode_ty opcode_ty;
struct opcode_ty
{
	struct opcode_method_ty *method;
};

void opcode_delete _((opcode_ty *));
opcode_status_ty opcode_execute _((const opcode_ty *,
	struct opcode_context_ty *));
opcode_status_ty opcode_script _((const opcode_ty *,
	struct opcode_context_ty *));
void opcode_disassemble _((const opcode_ty *));

#endif /* COOK_OPCODE_H */
