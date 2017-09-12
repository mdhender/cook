/*
 *      cook - file construction tool
 *      Copyright (C) 1990-1994, 1997, 1998, 2004, 2006-2008 Peter Miller
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

#ifndef COOK_LEX_H
#define COOK_LEX_H

#include <common/main.h>
#include <common/str.h>

/*
 *  lex_mode() arguments
 */
enum lex_mode_ty
{
        LM_NORMAL,
        LM_DATA,
        LM_SQUOTE,
        LM_DQUOTE,
        LM_COMMENT
};
typedef enum lex_mode_ty lex_mode_ty;

int hashline_lex(void);
void hashline_lex_reset(void);
int lex_cur_line(void);
lex_mode_ty lex_mode(lex_mode_ty);
int parse_lex(void);
string_ty *lex_cur_file(void);
string_ty *lex_cur_physical_file(void);
void lex_lino_set(string_ty *, string_ty *);
void lex_close(void);
void lex_error(struct sub_context_ty *, char *);
void parse_error(char *);
void lex_warning(struct sub_context_ty *, char *);
void lex_initialize(void);
void lex_open(string_ty *, string_ty *);
void lex_open_include(string_ty *, string_ty *);
void lex_passing(int);
void lex_trace(char*, ...);

#endif /* COOK_LEX_H */
