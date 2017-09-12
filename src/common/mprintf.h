/*
 *      cook - file construction tool
 *      Copyright (C) 1994, 1995, 1997, 2006, 2007 Peter Miller;
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

#ifndef COMMON_MPRINTF_H
#define COMMON_MPRINTF_H

#include <common/ac/stdarg.h>
#include <common/main.h>

char *mprintf(const char *fmt, ...)                         FORMAT_PRINTF(1, 2);
char *mprintfe(const char *fmt, ...)                        FORMAT_PRINTF(1, 2);
char *vmprintf(const char *fmt, va_list)                    FORMAT_VPRINTF(1);
char *vmprintfe(const char *fmt, va_list)                   FORMAT_VPRINTF(1);
struct string_ty *vmprintfes(const char *fmt, va_list)      FORMAT_VPRINTF(1);

#endif /* COMMON_MPRINTF_H */
