/*
 *      cook - file construction tool
 *      Copyright (C) 1999, 2006-2008 Peter Miller
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

#include <common/input/crlf.h>
#include <common/input/file.h>
#include <common/input/file_text.h>
#include <common/str.h>


input_ty *
input_file_text_open(string_ty *fn)
{
    return input_crlf(input_file_open(fn), 1);
}
