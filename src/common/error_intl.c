/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 1998, 2001, 2004, 2006-2009 Peter Miller
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
#include <common/ac/limits.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>
#include <common/ac/wchar.h>
#include <common/ac/wctype.h>

#include <common/error.h>
#include <common/error_intl.h>
#include <common/fflush_slow.h>
#include <common/language.h>
#include <common/page.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/star.h>
#include <common/trace.h>
#include <common/verbose.h>
#include <common/wstr.h>


/*
 * NAME
 *      column_width - determine column width of a wide character
 *
 * SYNOPSIS
 *      int column_width(wchar_t);
 *
 * DESCRIPTION
 *      The column_width function is used to determine the column width
 *      if a wide character.  This is particularly hard to do,
 *      especially if you have read the ISO C standard ammendments.
 *
 * WEASEL WORDS
 *      This is the phrase used by P.J. Plauger in his CUJ columns about
 *      standard C and C++, specially when the standard dances all
 *      around the issue, rather than actually solving anything.  Take a
 *      squiz at these classic weasel words...
 *
 *      In the original standard, ISO/IEC 9899:1990, 7.3 Character
 *      handling <ctype.h> reads as follows (3rd paragraph):
 *
 *              The term <i>printing character</i> refers to a member of
 *              an implementation defined set of characters, each of
 *              which occupies one printing position on a display
 *              device; the term <i>control character</i> refers to a
 *              member of an implementation defined set of characters
 *              that are not printing characters.
 *
 *      The following 2 sections are from ISO/IEC 9899:1990/Amd. 1:1995 (E):
 *
 *      7.15.2 Wide-character classification utilities (2nd paragraph)
 *
 *              The term <i>printing wide character</i> refers to a
 *              member of a locale-specific set of wide characters, each
 *              of which occupies at least one printing position on a
 *              display device; the term <i>control wide character</i>
 *              refers to a member of a locale-specific set of wide
 *              characters that are not printing wide characters.
 *
 *      [ Notice how they weasel out by not-quite contradicting 7.3: a
 *      printing <i>char</i> is exactly one printing position wide, but
 *      a printing <i>wchar_t</i> is one or more printing positions
 *      wide. ]
 *
 *      H.14 Column width
 *
 *              The number of characters to be read or written can be
 *              specified in existing formatted i/o functions.  On a
 *              traditional display device that displays characters with
 *              fixed pitch, the number of characters is directly
 *              proportional to the width occupied by the characters.
 *              So the display format can be specified through the field
 *              width and/or the precision.
 *
 *              In formatted wide-character i/o functions, the field
 *              width and the precision specify the number of wide
 *              characters to be read or written.  The number of wide
 *              characters is not always directly proportional to the
 *              width of their display.  For example, with Japanese
 *              traditional display devices, a single-byte character
 *              such as an ASCII character has half the width of a Kanji
 *              character, even though each of them is treated as one
 *              wide character.  To control the display format for wide
 *              characters, a set of formatted wide-character i/o
 *              functions were proposed whose metric was the column
 *              width instead of the character count.
 *
 *              This proposal was supported only by Japan.  Critics
 *              observed that the proposal was based on such traditional
 *              display devices with a fixed width of characters, while
 *              many modern display devices support a broad assortment
 *              of proportional pitch type faces.  Hence, it was
 *              questioned whether the extra i/o functions in this
 *              proposal were really needed or were sufficiently
 *              general.  Also considered were another set of functions
 *              that return the column width for any kind of display
 *              devices for a given wide-character string; but these
 *              seemed to be beyond the scope of the C language.  Thus
 *              all proposals regarding column width were withdrawn.
 *
 *      [ Notice how 7.15.2 specifically states that each printing
 *      character has a non-zero width measurable in <i>printing
 *      positions</i>.  Why is this metric is unavailable to the
 *      C programmer?  Presumably it is OK for an informational appendix
 *      to contradict the body of the standard. ]
 *
 *      [ The section ends with a compliant-but-non-standard way a
 *      standard C library implementor may choose to do this.  You can't
 *      reply on it being there, and you can't reply on the suggested
 *      semantics being used, so don't even bother having ./configure go
 *      look for it. ]
 *
 * SO FAKE IT
 *      Since there is no standard way to determine character width, we
 *      will have to fake it.  Hopefully, locales that need it will
 *      define something useful.  If you know of any, please let me
 *      know.
 */

static int
column_width(wchar_t wc)
{
#ifdef HAVE_ISWCTYPE
    static int      kanji_set;
    static wctype_t kanji;

    if (!kanji_set)
    {
        kanji = wctype("kanji");
        kanji_set = 1;
    }
    if (kanji && iswctype(kanji, wc))
        return 2;
#endif
    return 1;
}


static int
wcs_column_width(wchar_t *wcs)
{
    int             result;

    result = 0;
    while (*wcs)
        result += column_width(*wcs++);
    return result;
}


