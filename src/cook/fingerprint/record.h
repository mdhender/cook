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
 * MANIFEST: interface definition for cook/fingerprint/record.c
 */

#ifndef COOK_FINGERPRINT_RECORD_H
#define COOK_FINGERPRINT_RECORD_H

#include <fingerprint/value.h>

typedef struct fp_record_ty fp_record_ty;
struct fp_record_ty
{
	struct string_ty *filename;
	struct fp_subdir_ty *parent;
	int		exists;
	fp_value_ty	value;
};

fp_record_ty *fp_record_new _((struct string_ty *, struct fp_subdir_ty *));
fp_record_ty *fp_record_new2 _((struct string_ty *, struct fp_subdir_ty *,
	fp_value_ty *));
void fp_record_delete _((fp_record_ty *));

#include <ac/stdio.h>
void fp_record_write _((fp_record_ty *, string_ty *, FILE *));
void fp_record_update _((fp_record_ty *, fp_value_ty *));
void fp_record_clear _((fp_record_ty *));
void fp_record_tweak _((fp_record_ty *, time_t, string_ty *));

#endif /* COOK_FINGERPRINT_RECORD_H */
