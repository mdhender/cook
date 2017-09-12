/*
 *      cook - file construction tool
 *      Copyright (C) 1993-2007 Peter Miller
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

#include <common/ac/ctype.h>
#include <common/ac/errno.h>
#include <common/ac/stddef.h>
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>
#include <common/ac/stdarg.h>

#include <common/error.h>
#include <common/fflush_slow.h>
#include <common/mprintf.h>
#include <common/page.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/star.h>


/*
 * NAME
 *      wrap - wrap s string over lines
 *
 * SYNOPSIS
 *      void wrap(char *);
 *
 * DESCRIPTION
 *      The wrap function is used to print error messages onto stderr
 *      wrapping ling lines.
 *
 * CAVEATS
 *      Line length is assumed to be 80 characters.
 */

static void
wrap(char *s)
{
    static char     escapes[] = "\rr\nn\ff\bb\tt";
    char            tmp[MAX_PAGE_WIDTH + 2];
    int             first_line;
    char            *tp;
    int             bomb_later;
    char            *progname;
    int             page_width;

    /*
     * Flush stdout so that errors are in sync with the output.
     * If you get an error doing this, whinge about it _after_ reporting
     * the originating error.  Also, clear the error on stdout to
     * avoid getting caught in an infinite loop.
     */
    star_eoln();
    if (fflush_slowly(stdout))
    {
        bomb_later = errno;
    }
    else
        bomb_later = 0;

    /*
     * Ask the system how wide the terminal is.
     * Don't use last column, many terminals are dumb.
     */
    page_width = page_width_get();

    progname = progname_get();
    first_line = 1;
    while (*s)
    {
        char            *ep;
        int             ocol;

        /*
         * Work out how many characters fit on the line.
         */
        if (first_line)
            ocol = strlen(progname) + 2;
        else
            ocol = 8;
        for (ep = s; *ep; ++ep)
        {
            int             cw;
            int             c;

            c = (unsigned char)*ep;
            if (isprint(c))
                cw = 1 + (c == '\\');
            else
                cw = (strchr(escapes, c) ? 2 : 4);
            if (ocol + cw > page_width)
                break;
            ocol += cw;
        }

        /*
         * see if there is a better place to break the line
         */
        if (*ep && *ep != ' ')
        {
            char            *mp;
            char            *bp_space;
            char            *bp_slash;

            bp_space = 0;
            for (mp = ep; mp > s; --mp)
            {
                if (mp[-1] == ' ')
                {
                    bp_space = mp;
                    break;
                }
            }

            bp_slash = 0;
            for (mp = ep; mp > s; --mp)
            {
                if (strchr("\\/", mp[-1]))
                {
                    bp_slash = mp;
                    break;
                }
            }

            /*
             * We could break it at the space, and only use
             * the slash if there are no spaces on the line.
             * This can lead to large amounts of wasted
             * space, particularly for link commands.  So, if
             * both breaks are possible, and the space break
             * is before the slash break, and the space
             * break is in the left half of the line, use
             * the slash break.
             */
            if
            (
                bp_space
            &&
                bp_slash
            &&
                bp_space < bp_slash
            &&
                bp_space < s + 30
            )
                bp_space = 0;

            /*
             * use the break if available
             */
            if (bp_space)
                ep = bp_space;
            else if (bp_slash)
                ep = bp_slash;
        }

        /*
         * ignore trailing blanks
         */
        while (ep > s && ep[-1] == ' ')
            ep--;

        /*
         * print the line
         */
        if (first_line)
            snprintf(tmp, sizeof(tmp), "%s: ", progname);
        else
            strendcpy(tmp, "\t", tmp + sizeof(tmp));
        tp = tmp + strlen(tmp);
        while (s < ep)
        {
            int             c;

            c = (unsigned char)*s++;
            if (isprint(c))
            {
                if (c == '\\')
                    *tp++ = '\\';
                *tp++ = c;
            }
            else
            {
                char            *esc;

                esc = strchr(escapes, c);
                if (esc)
                {
                    *tp++ = '\\';
                    *tp++ = esc[1];
                }
                else
                {
                    snprintf(tp, tmp + sizeof(tmp) - tp, "\\%3.3o", c);
                    tp += strlen(tp);
                }
            }
        }
        *tp++ = '\n';
        *tp = 0;
        fputs(tmp, stderr);
        if (fflush_slowly(stderr))
            break;

        /*
         * skip leading spaces for subsequent lines
         */
        while (*s == ' ')
            s++;
        first_line = 0;
    }
    if (fflush_slowly(stderr))
    {
        /* don't print why, there is no point! */
        quit(1);
    }
    if (bomb_later)
    {
        errno = bomb_later;
        nfatal_raw("standard output");
    }
}


