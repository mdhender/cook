/*
 *      cook - file construction tool
 *      Copyright (C) 1994, 1997, 1998, 2006, 2007 Peter Miller;
 *      All rights reserved.
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

#ifndef MAKE2COOK_LEX_H
#define MAKE2COOK_LEX_H

#include <common/main.h>

struct string_ty; /* existence */
struct sub_context_ty; /* existence */

void lex_open(char *);
void lex_close(void);

void gram_error(char *);
void lex_error(struct sub_context_ty *, char *);
void gram_trace(char *, ...);
void gram_trace2(void *, char *, ...);
int gram_lex(void);

struct blob_ty *lex_blob(struct string_ty *);

#endif /* MAKE2COOK_LEX_H */
