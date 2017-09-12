/*
 *      cook - file construction tool
 *      Copyright (C) 1998, 2006-2008 Peter Miller
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

#ifndef COOK_GRAPH_LEAF_H
#define COOK_GRAPH_LEAF_H

#include <common/main.h>

enum leaf_ness_ty
{
        leaf_ness_error,
        leaf_ness_leaf_exists,
        leaf_ness_leaf_explicit,
        leaf_ness_interior_exists,
        leaf_ness_interior_explicit,
        leaf_ness_exterior_explicit,
        leaf_ness_indeterminate
};
typedef enum leaf_ness_ty leaf_ness_ty;

void leaf_reset(void);
leaf_ness_ty leaf_query(struct string_ty *, int);
const char *leaf_ness_name(leaf_ness_ty);

#endif /* COOK_GRAPH_LEAF_H */
