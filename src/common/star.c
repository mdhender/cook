/*
 *	cook - file construction tool
 *	Copyright (C) 1997, 2001, 2003, 2004 Peter Miller;
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
 * MANIFEST: functions to manipulate stars
 */

#include <ac/stdio.h>
#include <ac/string.h>
#include <ac/time.h>

#include <fflush_slow.h>
#include <page.h>
#include <progname.h>
#include <quit.h>
#include <star.h>
#include <trace.h> /* for assert */

static	int	previous;
static	int	star_flag;
static	int	star_col;
static	time_t	star_time;


/*
 * NAME
 *	star
 *
 * SYNOPSIS
 *	void star(void);
 *
 * DESCRIPTION
 *	The star function is used to emit a progress star, if they are
 *	enabled.
 */

void
star()
{
	star_as_specified('*');
}


/*
 * NAME
 *	star_as_specified
 *
 * SYNOPSIS
 *	void star_as_specified(int);
 *
 * DESCRIPTION
 *	The star_as_specified function is used to emit the given
 *	character as a progress star, if they are enabled.
 */

void
star_as_specified(c)
	int		c;
{
	int		immediate;
	time_t		now;
	static int	page_width;

	if (!star_flag)
		return;
	if (!page_width)
	    page_width = page_width_get() - 1;
	if (c & STAR_IMMEDIATE)
	{
		c &= ~STAR_IMMEDIATE;
		immediate = 1;
	}
	else
		immediate = 0;
	immediate |= (c != previous);
	time(&now);
	if (now < star_time && !immediate)
		return;
	previous = c;
	if (!star_col)
	{
		char	*progname;

		progname = progname_get();
		fprintf(stderr, "%s: ", progname);
		star_col = strlen(progname) + 2;
	}
	fputc(c, stderr);
	star_col++;
	if (star_col >= page_width)
	{
		fputc('\n', stderr);
		star_col = 0;
	}
	fflush_slowly(stderr);
	star_time = now + 1;
}


/*
 * NAME
 *	star_eoln
 *
 * SYNOPSIS
 *	void star_eoln(void);
 *
 * DESCRIPTION
 *	The star_eoln function is used to end a line of progress stars,
 *	if any have been issued.  This should be done prior to any
 *	output.
 */

void
star_eoln()
{
	time_t		now;

	if (!star_flag)
		return;
	if (star_col)
	{
		fputc('\n', stderr);
		star_col = 0;
		fflush_slowly(stderr);
	}
	time(&now);
	star_time = now + 1;
}


/*
 * NAME
 *	star_sync
 *
 * SYNOPSIS
 *	void star_sync(void);
 *
 * DESCRIPTION
 *	The star_eoln function is used to end a line of progress stars,
 *	if any have been issued.  It also introduces a delay of 5
 *	seconds, rather than the defalt 1 second.  This should be done
 *	prior to any echoed command.
 */

void
star_sync()
{
	time_t		when;

	if (!star_flag)
		return;
	if (star_col)
	{
		fputc('\n', stderr);
		star_col = 0;
		fflush(stderr);
	}
	time(&when);
	when += 5;
	if (when > star_time)
		star_time = when;
}


/*
 * NAME
 *	star_quit_handler
 *
 * SYNOPSIS
 *	void star_quit_handler(void);
 *
 * DESCRIPTION
 *	The star_quit_handler function is used to clean up any trailing
 *	lines of stars at quit() time.
 *
 * RETURNS
 */

static void star_quit_handler _((void));

static void
star_quit_handler()
{
	star_flag = 0;
	if (star_col)
	{
		fputc('\n', stderr);
		star_col = 0;
	}
}


/*
 * NAME
 *	star_enable
 *
 * SYNOPSIS
 *	void star_enable(void);
 *
 * DESCRIPTION
 *	The star_enable function is used to turn on the emitting of
 *	progress stars.  The default it silence.
 */

void
star_enable()
{
	if (star_flag)
		return;
	star_flag = 1;
	quit_handler(star_quit_handler);
	trace(("enabled\n"));
}


/*
 * NAME
 *	star_bang
 *
 * SYNOPSIS
 *	void star_bang(void);
 *
 * DESCRIPTION
 *	The star function is used to emit a iminus as a progress star,
 *	if they are enabled.  (Silent command execution indicator.)
 */

void
star_bang()
{
	star_as_specified('-');
}
