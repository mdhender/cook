/*
 *      cook - file construction tool
 *      Copyright (C) 1990-1993, 1997, 1999, 2006-2008 Peter Miller
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

#ifndef COOK_STMT_H
#define COOK_STMT_H

#include <common/main.h>

struct match_ty; /* existence */
struct opcode_list_ty; /* existence */


enum stmt_result_ty
{
        STMT_OK = 0,
        STMT_ERROR = -3
};
typedef enum stmt_result_ty stmt_result_ty;


typedef struct stmt_ty stmt_ty;
struct stmt_ty
{
        struct stmt_method_ty *method;
        long            s_references;
};

typedef struct stmt_method_ty stmt_method_ty;
struct stmt_method_ty
{
        char    *name;
        int     size;
        void    (*destructor)(stmt_ty *);
        stmt_result_ty (*code_generate)(stmt_ty *, struct opcode_list_ty *);
};


stmt_ty *stmt_copy(stmt_ty *);
void stmt_delete(stmt_ty *);
stmt_result_ty stmt_evaluate(stmt_ty *, const struct match_ty *);
stmt_result_ty stmt_code_generate(stmt_ty *, struct opcode_list_ty *);
struct opcode_list_ty *stmt_compile(stmt_ty *);

/* PRIVATE: for use only by derived stmt classes... */
stmt_ty *stmt_private_new(stmt_method_ty *);

#endif /* COOK_STMT_H */
