/*
 *      cook - file construction tool
 *      Copyright (C) 1994, 1997, 1998, 2006-2008 Peter Miller
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

#include <common/ac/ctype.h>
#include <common/ac/string.h>

#include <make2cook/emit.h>
#include <common/mem.h>
#include <make2cook/stmt/include.h>
#include <make2cook/variable.h>

typedef struct stmt_include_ty stmt_include_ty;
struct stmt_include_ty
{
    STMT
    int             type;
    blob_list_ty    *body;
};


static void
destructor(stmt_ty *that)
{
    stmt_include_ty *this;

    this = (stmt_include_ty *)that;
    blob_list_free(this->body);
}


static int
wildchars(blob_ty *s)
{
    char            *cp;

    cp = s->text->str_text;
    while (*cp)
    {
        if (strchr("?[*]", *cp))
            return 1;
        ++cp;
    }
    return 0;
}


static void
emit(stmt_ty *that)
{
    stmt_include_ty *this;
    size_t          j;
    int             wild;
    int             wild2;

    this = (stmt_include_ty *)that;
    if (!this->body->length)
        return;
    emit_line_number
    (
        this->body->list[0]->line_number,
        this->body->list[0]->file_name
    );
    wild = 0;
    switch (this->type)
    {
    default:
        emit_str("#include");
        break;

    case 2:
        emit_str("#include-cooked");
        wild = 1;
        break;

    case 3:
        emit_str("#include-cooked-nowarn");
        wild = 1;
        break;
    }
    for (j = 0; j < this->body->length; ++j)
    {
        emit_char(' ');
        wild2 = (wild && wildchars(this->body->list[j]));
        if (wild2)
            emit_str("[wildcard ");
        emit_string(this->body->list[j]->text);
        if (wild2)
            emit_char(']');
    }
    emit_bol();
}


static stmt_method_ty method =
{
    sizeof(stmt_include_ty),
    "include",
    0,                          /* constructor */
    destructor,
    emit,
    0,                          /* regroup */
    0,                          /* sort */
};


stmt_ty *
stmt_include_alloc(blob_list_ty *body, int type)
{
    stmt_include_ty *result;
    blob_list_ty    *body2;

    result = (stmt_include_ty *) stmt_alloc(&method);
    body2 = blob_list_alloc();
    variable_rename_list(body, body2, &result->ref, VAREN_QUOTE_SPACES);
    blob_list_free(body);
    result->body = body2;
    result->type = type;
    return (stmt_ty *)result;
}
