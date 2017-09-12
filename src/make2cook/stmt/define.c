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

#include <make2cook/emit.h>
#include <common/mem.h>
#include <make2cook/stmt/define.h>

typedef struct stmt_define_ty stmt_define_ty;
struct stmt_define_ty
{
    STMT
    blob_ty         *first;
    blob_list_ty    *body;
};


static void
constructor(stmt_ty *that)
{
    stmt_define_ty  *this;

    this = (stmt_define_ty *)that;
    this->first = 0;
    this->body = blob_list_alloc();
}


static void
destructor(stmt_ty *that)
{
    stmt_define_ty  *this;

    this = (stmt_define_ty *)that;
    if (this->first)
        blob_free(this->first);
    blob_list_free(this->body);
}


static void
emit(stmt_ty *that)
{
    stmt_define_ty  *this;
    size_t          j;

    this = (stmt_define_ty *)that;
    blob_emit(this->first);
    emit_str(" =\n");

    for (j = 0; j < this->body->length; ++j)
    {
        emit_str("    ");
        blob_emit(this->body->list[j]);
        emit_str(" \"\\n\"\n");
    }
    emit_str("    ;\n");
}


static stmt_method_ty method =
{
    sizeof(stmt_define_ty),
    "define",
    constructor,
    destructor,
    emit,
    0,
    0,
};


stmt_ty *
stmt_define_alloc(blob_ty *first)
{
    stmt_define_ty  *result;

    result = (stmt_define_ty *)stmt_alloc(&method);
    result->first = first;
    return (stmt_ty *)result;
}


void
stmt_define_append(stmt_ty *that, blob_ty *lp)
{
    stmt_define_ty  *this;

    this = (stmt_define_ty *)that;
    blob_list_append(this->body, lp);
}
