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
 * MANIFEST: interface definition for cook/fingerprint/subdir.c
 */

#ifndef COOK_FINGERPRINT_SUBDIR_H
#define COOK_FINGERPRINT_SUBDIR_H

#include <main.h>

struct string_ty; /* existence */

typedef struct fp_subdir_ty fp_subdir_ty;
struct fp_subdir_ty
{
	struct symtab_ty *stp;
	struct string_ty *path;
	int		dirty;
	int		cache_in_dot;
	int		need_to_read;
};

fp_subdir_ty *fp_subdir_new _((struct string_ty *));
void fp_subdir_delete _((fp_subdir_ty *));
void fp_subdir_read _((fp_subdir_ty *));
void fp_subdir_write _((fp_subdir_ty *, int *));
void fp_subdir_dirty_notify _((fp_subdir_ty *, struct string_ty *));
void fp_subdir_tweak _((fp_subdir_ty *));

#endif /* COOK_FINGERPRINT_SUBDIR_H */
