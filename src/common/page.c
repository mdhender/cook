/*
 *	cook - file construction tool
 *	Copyright (C) 1997, 1998, 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate pages
 */

#include <ac/stdlib.h>
#include <ac/sys/ioctl.h>
#include <ac/termios.h>

#include <arglex.h>
#include <error_intl.h>
#include <help.h>
#include <page.h>


#define MIN_PAGE_WIDTH 40
/* MAX_PAGE_WIDTH is defined in common/page.h */
#define DEFAULT_PAGE_WIDTH 80
#define MIN_PAGE_LENGTH 10
#define MAX_PAGE_LENGTH 30000
#define DEFAULT_PAGE_LENGTH 24

static	int	page_length;
static	int	page_width;


static void default_page_sizes _((void));

static void
default_page_sizes()
{
	if (!page_width)
	{
		char	*cp;

		cp = getenv("COLS");
		if (cp)
		{
			int	n;

			n = atoi(cp);
			if (n > 0)
			{
				if (n < MIN_PAGE_WIDTH)
					n = MIN_PAGE_WIDTH;
				if (n > MAX_PAGE_WIDTH)
					n = MAX_PAGE_WIDTH;
				page_width = n;
			}
		}
	}
	if (!page_length)
	{
		char	*cp;

		cp = getenv("LINES");
		if (cp)
		{
			int	n;

			n = atoi(cp);
			if (n > 0)
			{
				if (n < MIN_PAGE_LENGTH)
					n = MIN_PAGE_LENGTH;
				if (n > MAX_PAGE_LENGTH)
					n = MAX_PAGE_LENGTH;
				page_length = n;
			}
		}
	}

#ifdef TIOCGWINSZ
	if (!page_width || !page_length)
	{
		struct winsize	ws;

		if (ioctl(0, TIOCGWINSZ, &ws) == 0)
		{
			if (!page_width && ws.ws_col > 0)
			{
				page_width = ws.ws_col;
				if (page_width < MIN_PAGE_WIDTH)
					page_width = MIN_PAGE_WIDTH;
				if (page_width > MAX_PAGE_WIDTH)
					page_width = MAX_PAGE_WIDTH;
			}
			if (!page_length && ws.ws_row > 0)
			{
				page_length = ws.ws_row;
				if (page_length < MIN_PAGE_LENGTH)
					page_length = MIN_PAGE_LENGTH;
				if (page_length > MAX_PAGE_LENGTH)
					page_length = MAX_PAGE_LENGTH;
			}
		}
	}
#endif

	if (!page_width)
		page_width = DEFAULT_PAGE_WIDTH;
	if (!page_length)
		page_length = DEFAULT_PAGE_LENGTH;
}


void
page_width_set(n)
	int		n;
{
	sub_context_ty	*scp;

	if (page_width)
	{
		arg_duplicate(arglex_token_page_width, 0);
		/* NOTREACHED */
	}
	if (n < MIN_PAGE_WIDTH || n > MAX_PAGE_WIDTH)
	{
		scp = sub_context_new();
		sub_var_set(scp, "Number", "%d", n);
		fatal_intl(scp, i18n("page width $number out of range"));
		/* NOTREACHED */
	}
	page_width = n;
}


int
page_width_get()
{
	/*
	 * must not generate a fatal error in this function,
	 * as it is used by 'error.c' when reporting fatal errors.
	 *
	 * must not put tracing in this function,
	 * because 'trace.c' uses it to determine the width.
	 */
	if (!page_width)
		default_page_sizes();
	return page_width;
}


void
page_length_set(n)
	int		n;
{
	sub_context_ty	*scp;

	if (page_length)
	{
		arg_duplicate(arglex_token_page_length, 0);
		/* NOTREACHED */
	}
	if (n < MIN_PAGE_LENGTH || n > MAX_PAGE_LENGTH)
	{
		scp = sub_context_new();
		sub_var_set(scp, "Number", "%d", n);
		fatal_intl(scp, i18n("page length $number out of range"));
		/* NOTREACHED */
	}
	page_length = n;
}


int
page_length_get()
{
	if (!page_length)
		default_page_sizes();
	return page_length;
}
