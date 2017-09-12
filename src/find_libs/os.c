/*
 *      cook - file construction tool
 *      Copyright (C) 1990-1993, 1997-1999, 2006, 2007 Peter Miller;
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

#include <common/ac/errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <common/error_intl.h>
#include <c_incl/os_interface.h>

/*
 *  NAME
 *      os_exists - tests for the existence of a file
 *
 *  SYNOPSIS
 *      int os_exists(char *filename);
 *
 *  DESCRIPTION
 *      Os_returns 1 if the file exists, 0 if it does not.
 */

int
os_exists(char *filename)
{
    struct stat     st;

    if (stat(filename, &st))
    {
        switch (errno)
        {
        case ENOENT:
        case ENOTDIR:
            break;

        default:
            fatal_intl_stat(filename);
            /* NOTREACHED */
        }
        return 0;
    }
    return 1;
}
