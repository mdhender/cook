/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 2006-2008 Peter Miller
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 3 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program. If not, see
 *      <http://www.gnu.org/licenses/>.
 */

#ifndef COOK_GRAPH_BUILD_H
#define COOK_GRAPH_BUILD_H

#include <common/main.h>

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

graph_build_status_ty graph_build(struct graph_ty *, struct string_ty *,
        graph_build_preference_ty, int);

graph_build_status_ty graph_build_list(struct graph_ty *,
        struct string_list_ty *, graph_build_preference_ty, int);

#endif /* COOK_GRAPH_BUILD_H */
