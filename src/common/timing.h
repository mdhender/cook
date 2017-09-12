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

#ifndef COMMON_TIMING_H
#define COMMON_TIMING_H

#include <common/main.h>

#ifdef __GNUC__

void timing_register(char *, void **);
void timing_push(void *);
void timing_pop(void *);

#define timing_entry() do { static void *__timing_p; if (!__timing_p) \
timing_register( __FILE__ ":" __FUNCTION__, &__timing_p); \
timing_push(__timing_p); } while (0)

#define timing_exit() do { static void *__timing_p; if (!__timing_p) \
timing_register( __FILE__ ":" __FUNCTION__, &__timing_p); \
timing_pop(__timing_p); } while (0)

#else

#define timing_entry() ((void)0)
#define timing_exit() ((void)0)

#endif

void timing_print(void);

#endif /* COMMON_TIMING_H */
