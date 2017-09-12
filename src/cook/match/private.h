/*
 *      cook - file construction tool
 *      Copyright (C) 1999, 2006, 2007 Peter Miller;
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

#ifndef COOK_MATCH_PRIVATE_H
#define COOK_MATCH_PRIVATE_H

#include <cook/match.h>

typedef struct match_method_ty match_method_ty;
struct match_method_ty
{
        char    *name;
        int     size;
        void (*destructor)(match_ty *);
        void (*constructor)(match_ty *);
        int (*compile)(match_ty *, struct string_ty *,
                const struct expr_position_ty *);
        int (*execute)(match_ty *, struct string_ty *,
                const struct expr_position_ty *);
        struct string_ty *(*reconstruct_lhs)(const match_ty *,
                struct string_ty *, const struct expr_position_ty *);
        struct string_ty *(*reconstruct_rhs)(const match_ty *,
                struct string_ty *, const struct expr_position_ty *);
        int (*usage_mask)(const match_ty *, struct string_ty *,
                const struct expr_position_ty *);
};

match_ty *match_private_new(match_method_ty *);

#endif /* COOK_MATCH_PRIVATE_H */
