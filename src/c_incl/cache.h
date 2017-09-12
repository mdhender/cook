/*
 *      cook - file construction tool
 *      Copyright (C) 1991-1994, 1997, 2006, 2007 Peter Miller;
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

#ifndef CACHE_H
#define CACHE_H

#include <sys/types.h>
#include <sys/stat.h>

#include <common/main.h>
#include <common/str_list.h>

typedef struct cache_ty cache_ty;
struct cache_ty
{
        struct stat     st;
        string_list_ty  ingredients;
};

void cache_initialize(void);
cache_ty *cache_search(string_ty *filename);
void cache_read(void);
void cache_write(void);
void cache_update_notify(void);

#endif /* CACHE_H */
