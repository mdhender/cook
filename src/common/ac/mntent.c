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

#include <common/ac/mntent.h>


#ifndef HAVE_MNTENT_H

FILE *
setmntent(filename, mode)
        const char      *filename;
        const char      *mode;
{
        return fopen(filename, mode);
}


struct mntent *
getmntent(fp)
        FILE            *fp;
{
        /* do nothing */
        return 0;
}


int
endmntent(fp)
        FILE            *fp;
{
        return fclose(fp);
}

#endif /* HAVE_MNTENT_H */
