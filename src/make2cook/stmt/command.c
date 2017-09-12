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

#include <make2cook/blob.h>
#include <make2cook/emit.h>
#include <make2cook/stmt/command.h>
#include <common/trace.h>
#include <make2cook/variable.h>
#include <common/str_list.h>


typedef struct stmt_command_ty stmt_command_ty;
struct stmt_command_ty
{
    STMT
    blob_ty        *text;
};


static void
constructor(stmt_ty *that)
{
    stmt_command_ty *this;

    trace(("command::constructor()\n{\n"));
    this = (stmt_command_ty *)that;
    this->text = 0;
    trace(("}\n"));
}


static void
destructor(stmt_ty *that)
{
    stmt_command_ty *this;

    trace(("command::destructor()\n{\n"));
    this = (stmt_command_ty *) that;
    if (this->text)
        blob_free(this->text);
    trace(("}\n"));
}


static void
emit(stmt_ty *that)
{
    stmt_command_ty *this;
    char            *cp;
    string_ty       *s;
    blob_ty         *bp;
    string_list_ty  flag;
    size_t          j;
    static string_ty *empty_quotes;

    trace(("command::emit()\n{\n"));
    this = (stmt_command_ty *)that;
    bp = this->text;
    if (!empty_quotes)
        empty_quotes = str_from_c("\"\"");
    if (!bp || bp->text->str_length == 0 || str_equal(bp->text, empty_quotes))
    {
        emit_str(";\n");
        trace(("}\n"));
        return;
    }

    cp = bp->text->str_text;
    string_list_constructor(&flag);
    for (;;)
    {
        switch (*cp)
        {
        case '-':
            s = str_from_c("errok");
            string_list_append(&flag, s);
            str_free(s);
            ++cp;
            continue;

        case '@':
            s = str_from_c("silent");
            string_list_append(&flag, s);
            str_free(s);
            ++cp;
            continue;

        case '+':
            s = str_from_c("notouch");
            string_list_append(&flag, s);
            str_free(s);
            ++cp;
            continue;

        default:
            break;
        }
        break;
    }
    emit_line_number(bp->line_number, bp->file_name);
    emit_str(cp);

    if (flag.nstrings)
    {
        emit_bol();
        emit_indent_more();
        emit_str("set");
        for (j = 0; j < flag.nstrings; ++j)
        {
            emit_char(' ');
            emit_string(flag.string[j]);
        }
        emit_indent_less();
    }
    string_list_destructor(&flag);
    emit_str(";\n");
    trace(("}\n"));
}


static stmt_method_ty method =
{
    sizeof(stmt_command_ty),
    "command",
    constructor,
    destructor,
    emit,
    0,
    0,
};


stmt_ty *
stmt_command_alloc(blob_ty *bp)
{
    stmt_ty         *result;
    stmt_command_ty *this;
    blob_list_ty    *blp;
    string_ty       *s;
    size_t          j;

    /*
     * allocate the statement
     */
    trace(("stmt_command_alloc()\n{\n"));
    result = stmt_alloc(&method);
    this = (stmt_command_ty *)result;

    /*
     * replace the variable names
     */
    blp = blob_list_alloc();
    variable_rename(bp, blp, &this->rref, VAREN_NO_FLAGS);

    /*
     * reconstruct as a single string
     */
    s = blp->length ? str_copy(blp->list[0]->text) : str_from_c("");
    for (j = 1; j < blp->length; ++j)
    {
        string_ty       *s2;

        s2 = str_format("%s %s", s->str_text, blp->list[j]->text->str_text);
        str_free(s);
        s = s2;
    }
    blob_list_free(blp);
    this->text = blob_alloc(s, bp->file_name, bp->line_number);
    blob_free(bp);

    /*
     * all done
     */
    trace(("}\n"));
    return result;
}
