/*
 *      cook - file construction tool
 *      Copyright (C) 1998, 2006, 2007 Peter Miller;
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

#ifndef COOK_BUILTIN_EXPR_LEX_H
#define COOK_BUILTIN_EXPR_LEX_H

#include <common/main.h>

struct string_list_ty; /* existence */
struct expr_position_ty; /* existence */

void builtin_expr_lex_open(const struct string_list_ty *,
        const struct expr_position_ty *);
int builtin_expr_lex_error_count(void);
void builtin_expr_lex_close(void);
void builtin_expr_parse_error(char *);
int builtin_expr_parse_lex(void);

#endif /* COOK_BUILTIN_EXPR_LEX_H */
