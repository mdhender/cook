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

#include <cook/lex/filename.h>
#include <cook/lex/filenamelist.h>
#include <common/mem.h>


void
lex_filename_list_constructor(lex_filename_list_ty *this)
{
    this->length = 0;
    this->maximum = 0;
    this->list = 0;
}


lex_filename_list_ty *
lex_filename_list_new(void)
{
    lex_filename_list_ty *this;

    this = mem_alloc(sizeof(lex_filename_list_ty));
    lex_filename_list_constructor(this);
    return this;
}


void
lex_filename_list_destructor(lex_filename_list_ty *this)
{
    size_t          j;

    for (j = 0; j < this->length; ++j)
        lex_filename_delete(this->list[j]);
    if (this->list)
        mem_free(this->list);
    this->length = 0;
    this->maximum = 0;
    this->list = 0;
}


void
lex_filename_list_delete(lex_filename_list_ty *this)
{
    lex_filename_list_destructor(this);
    mem_free(this);
}


void
lex_filename_list_push_back(lex_filename_list_ty *this, lex_filename_ty *p)
{
    if (this->length >= this->maximum)
    {
        size_t          nbytes;

        this->maximum = this->maximum * 2 + 4;
        nbytes = this->maximum * sizeof(this->list[0]);
        this->list = mem_change_size(this->list, nbytes);
    }
    this->list[this->length++] = p;
}


lex_filename_ty *
lex_filename_list_pop_front(lex_filename_list_ty *this)
{
    lex_filename_ty *p;
    size_t          j;

    if (!this->length)
        return 0;
    p = this->list[0];
    for (j = 1; j < this->length; ++j)
        this->list[j - 1] = this->list[j];
    this->length--;
    return p;
}