/*
 * NAME
 *      wrap - wrap s string over lines
 *
 * SYNOPSIS
 *      void wrap(wstring_ty *);
 *
 * DESCRIPTION
 *      The wrap function is used to print error messages onto stderr
 *      wrapping ling lines.  Be very careful of multi-byte characters
 *      in international character sets.
 *
 * CAVEATS
 *      Line length is assumed to be 80 characters.
 */

static void
wrap_inner(const wchar_t *s, const wchar_t *end)
{
    char            *progname;
    int             page_width;
    char            tmp[(MAX_PAGE_WIDTH + 2) * MB_LEN_MAX];
    int             first_line;
    char            *tp;
    int             nbytes;
    static int      progname_width;
    int             midway;

    /*
     * flush any pending output,
     * so the error message appears in a sensible place.
     */
    star_eoln();
    if (fflush_slowly(stdout))
        nfatal_raw("standard output");

    /*
     * Ask the system how wide the terminal is.
     * Don't use last column, many terminals are dumb.
     */
    page_width = page_width_get() - 1;
    midway = (page_width + 8) / 2;

    /*
     * Because it must be a legal UNIX file name, it is unlikely to
     * be stupid - unprintable characters are hard to type, and most
     * file systems don't allow high-bit-on characters in file
     * names.  Thus, assume progname is all legal characters.
     */
    progname = progname_get();
    if (!progname_width)
    {
        wstring_ty      *ws;

        ws = wstr_from_c(progname);
        progname_width = wcs_column_width(ws->wstr_text);
        wstr_free(ws);
    }

    /*
     * the message is for a human, so
     * use the human's locale
     */
    language_human();

    /*
     * Emit the message a line at a time, wrapping as we go.  The
     * first line starts with the program name, subsequent lines are
     * indented by a tab.
     */
    first_line = 1;
    while (s < end)
    {
        const wchar_t   *ep;
        int             ocol;
        const wchar_t   *break_space;
        int             break_space_col;
        const wchar_t   *break_punct;
        int             break_punct_col;

        /*
         * Work out how many characters fit on the line.
         */
        if (first_line)
            ocol = progname_width + 2;
        else
            ocol = 8;

        if (wctomb(NULL, 0) == -1)
        {
            assert(!"assume success");
        }
        ep = s;
        break_space = 0;
        break_space_col = 0;
        break_punct = 0;
        break_punct_col = 0;
        while (ep < end)
        {
            char            dummy[MB_LEN_MAX];
            int             cw;
            wchar_t         c;

            /*
             * Keep printing characters.  Use a dummy
             * character for unprintable sequences (which
             * should not happen).
             */
            c = *ep;
            if (iswspace(c))
                c = ' ';
            else if (!iswprint(c))
                c = '?';
            nbytes = wctomb(dummy, c);

            cw = column_width(c);
            if (nbytes <= 0)
            {
                /*
                 * This should not happen!  All
                 * unprintable characters should have
                 * been turned into C escapes inside the
                 * common/wstr.c file when converting from C
                 * string to wide strings.
                 *
                 * Replace invalid wide characters with
                 * a C escape.
                 */
                cw = 4;
                nbytes = 4;

                /*
                 * The wctomb state will be ''error'',
                 * so reset it and brave the worst.  No
                 * need to reset the wctomb state, it is
                 * not broken.
                 */
                if (wctomb(NULL, 0) == -1)
                {
                    assert(!"assume success");
                }
            }

            /*
             * Keep track of good places to break the line,
             * but try to avoid runs of white space.  There
             * is a pathological case where the line is
             * entirely composed of white space, but it does
             * not happen often.
             */
            if (iswspace(c))
            {
                break_space = ep;
                break_space_col = ocol;
                while (break_space > s && iswspace(break_space[-1]))
                {
                    --break_space;
                    --break_space_col;
                }
            }
            if (iswpunct(c) && ocol + cw <= page_width)
            {
                break_punct = ep + 1;
                break_punct_col = ocol + cw;
            }

            /*
             * if we have run out of room, break here
             */
            if (ocol + cw > page_width)
                break;
            ocol += cw;
            ++ep;
        }

        /*
         * see if there is a better place to break the line
         *
         * Break the line at space characters, otherwise break
         * at punctuator characters.  If it is possible to break
         * on either a space or a punctuator, choose the space.
         *
         * However, if the space is in the left half of the
         * line, things look very unbalanced, so break on a
         * punctuator in that case.
         */
        if (ep < end && !iswspace(*ep))
        {
            if (break_space == s)
                break_space = 0;
            if
            (
                break_space
            &&
                break_punct
            &&
                break_space_col < midway
            &&
                break_punct_col >= midway
            )
                ep = break_punct;
            else if (break_space)
                ep = break_space;
            else if (break_punct)
                ep = break_punct;
        }

        /*
         * print the line
         */
        if (first_line)
        {
            strendcpy
            (
                strendcpy(tmp, progname, tmp + sizeof(tmp)),
                ": ",
                tmp + sizeof(tmp)
            );
        }
        else
            strendcpy(tmp, "\t", tmp + sizeof(tmp));
        tp = tmp + strlen(tmp);

        /*
         * Turn the input into a multi byte characters.
         */
        if (wctomb(NULL, 0) == -1)
        {
            assert(!"assume success");
        }
        while (s < ep)
        {
            wchar_t         c;

            /*
             * Keep printing characters.  Use a dummy
             * character for unprintable sequences (which
             * should not happen).
             */
            c = *s++;
            if (iswspace(c))
                c = ' ';
            else if (!iswprint(c))
                c = '?';
            nbytes = wctomb(tp, c);

            if (nbytes <= 0)
            {
                /*
                 * This should not happen!  All
                 * unprintable characters should have
                 * been turned into C escapes inside the
                 * wstring.c file when converting from C
                 * string to wide strings.
                 *
                 * Replace invalid wide characters with
                 * a C escape.
                 */
                nbytes = 4;
                tp[0] = '\\';
                tp[1] = '0' + ((c >> 6) & 7);
                tp[2] = '0' + ((c >> 3) & 7);
                tp[3] = '0' + (c & 7);

                /*
                 * The wctomb state will be ''error'',
                 * so reset it and brave the worst.  No
                 * need to reset the wctomb state, it is
                 * not broken.
                 */
                if (wctomb(NULL, 0) == -1)
                {
                    assert(!"assume success");
                }
            }
            tp += nbytes;
        }

        /*
         * Add a newline and end any outstanding shift state and
         * add a NUL character.
         */
        nbytes = wctomb(tp, (wchar_t)'\n');
        if (nbytes > 0)
            tp += nbytes;
        nbytes = wctomb(tp, (wchar_t)0);
        if (nbytes > 0)
            tp += nbytes;

        /*
         * Emit the line to stderr.  It is important to do this
         * a whole line at a time, otherwise performance is
         * terrible - stderr by default is character buffered.
         */
        fputs(tmp, stderr);
        if (fflush_slowly(stderr))
            break;

        /*
         * skip leading spaces for subsequent lines
         */
        while (iswspace(*s))
            ++s;
        first_line = 0;
    }

    /*
     * done with humans
     */
    language_C();

    /*
     * make sure nothing went wrong
     */
    if (fflush_slowly(stderr))
        nfatal_raw("standard error");
}


