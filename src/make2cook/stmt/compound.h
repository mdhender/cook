/*
 *	cook - file construction tool
 *	Copyright (C) 1994, 1997 Peter Miller;
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
 * MANIFEST: interface definition for make2cook/stmt/compound.c
 */

#ifndef MAKE2COOK_STMT_COMPOUND_H
#define MAKE2COOK_STMT_COMPOUND_H

#include <stmt.h>

stmt_ty *stmt_compound_alloc _((void));
void stmt_compound_prepend _((stmt_ty *, stmt_ty *));
void stmt_compound_append _((stmt_ty *, stmt_ty *));

#endif /* MAKE2COOK_STMT_COMPOUND_H */
