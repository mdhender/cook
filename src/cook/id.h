/*
 *	cook - file construction tool
 *	Copyright (C) 1990, 1991, 1992, 1993, 1994, 1997, 1999 Peter Miller;
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
 * MANIFEST: interface definition for cook/id.c
 */

#ifndef COOK_ID_H
#define COOK_ID_H

#include <str_list.h>

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

void id_initialize _((void));
void id_reset _((void));
void id_dump _((char *));

int id_interpret _((id_ty *, struct opcode_context_ty *,
	const struct expr_position_ty *));
int id_interpret_script _((id_ty *, struct opcode_context_ty *,
	const struct expr_position_ty *));

#endif /* COOK_ID_H */
