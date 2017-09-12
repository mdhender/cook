/*
 *      cook - file construction tool
 *      Copyright (C) 1999, 2006, 2007 Peter Miller;
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

#include <common/ac/time.h>

#include <cook/fingerprint/find.h>
#include <cook/fingerprint/sync.h>


/*
 * NAME
 *      fp_sync
 *
 * SYNOPSIS
 *      void fp_sync(void);
 *
 * DESCRIPTION
 *      The fp_sync function is used to write the fingerprint cache
 *      out to disk periodically.  No matter how often it it called,
 *      the fingerprint cache will only be written out once a minute.
 *      This should be called reasonably often.
 */

void
fp_sync(void)
{
    static time_t   next_time;
    time_t          now;

    time(&now);
    if (!next_time)
        next_time = now + 60;
    else if (now >= next_time)
    {
        next_time = now + 60;
        fp_find_flush();
    }
}
