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

#include <cook/match/cook.h>
#include <cook/match/private.h>
#include <cook/match/regex.h>
#include <cook/option.h>


match_ty *
match_new(void)
{
    match_ty        *this;

    if (option_test(OPTION_MATCH_MODE_REGEX))
        this = match_regex_new();
    else
        this = match_cook_new();
    return this;
}
