/*
 *	cook - file construction tool
 *	Copyright (C) 1999 Peter Miller;
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
 * MANIFEST: interface definition for cook/match/private.c
 */

#ifndef COOK_MATCH_PRIVATE_H
#define COOK_MATCH_PRIVATE_H

#include <match.h>

typedef struct match_method_ty match_method_ty;
struct match_method_ty
{
	char	*name;
	int	size;
	void (*destructor)_((match_ty *));
	void (*constructor)_((match_ty *));
	int (*compile)_((match_ty *, struct string_ty *,
		const struct expr_position_ty *));
	int (*execute)_((match_ty *, struct string_ty *,
		const struct expr_position_ty *));
	struct string_ty *(*reconstruct_lhs)_((const match_ty *,
		struct string_ty *, const struct expr_position_ty *));
	struct string_ty *(*reconstruct_rhs)_((const match_ty *,
		struct string_ty *, const struct expr_position_ty *));
	int (*usage_mask)_((const match_ty *, struct string_ty *,
		const struct expr_position_ty *));
};

match_ty *match_private_new _((match_method_ty *));

#endif /* COOK_MATCH_PRIVATE_H */
