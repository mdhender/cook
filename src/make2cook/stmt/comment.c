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

#include <make2cook/emit.h>
#include <common/mem.h>
#include <make2cook/stmt/comment.h>
#include <common/str_list.h>

typedef struct stmt_comment_ty stmt_comment_ty;
struct stmt_comment_ty
{
    STMT
    blob_ty         *body;
};


static void
constructor(stmt_ty *that)
{
    stmt_comment_ty *this;

    this = (stmt_comment_ty *)that;
    this->body = 0;
}


static void
destructor(stmt_ty *that)
{
    stmt_comment_ty *this;

    this = (stmt_comment_ty *)that;
    if (this->body)
        blob_free(this->body);
}


static void
emit(stmt_ty *that)
{
    stmt_comment_ty *this;
    blob_ty         *bp;
    size_t          min;
    size_t          max;
    size_t          j;
    string_list_ty  wl;

    this = (stmt_comment_ty *)that;
    bp = this->body;
    if (!bp->text->str_length)
        return;
    str2wl(&wl, bp->text, "\n", 0);
    for (min = 0; min < wl.nstrings; ++min)
        if (wl.string[min]->str_length)
            break;
    if (min >= wl.nstrings)
        return;
    for (max = wl.nstrings; max > 0; --max)
        if (wl.string[max - 1]->str_length)
            break;
    emit_line_number(bp->line_number - (min == 0), bp->file_name);
    emit_str("/*\n");
    for (j = min; j < max; ++j)
    {
        static string_ty *t1;
        static string_ty *t2;
        string_ty       *tmp;

        if (!t1)
        {
            t1 = str_from_c("*/");
            t2 = str_from_c("* /");
        }
        tmp = str_substitute(t1, t2, wl.string[j]);
        emit_str(" *");
        if (tmp->str_length)
        {
            emit_char(' ');
            emit_str(tmp->str_text);
        }
        emit_char('\n');
        str_free(tmp);
    }
    string_list_destructor(&wl);
    emit_str(" */\n");
}


static stmt_method_ty method =
{
    sizeof(stmt_comment_ty),
    "comment",
    constructor,
    destructor,
    emit,
    0,                          /* regroup */
    0,                          /* sort */
};


stmt_ty *
stmt_comment_alloc(blob_ty *bp)
{
    stmt_ty         *result;
    stmt_comment_ty *this;

    result = stmt_alloc(&method);
    result->white_space = 1;
    this = (stmt_comment_ty *) result;
    this->body = bp;
    return result;
}
