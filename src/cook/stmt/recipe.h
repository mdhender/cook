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
 * MANIFEST: interface definition for cook/stmt/recipe.c
 */

#ifndef COOK_STMT_RECIPE_H
#define COOK_STMT_RECIPE_H

#include <main.h>

struct expr_ty;
struct expr_list_ty;
struct expr_position_ty;
struct stmt_ty;

struct stmt_ty *stmt_recipe_new _((struct expr_list_ty *,
	struct expr_list_ty *, struct expr_list_ty *, struct expr_list_ty *,
	int, struct expr_ty *, struct expr_list_ty *, struct expr_list_ty *,
	struct stmt_ty *, struct stmt_ty *, struct expr_position_ty *));

#endif /* COOK_STMT_RECIPE_H */
