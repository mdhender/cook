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

#ifndef COOK_FUNCTION_H
#define COOK_FUNCTION_H

#include <common/main.h>

struct string_ty; /* existence */
struct stmt_ty; /* existence */

int function_definition(struct string_ty *, struct stmt_ty *);
struct opcode_list_ty *function_search(struct string_ty *);
int function_search_fuzzy(struct string_ty *, struct string_ty **);

#endif /* COOK_FUNCTION_H */
