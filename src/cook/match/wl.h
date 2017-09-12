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
 * MANIFEST: interface definition for cook/match/wl.c
 */

#ifndef COOK_MATCH_WL_H
#define COOK_MATCH_WL_H

#include <match.h>

struct string_list_ty; /* existence */

int match_wl_attempt _((match_ty *, struct string_list_ty *,
	struct string_ty *, const struct expr_position_ty *));
int match_wl_reconstruct_lhs _((const match_ty *, struct string_list_ty *,
	struct string_list_ty *, const struct expr_position_ty *));
int match_wl_reconstruct_rhs _((const match_ty *, struct string_list_ty *,
	struct string_list_ty *, const struct expr_position_ty *));
int match_wl_usage_mask _((const match_ty *, struct string_list_ty *,
	const struct expr_position_ty *));

#endif /* COOK_MATCH_WL_H */
