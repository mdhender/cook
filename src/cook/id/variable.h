/*
 *	cook - file construction tool
 *	Copyright (C) 1997 Peter Miller;
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
 * MANIFEST: interface definition for cook/id/variable.c
 */

#ifndef COOK_ID_VARIABLE_H
#define COOK_ID_VARIABLE_H

#include <main.h>

struct string_list_ty; /* instance */

struct id_ty *id_variable_new _((struct string_list_ty *));
void id_variable_query _((struct id_ty *, struct string_list_ty *));
struct string_list_ty *id_variable_query2 _((struct id_ty *));

#endif /* COOK_ID_VARIABLE_H */
