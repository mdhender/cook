/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 1999, 2001, 2006, 2007 Peter Miller;
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

#ifndef COOK_ID_PRIVATE_H
#define COOK_ID_PRIVATE_H

#include <cook/id.h>

struct opcode_context_ty; /* existence */
struct expr_position_ty; /* existence */

typedef struct id_method_ty id_method_ty;
struct id_method_ty
{
        char            *name;
        int             size;
        void            (*destructor)(id_ty *);
        int             (*interpret)(id_ty *, struct opcode_context_ty *,
                                const struct expr_position_ty *);
        int             (*script)(id_ty *, struct opcode_context_ty *,
                                const struct expr_position_ty *);
};

id_ty *id_instance_new(id_method_ty *);
void id_instance_delete(id_ty *);

#endif /* COOK_ID_PRIVATE_H */
