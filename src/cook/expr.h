/*
 *      cook - file construction tool
 *      Copyright (C) 1990-1993, 1995, 1997, 2001, 2006, 2007 Peter Miller;
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

#ifndef COOK_EXPR_H
#define COOK_EXPR_H

#include <common/ac/stddef.h>
#include <cook/expr/position.h>

struct expr_ty; /* forward */
struct opcode_list_ty; /* existence */
struct match_ty; /* existence */

typedef struct expr_method_ty expr_method_ty;
struct expr_method_ty
{
        char    *name;
        size_t  size;
        void    (*destructor)(struct expr_ty *);
        int     (*equal)(const struct expr_ty *, const struct expr_ty *);
        void    (*code_generate)(const struct expr_ty *,
                        struct opcode_list_ty *);
};

typedef struct expr_ty expr_ty;
struct expr_ty
{
        expr_method_ty  *method;
        long            e_references;
        expr_position_ty e_position;
};

expr_ty *expr_copy(expr_ty *);
void expr_delete(expr_ty *);
int expr_eval_condition(const expr_ty *, const struct match_ty *);
struct string_list_ty *expr_evaluate(const expr_ty *,
        const struct match_ty *);
int expr_equal(const expr_ty *, const expr_ty *);
void expr_code_generate(const expr_ty *, struct opcode_list_ty *);

/* PRIVATE: derived expr classes only... */
expr_ty *expr_private_new(expr_method_ty *);

#endif /* COOK_EXPR_H */
