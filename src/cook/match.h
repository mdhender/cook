/*
 *	cook - file construction tool
 *	Copyright (C) 1990, 1992, 1993, 1997, 1999 Peter Miller;
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
 * MANIFEST: interface definition for cook/match.c
 */

#ifndef MATCH_H
#define MATCH_H

#include <main.h>

struct string_ty; /* existence */
struct expr_position_ty; /* existence */

typedef struct match_ty match_ty;
struct match_ty
{
	struct match_method_ty *vptr;
};


match_ty *match_new _((void));
match_ty *match_clone _((match_ty *));
void match_delete _((match_ty *));
int match_attempt _((match_ty *, struct string_ty *, struct string_ty *,
	const struct expr_position_ty *));
int match_compile _((match_ty *, struct string_ty *,
	const struct expr_position_ty *));
int match_execute _((match_ty *, struct string_ty *,
	const struct expr_position_ty *));
struct string_ty *match_reconstruct_lhs _((const match_ty *, struct string_ty *,
	const struct expr_position_ty *));
struct string_ty *match_reconstruct_rhs _((const match_ty *, struct string_ty *,
	const struct expr_position_ty *));
int match_usage_mask _((const match_ty *, struct string_ty *,
	const struct expr_position_ty *));

#endif /* MATCH_H */
