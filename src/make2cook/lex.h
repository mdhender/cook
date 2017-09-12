/*
 *	cook - file construction tool
 *	Copyright (C) 1994, 1997, 1998 Peter Miller;
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
 * MANIFEST: interface definition for make2cook/lex.c
 */

#ifndef MAKE2COOK_LEX_H
#define MAKE2COOK_LEX_H

#include <main.h>

struct string_ty; /* existence */
struct sub_context_ty; /* existence */

void lex_open _((char *));
void lex_close _((void));

void gram_error _((char *));
void lex_error _((struct sub_context_ty *, char *));
void gram_trace _((char *, ...));
void gram_trace2 _((void *, char *, ...));
int gram_lex _((void));

struct blob_ty *lex_blob _((struct string_ty *));

#endif /* MAKE2COOK_LEX_H */
