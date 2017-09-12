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

#include <common/ac/stdarg.h>
#include <common/ac/stdio.h>


#ifndef HAVE_SNPRINTF
#undef sprintf

int
snprintf(char *buffer, size_t nbytes, const char *fmt, ...)
{
    va_list         ap;

    va_start(ap, fmt);
    vsnprintf(buffer, nbytes, fmt, ap);
    va_end(ap);
}

#endif


#ifndef HAVE_VSNPRINTF
#undef vsprintf

int
vsnprintf(char *buffer, size_t nbytes, const char *fmt, ...)
{
    vsprintf(buffer, fmt, ap);
}

#endif
