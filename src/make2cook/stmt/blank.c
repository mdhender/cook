/*
 *      cook - file construction tool
 *      Copyright (C) 1994, 1997, 2001, 2006-2008 Peter Miller
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

#include <make2cook/stmt/blank.h>

static stmt_method_ty method =
{
    sizeof(stmt_ty),
    "blank",
    0,                          /* constructor */
    0,                          /* destructor */
    0,                          /* emit */
    0,                          /* regroup */
    0,                          /* sort */
};


stmt_ty *
stmt_blank_alloc(void)
{
    stmt_ty         *result;

    result = stmt_alloc(&method);
    result->white_space = 1;
    return result;
}
