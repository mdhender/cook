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
 * MANIFEST: functions to manipulate graph file lists
 */

#include <graph/file.h>
#include <graph/file_list.h>
#include <mem.h>
#include <trace.h>


/*
 * NAME
 *	graph_file_list_constructor
 *
 * SYNOPSIS
 *	void graph_file_list_constructor(graph_file_list_ty *);
 *
 * DESCRIPTION
 *	The graph_file_list_constructor function is used to initialize
 *	a graph file list to empty.
 */

void
graph_file_list_constructor(gflp)
	graph_file_list_ty *gflp;
{
	trace(("graph_file_list_constructor(gflp = %8.8lX)\n{\n"/*}*/,
		(long)gflp));
	gflp->nfiles = 0;
	gflp->nfiles_max = 0;
	gflp->item = 0;
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	graph_file_list_copy_constructor
 *
 * SYNOPSIS
 *	void graph_file_list_copy_constructor(graph_file_list_ty *to,
 *		graph_file_list_ty*from);
 *
 * DESCRIPTION
 *	The graph_file_list_copy_constructor function is used to
 *	to initialize a graph file list by copying an existing list.
 */

void
graph_file_list_copy_constructor(gflp, from)
	graph_file_list_ty *gflp;
	graph_file_list_ty *from;
{
	trace(("graph_file_list_copy_constructor(gflp = %8.8lX)\n{\n"/*}*/,
		(long)gflp));
	graph_file_list_constructor(gflp);
	graph_file_list_append_list(gflp, from);
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	graph_file_list_destructor
 *
 * SYNOPSIS
 *	void graph_file_list_destructor(graph_file_list_ty *);
 *
 * DESCRIPTION
 *	The graph_file_list_destructor function is used to release the
 *	resources held by a graph file list.
 */

void
graph_file_list_destructor(gflp)
	graph_file_list_ty *gflp;
{
	size_t		j;

	trace(("graph_file_list_destructor(gflp = %8.8lX)\n{\n"/*}*/,
		(long)gflp));
	for (j = 0; j < gflp->nfiles; ++j)
		graph_file_delete(gflp->item[j].file);
	if (gflp->item)
		mem_free(gflp->item);
	gflp->nfiles = 0;
	gflp->nfiles_max = 0;
	gflp->item = 0;
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	graph_file_list_append
 *
 * SYNOPSIS
 *	void graph_file_list_append(graph_file_list_ty *, graph_file_ty *);
 *
 * DESCRIPTION
 *	The graph_file_list_append function is used to append a graph
 *	file to a list.  Non-unique copies are silently ignored.
 */

void
graph_file_list_append(gflp, gfp, et)
	graph_file_list_ty *gflp;
	graph_file_ty	*gfp;
	edge_type_ty	et;
{
	size_t		j;
	graph_file_and_type_ty *fat;

	trace(("graph_file_list_append(gflp = %8.8lX, gfp = %8.8lX)\n{\n"/*}*/,
		(long)gflp, (long)gfp));
	for (j = 0; j < gflp->nfiles; ++j)
	{
		fat = gflp->item + j;
		if (fat->file == gfp)
		{
			fat->edge_type |= et;
			trace((/*{*/"}\n"));
			return;
		}
	}
	if (gflp->nfiles >= gflp->nfiles_max)
	{
		size_t		nbytes;

		gflp->nfiles_max = gflp->nfiles_max * 2 + 4;
		nbytes = gflp->nfiles_max * sizeof(gflp->item[0]);
		gflp->item = mem_change_size(gflp->item, nbytes);
	}
	fat = gflp->item + gflp->nfiles++;
	fat->file = graph_file_copy(gfp);
	fat->edge_type = et;
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	graph_file_list_append_list
 *
 * SYNOPSIS
 *	void graph_file_list_append_list(graph_file_list_ty *to,
 *		graph_file_list_ty *from);
 *
 * DESCRIPTION
 *	The graph_file_list_append_list function is used to append a the
 *	graph files from one list to another list.  Non-unique copies
 *	are silently ignored.
 */

void
graph_file_list_append_list(to, from)
	graph_file_list_ty *to;
	graph_file_list_ty *from;
{
	size_t		j;

	trace(("graph_file_list_append_list(to = %8.8lX, \
from = %8.8lX)\n{\n"/*}*/,
		(long)to, (long)from));
	for (j = 0; j < from->nfiles; ++j)
	{
		graph_file_and_type_ty *fat;

		fat = from->item + j;
		graph_file_list_append(to, fat->file, fat->edge_type);
	}
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	graph_file_list_new
 *
 * SYNOPSIS
 *	graph_file_list_ty *graph_file_list_new(void);
 *
 * DESCRIPTION
 *	The graph_file_list_new function is used to allocate a new empty
 *	graph file list in dynamic memory.
 *
 * RETURNS
 *	graph_file_list_ty *
 *
 * CAVEAT
 *	Use graph_file_list_delete when you are done with it.
 */

graph_file_list_ty *
graph_file_list_new()
{
	graph_file_list_ty *gflp;

	trace(("graph_file_list_new()\n{\n"/*}*/));
	gflp = mem_alloc(sizeof(graph_file_list_ty));
	graph_file_list_constructor(gflp);
	trace(("return %8.8lX;\n", (long)gflp));
	trace((/*{*/"}\n"));
	return gflp;
}


/*
 * NAME
 *	graph_file_list_delete
 *
 * SYNOPSIS
 *	void graph_file_list_delete(graph_file_list_ty *);
 *
 * DESCRIPTION
 *	The graph_file_list_delete function is used to release the
 *	resources held by a graph file list in dynamic memory.
 */

void
graph_file_list_delete(gflp)
	graph_file_list_ty *gflp;
{
	trace(("graph_file_list_delete(gflp = %8.8lX)\n{\n"/*}*/, (long)gflp));
	graph_file_list_destructor(gflp);
	mem_free(gflp);
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	graph_file_list_nrc_constructor
 *
 * SYNOPSIS
 *	void graph_file_list_nrc_constructor(graph_file_list_ty *);
 *
 * DESCRIPTION
 *	The graph_file_list_nrc_constructor function is used to initialize
 *	a graph file list to empty.
 */

void
graph_file_list_nrc_constructor(gflp)
	graph_file_list_nrc_ty *gflp;
{
	trace(("graph_file_list_nrc_constructor(gflp = %8.8lX)\n{\n"/*}*/,
		(long)gflp));
	gflp->nfiles = 0;
	gflp->nfiles_max = 0;
	gflp->item = 0;
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	graph_file_list_nrc_copy_constructor
 *
 * SYNOPSIS
 *	void graph_file_list_nrc_copy_constructor(graph_file_list_nrc_ty *to,
 *		graph_file_list_nrc_ty*from);
 *
 * DESCRIPTION
 *	The graph_file_list_nrc_copy_constructor function is used to
 *	to initialize a graph file list by copying an existing list.
 *
 * CAVEAT
 *	Reference counts NOT updated.
 */

void
graph_file_list_nrc_copy_constructor(gflp, from)
	graph_file_list_nrc_ty *gflp;
	graph_file_list_nrc_ty *from;
{
	trace(("graph_file_list_nrc_copy_constructor(gflp = %8.8lX)\n{\n"/*}*/,
		(long)gflp));
	graph_file_list_nrc_constructor(gflp);
	graph_file_list_nrc_append_list(gflp, from);
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	graph_file_list_nrc_destructor
 *
 * SYNOPSIS
 *	void graph_file_list_nrc_destructor(graph_file_list_nrc_ty *);
 *
 * DESCRIPTION
 *	The graph_file_list_nrc_destructor function is used to release the
 *	resources held by a graph file list.
 *
 * CAVEAT
 *	Reference counts NOT updated.
 */

void
graph_file_list_nrc_destructor(gflp)
	graph_file_list_nrc_ty *gflp;
{
	trace(("graph_file_list_nrc_destructor(gflp = %8.8lX)\n{\n"/*}*/,
		(long)gflp));
	/* do not delete references */
	if (gflp->item)
		mem_free(gflp->item);
	gflp->nfiles = 0;
	gflp->nfiles_max = 0;
	gflp->item = 0;
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	graph_file_list_nrc_append
 *
 * SYNOPSIS
 *	void graph_file_list_nrc_append(graph_file_list_nrc_ty *,
 *		graph_file_ty *);
 *
 * DESCRIPTION
 *	The graph_file_list_nrc_append function is used to append a graph
 *	file to a list.  Non-unique copies are silently ignored.
 *
 * CAVEAT
 *	Reference counts NOT updated.
 */

void
graph_file_list_nrc_append(gflp, gfp, et)
	graph_file_list_nrc_ty *gflp;
	graph_file_ty	*gfp;
	edge_type_ty	et;
{
	size_t		j;
	graph_file_and_type_ty *fat;

	trace(("graph_file_list_nrc_append(gflp = %8.8lX, \
gfp = %8.8lX)\n{\n"/*}*/,
		(long)gflp, (long)gfp));
	for (j = 0; j < gflp->nfiles; ++j)
	{
		fat = gflp->item + j;
		if (fat->file == gfp)
		{
			fat->edge_type |= et;
			trace((/*{*/"}\n"));
			return;
		}
	}
	if (gflp->nfiles >= gflp->nfiles_max)
	{
		size_t		nbytes;

		gflp->nfiles_max = gflp->nfiles_max * 2 + 4;
		nbytes = gflp->nfiles_max * sizeof(gflp->item[0]);
		gflp->item = mem_change_size(gflp->item, nbytes);
	}
	/* do not bump reference count */
	fat = gflp->item + gflp->nfiles++;
	fat->file = gfp;
	fat->edge_type = et;
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	graph_file_list_nrc_append_list
 *
 * SYNOPSIS
 *	void graph_file_list_nrc_append_list(graph_file_list_nrc_ty *to,
 *		graph_file_list_nrc_ty *from);
 *
 * DESCRIPTION
 *	The graph_file_list_nrc_append_list function is used to append a the
 *	graph files from one list to another list.  Non-unique copies
 *	are silently ignored.
 *
 * CAVEAT
 *	Reference counts NOT updated.
 */

void
graph_file_list_nrc_append_list(to, from)
	graph_file_list_nrc_ty *to;
	graph_file_list_nrc_ty *from;
{
	size_t		j;

	trace(("graph_file_list_nrc_append_list(to = %8.8lX, \
from = %8.8lX)\n{\n"/*}*/,
		(long)to, (long)from));
	for (j = 0; j < from->nfiles; ++j)
	{
		graph_file_and_type_ty *fat;

		fat = from->item + j;
		graph_file_list_nrc_append(to, fat->file, fat->edge_type);
	}
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	graph_file_list_nrc_new
 *
 * SYNOPSIS
 *	graph_file_list_nrc_ty *graph_file_list_nrc_new(void);
 *
 * DESCRIPTION
 *	The graph_file_list_nrc_new function is used to allocate a new empty
 *	graph file list in dynamic memory.
 *
 * RETURNS
 *	graph_file_list_nrc_ty *
 *
 * CAVEAT
 *	Use graph_file_list_nrc_delete when you are done with it.
 */

graph_file_list_nrc_ty *
graph_file_list_nrc_new()
{
	graph_file_list_nrc_ty *gflp;

	trace(("graph_file_list_nrc_new()\n{\n"/*}*/));
	gflp = mem_alloc(sizeof(graph_file_list_nrc_ty));
	graph_file_list_nrc_constructor(gflp);
	trace(("return %8.8lX;\n", (long)gflp));
	trace((/*{*/"}\n"));
	return gflp;
}


/*
 * NAME
 *	graph_file_list_nrc_delete
 *
 * SYNOPSIS
 *	void graph_file_list_nrc_delete(graph_file_list_nrc_ty *);
 *
 * DESCRIPTION
 *	The graph_file_list_nrc_delete function is used to release the
 *	resources held by a graph file list in dynamic memory.
 *
 * CAVEAT
 *	Reference counts NOT updated.
 */

void
graph_file_list_nrc_delete(gflp)
	graph_file_list_nrc_ty *gflp;
{
	trace(("graph_file_list_nrc_delete(gflp = %8.8lX)\n{\n"/*}*/,
		(long)gflp));
	graph_file_list_nrc_destructor(gflp);
	mem_free(gflp);
	trace((/*{*/"}\n"));
}
