/*
 *      cook - file construction tool
 *      Copyright (C) 1999, 2001, 2006, 2007 Peter Miller;
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

#include <common/ac/string.h>

#include <common/gmatch.h>
#include <common/str.h>
#include <common/sub.h>
#include <common/trace.h>


static const char *original;
static gmatch_fp erf;
static void    *erf_aux;


void
gmatch_error_handler(gmatch_fp fp, void *aux)
{
    erf = fp;
    erf_aux = aux;
}


static void
report_error(const char *s)
{
    sub_context_ty  *scp;
    string_ty       *message;

    if (!erf)
        return;

    scp = sub_context_new();
    sub_var_set(scp, "Name", "%s", original);
    message = subst_intl(scp, s);
    sub_context_delete(scp);
    erf(erf_aux, message);
    str_free(message);
}


static void
missing_closing_bracket(void)
{
    report_error(i18n("pattern \"$name\" missing closing ']'"));
}


/*
 * NAME
 *      gmatch - match entryname pattern
 *
 * SYNOPSIS
 *      int gmatch(char *formal, char *formal_end, char *actual);
 *
 * DESCRIPTION
 *      The formal strings is used as a template to match the given actual
 *      string against.
 *
 *      The pattern elements understood are
 *      *       match zero or more of any character
 *      ?       match any single character
 *      [^xxx]  match any single character not in the set given.
 *      [xxx]   match any single character not in the set given.
 *              The - character is understood to be a range indicator.
 *              If the ] character is the first of the set it is considered
 *              as part of the set, not the terminator.
 *
 * RETURNS
 *      the gmatch function returns zero if they do not match,
 *      and nonzero if they do.  Returns -1 on error.
 *
 * CAVEAT
 *      This is a limited set of the sh(1) patterns.
 *      Assumes that the `original' global variable has been initialized, it is
 *      used for error reporting.
 */

static int
gmatch_inner(const char *formal, const char *formal_end, const char *actual,
    const char *actual_end)
{
    const char      *cp;
    int             result;

    trace(("gmatch_inner(formal = %8.8lX, formal_end = %8.8lX, "
        "actual = %8.8lX, actual_end = %8.8lX)\n{\n", (long)formal,
        (long)formal_end, (long)actual, (long)actual_end));
    while (formal < formal_end)
    {
        trace(("formal == \"%.*s\";\n", formal_end - formal, formal));
        trace(("actual = \"%.*s\";\n", actual_end - actual, actual));
        switch (*formal)
        {
        default:
            if (*actual++ != *formal++)
            {
                result = 0;
                goto ret;
            }
            break;

        case '?':
            if (actual >= actual_end)
            {
                result = 0;
                goto ret;
            }
            ++actual;
            ++formal;
            break;

        case '*':
            ++formal;

            /*
             * Look for additional wild-cards or single
             * character wilds, and accumulate them.  No
             * sense recursing twice.  This takes care of
             * weird patterns like "**" or "*?" etc, and
             * allows us to use the trailing wild-card
             * optimization.
             */
            while (formal < formal_end)
            {
                if (*formal == '*')
                    ++formal;
                else if (*formal == '?')
                {
                    ++formal;
                    if (actual >= actual_end)
                    {
                        result = 0;
                        goto ret;
                    }
                    ++actual;
                }
                else
                    break;
            }

            /*
             * If we are at the end of the formal pattern,
             * then we have a trailing wild-card.  This
             * matches anything, so return success no matter
             * how much actual is left.
             */
            if (formal >= formal_end)
            {
                result = 1;
                goto ret;
            }

            /*
             * Try to find the longest match possible for
             * the wild-card, so start from the right hand
             * end of the actual string.
             */
            cp = actual_end;
            for (;;)
            {
                if (gmatch_inner(formal, formal_end, cp, actual_end))
                {
                    result = 1;
                    goto ret;
                }
                --cp;
                if (cp < actual)
                {
                    result = 0;
                    goto ret;
                }
            }

        case '[':
            ++formal;
            if (*formal == '^')
            {
                ++formal;
                for (;;)
                {
                    if (formal >= formal_end)
                    {
                      no_close:
                        missing_closing_bracket();
                        result = -1;
                        goto ret;
                    }
                    /*
                     * Note: this allows leading
                     * ']' elegantly.
                     */
                    if
                    (
                        formal_end >= formal + 3
                    &&
                        formal[1] == '-'
                    &&
                        formal[2] != ']'
                    )
                    {
                        char            c1;
                        char            c2;

                        c1 = formal[0];
                        c2 = formal[2];
                        formal += 3;
                        if
                        (
                            c1 <= c2
                        ?
                            (c1 <= *actual && *actual <= c2)
                        :
                            (c2 <= *actual && *actual <= c1)
                        )
                        {
                            result = 0;
                            goto ret;
                        }
                    }
                    else if (*actual == *formal++)
                    {
                        result = 0;
                        goto ret;
                    }
                    if (*formal == ']')
                        break;
                }
                ++formal;
            }
            else
            {
                for (;;)
                {
                    if (formal >= formal_end)
                        goto no_close;
                    /*
                     * Note: this allows leading
                     * ']' elegantly.
                     */
                    trace(("formal == \"%.*s\";\n",
                            formal_end - formal, formal));
                    trace(("actual = \"%.*s\";\n",
                            actual_end - actual, actual));
                    if
                    (
                        formal_end >= formal + 3
                    &&
                        formal[1] == '-'
                    &&
                        formal[2] != ']'
                    )
                    {
                        char            c1;
                        char            c2;

                        c1 = formal[0];
                        c2 = formal[2];
                        formal += 3;
                        if
                        (
                            c1 <= c2
                        ?
                            (c1 <= *actual && *actual <= c2)
                        :
                            (c2 <= *actual && *actual <= c1)
                        )
                            break;
                    }
                    else if (*actual == *formal++)
                        break;
                    if (*formal == ']')
                    {
                        result = 0;
                        goto ret;
                    }
                }
                for (;;)
                {
                    if (formal >= formal_end)
                        goto no_close;
                    trace(("formal == \"%.*s\";\n",
                            formal_end - formal, formal));
                    trace(("actual = \"%.*s\";\n",
                            actual_end - actual, actual));
                    if (*formal++ == ']')
                        break;
                }
            }
            ++actual;
            break;
        }
    }
    result = (actual >= actual_end);
  ret:
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}


int
gmatch2(const char *formal, const char *formal_end, const char *actual)
{
    original = formal;
    return gmatch_inner(formal, formal_end, actual, actual + strlen(actual));
}


int
gmatch(const char *formal, const char *actual)
{
    return gmatch2(formal, formal + strlen(formal), actual);
}
