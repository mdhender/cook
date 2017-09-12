/*
 *	cook - file construction tool
 *	Copyright (C) 1997, 2001 Peter Miller;
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
 * MANIFEST: interface definition for cook/graph/file_list.c
 */

#ifndef COOK_GRAPH_FILE_LIST_H
#define COOK_GRAPH_FILE_LIST_H

#include <ac/stddef.h>
#include <main.h>
#include <graph/edge_type.h>

typedef struct graph_file_and_type_ty graph_file_and_type_ty;
struct graph_file_and_type_ty
{
	struct graph_file_ty *file;
	edge_type_ty	edge_type;
};

typedef struct graph_file_list_ty graph_file_list_ty;
struct graph_file_list_ty
{
	size_t		nfiles;
	size_t		nfiles_max;
	graph_file_and_type_ty *item;
};

void graph_file_list_constructor _((graph_file_list_ty *));
void graph_file_list_copy_constructor _((graph_file_list_ty *,
	graph_file_list_ty *));
void graph_file_list_destructor _((graph_file_list_ty *));

void graph_file_list_append _((graph_file_list_ty *, struct graph_file_ty *,
	edge_type_ty));
void graph_file_list_append_list _((graph_file_list_ty *,
	struct graph_file_list_ty *));

graph_file_list_ty *graph_file_list_new _((void));
void graph_file_list_delete _((graph_file_list_ty *));


/*
 * again, this time without touching the reference counts...
 */
typedef struct graph_file_list_nrc_ty graph_file_list_nrc_ty;
struct graph_file_list_nrc_ty
{
	size_t		nfiles;
	size_t		nfiles_max;
	graph_file_and_type_ty *item;
};

void graph_file_list_nrc_constructor _((graph_file_list_nrc_ty *));
void graph_file_list_nrc_copy_constructor _((graph_file_list_nrc_ty *,
	graph_file_list_nrc_ty *));
void graph_file_list_nrc_destructor _((graph_file_list_nrc_ty *));

void graph_file_list_nrc_append _((graph_file_list_nrc_ty *,
	struct graph_file_ty *, edge_type_ty));
void graph_file_list_nrc_append_list _((graph_file_list_nrc_ty *,
	struct graph_file_list_nrc_ty *));

graph_file_list_nrc_ty *graph_file_list_nrc_new _((void));
void graph_file_list_nrc_delete _((graph_file_list_nrc_ty *));

#endif /* COOK_GRAPH_FILE_LIST_H */
