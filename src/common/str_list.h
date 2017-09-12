/*
 *      cook - file construction tool
 *      Copyright (C) 1991-1994, 1997, 2000, 2001, 2006-2008 Peter Miller
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

#ifndef COMMON_STR_LIST_H
#define COMMON_STR_LIST_H

#include <common/str.h>

typedef struct string_list_ty string_list_ty;
struct  string_list_ty
{
        size_t          nstrings;
        size_t          nstrings_max;
        string_ty       **string;
};

int string_list_member(const string_list_ty *, string_ty *);
int string_list_intersect(const string_list_ty *, const string_list_ty *);
string_ty *wl2str_respect_empty(const string_list_ty *, int, int,
        const char *, int);
string_ty *wl2str(const string_list_ty *, int, int, const char *);
void str2wl(string_list_ty *, string_ty *, char *, int);
void string_list_append(string_list_ty *, string_ty *);
void string_list_append_list(string_list_ty *, const string_list_ty *);
void string_list_prepend(string_list_ty *, string_ty *);
void string_list_append_unique(string_list_ty *, string_ty *);
void string_list_append_list_unique(string_list_ty *,
        const string_list_ty *);
void string_list_copy_constructor(string_list_ty *, const string_list_ty *);
void string_list_remove(string_list_ty *, string_ty *);
void string_list_remove_list(string_list_ty *, const string_list_ty *);
void string_list_destructor(string_list_ty *);
void string_list_constructor(string_list_ty *);

string_list_ty *string_list_new(void);
string_list_ty *string_list_new_copy(const string_list_ty *);
void string_list_delete(string_list_ty *);
int string_list_bool(const string_list_ty *);
void string_list_sort(string_list_ty *);

#endif /* COMMON_STR_LIST_H */
