/*
 *      cook - file construction tool
 *      Copyright (C) 2006, 2007 Peter Miller;
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

#ifndef COMMON_FORMAT_PRINT_H
#define COMMON_FORMAT_PRINT_H

#ifdef __GNUC__
#define FORMAT_PRINTF(x, y) __attribute__((format(printf, x, y)))
#else
#define FORMAT_PRINTF(x, y)
#endif
#define FORMAT_VPRINTF(x) FORMAT_PRINTF(x, 0)

#endif /* COMMON_FORMAT_PRINT_H */
