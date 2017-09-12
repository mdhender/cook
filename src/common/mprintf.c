/*
 *      cook - file construction tool
 *      Copyright (C) 1991-1994, 1997, 1999, 2001, 2006, 2007 Peter Miller;
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

#include <common/ac/errno.h>
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/error.h>
#include <common/mprintf.h>
#include <common/str.h>
#include <common/trace.h>

/*
 * size to grow memory by
 */
#define QUANTUM 200

/*
 * maximum width for numbers
 */
#define MAX_WIDTH (QUANTUM - 1)

/*
 * the buffer for storing results
 */
static size_t   tmplen;
static size_t   length;
static char    *tmp;


/*
 * NAME
 *      bigger - grow dynamic memory buffer
 *
 * SYNOPSIS
 *      int bigger(void);
 *
 * DESCRIPTION
 *      The bigger function is used to grow the dynamic memory buffer
 *      used by vmprintf to store the formatting results.
 *      The buffer is increased by QUANTUM bytes.
 *
 * RETURNS
 *      int; zero if failed to realloc memory, non-zero if successful.
 *
 * CAVEATS
 *      The existing buffer is still valid after failure.
 */

static int
bigger(void)
{
    char            *hold;
    size_t          nbytes;

    nbytes = tmplen + QUANTUM;
    errno = 0;
    hold = realloc(tmp, nbytes);
    if (!hold)
    {
        if (!errno)
            errno = ENOMEM;
        return 0;
    }
    tmplen = nbytes;
    tmp = hold;
    return 1;
}


/*
 * NAME
 *      build fake - construct formatting specifier string
 *
 * SYNOPSIS
 *      void build_fake(char *fake, int flag, int width, int prec, int qual,
 *              int spec);
 *
 * DESCRIPTION
 *      The build_fake function is used to construct a format
 *      specification string from the arguments presented.  This is
 *      used to guarantee exact replication of sprintf behaviour.
 *
 * ARGUMENTS
 *      fake    - buffer to store results
 *      flag    - the flag specified (zero if not)
 *      width   - the width specified (zero if not)
 *      prec    - the precision specified (zero if not)
 *      qual    - the qualifier specified (zero if not)
 *      spec    - the formatting specifier specified
 */

static void
build_fake(char *fake, size_t fake_len, int flag, int width, int precision,
    int qualifier, int specifier)
{
    char            *fp;
    char            *fe;

    fp = fake;
    fe = fake + fake_len;
    *fp++ = '%';
    if (flag)
        *fp++ = flag;
    if (width > 0)
    {
        snprintf(fp, fe - fp, "%d", width);
        fp += strlen(fp);
    }
    *fp++ = '.';
    snprintf(fp, fe - fp, "%d", precision);
    fp += strlen(fp);
    if (qualifier)
        *fp++ = qualifier;
    *fp++ = specifier;
    *fp = 0;
}


/*
 * NAME
 *      vmprintf - build a formatted string in dynamic memory
 *
 * SYNOPSIS
 *      char *vmprintf(char *fmt, va_list ap);
 *
 * DESCRIPTION
 *      The vmprintf function is used to build a formatted string in memory.
 *      It understands all of the ANSI standard sprintf formatting directives.
 *      Additionally, "%S" may be used to manipulate (string_ty *) strings.
 *
 * ARGUMENTS
 *      fmt     - string spefiifying formatting to perform
 *      ap      - arguments of types as indicated by the format string
 *
 * RETURNS
 *      char *; pointer to buffer containing formatted string
 *              NULL if there is an error (sets errno)
 *
 * CAVEATS
 *      The contents of the buffer pointed to will change between calls
 *      to vmprintf.  The buffer itself may move between calls to vmprintf.
 *      DO NOT hand the result of vmprintf to free().
 */

