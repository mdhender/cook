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
 * MANIFEST: interface definition for cook/graph/build.c
 */

#ifndef COOK_GRAPH_BUILD_H
#define COOK_GRAPH_BUILD_H

#include <main.h>

struct graph_ty; /* existence */
struct string_ty; /* existence */

/*
 * This indicates the result of attempting to build a graph.
 */
enum graph_build_status_ty
{
	graph_build_status_success,
	graph_build_status_error,
	graph_build_status_backtrack
};
typedef enum graph_build_status_ty graph_build_status_ty;

/*
 * This controls the behaviour when a target cannot be built (or a
 * sub-target).  The choice is a fatal error reported on stderr and an
 * error status; OR silence and a backtrace return.
 */
enum graph_build_preference_ty
{
	graph_build_preference_error,
	graph_build_preference_backtrack
};
typedef enum graph_build_preference_ty graph_build_preference_ty;

graph_build_status_ty graph_build _((struct graph_ty *, struct string_ty *,
	graph_build_preference_ty, int));

graph_build_status_ty graph_build_list _((struct graph_ty *,
	struct string_list_ty *, graph_build_preference_ty, int));

#endif /* COOK_GRAPH_BUILD_H */
