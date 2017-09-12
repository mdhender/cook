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
 * MANIFEST: interface definition for make2cook/emit.c
 */

#ifndef MAKE2COOK_EMIT_H
#define MAKE2COOK_EMIT_H

#include <str.h>

extern int emit_line_numbers;

void emit_open _((char *));
void emit_close _((void));
void emit_char _((int));
void emit_str _((char *));
void emit_string _((string_ty *));
void emit_line_number _((long, string_ty *));
void emit_set_file _((string_ty *));
void emit_bol _((void));
void emit_indent_more _((void));
void emit_indent_less _((void));

#endif /* MAKE2COOK_EMIT_H */
