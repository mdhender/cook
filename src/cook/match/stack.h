/*
 *	cook - file construction tool
 *	Copyright (C) 1999 Peter Miller;
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
 * MANIFEST: interface definition for cook/match/stack.c
 */

#ifndef COOK_MATCH_STACK_H
#define COOK_MATCH_STACK_H

#include <ac/stddef.h>

#include <match.h>

typedef struct match_stack_ty match_stack_ty;
struct match_stack_ty
{
	size_t		stack_depth;
	size_t		stack_depth_max;
	const match_ty	**stack;
};

match_stack_ty *match_stack_new _((void));
void match_stack_delete _((match_stack_ty *));
void match_stack_push _((match_stack_ty *, const match_ty *));
const match_ty *match_stack_pop _((match_stack_ty *));
const match_ty *match_stack_top _((const match_stack_ty *));

#endif /* COOK_MATCH_STACK_H */