char *
vmprintf(const char *fmt, va_list ap)
{
    int             width;
    int             width_set;
    int             prec;
    int             prec_set;
    int             c;
    const char      *s;
    int             qualifier;
    int             flag;
    char            fake[QUANTUM - 1];

    /*
     * Build the result string in a temporary buffer.
     * Grow the temporary buffer as necessary.
     *
     * It is important to only make one pass across the variable argument
     * list.  Behaviour is undefined for more than one pass.
     */
    if (!tmplen)
    {
        tmplen = 500;
        errno = 0;
        tmp = malloc(tmplen);
        if (!tmp)
        {
            if (!errno)
                errno = ENOMEM;
            return 0;
        }
    }

    length = 0;
    s = fmt;
    while (*s)
    {
        c = *s++;
        if (c != '%')
        {
          normal:
            if (length >= tmplen && !bigger())
                return 0;
            tmp[length++] = c;
            continue;
        }
        c = *s++;

        /*
         * get optional flag
         */
        switch (c)
        {
        case '+':
        case '-':
        case '#':
        case '0':
        case ' ':
            flag = c;
            c = *s++;
            break;

        default:
            flag = 0;
            break;
        }

        /*
         * get optional width
         */
        width = 0;
        width_set = 0;
        switch (c)
        {
        case '*':
            width = va_arg(ap, int);
            if (width < 0)
            {
                flag = '-';
                width = -width;
            }
            c = *s++;
            width_set = 1;
            break;

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            for (;;)
            {
                width = width * 10 + c - '0';
                c = *s++;
                switch (c)
                {
                default:
                    break;

                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    continue;
                }
                break;
            }
            width_set = 1;
            break;

        default:
            break;
        }

        /*
         * get optional precision
         */
        prec = 0;
        prec_set = 0;
        if (c == '.')
        {
            c = *s++;
            switch (c)
            {
            default:
                prec_set = 1;
                break;

            case '*':
                c = *s++;
                prec = va_arg(ap, int);
                if (prec < 0)
                {
                    prec = 0;
                    break;
                }
                prec_set = 1;
                break;

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                for (;;)
                {
                    prec = prec * 10 + c - '0';
                    c = *s++;
                    switch (c)
                    {
                    default:
                        break;

                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        continue;
                    }
                    break;
                }
                prec_set = 1;
                break;
            }
        }

        /*
         * get the optional qualifier
         */
        switch (c)
        {
        default:
            qualifier = 0;
            break;

        case 'l':
        case 'h':
        case 'L':
            qualifier = c;
            c = *s++;
            break;
        }

        /*
         * get conversion specifier
         */
        switch (c)
        {
        default:
            errno = EINVAL;
            return 0;

        case '%':
            goto normal;

        case 'c':
            {
                int             a;
                char            num[MAX_WIDTH + 1];
                size_t          len;

                a = (unsigned char)va_arg(ap, int);
                if (!prec_set)
                    prec = 1;
                if (width > MAX_WIDTH)
                    width = MAX_WIDTH;
                if (prec > MAX_WIDTH)
                    prec = MAX_WIDTH;
                build_fake(fake, sizeof(fake), flag, width, prec, 0, c);
                snprintf(num, sizeof(num), fake, a);
                len = strlen(num);
                assert(len < QUANTUM);
                if (length + len > tmplen && !bigger())
                    return 0;
                memcpy(tmp + length, num, len);
                length += len;
            }
            break;

        case 'd':
        case 'i':
            {
                long            a;
                char            num[MAX_WIDTH + 1];
                size_t          len;

                switch (qualifier)
                {
                case 'l':
                    a = va_arg(ap, long);
                    break;

                case 'h':
                    a = (short)va_arg(ap, int);
                    break;

                default:
                    a = va_arg(ap, int);
                    break;
                }
                if (!prec_set)
                    prec = (flag == '0' ? width : 1);
                if (width > MAX_WIDTH)
                    width = MAX_WIDTH;
                if (prec > MAX_WIDTH)
                    prec = MAX_WIDTH;
                build_fake(fake, sizeof(fake), flag, width, prec, 'l', c);
                snprintf(num, sizeof(num), fake, a);
                len = strlen(num);
                assert(len < QUANTUM);
                if (length + len > tmplen && !bigger())
                    return 0;
                memcpy(tmp + length, num, len);
                length += len;
            }
            break;

        case 'e':
        case 'f':
        case 'g':
        case 'E':
        case 'F':
        case 'G':
            {
                double          a;
                char            num[MAX_WIDTH + 1];
                size_t          len;

                /*
                 * Ignore "long double" for now,
                 * traditional implementations no grok.
                 */
                a = va_arg(ap, double);
                if (!prec_set)
                    prec = 6;
                if (width > MAX_WIDTH)
                    width = MAX_WIDTH;
                if (prec > MAX_WIDTH)
                    prec = MAX_WIDTH;
                build_fake(fake, sizeof(fake), flag, width, prec, 0, c);
                snprintf(num, sizeof(num), fake, a);
                len = strlen(num);
                assert(len < QUANTUM);
                if (length + len > tmplen && !bigger())
                    return 0;
                memcpy(tmp + length, num, len);
                length += len;
            }
            break;

        case 'n':
            switch (qualifier)
            {
            case 'l':
                {
                    long            *a;

                    a = va_arg(ap, long *);
                    *a = length;
                }
                break;

            case 'h':
                {
                    short           *a;

                    a = va_arg(ap, short *);
                    *a = length;
                }
                break;

            default:
                {
                    int             *a;

                    a = va_arg(ap, int *);
                    *a = length;
                }
                break;
            }
            break;

        case 'u':
        case 'o':
        case 'x':
        case 'X':
            {
                unsigned long   a;
                char            num[MAX_WIDTH + 1];
                size_t          len;

                switch (qualifier)
                {
                case 'l':
                    a = va_arg(ap, unsigned long);
                    break;

                case 'h':
                    a = (unsigned short)va_arg(ap, unsigned int);
                    break;

                default:
                    a = va_arg(ap, unsigned int);
                    break;
                }
                if (!prec_set)
                    prec = (flag == '0' ? width : 1);
                if (prec > MAX_WIDTH)
                    prec = MAX_WIDTH;
                if (width > MAX_WIDTH)
                    width = MAX_WIDTH;
                build_fake(fake, sizeof(fake), flag, width, prec, 'l', c);
                snprintf(num, sizeof(num), fake, a);
                len = strlen(num);
                assert(len < QUANTUM);
                if (length + len > tmplen && !bigger())
                    return 0;
                memcpy(tmp + length, num, len);
                length += len;
            }
            break;

        case 's':
            {
                char            *a;
                int             len;

                a = va_arg(ap, char *);
                if (prec_set)
                {
                    char            *ep;

                    ep = (char *)memchr(a, 0, prec);
                    if (ep)
                        len = ep - a;
                    else
                        len = prec;
                }
                else
                    len = strlen(a);
                if (!prec_set || len < prec)
                    prec = len;
                if (!width_set || width < prec)
                    width = prec;
                len = width;
                while (length + len > tmplen)
                {
                    if (!bigger())
                        return 0;
                }
                if (flag != '-')
                {
                    while (width > prec)
                    {
                        tmp[length++] = ' ';
                        width--;
                    }
                }
                memcpy(tmp + length, a, prec);
                length += prec;
                width -= prec;
                if (flag == '-')
                {
                    while (width > 0)
                    {
                        tmp[length++] = ' ';
                        width--;
                    }
                }
            }
            break;

        case 'S':
            {
                string_ty       *a;
                int             len;

                a = va_arg(ap, string_ty *);
                len = a->str_length;
                if (!prec_set)
                    prec = len;
                if (len < prec)
                    prec = len;
                if (!width_set)
                    width = prec;
                if (width < prec)
                    width = prec;
                len = width;
                while (length + len > tmplen)
                {
                    if (!bigger())
                        return 0;
                }
                if (flag != '-')
                {
                    while (width > prec)
                    {
                        tmp[length++] = ' ';
                        width--;
                    }
                }
                memcpy(tmp + length, a->str_text, prec);
                length += prec;
                width -= prec;
                if (flag == '-')
                {
                    while (width > 0)
                    {
                        tmp[length++] = ' ';
                        width--;
                    }
                }
            }
            break;
        }
    }

    /*
     * append a trailing NUL
     */
    if (length >= tmplen && !bigger())
        return 0;
    tmp[length] = 0;

    /*
     * return the temporary string
     */
    return tmp;
}


