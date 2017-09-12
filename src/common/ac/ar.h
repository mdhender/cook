/*
 *	cook - file construction tool
 *	Copyright (C) 1994, 1997 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: isolate <ar.h> differences
 */

#ifndef COMMON_AC_AR_H
#define COMMON_AC_AR_H

#include <config.h>

/*
 * On many System V release N derivatives, ar.h defines two different
 * archive formats depending upon whether you have defined PORTAR
 * (normal) or PORT5AR (System V Release 1).  There is no default, one
 * or the other must be defined to have a nonzero value.
 */
#undef PORTAR
#undef PORT5AR

/*
 * SCO Unix's compiler defines both of these.
 */
#ifdef M_UNIX
#undef M_XENIX
#endif

/*
 * According to Jim Sievert <jas1@rsvl.unisys.com>, for SCO XENIX
 * defining PORTAR to 1 gets the wrong archive format, and defining it
 * to 0 gets the right one.
 */
#ifdef M_XENIX
#define PORTAR 0
#else
#define PORTAR 1
#endif

/*
 * include the system's ar.h
 */
#ifdef HAVE_AR_H
#include <ar.h>
#endif

#endif /* COMMON_AC_AR_H */
