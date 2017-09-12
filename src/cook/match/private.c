/*
 *      cook - file construction tool
 *      Copyright (C) 1999, 2006, 2007 Peter Miller;
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

#include <cook/match/private.h>
#include <common/mem.h>


match_ty *
match_private_new(match_method_ty *vptr)
{
    match_ty        *this;

    this = mem_alloc(vptr->size);
    this->vptr = vptr;
    if (this->vptr->constructor)
        this->vptr->constructor(this);
    return this;
}


match_ty *
match_clone(match_ty *this)
{
    return match_private_new(this->vptr);
}
