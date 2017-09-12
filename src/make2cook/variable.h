/*
 *      cook - file construction tool
 *      Copyright (C) 1994, 1997, 2002, 2006, 2007 Peter Miller;
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

#ifndef MAKE2COOK_VARIABLE_H
#define MAKE2COOK_VARIABLE_H

#include <make2cook/blob.h>
#include <common/str_list.h>

/*
 * These are the flags bits for the last argument to variable_rename and
 * variable_rename_list.  Add them together to combine them.
 */
#define VAREN_NO_FLAGS 0        /* nothing special */
#define VAREN_QUOTE_SPACES 1    /* spaces need to be quoted */
#define VAREN_KNOW_ARCHIVE 2    /* archive names like a(b) to be translated */
#define VAREN_NO_QUOQUO 4       /* don't quote doutes */

void variable_rename(blob_ty *, blob_list_ty *, string_list_ty *, int);
void variable_rename_list(blob_list_ty *, blob_list_ty *, string_list_ty *,
    int);
void variable_archive(string_ty *target, string_ty *member);

/* for use by vargram */
int vargram_lex(void);
void vargram_error(char *);
void variable_mangle_result(string_ty *);
string_ty *variable_mangle_lookup(string_ty *);
void variable_mangle_forget(string_ty *);

void vargram_trace(char *, ...);
void vargram_trace2(void *, char *, ...);

#endif /* MAKE2COOK_VARIABLE_H */
