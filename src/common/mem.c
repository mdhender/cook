/*
 *	cook - file construction tool
 *	Copyright (C) 1991, 1992, 1993, 1994, 1997, 1999, 2001 Peter Miller;
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
 * MANIFEST: functions to manipulate dynamic memory
 */

#include <ac/stddef.h>
#include <ac/string.h>
#include <ac/stdlib.h>
#include <ac/errno.h>

#include <error.h>
#include <mem.h>


/*
 * NAME
 *	memory_error - diagnostic
 *
 * SYNOPSIS
 *	void memory_error(void);
 *
 * DESCRIPTION
 *	The memory_error function is used to report fatal problems with the
 *	memory allocator.
 *
 * RETURNS
 *	The memory_error function does not return.
 */

static void memory_error _((void));

static void
memory_error()
{
#ifdef DEBUG
	nerror_raw("memory allocator");
	abort();
#else
	nfatal_raw("memory allocator");
#endif
}


/*
 * NAME
 *	mem_alloc - allocate and clear memory
 *
 * SYNOPSIS
 *	char *mem_alloc(size_t n);
 *
 * DESCRIPTION
 *	Mem_alloc uses malloc to allocate the required sized chunk of memory.
 *	If any error is returned from malloc() an fatal diagnostic is issued.
 *	The memory is zeroed befor it is returned.
 *
 * CAVEAT
 *	It is the responsibility of the caller to ensure that the space is
 *	freed when finished with, by a call to free().
 */

void *
mem_alloc(n)
	size_t		n;
{
	void		*p;

	if (n < 1)
		n = 1;
	errno = ENOMEM;
	p = malloc(n);
	if (!p)
		memory_error();
	return p;
}


/*
 * NAME
 *	mem_alloc_clear - allocate and clear memory
 *
 * SYNOPSIS
 *	char *mem_alloc_clear(size_t n);
 *
 * DESCRIPTION
 *	Mem_alloc_clear uses malloc to allocate the required sized chunk
 *	of memory.  If any error is returned from malloc() an fatal
 *	diagnostic is issued.  The memory is zeroed befor it is returned.
 *
 * CAVEAT
 *	It is the responsibility of the caller to ensure that the space is
 *	freed when finished with, by a call to free().
 */

void *
mem_alloc_clear(n)
	size_t		n;
{
	void		*p;

	p = mem_alloc(n);
	memset(p, 0, n);
	return p;
}


void *
mem_change_size(p, n)
	void		*p;
	size_t		n;
{
	if (n < 1)
		n = 1;
	errno = ENOMEM;
	if (!p)
		p = malloc(n);
	else
		p = realloc(p, n);
	if (!p)
		memory_error();
	return p;
}


void
mem_free(p)
	void		*p;
{
	free(p);
}


char *
mem_copy_string(s)
	char		*s;
{
	char		*cp;

	cp = mem_alloc(strlen(s) + 1);
	strcpy(cp, s);
	return cp;
}
