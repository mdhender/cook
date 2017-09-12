/*
 *	cook - file construction tool
 *	Copyright (C) 1994, 1997, 1999, 2001 Peter Miller;
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
 * MANIFEST: interface definition for cook/fingerprint.c
 */

#ifndef COOK_FNGRPRNT_H
#define COOK_FNGRPRNT_H

#include <main.h>

struct string_ty; /* existence */
struct fp_value_ty; /* existence */

struct fp_value_ty *fp_search _((struct string_ty *path));
void fp_assign _((struct string_ty *, struct fp_value_ty *));
void fp_delete _((struct string_ty *));
struct string_ty *fp_fingerprint _((struct string_ty *path));
struct string_ty *fp_fingerprint_string _((struct string_ty *value));
void fp_tweak _((void));

int fp_ingredients_fingerprint_differs _((struct string_ty *,
	struct string_ty *));

#endif /* COOK_FNGRPRNT_H */
