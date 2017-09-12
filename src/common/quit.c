/*
 *	cook - file construction tool
 *	Copyright (C) 1997, 1999, 2001 Peter Miller;
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
 * MANIFEST: functions to quit the program
 */

#include <ac/stdlib.h>

#include <quit.h>
#include <star.h>


static quit_ty	quit_list_prio[4];
static int	quit_list_prio_length;

static quit_ty	quit_list[32];
static int	quit_list_length;


/*
 * NAME
 *	quit_handler
 *
 * SYNOPSIS
 *	int quit_handler(quit_ty);
 *
 * DESCRIPTION
 *	The quit_handler function registers the function pointed to by func,
 *	to be called without arguments at normal program termination.
 */

void
quit_handler(f)
	quit_ty		f;
{
	/* assert(quit_list_length < SIZEOF(quit_list)); */
	if (quit_list_length < SIZEOF(quit_list))
		quit_list[quit_list_length++] = f;
}

void
quit_handler_prio(f)
	quit_ty		f;
{
	/* assert(quit_list_prio_length < SIZEOF(quit_list)); */
	if (quit_list_prio_length < SIZEOF(quit_list))
		quit_list_prio[quit_list_prio_length++] = f;
}


/*
 * NAME
 *	quit - leave program
 *
 * SYNOPSIS
 *	void quit(int status);
 *
 * DESCRIPTION
 *	The quit function causes normal program termination to occur.
 *
 *	First, all functions registered by the quit_handler function are
 *	called, in the reverse order of their registration.
 *
 *	Next, the program is terminated using the exit() function.
 *
 * CAVEAT
 *	The quit function never returns to its caller.
 */

void
quit(n)
	int		n;
{
	star_eoln();
	while (quit_list_prio_length > 0)
		(*quit_list_prio[--quit_list_prio_length])();
	while (quit_list_length > 0)
		(*quit_list[--quit_list_length])();
	exit(n);
}
