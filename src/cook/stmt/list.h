/*
 *	cook - file construction tool
 *	Copyright (C) 1997, 1999 Peter Miller;
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
 * MANIFEST: interface definition for cook/stmt/list.c
 */

#ifndef COOK_STMT_LIST_H
#define COOK_STMT_LIST_H

#include <ac/stddef.h>
#include <main.h>

struct stmt_ty;

typedef struct stmt_list_ty stmt_list_ty;
struct stmt_list_ty
{
	size_t		sl_nstmts;
	size_t		sl_nstmts_max;
	struct stmt_ty	**sl_stmt;
};

void stmt_list_constructor _((stmt_list_ty *));
void stmt_list_copy_constructor _((stmt_list_ty *, const stmt_list_ty *));
void stmt_list_destructor _((stmt_list_ty *));
void stmt_list_append _((stmt_list_ty *, struct stmt_ty *));

stmt_list_ty *stmt_list_new _((void));
void stmt_list_delete _((stmt_list_ty *));

#endif /* COOK_STMT_LIST_H */
