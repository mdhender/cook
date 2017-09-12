/*
 *	cook - file construction tool
 *	Copyright (C) 1997 Peter Miller;
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
 * MANIFEST: implement missing functions from <libintl.h>
 */

#include <ac/libintl.h>


/*
 * NAME
 *	gettext
 *
 * DESCRIPTION
 *	The gettext function is used to translate messages into a
 *	language dictated by the LANG environment variable (et al).  If
 *	the current operating system does not supply one, then pass the
 *	message through unchanged.  (This is what gettext will do for
 *	error messages it does not have a translation for.)
 */

#ifndef HAVE_GETTEXT

char *
gettext(s)
	const char	*s;
{
	return (char *)s;
}

#endif /* !HAVE_GETTEXT */