static void
double_jeopardy(void)
{
    char            buffer[200];

    snprintf
    (
        buffer,
        sizeof(buffer),
        "while attempting to construct an error message: %s (fatal)",
        strerror(errno)
    );
    wrap(buffer);
    quit(1);
}


static char *
copy_a_string(char *s)
{
    char            *cp;
    size_t          len;

    errno = 0;
    len = strlen(s) + 1;
    cp = malloc(len);
    if (!cp)
    {
        if (!errno)
            errno = ENOMEM;
        double_jeopardy();
    }
    memcpy(cp, s, len);
    return cp;
}


/*
 * NAME
 *      error - place a message on the error stream
 *
 * SYNOPSIS
 *      void error(char *s, ...);
 *
 * DESCRIPTION
 *      Error places a message on the error output stream.
 *      The first argument is a printf-like format string,
 *      optionally followed by other arguments.
 *      The message will be prefixed by the program name and a colon,
 *      and will be terminated with a newline, automatically.
 *
 * CAVEAT
 *      Things like "error(filename)" blow up if the filename
 *      contains a '%' character.
 */

void
error_raw(const char *s, ...)
{
    va_list         ap;
    char            *msg;

    va_start(ap, s);
    msg = vmprintf(s, ap);
    va_end(ap);
    if (!msg)
        double_jeopardy();
    wrap(msg);
}


/*
 * NAME
 *      nerror - place a system fault message on the error stream
 *
 * SYNOPSIS
 *      void nerror(char *s, ...);
 *
 * DESCRIPTION
 *      Nerror places a message on the error output stream.
 *      The first argument is a printf-like format string,
 *      optionally followed by other arguments.
 *      The message will be prefixed by the program name and a colon,
 *      and will be terminated with a text description of the error
 *      indicated by the 'errno' global variable, automatically.
 *
 * CAVEAT
 *      Things like "nerror(filename)" blow up if the filename
 *      contains a '%' character.
 */

void
nerror_raw(const char *s, ...)
{
    va_list         ap;
    int             n;
    char            *msg;

    n = errno;
    va_start(ap, s);
    msg = vmprintf(s, ap);
    va_end(ap);
    if (!msg)
        double_jeopardy();
    msg = copy_a_string(msg);
    error_raw("%s: %s", msg, strerror(n));
    free(msg);
}


/*
 * NAME
 *      nfatal - place a system fault message on the error stream and exit
 *
 * SYNOPSIS
 *      void nfatal(char *s, ...);
 *
 * DESCRIPTION
 *      Nfatal places a message on the error output stream and exits.
 *      The first argument is a printf-like format string,
 *      optionally followed by other arguments.
 *      The message will be prefixed by the program name and a colon,
 *      and will be terminated with a text description of the error
 *      indicated by the 'errno' global variable, automatically.
 *
 * CAVEAT
 *      Things like "nfatal(filename)" blow up if the filename
 *      contains a '%' character.
 *
 *      This function does NOT return.
 */

void
nfatal_raw(const char *s, ...)
{
    va_list         ap;
    int             n;
    char            *msg;

    n = errno;
    va_start(ap, s);
    msg = vmprintf(s, ap);
    va_end(ap);
    if (!msg)
        double_jeopardy();
    msg = copy_a_string(msg);
    error_raw("%s: %s", msg, strerror(n));
    free(msg);
    quit(1);
}


/*
 * NAME
 *      fatal - place a message on the error stream and exit
 *
 * SYNOPSIS
 *      void fatal(char *s, ...);
 *
 * DESCRIPTION
 *      Fatal places a message on the error output stream and exits.
 *      The first argument is a printf-like format string,
 *      optionally followed by other arguments.
 *      The message will be prefixed by the program name and a colon,
 *      and will be terminated with a newline, automatically.
 *
 * CAVEAT
 *      Things like "error(filename)" blow up if the filename
 *      contains a '%' character.
 *
 *      This function does NOT return.
 */

void
fatal_raw(const char *s, ...)
{
    va_list         ap;
    char            *msg;

    va_start(ap, s);
    msg = vmprintf(s, ap);
    va_end(ap);
    if (!msg)
        double_jeopardy();
    wrap(msg);
    quit(1);
}
