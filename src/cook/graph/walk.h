/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 2006, 2007 Peter Miller;
 *      All rights reserved.
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

#ifndef COOK_GRAPH_WALK_H
#define COOK_GRAPH_WALK_H

#include <common/main.h>

struct graph_ty; /* existence */

enum graph_walk_status_ty
{
        graph_walk_status_done,
        graph_walk_status_done_stop,
        graph_walk_status_error,
        graph_walk_status_uptodate,
        graph_walk_status_uptodate_done,
        graph_walk_status_wait
};
typedef enum graph_walk_status_ty graph_walk_status_ty;

char *graph_walk_status_name(graph_walk_status_ty);

graph_walk_status_ty graph_walk(struct graph_ty *);
graph_walk_status_ty graph_walk_pairs(struct graph_ty *);
graph_walk_status_ty graph_walk_script(struct graph_ty *);
int graph_isit_uptodate(struct graph_ty *);

#endif /* COOK_GRAPH_WALK_H */
