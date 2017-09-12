/*
 *	cook - file construction tool
 *	Copyright (C) 1997, 1999, 2001 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: interface definition for cook/id/private.c
 */

#ifndef COOK_ID_PRIVATE_H
#define COOK_ID_PRIVATE_H

#include <id.h>

struct opcode_context_ty; /* existence */
struct expr_position_ty; /* existence */

typedef struct id_method_ty id_method_ty;
struct id_method_ty
{
	char		*name;
	int		size;
	void		(*destructor)_((id_ty *));
	int		(*interpret)_((id_ty *, struct opcode_context_ty *,
				const struct expr_position_ty *));
	int		(*script)_((id_ty *, struct opcode_context_ty *,
				const struct expr_position_ty *));
};

id_ty *id_instance_new _((id_method_ty *));
void id_instance_delete _((id_ty *));

#endif /* COOK_ID_PRIVATE_H */
