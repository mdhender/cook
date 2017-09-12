/*
 *      cook - file construction tool
 *      Copyright (C) 1997-1999, 2001, 2006, 2007 Peter Miller;
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

#include <common/ac/signal.h>
#include <common/ac/string.h>

#include <cook/desist.h>
#include <common/error_intl.h>
#include <cook/os_interface.h>
#include <common/star.h>


static int      desist_flag;


/*
 * NAME
 *      interrupt - handle interrupts by signals
 *
 * SYNOPSIS
 *      void interrupt(int);
 *
 * DESCRIPTION
 *      The interrupt function is used to set the appropriate flags when an
 *      interrupt occurs.
 *
 * RETURNS
 *      void
 *
 * CAVEAT
 *      Don't use too extravagantly, because it just returns!
 */

static RETSIGTYPE
interrupt(int n)
{
    sub_context_ty  *scp;

    star_eoln();
    desist_flag++;
    scp = sub_context_new();
    sub_var_set(scp, "Name", "%s", safe_strsignal(n));
    error_intl(scp, i18n("interrupted by $name"));
    sub_context_delete(scp);
    signal(n, interrupt);
}


/*
 * NAME
 *      desist_requested
 *
 * SYNOPSIS
 *      int desist_requested(void);
 *
 * DESCRIPTION
 *      The desist_requested function is used to test whether the user
 *      has requested that we deist by sending an interrupt.
 */

int
desist_requested(void)
{
    int             result;

    result = (desist_flag != 0);
    desist_flag = 0;
    return result;
}


/*
 * NAME
 *      desist_enable
 *
 * SYNOPSIS
 *      void desist_enable(void);
 *
 * DESCRIPTION
 *      The desist_enable function is used to set the signal handlers
 *      which will trap user desist requests.
 */

void
desist_enable(void)
{
    if (signal(SIGINT, SIG_IGN) != SIG_IGN)
        signal(SIGINT, interrupt);
    if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
        signal(SIGHUP, interrupt);
    if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
        signal(SIGQUIT, interrupt);
    if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
        signal(SIGTERM, interrupt);
}
