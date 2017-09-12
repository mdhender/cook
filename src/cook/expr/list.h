/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 1999, 2001, 2006-2008 Peter Miller
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

#ifndef COOK_EXPR_LIST_H
#define COOK_EXPR_LIST_H

#include <common/ac/stddef.h>
#include <common/main.h>

struct expr_ty; /* existence */
struct opcode_list_ty; /* existence */
struct match_ty; /* existence */


typedef struct expr_list_ty expr_list_ty;
struct  expr_list_ty
{
        size_t          el_nexprs;
        size_t          el_nexprs_max;
        struct expr_ty  **el_expr;
};


void expr_list_append(expr_list_ty *, struct expr_ty *);
void expr_list_destructor(expr_list_ty *);
void expr_list_delete(expr_list_ty *);
void expr_list_copy_constructor(expr_list_ty *, const expr_list_ty *);
struct string_list_ty *expr_list_evaluate(const expr_list_ty *,
        const struct match_ty *);
void expr_list_constructor(expr_list_ty *);
expr_list_ty *expr_list_new(void);
void expr_list_code_generate(const expr_list_ty *, struct opcode_list_ty *);
struct expr_position_ty *expr_list_position(const expr_list_ty *);

#endif /* COOK_EXPR_LIST_H */
