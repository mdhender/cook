/*
 *	cook - file construction tool
 *	Copyright (C) 1998 Peter Miller;
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
 * MANIFEST: interface definition for cook/graph/leaf.c
 */

#ifndef COOK_GRAPH_LEAF_H
#define COOK_GRAPH_LEAF_H

#include <main.h>

enum leaf_ness_ty
{
	leaf_ness_error,
	leaf_ness_leaf_exists,
	leaf_ness_leaf_explicit,
	leaf_ness_interior_exists,
	leaf_ness_interior_explicit,
	leaf_ness_exterior_explicit,
	leaf_ness_indeterminate
};
typedef enum leaf_ness_ty leaf_ness_ty;

void leaf_reset _((void));
leaf_ness_ty leaf_query _((struct string_ty *, int));
const char *leaf_ness_name _((leaf_ness_ty));

#endif /* COOK_GRAPH_LEAF_H */
