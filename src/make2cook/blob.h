/*
 *      cook - file construction tool
 *      Copyright (C) 1994, 1997, 1998, 2006, 2007 Peter Miller;
 *      All rights reserved.
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

#ifndef MAKE2COOK_BLOB_H
#define MAKE2COOK_BLOB_H

#include <common/str.h>

struct sub_context_ty; /* existence */

typedef struct blob_ty blob_ty;
struct blob_ty
{
        long            reference_count;
        string_ty       *text;
        string_ty       *file_name;
        long            line_number;
};

typedef struct blob_list_ty blob_list_ty;
struct blob_list_ty
{
        blob_ty         **list;
        size_t          length;
        size_t          maximum;
};

typedef void (*blob_efunc)(void);

blob_ty *blob_alloc(string_ty *text, string_ty *fn, long lnum);
void blob_free(blob_ty *);
blob_ty *blob_copy(blob_ty *);

void blob_error_notify(blob_efunc);
void blob_error(blob_ty *, struct sub_context_ty *, char *);
void blob_warning(blob_ty *, struct sub_context_ty *, char *);

blob_list_ty *blob_list_alloc(void);
void blob_list_free(blob_list_ty *);
void blob_list_append(blob_list_ty *, blob_ty *);
void blob_list_prepend(blob_list_ty *, blob_ty *);
void blob_list_delete(blob_list_ty *, blob_ty *);
void blob_emit(blob_ty *);

#endif /* MAKE2COOK_BLOB_H */
