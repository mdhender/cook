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

#ifndef COMMON_AC_REGEX_H
#define COMMON_AC_REGEX_H

#include <common/config.h>

#ifdef HAVE_RXPOSIX_H
#include <rxposix.h>
#else
#ifdef HAVE_REGEX_H
#include <sys/types.h>
#include <regex.h>
#else

#define regex_t int
#define REG_EXTENDED 0
#define REG_NOSUB 0
#define REG_NOMATCH 0
#define REG_NOTBOL 0

#endif
#endif

#ifndef REG_BASIC
#define REG_BASIC 0
#endif

#endif /* COMMON_AC_REGEX_H */
