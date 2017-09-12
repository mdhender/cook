/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 1999, 2001, 2006-2008 Peter Miller
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

#ifndef COMMON_QUIT_H
#define COMMON_QUIT_H

#include <common/main.h>
#include <common/noreturn.h>

typedef void (*quit_ty)(void);
void quit_handler(quit_ty);
void quit_handler_prio(quit_ty);
void quit(int) NORETURN;

#endif /* COMMON_QUIT_H */
