/*
 *      cook - file construction tool
 *      Copyright (C) 1998, 2006-2009 Peter Miller
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

#include <common/ac/string.h>

#include <common/mem.h>
#include <common/stracc.h>
#include <common/trace.h>


void
stracc_constructor(stracc *sap)
{
    sap->sa_max = 0;
    sap->sa_len = 0;
    sap->sa_buf = 0;
    sap->sa_inuse = 0;
}


stracc *
stracc_new(void)
{
    stracc          *sap;

    sap = mem_alloc(sizeof(stracc));
    stracc_constructor(sap);
    return sap;
}


void
stracc_destructor(stracc *sap)
{
    assert(!sap->sa_inuse);
    if (sap->sa_buf)
        mem_free(sap->sa_buf);
    sap->sa_max = 0;
    sap->sa_len = 0;
    sap->sa_buf = 0;
    sap->sa_inuse = 0;
}


/*
 * NAME
 *      sa_open - start accumulating a string.
 *
 * SYNOPSIS
 *      void sa_open(stracc *);
 *
 * DESCRIPTION
 *      Sa_open begins accumulating a string for the lexical analyser.
 *      This allows virtually infinite length constructs within the
 *      lexical analyser.
 */

void
sa_open(stracc *sap)
{
    trace(("sa_open()\n{\n"));
    assert(!sap->sa_inuse);
    sap->sa_inuse = 1;
    sap->sa_len = 0;
    trace(("}\n"));
}


/*
 * NAME
 *      sa_char - add a character to the accumulating string
 *
 * SYNOPSIS
 *      void sa_char(stracc *, int c);
 *
 * DESCRIPTION
 *      Sa_char adds a character to the accumulating string.
 *
 * CAVEAT
 *      Sa_open must have been called previously.
 */

void
sa_char(stracc *sap, int c)
{
    trace(("sa_char(c = '%c')\n{\n", c));
    assert(sap->sa_inuse);
    if (sap->sa_len >= sap->sa_max)
    {
        sap->sa_max = sap->sa_max * 2 + 16;
        sap->sa_buf = mem_change_size(sap->sa_buf, sap->sa_max);
    }
    sap->sa_buf[sap->sa_len++] = c;
    trace(("}\n"));
}


/*
 * NAME
 *      sa_close - finish accumulating a string
 *
 * SYNOPSIS
 *      string_ty *sa_close(stracc *);
 *
 * DESCRIPTION
 *      Sa_close finished accumulating a string and
 *      returns a pointer to the string in dynamic memory.
 *
 * CAVEAT
 *      Sa_open must have been called previously.
 *
 *      The value returned by this function is allocated in dynamic memory.
 *      It is the responsibility of the caller to ensure that it is freed when
 *      finished with, by a call to str_free().
 */

string_ty *
sa_close(stracc *sap)
{
    string_ty       *val;

    trace(("sa_close()\n{\n"));
    assert(sap->sa_inuse);
    val = str_n_from_c(sap->sa_buf, sap->sa_len);
    sap->sa_inuse = 0;
    trace(("return %p;\n", val));
    trace(("}\n"));
    return val;
}


/*
 * NAME
 *      sa_mark - mark place when accumulating a string
 *
 * SYNOPSIS
 *      size_t sa_mark(stracc *);
 *
 * DESACRIPTION
 *      Sa_mark returns an indicator as to where the string being accumulated
 *      is up to.
 *
 * CAVEAT
 *      Sa_open must have been called previously.
 *      Do not use the returned value for anything other than passing to
 *      sa_goto as an argument.
 */

size_t
sa_mark(stracc *sap)
{
    assert(sap->sa_inuse);
    return (sap->sa_len);
}


/*
 * NAME
 *      sa_goto - shorten the accumulated string
 *
 * SYNOPSIS
 *      void sa_goto(stracc *, size_t mark);
 *
 * DESACRIPTION
 *      Sa_goto shotens the accumulating string to a point
 *      previously determined by sa_mark.
 *
 * CAVEAT
 *      Sa_open must have been called previously.
 *      The mark argument must have previously been returned by sa_mark,
 *      and sa_close not called in the interim.
 */

void
sa_goto(stracc *sap, size_t n)
{
    assert(sap->sa_inuse);
    assert(n <= sap->sa_len);
    sap->sa_len = n;
}


void
sa_chars(stracc *sap, const char *cp, size_t len)
{
    trace(("sa_chars()\n{\n"));
    assert(sap->sa_inuse);
    while (sap->sa_len + len > sap->sa_max)
    {
        sap->sa_max = sap->sa_max * 2 + 16;
        sap->sa_buf = mem_change_size(sap->sa_buf, sap->sa_max);
    }
    memcpy(sap->sa_buf + sap->sa_len, cp, len);
    sap->sa_len += len;
    trace(("}\n"));
}
