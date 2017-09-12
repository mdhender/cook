/*
 *      cook - file construction tool
 *      Copyright (C) 1994, 1997, 2006-2008 Peter Miller
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 3 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program. If not, see
 *      <http://www.gnu.org/licenses/>.
 */

#ifndef MAKE2COOK_EMIT_H
#define MAKE2COOK_EMIT_H

#include <common/str.h>

extern int emit_line_numbers;

void emit_open(char *);
void emit_close(void);
void emit_char(int);
void emit_str(char *);
void emit_string(string_ty *);
void emit_line_number(long, string_ty *);
void emit_set_file(string_ty *);
void emit_bol(void);
void emit_indent_more(void);
void emit_indent_less(void);

#endif /* MAKE2COOK_EMIT_H */
