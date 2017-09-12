/*
 *      cook - file construction tool
 *      Copyright (C) 1994, 1997, 1999, 2006-2008 Peter Miller
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

#ifndef COOK_BUILTIN_TEXT_H
#define COOK_BUILTIN_TEXT_H

#include <cook/builtin/private.h>

extern builtin_ty builtin_catenate;
extern builtin_ty builtin_count;
extern builtin_ty builtin_downcase;
extern builtin_ty builtin_firstword;
extern builtin_ty builtin_head;
extern builtin_ty builtin_prepost;
extern builtin_ty builtin_quote;
extern builtin_ty builtin_sort;
extern builtin_ty builtin_tail;
extern builtin_ty builtin_upcase;
extern builtin_ty builtin_words;

#endif /* COOK_BUILTIN_TEXT_H */
