/*
 *	cook - file construction tool
 *	Copyright (C) 1997, 1998, 2001 Peter Miller;
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
 * MANIFEST: interface definition for cook/expr/position.c
 */

#ifndef COOK_EXPR_POSITION_H
#define COOK_EXPR_POSITION_H

#include <main.h>

struct sub_context_ty; /* existence */

typedef struct expr_position_ty expr_position_ty;
struct expr_position_ty
{
	struct string_ty *pos_name;
	int		pos_line;

	/*
	 * This is not very pretty.  I wanted to overload a colon to
	 * provide the position within the parse, and also how many
	 * colons (one or two).  The easiest place to do it was here.
	 */
	int		multi;
};

void expr_position_constructor _((expr_position_ty *, struct string_ty *, int));
void expr_position_constructorC _((expr_position_ty *, char *, int));
void expr_position_copy_constructor _((expr_position_ty *,
	const expr_position_ty *));
void expr_position_destructor _((expr_position_ty *));
void error_with_position _((const expr_position_ty *, struct sub_context_ty *,
	char *));

#endif /* COOK_EXPR_POSITION_H */
