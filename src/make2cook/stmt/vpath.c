/*
 *	cook - file construction tool
 *	Copyright (C) 1994, 1997 Peter Miller;
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
 * MANIFEST: functions to manipulate vpath statementss
 */

#include <stmt/assign.h>
#include <stmt/vpath.h>


static blob_list_ty	*vpath;
static blob_list_ty	*VPATH;


static void dismember _((blob_list_ty *, blob_list_ty *));

static void
dismember(path, rhs)
	blob_list_ty	*path;
	blob_list_ty	*rhs;
{
	size_t		j;

	for (j = 0; j < rhs->length; ++j)
	{
		blob_ty		*bp;
		string_list_ty	wl;
		size_t		k;

		bp = rhs->list[j];
		str2wl(&wl, bp->text, ":", 1);
		for (k = 0; k < wl.nstrings; ++k)
		{
			blob_list_append
			(
				path,
				blob_alloc
				(
					str_copy(wl.string[k]),
					bp->file_name,
					bp->line_number
				)
			);
		}
		string_list_destructor(&wl);
	}
}


void
stmt_vpath_remember1(rhs)
	blob_list_ty	*rhs;
{
	if (!vpath)
		vpath = blob_list_alloc();
	dismember(vpath, rhs);
	blob_list_free(rhs);
}


void
stmt_vpath_remember2(rhs)
	blob_list_ty	*rhs;
{
	if (!VPATH)
		VPATH = blob_list_alloc();
	dismember(VPATH, rhs);
	blob_list_free(rhs);
}


stmt_ty *
stmt_vpath_default()
{
	static string_ty *builtin;
	blob_ty		*lhs;
	blob_list_ty	*rhs;
	size_t		j;

	if (!vpath && !VPATH)
		return 0;
	if (!builtin)
		builtin = str_from_c("builtin");
	lhs = blob_alloc(str_from_c("search_list"), builtin, 999);
	rhs = blob_list_alloc();
	blob_list_append(rhs, blob_alloc(str_from_c("."), builtin, 999));
	if (vpath)
	{
		for (j = 0; j < vpath->length; ++j)
			blob_list_append(rhs, blob_copy(vpath->list[j]));
		blob_list_free(vpath);
		vpath = 0;
	}
	if (VPATH)
	{
		for (j = 0; j < VPATH->length; ++j)
			blob_list_append(rhs, blob_copy(VPATH->list[j]));
		blob_list_free(VPATH);
		VPATH = 0;
	}

	return stmt_assign_alloc(1, lhs, stmt_assign_op_colon, rhs);
}
