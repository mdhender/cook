/*
 *	cook - file construction tool
 *	Copyright (C) 1997 Peter Miller;
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
 * MANIFEST: interface definition for cook/opcode/recipe.c
 */

#ifndef COOK_OPCODE_RECIPE_H
#define COOK_OPCODE_RECIPE_H

#include <main.h>

struct opcode_list_ty; /* existence */
struct expr_position_ty; /* existence */

struct opcode_ty *opcode_recipe_new _((
	struct opcode_list_ty *,	/* need1 */
	struct opcode_list_ty *,	/* need2 */
	struct opcode_list_ty *,	/* precondition */
	int,				/* multiple */
	struct opcode_list_ty *,	/* single_thread */
	struct opcode_list_ty *,	/* host_binding */
	struct opcode_list_ty *,	/* out_of_date */
	struct opcode_list_ty *,	/* up_to_date */
	struct expr_position_ty *));

#endif /* COOK_OPCODE_RECIPE_H */
