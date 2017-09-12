/*
 *	cook - file construction tool
 *	Copyright (C) 1997, 2001 Peter Miller;
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
 * MANIFEST: interface definition for cook/lex/filenamelist.c
 */

#ifndef COOK_LEX_FILENAMELIST_H
#define COOK_LEX_FILENAMELIST_H

#include <ac/stddef.h>
#include <main.h>

typedef struct lex_filename_list_ty lex_filename_list_ty;
struct lex_filename_list_ty
{
	size_t		length;
	size_t		maximum;
	struct lex_filename_ty **list;
};

void lex_filename_list_constructor _((lex_filename_list_ty *));
lex_filename_list_ty *lex_filename_list_new _((void));
void lex_filename_list_destructor _((lex_filename_list_ty *));
void lex_filename_list_delete _((lex_filename_list_ty *));
void lex_filename_list_push_back _((lex_filename_list_ty *,
	struct lex_filename_ty *));
struct lex_filename_ty *lex_filename_list_pop_front _((lex_filename_list_ty *));

#endif /* COOK_LEX_FILENAMELIST_H */
