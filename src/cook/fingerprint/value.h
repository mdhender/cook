/*
 *      cook - file construction tool
 *      Copyright (C) 1999, 2001, 2002, 2006-2008 Peter Miller
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

#ifndef COOK_FINGERPRINT_VALUE_H
#define COOK_FINGERPRINT_VALUE_H

#include <common/ac/stdio.h>
#include <common/ac/time.h>
#include <common/str.h>

typedef struct fp_value_ty fp_value_ty;
struct fp_value_ty
{
    time_t          oldest;
    time_t          newest;
    time_t          stat_mod_time;
    string_ty       *contents_fingerprint;
    string_ty       *ingredients_fingerprint;
};

void fp_value_constructor(fp_value_ty *);
void fp_value_constructor_copy(fp_value_ty *, const fp_value_ty *);
void fp_value_constructor3(fp_value_ty *, time_t, time_t, string_ty *);
void fp_value_constructor4(fp_value_ty *, time_t, time_t, string_ty *,
    string_ty *);
void fp_value_constructor5(fp_value_ty *, time_t, time_t, time_t,
    string_ty *, string_ty *);
void fp_value_destructor(fp_value_ty *);
fp_value_ty *fp_value_new(void);
void fp_value_delete(fp_value_ty *);
void fp_value_copy(fp_value_ty *, const fp_value_ty *);
int fp_value_equal(const fp_value_ty *, const fp_value_ty *);
int fp_value_equal_all(const fp_value_ty *, const fp_value_ty *);
void fp_value_write(fp_value_ty *, string_ty *, FILE *);

#endif /* COOK_FINGERPRINT_VALUE_H */
