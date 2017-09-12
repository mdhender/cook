/*
 *      cook - file construction tool
 *      Copyright (C) 2001, 2006-2008 Peter Miller
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

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/unistd.h>

#include <common/str.h>
#include <cook/tempfilename.h>


string_ty *
temporary_filename(void)
{
    char            iname[100];
    static int      nnn;

#ifdef __CYGWIN32__
    /*
     * For some reason CygWin32's tmpnam() always produces
     * filenames which give ``No such file or directory''
     */
    snprintf(iname, sizeof(iname), "t%dp%d.tmp", ++nnn, getpid());
#else
    /*
     * I'd use tmpnam(), but the GNU linker now says it is too
     * dangerous and won't link it any more.
     */
    char *tmpdir = 0;
    if (!geteuid())
        tmpdir = "/tmp";
    else
    {
        tmpdir = getenv("TMPDIR");
        if (!tmpdir || *tmpdir != '/')
            tmpdir = "/tmp";
    }
    snprintf(iname, sizeof(iname), "%s/t%dp%d", tmpdir, ++nnn, getpid());
#endif
    return str_from_c(iname);
}


string_ty *
dot_temporary_filename(void)
{
    static long     temp_file_number;

    return str_format(".%d.%ld", getpid(), ++temp_file_number);
}
