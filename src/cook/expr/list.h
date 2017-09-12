/*
 *	cook - file construction tool
 *	Copyright (C) 1997, 1999, 2001 Peter Miller;
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
 * MANIFEST: interface definition for cook/expr/list.c
 */

#ifndef COOK_EXPR_LIST_H
#define COOK_EXPR_LIST_H

#include <ac/stddef.h>
#include <main.h>

struct expr_ty; /* existence */
struct opcode_list_ty; /* existence */
struct match_ty; /* existence */


typedef struct expr_list_ty expr_list_ty;
struct	expr_list_ty
{
	size_t		el_nexprs;
	size_t		el_nexprs_max;
	struct expr_ty	**el_expr;
};


void expr_list_append _((expr_list_ty *, struct expr_ty *));
void expr_list_destructor _((expr_list_ty *));
void expr_list_delete _((expr_list_ty *));
void expr_list_copy_constructor _((expr_list_ty *, const expr_list_ty *));
struct string_list_ty *expr_list_evaluate _((const expr_list_ty *,
	const struct match_ty *));
void expr_list_constructor _((expr_list_ty *));
expr_list_ty *expr_list_new _((void));
void expr_list_code_generate _((const expr_list_ty *, struct opcode_list_ty *));
struct expr_position_ty *expr_list_position _((const expr_list_ty *));

#endif /* COOK_EXPR_LIST_H */
