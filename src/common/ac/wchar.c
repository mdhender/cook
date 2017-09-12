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
 * MANIFEST: impliment missing functions from <wchar.h>
 */

#include <ac/wchar.h>


#ifndef HAVE_WCSLEN


size_t
wcslen(wcs)
	const wchar_t	*wcs;
{
	const wchar_t	*start;

	start = wcs;
	while (*wcs)
		++wcs;
	return (wcs - start);
}


#endif /* !HAVE_WCSLEN */
