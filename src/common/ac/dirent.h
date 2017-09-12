/*
 *	cook - file construction tool
 *	Copyright (C) 1994, 1997, 1998, 1999 Peter Miller;
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
 * MANIFEST: insulate against <dirent.h> vs <ndir.h> differences
 */

#ifndef COMMON_AC_DIRENT_H
#define COMMON_AC_DIRENT_H

#include <config.h>

#ifdef HAVE_DIRENT_H
#include <sys/types.h>
#include <dirent.h>
#define NLENGTH(dirent) (strlen((dirent)->d_name))
#else /* not dirent.h */
#define dirent direct
#define NLENGTH(dirent) ((dirent)->d_namlen)
#ifdef HAVE_SYS_NDIR_H
#include <sys/ndir.h>
#endif /* SYSNDIR */
#ifdef HAVE_SYS_DIR_H
#include <sys/dir.h>
#endif /* SYSDIR */
#ifdef HAVE_NDIR_H
#include <ndir.h>
#endif /* NDIR */
#endif /* not dirent.h */

#endif /* COMMON_AC_DIRENT_H */
