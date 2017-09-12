/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 2006-2008 Peter Miller
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

#ifndef COMMON_WSTR_LIST_H
#define COMMON_WSTR_LIST_H

#include <common/wstr.h>

typedef struct wstring_list_ty wstring_list_ty;
struct wstring_list_ty
{
        size_t          nitems;
        size_t          nitems_max;
        wstring_ty      **item;
};

int wstring_list_member(wstring_list_ty *, wstring_ty *);
wstring_ty *wstring_list_to_wstring(wstring_list_ty *, int, int, char *);
void wstring_to_wstring_list(wstring_list_ty *, wstring_ty *, char *, int);
void wstring_list_prepend(wstring_list_ty *, wstring_ty *);
void wstring_list_append(wstring_list_ty *, wstring_ty *);
void wstring_list_append_unique(wstring_list_ty *, wstring_ty *);
void wstring_list_copy(wstring_list_ty *, wstring_list_ty *);
void wstring_list_delete(wstring_list_ty *, wstring_ty *);
void wstring_list_free(wstring_list_ty *);
void wstring_list_zero(wstring_list_ty *);

int wstring_list_equal(wstring_list_ty *, wstring_list_ty *);
int wstring_list_subset(wstring_list_ty *, wstring_list_ty *);

#endif /* COMMON_WSTR_LIST_H */
