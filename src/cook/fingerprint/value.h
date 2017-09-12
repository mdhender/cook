/*
 *	cook - file construction tool
 *	Copyright (C) 1999, 2001, 2002 Peter Miller;
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
 * MANIFEST: interface definition for cook/fingerprint/value.c
 */

#ifndef COOK_FINGERPRINT_VALUE_H
#define COOK_FINGERPRINT_VALUE_H

#include <ac/stdio.h>
#include <ac/time.h>
#include <str.h>

typedef struct fp_value_ty fp_value_ty;
struct fp_value_ty
{
    time_t	    oldest;
    time_t	    newest;
    time_t	    stat_mod_time;
    string_ty	    *contents_fingerprint;
    string_ty	    *ingredients_fingerprint;
};

void fp_value_constructor _((fp_value_ty *));
void fp_value_constructor_copy _((fp_value_ty *, const fp_value_ty *));
void fp_value_constructor3 _((fp_value_ty *, time_t, time_t, string_ty *));
void fp_value_constructor4 _((fp_value_ty *, time_t, time_t, string_ty *,
    string_ty *));
void fp_value_constructor5 _((fp_value_ty *, time_t, time_t, time_t,
    string_ty *, string_ty *));
void fp_value_destructor _((fp_value_ty *));
fp_value_ty *fp_value_new _((void));
void fp_value_delete _((fp_value_ty *));
void fp_value_copy _((fp_value_ty *, const fp_value_ty *));
int fp_value_equal _((const fp_value_ty *, const fp_value_ty *));
int fp_value_equal_all _((const fp_value_ty *, const fp_value_ty *));
void fp_value_write _((fp_value_ty *, string_ty *, FILE *));

#endif /* COOK_FINGERPRINT_VALUE_H */
