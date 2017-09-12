/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 1999, 2004, 2006, 2007 Peter Miller;
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

#include <etc/libdir-h>
#include <common/libdir.h>


const char *
library_directory_get(void)
{
    return LIBDIR;
}


const char *
data_directory_get(void)
{
    return DATADIR;
}


const char *
manual_directory_get(void)
{
    return MANDIR;
}


const char *
executable_extension_get(void)
{
    return EXEEXT;
}


const char *
configured_nlsdir(void)
{
    return NLSDIR;
}
