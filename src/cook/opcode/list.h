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
 * MANIFEST: interface definition for cook/opcode/list.c
 */

#ifndef COOK_OPCODE_LIST_H
#define COOK_OPCODE_LIST_H

#include <ac/stddef.h>
#include <main.h>

struct match_ty; /* existence */

typedef struct opcode_list_ty opcode_list_ty;
struct opcode_list_ty
{
	long		reference_count;
	size_t		length;
	size_t		maximum;
	struct opcode_ty **list;
	struct opcode_label_ty *break_label;
	struct opcode_label_ty *continue_label;
	struct opcode_label_ty *return_label;
};


opcode_list_ty *opcode_list_new _((void));
opcode_list_ty *opcode_list_copy _((opcode_list_ty *));
void opcode_list_delete _((opcode_list_ty *));
void opcode_list_append _((opcode_list_ty *, struct opcode_ty *));
void opcode_list_disassemble _((opcode_list_ty *));

struct string_list_ty *opcode_list_run _((opcode_list_ty *,
	const struct match_ty *));
int opcode_list_run_bool _((opcode_list_ty *, const struct match_ty *));

#endif /* COOK_OPCODE_LIST_H */
