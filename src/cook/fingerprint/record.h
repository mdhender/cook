/*
 *      cook - file construction tool
 *      Copyright (C) 1999, 2006-2008 Peter Miller
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

#ifndef COOK_FINGERPRINT_RECORD_H
#define COOK_FINGERPRINT_RECORD_H

#include <cook/fingerprint/value.h>

typedef struct fp_record_ty fp_record_ty;
struct fp_record_ty
{
        struct string_ty *filename;
        struct fp_subdir_ty *parent;
        int             exists;
        fp_value_ty     value;
};

fp_record_ty *fp_record_new(struct string_ty *, struct fp_subdir_ty *);
fp_record_ty *fp_record_new2(struct string_ty *, struct fp_subdir_ty *,
        fp_value_ty *);
void fp_record_delete(fp_record_ty *);

#include <common/ac/stdio.h>
void fp_record_write(fp_record_ty *, string_ty *, FILE *);
void fp_record_update(fp_record_ty *, fp_value_ty *);
void fp_record_clear(fp_record_ty *);
void fp_record_tweak(fp_record_ty *, time_t, string_ty *);

#endif /* COOK_FINGERPRINT_RECORD_H */