/*
 * NAME
 *      mprintf - build a formatted string in dynamic memory
 *
 * SYNOPSIS
 *      char *mprintf(char *fmt, ...);
 *
 * DESCRIPTION
 *      The mprintf function is used to build a formatted string in memory.
 *      It understands all of the ANSI standard sprintf formatting directives.
 *      Additionally, "%S" may be used to manipulate (string_ty *) strings.
 *
 * ARGUMENTS
 *      fmt     - string spefiifying formatting to perform
 *      ...     - arguments of types as indicated by the format string
 *
 * RETURNS
 *      char *; pointer to buffer containing formatted string
 *              NULL if there is an error (sets errno)
 *
 * CAVEATS
 *      The contents of the buffer pointed to will change between calls
 *      to mprintf.  The buffer itself may move between calls to mprintf.
 *      DO NOT hand the result of mprintf to free().
 */

char *
mprintf(const char *fmt, ...)
{
    char            *result;
    va_list         ap;

    va_start(ap, fmt);
    result = vmprintf(fmt, ap);
    va_end(ap);
    return result;
}


/*
 * NAME
 *      vmprintfe - build a formatted string in dynamic memory
 *
 * SYNOPSIS
 *      char *vmprintfe(char *fmt, va_list ap);
 *
 * DESCRIPTION
 *      The vmprintfe function is used to build a formatted string in memory.
 *      It understands all of the ANSI standard sprintf formatting directives.
 *      Additionally, "%S" may be used to manipulate (string_ty *) strings.
 *
 * ARGUMENTS
 *      fmt     - string spefiifying formatting to perform
 *      ap      - arguments of types as indicated by the format string
 *
 * RETURNS
 *      char *; pointer to buffer containing formatted string
 *
 * CAVEATS
 *      On error, prints a fatal error message and exists; does not return.
 *
 *      The contents of the buffer pointed to will change between calls
 *      to vmprintfe.  The buffer itself may move between calls to vmprintfe.
 *      DO NOT hand the result of vmprintfe to free().
 */

