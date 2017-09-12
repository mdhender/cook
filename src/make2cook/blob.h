/*
 *	cook - file construction tool
 *	Copyright (C) 1994, 1997, 1998 Peter Miller;
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
 * MANIFEST: interface definition for make2cook/blob.c
 */

#ifndef MAKE2COOK_BLOB_H
#define MAKE2COOK_BLOB_H

#include <str.h>

struct sub_context_ty; /* existence */

typedef struct blob_ty blob_ty;
struct blob_ty
{
	long		reference_count;
	string_ty	*text;
	string_ty	*file_name;
	long		line_number;
};

typedef struct blob_list_ty blob_list_ty;
struct blob_list_ty
{
	blob_ty		**list;
	long		length;
	long		maximum;
};

typedef void (*blob_efunc)_((void));

blob_ty *blob_alloc _((string_ty *text, string_ty *fn, long lnum));
void blob_free _((blob_ty *));
blob_ty *blob_copy _((blob_ty *));

void blob_error_notify _((blob_efunc));
void blob_error _((blob_ty *, struct sub_context_ty *, char *));
void blob_warning _((blob_ty *, struct sub_context_ty *, char *));

blob_list_ty *blob_list_alloc _((void));
void blob_list_free _((blob_list_ty *));
void blob_list_append _((blob_list_ty *, blob_ty *));
void blob_list_prepend _((blob_list_ty *, blob_ty *));
void blob_list_delete _((blob_list_ty *, blob_ty *));
void blob_emit _((blob_ty *));

#endif /* MAKE2COOK_BLOB_H */
