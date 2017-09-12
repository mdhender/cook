/*
 *      cook - file construction tool
 *      Copyright (C) 1996, 1997, 2006, 2007 Peter Miller;
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

#include <common/ac/string.h>
#include <common/ac/sys/utsname.h>


#ifndef HAVE_UNAME

int
uname(p)
        struct utsname  *p;
{
        /*
         * Please remember that this function is only compiled on truly
         * ancient and stupid systems which do not provide the uname
         * system call.  As such, the code it pretty stupid, because it
         * represents just enough to distinguish all such stupid systems
         * the author has had the misfortune to use, which thankfully
         * isn't many.
         */
#if defined(sun) || defined(__sun__)
        strendcpy(p->sysname, "SunOS", p->sysname + sizeof(p->sysname));
#else
#if defined(bsd) || defined(__bsd__)
        strendcpy(p->sysname, "BSD", p->sysname + sizeof(p->sysname));
#else
#if defined(unix) || defined(__unix__) || defined(UNIX) || defined(__UNIX__)
        strendcpy(p->sysname, "UNIX", p->sysname + sizeof(p->sysname));
#else
        strendcpy(p->sysname, "unknown", p->sysname + sizeof(p->sysname));
#endif
#endif
#endif
#ifdef HAVE_GETHOSTNAME
        if (gethostname(p->nodename, sizeof(p->nodename)) != 0)
#endif
        strendcpy(p->nodename, "unknown", p->nodename + sizeof(p->nodename));
        strendcpy(p->release, "unknown", p->release + sizeof(p->release));
        strendcpy(p->version, "unknown", p->version + sizeof(p->version));
#if defined(sun3) || defined(__sun3__) || \
                defined(m68000) || defined(__m68000__) || \
                defined(mc68000) || defined(__mc68000__) || \
                defined(m68k) || defined(__m68k__)
        strendcpy(p->machine, "m68000", p->machine + sizeof(p->machine));
#else
        strendcpy(p->machine, "unknown", p->machine + sizeof(p->machine));
#endif
        return 0;
}

#endif
