/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 1998, 2006-2008 Peter Miller
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

#ifndef COMMON_SUB_PROGNAME_H
#define COMMON_SUB_PROGNAME_H

#include <common/main.h>

struct sub_context_ty; /* existence */
struct wstring_list_ty; /* existence */

struct wstring_ty *sub_progname (struct sub_context_ty *,
        struct wstring_list_ty *);

#endif /* COMMON_SUB_PROGNAME_H */
