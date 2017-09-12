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
#include <common/mem.h>
#include <common/str.h>


void
lex_filename_constructor(lex_filename_ty *this, string_ty *logical,
    string_ty *physical)
{
    this->logical = str_copy(logical);
    this->physical = str_copy(physical);
}


lex_filename_ty *
lex_filename_new(string_ty *logical, string_ty *physical)
{
    lex_filename_ty *this;

    this = mem_alloc(sizeof(lex_filename_ty));
    lex_filename_constructor(this, logical, physical);
    return this;
}


void
lex_filename_destructor(lex_filename_ty *this)
{
    str_free(this->logical);
    str_free(this->physical);
    this->logical = 0;
    this->physical = 0;
}


void
lex_filename_delete(lex_filename_ty *this)
{
    lex_filename_destructor(this);
    mem_free(this);
}
