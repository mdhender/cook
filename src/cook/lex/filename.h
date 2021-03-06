/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 2001, 2006-2008 Peter Miller
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

#ifndef COOK_LEX_FILENAME_H
#define COOK_LEX_FILENAME_H

#include <common/main.h>

typedef struct lex_filename_ty lex_filename_ty;
struct lex_filename_ty
{
        struct string_ty *logical;
        struct string_ty *physical;
};

void lex_filename_constructor(lex_filename_ty *, struct string_ty *,
        struct string_ty *);
lex_filename_ty *lex_filename_new(struct string_ty *, struct string_ty *);
void lex_filename_destructor(lex_filename_ty *);
void lex_filename_delete(lex_filename_ty *);

#endif /* COOK_LEX_FILENAME_H */
