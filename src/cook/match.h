/*
 *      cook - file construction tool
 *      Copyright (C) 1990, 1992, 1993, 1997, 1999, 2006, 2007 Peter Miller;
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

#ifndef MATCH_H
#define MATCH_H

#include <common/main.h>

struct string_ty; /* existence */
struct expr_position_ty; /* existence */

typedef struct match_ty match_ty;
struct match_ty
{
        struct match_method_ty *vptr;
};


match_ty *match_new(void);
match_ty *match_clone(match_ty *);
void match_delete(match_ty *);
int match_attempt(match_ty *, struct string_ty *, struct string_ty *,
        const struct expr_position_ty *);
int match_compile(match_ty *, struct string_ty *,
        const struct expr_position_ty *);
int match_execute(match_ty *, struct string_ty *,
        const struct expr_position_ty *);
struct string_ty *match_reconstruct_lhs(const match_ty *, struct string_ty *,
        const struct expr_position_ty *);
struct string_ty *match_reconstruct_rhs(const match_ty *, struct string_ty *,
        const struct expr_position_ty *);
int match_usage_mask(const match_ty *, struct string_ty *,
        const struct expr_position_ty *);

#endif /* MATCH_H */