char *
vmprintfe(const char *fmt, va_list ap)
{
    char            *result;

    result = vmprintf(fmt, ap);
    if (!result)
        nfatal_raw("mprintf \"%s\"", fmt);
    return result;
}


/*
 * NAME
 *      mprintfe - build a formatted string in dynamic memory
 *
 * SYNOPSIS
 *      char *mprintfe(char *fmt, ...);
 *
 * DESCRIPTION
 *      The mprintfe function is used to build a formatted string in memory.
 *      It understands all of the ANSI standard sprintf formatting directives.
 *      Additionally, "%S" may be used to manipulate (string_ty *) strings.
 *
 * ARGUMENTS
 *      fmt     - string spefiifying formatting to perform
 *      ...     - arguments of types as indicated by the format string
 *
 * RETURNS
 *      char *; pointer to buffer containing formatted string
 *
 * CAVEATS
 *      On error, prints a fatal error message and exists; does not return.
 *
 *      The contents of the buffer pointed to will change between calls
 *      to mprintfe.  The buffer itself may move between calls to mprintfe.
 *      DO NOT hand the result of mprintfe to free().
 */

char *
mprintfe(const char *fmt, ...)
{
    char            *result;
    va_list         ap;

    va_start(ap, fmt);
    result = vmprintfe(fmt, ap);
    va_end(ap);
    return result;
}


/*
 * NAME
 *      vmprintfes - build a formatted string in dynamic memory
 *
 * SYNOPSIS
 *      char *vmprintfes(char *fmt, va_list ap);
 *
 * DESCRIPTION
 *      The vmprintfes function is used to build a formatted string in memory.
 *      It understands all of the ANSI standard sprintf formatting directives.
 *      Additionally, "%S" may be used to manipulate (string_ty *) strings.
 *
 * ARGUMENTS
 *      fmt     - string spefiifying formatting to perform
 *      ap      - arguments of types as indicated by the format string
 *
 * RETURNS
 *      string_ty *; string containing formatted string
 *
 * CAVEATS
 *      On error, prints a fatal error message and exists; does not return.
 *
 *      It is the resposnsibility of the caller to invoke str_free to release
 *      the results when finished with.
 */

string_ty *
vmprintfes(const char *fmt, va_list ap)
{
    if (!vmprintf(fmt, ap))
        nfatal_raw("mprintf \"%s\"", fmt);
    return str_n_from_c(tmp, length);
    trace(("to shut up warnings\n"));
}
