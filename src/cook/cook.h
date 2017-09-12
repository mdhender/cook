/*
 *      cook - file construction tool
 *      Copyright (C) 1993-1997, 1999, 2001, 2006, 2007 Peter Miller;
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

#ifndef COOK_COOK_H
#define COOK_COOK_H

#include <common/ac/time.h>

#include <common/main.h>

struct opcode_context_ty; /* existence */
struct recipe_ty; /* existence */
struct string_ty; /* existence */
struct string_list_ty; /* existence */

int cook(struct string_list_ty *);
int cook_pairs(struct string_list_ty *);
int cook_script(struct string_list_ty *);
int cook_web(struct string_list_ty *);

time_t cook_mtime_oldest(const struct opcode_context_ty *,
        struct string_ty *, long *, long);
time_t cook_mtime_newest(const struct opcode_context_ty *,
        struct string_ty *, long *, long);

/**
  * The cook_mtime_resolve function is used to determine the actual path
  * of a list of files, by examining files in the search path.
  *
  * @param ocp
  *     Pointer to the opcode context.
  * @param out
  *     Resulting list of strings
  * @param in
  *     Input list of strings
  * @param start
  *     element to start from
  * @returns
  *     0 on success, -1 on failure
  */
int cook_mtime_resolve(const struct opcode_context_ty *ocp,
        struct string_list_ty *out, const struct string_list_ty *in, int start);

/**
  * The cook_mtime_resolve1 function is used to determine the actual path
  * of a file, by examining files in the search path.
  *
  * @param ocp
  *     Pointer to the opcode context.
  * @param fn
  *     Name of the file file to search for
  * @returns
  *     pointer to new string on success, NULL on failure
  */
struct string_ty *cook_mtime_resolve1(const struct opcode_context_ty *ocp,
        struct string_ty *fn);

void cook_auto(struct string_list_ty *);
int cook_auto_required(void);
void cook_reset(void);
void cook_find_default(struct string_list_ty *);
void cook_search_list(const struct opcode_context_ty *,
        struct string_list_ty *slp);

void cook_explicit_append(struct recipe_ty *);
const struct recipe_list_ty *cook_explicit_by_name(struct string_ty *);
void cook_implicit_append(struct recipe_ty *);
struct recipe_ty *cook_implicit_nth(long);
struct recipe_ty *cook_implicit_nth_by_name(long, struct string_ty *);

#endif /* COOK_COOK_H */
