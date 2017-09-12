/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 2006, 2007 Peter Miller;
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

#include <make2cook/emit.h>
#include <make2cook/stmt/export.h>
#include <common/symtab.h>
#include <make2cook/variable.h>

typedef struct stmt_export_ty stmt_export_ty;
struct stmt_export_ty
{
    STMT
    blob_ty         *name;
};


static void
destructor(stmt_ty *that)
{
    stmt_export_ty  *this;

    this = (stmt_export_ty *)that;
    blob_free(this->name);
}


static void
emit(stmt_ty *that)
{
    stmt_export_ty  *this;

    this = (stmt_export_ty *)that;
    emit_line_number(this->name->line_number, this->name->file_name);
    emit_str("setenv ");
    emit_string(this->name->text);
    emit_str(" = [");
    emit_string(this->name->text);
    emit_str("];\n");
}


static stmt_method_ty method =
{
    sizeof(stmt_export_ty),
    "export",
    0,                          /* constructor */
    destructor,
    emit,
    0,
    0,
};


stmt_ty *
stmt_export_alloc(blob_ty *name)
{
    stmt_export_ty  *this;
    blob_list_ty    *tmp;

    this = (stmt_export_ty *)stmt_alloc(&method);

    /*
     * turn the make names into cook names
     */
    tmp = blob_list_alloc();
    variable_rename(name, tmp, &this->ref, VAREN_QUOTE_SPACES);
    blob_free(name);

    this->name = blob_copy(tmp->list[0]);
    blob_list_free(tmp);

    return (stmt_ty *)this;
}
