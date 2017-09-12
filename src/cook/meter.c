/*
 *      cook - file construction tool
 *      Copyright (C) 2003, 2006, 2007 Peter Miller;
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

#include <common/ac/stdio.h>
#include <common/ac/string.h>

#include <common/mem.h>
#include <cook/meter.h>


meter_ty *
meter_alloc(void)
{
    meter_ty        *result;

    result = mem_alloc(sizeof(meter_ty));
    memset(result, 0, sizeof(*result));
    return result;
}


void
meter_free(meter_ty *mp)
{
    mem_free(mp);
}


/*
 * NAME
 *    meter_ptime - print timing info
 *
 * SYNOPSIS
 *    void meter_ptime(double t, char *s);
 *
 * DESCRIPTION
 *    The meter_ptime function is used to print e representation of the
 *    time, in seconds, given in the `t' argument.  The `s' argument is a
 *    title string.
 */

static void
meter_ptime(double t, char *s, double elapsed)
{
    long            hour;
    long            min;
    long            sec;
    long            frac;
    char            buffer[50];

    if (elapsed > 0)
        snprintf(buffer, sizeof(buffer), " %5.1f%%", 100. * t / elapsed);
    else
        buffer[0] = 0;

    frac = t * 1000 + 0.5;
    sec = frac / 1000;
    frac %= 1000;
    min = sec / 60;
    sec %= 60;
    hour = min / 60;
    min %= 60;
    fprintf
    (
        stderr,
        "%2ld:%02ld:%02ld.%03ld %s%s\n",
        hour,
        min,
        sec,
        frac,
        s,
        buffer
    );
}


#define timeval2double(tv) ((tv).tv_sec + (tv).tv_usec * 1.0e-6)

/*
 * NAME
 *    meter_print - end a metering interval
 *
 * SYNOPSIS
 *    void meter_print(struct rusage *);
 *
 * DESCRIPTION
 *    The meter_end function is used to end a metering interval and print
 *    the metered information.
 */

void
meter_print(meter_ty *mp)
{
    double          elapsed;
#ifdef HAVE_WAIT3
    double          usr;
    double          sys;
#endif

#ifdef HAVE_GETTIMEOFDAY
    {
        struct timeval  tv;

        gettimeofday(&tv, 0);
        elapsed = timeval2double(tv) - timeval2double(mp->start);
    }
#else
    {
        time_t          now;

        time(&now);
        elapsed = now - mp->start;
    }
#endif
    meter_ptime(elapsed, "elapsed", 0);
#ifdef HAVE_WAIT3
    usr = timeval2double(mp->ru.ru_utime);
    meter_ptime(usr, "usr", elapsed);
    sys = timeval2double(mp->ru.ru_stime);
    meter_ptime(sys, "sys", elapsed);
#endif /* HAVE_WAIT3 */
}


void
meter_begin(meter_ty *mp)
{
#ifdef HAVE_GETTIMEOFDAY
    gettimeofday(&mp->start, 0);
#else
    time(&mp->start);
#endif
}