static void
wrap(const wchar_t *s)
{
    for (;;)
    {
        const wchar_t   *start;
        const wchar_t   *end;
        int             done;

        while (*s && iswspace(*s))
            ++s;
        start = s;
        for (;;)
        {
            wchar_t         c;

            c = *s;
            if (!c || c == '\n')
                break;
            ++s;
        }
        end = s;
        done = !*s++;

        while (start < end && iswspace(end[-1]))
            --end;
        wrap_inner(start, end);

        if (done)
            break;
    }
}


void
error_intl(sub_context_ty *scp, char *s)
{
    wstring_ty      *message;
    int             need_to_delete;

    if (scp)
        need_to_delete = 0;
    else
    {
        scp = sub_context_new();
        need_to_delete = 1;
    }

    message = subst_intl_wide(scp, s);
    wrap(message->wstr_text);
    wstr_free(message);

    if (need_to_delete)
        sub_context_delete(scp);
}


void
fatal_intl(sub_context_ty *scp, char *s)
{
    wstring_ty      *message;
    static char     *double_jeopardy;

    /*
     * Make sure that there isn't an infinite loop,
     * if there is a problem with a substitution
     * in an error message.
     */
    if (double_jeopardy)
    {
        /*
         * this error message can't be internationalized
         */
        fatal_raw
        (
            "a fatal_intl error (\"%s\") happened while attempting to "
                "report an earlier fatal_intl error (\"%s\").  This is "
                "a probably bug.",
            s,
            double_jeopardy
        );
    }
    double_jeopardy = s;

    if (!scp)
        scp = sub_context_new();

    message = subst_intl_wide(scp, s);
    wrap(message->wstr_text);
    double_jeopardy = 0;
    quit(1);
}


void
verbose_intl(sub_context_ty *scp, char *s)
{
    wstring_ty      *message;
    int             need_to_delete;

    if (scp)
        need_to_delete = 0;
    else
    {
        scp = sub_context_new();
        need_to_delete = 1;
    }

    if (verbose_get())
    {
        message = subst_intl_wide(scp, s);
        wrap(message->wstr_text);
        wstr_free(message);
    }
    else
        sub_var_clear(scp);

    if (need_to_delete)
        sub_context_delete(scp);

#ifdef DEBUG
    /* Silence gcc warning */
    (void)trace_pretest_result;
#endif
}


/*
 * These are extra messages generated by various tools, but which must
 * be translated all the same.
 */

#if 0

static void
bogus(void)
{
    /* bison */
    i18n("parse error");
    i18n("parse error; also virtual memory exceeded");
    i18n("parser stack overflow");

    /* yacc */
    i18n("syntax error");
}

#endif
