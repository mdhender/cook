/*
 *      cook - file construction tool
 *      Copyright (C) 2004, 2006-2008 Peter Miller
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

#include <common/ac/errno.h>
#include <common/ac/unistd.h>

#include <common/fflush_slow.h>
#include <common/progname.h>

/*
 * If the fflush call reports and error, we allow this many seconds for
 * recovery.  After that, we report the error as it happened.
 */
#define MAX_FLUSH_TRY 10

static int fflush_retry_count = 0;


int
fflush_slowly(FILE *fp)
{
    int             tries_remaining;

    tries_remaining = MAX_FLUSH_TRY;
    for (;;)
    {
        if (!fflush(fp) && !ferror(fp))
        {
            /*
             * No problem - quit trying.
             * Report success.
             */
            return 0;
        }

        /*
         * Linux has a weird problem, not seen on any other operating
         * system, where stderr sometimes gives EAGAIN for no aparrent
         * reason.
         */
        switch (errno)
        {
        case EAGAIN:
        case EBUSY:
        case ENOSPC:
            break;

        default:
            /*
             * The error isn't "Resource temporarily unavailable" so
             * report it immediately.
             */
            return EOF;
        }

        if (tries_remaining <= 0)
        {
            /* ran out of tries - not temporary */
            return EOF;
        }

        --tries_remaining;
        ++fflush_retry_count;

        /*
         * Perhaps a little rest will clear the problem.
         */
        sleep(1);

        /*
         * Clear the error for next try.
         */
        clearerr(fp);
    }
}


void
fflush_slowly_report(void)
{
    if (fflush_retry_count)
    {
        fprintf
        (
            stderr,
            "%s: needed %d fflush retries\n",
            progname_get(),
            fflush_retry_count
        );
    }
}
