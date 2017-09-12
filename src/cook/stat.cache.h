/*
 *      cook - file construction tool
 *      Copyright (C) 1994, 1997, 1998, 2006, 2007 Peter Miller;
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

#ifndef COOK_STAT_CACHE_H
#define COOK_STAT_CACHE_H

#include <common/ac/time.h>
#include <common/str.h>

time_t stat_cache_oldest(string_ty *, int);
time_t stat_cache_newest(string_ty *, int);
void stat_cache_set(string_ty *, time_t, int);
void stat_cache_clear(string_ty *);

#endif /* COOK_STAT_CACHE_H */
