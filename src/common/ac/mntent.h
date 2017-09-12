/*
 *      cook - file construction tool
 *      Copyright (C) 1999, 2003, 2006-2008 Peter Miller
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

#ifndef COMMON_AC_MNTENT_H
#define COMMON_AC_MNTENT_H

#include <common/config.h>
#include <common/ac/stdio.h>

#ifdef HAVE_MNTENT_H
#include <mntent.h>
#else

#include <common/main.h>

struct mntent
{
        char *mnt_dir;
};


FILE *setmntent(const char *, const char *);
struct mntent *getmntent(FILE *);
int endmntent(FILE *);

#endif /* !HAVE_MNTENT_H */

/*
 * On AIX, mntent.h doesn't define MOUNTED
 * so this will be done on AIX, or on systems without mntent.h
 */
#ifndef MOUNTED
#define MOUNTED "/dev/null"
#endif

#endif /* COMMON_AC_MNTENT_H */
