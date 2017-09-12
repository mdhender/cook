/*
 *      cook - file construction tool
 *      Copyright (C) 1997, 1998, 2001, 2006, 2007 Peter Miller;
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

#include <common/ac/ctype.h>
#include <common/ac/string.h>

#include <common/input.h>
#include <c_incl/lang_m4.h>
#include <common/mem.h>
#include <common/str_list.h>
#include <common/trace.h>


/*
 * NAME
 *      directive
 *
 * SYNOPSIS
 *      void directive(char *line, string_list_ty *type1,
 *              string_list_ty *type2);
 *
 * DESCRIPTION
 *      The directive function is used to scan a . line for an
 *      include directive.  If one is found, the filename
 *      is resolved, and the path appended to the appropriate list.
 *
 * ARGUMENTS
 *      line    - the line of text from the program
 *      type1   - list of <filenames>
 *      type2   - list of "filenames"
 *
 * CAVEATS
 *      Just ignore anything we don't understand.
 */

static void
directive(char *s, string_list_ty *type1, string_list_ty *type2)
{
    string_ty       *path;
    size_t          s_len;
    char            **kwpp;

    static char *keyword[] =
    {
        "include",
        "m4_include",           /* -P */
        "sinclude",
        "m4_sinclude",          /* -P */
    };

    /*
     * gnaw off any leading white space
     */
    trace(("directive(s = \"%s\", type1 = %08lX, type2 = %08lX)\n{\n", s, type1,
        type2));
    (void)type2;
    s_len = strlen(s);
    while (isspace((unsigned char)*s))
    {
        ++s;
        --s_len;
    }

    /*
     * see if it is a keyword we like
     */
    for (kwpp = keyword;; ++kwpp)
    {
        char            *kwp;
        size_t          nbytes;

        if (kwpp >= ENDOF(keyword))
        {
            trace(("}\n"));
            return;
        }
        kwp = *kwpp;
        nbytes = strlen(kwp);
        if (s_len >= nbytes && 0 == memcmp(s, kwp, nbytes))
        {
            s += nbytes;
            s_len -= nbytes;
            break;
        }
    }

    /*
     * The M4 definition says the paren is always immediately after
     * the keyword.
     */
    if (*s != '(')
    {
        trace(("}\n"));
        return;
    }
    ++s;
    --s_len;
    while (isspace((unsigned char)*s))
    {
        ++s;
        --s_len;
    }

    /*
     * assume the default openning quote
     */
    if (*s != '`')
    {
        trace(("}\n"));
        return;
    }
    ++s;
    --s_len;

    /*
     * gnaw off any trailing white space
     */
    while (s_len > 0 && isspace((unsigned char)s[s_len - 1]))
        --s_len;

    /*
     * look for the closing paren
     */
    if (s_len < 1 || s[s_len - 1] != ')')
    {
        trace(("}\n"));
        return;
    }
    --s_len;

    while (s_len > 0 && isspace((unsigned char)s[s_len - 1]))
        --s_len;

    /*
     * look for the closing quote
     */
    if (s_len < 1 || s[s_len - 1] != '\'')
    {
        trace(("}\n"));
        return;
    }
    --s_len;

    /*
     * there should be something left
     */
    if (s_len < 1)
    {
        trace(("}\n"));
        return;
    }

    /*
     * extract the path
     */
    path = str_n_from_c(s, s_len);

    /*
     * dispatch the path to the appropriate list
     */
    string_list_append_unique(type1, path);
    str_free(path);
    trace(("}\n"));
}


/*
 * NAME
 *      lang_m4_scan
 *
 * SYNOPSIS
 *      int lang_m4_scan(input_ty *fp, string_list_ty *type1,
 *              string_list_ty *type2);
 *
 * DESCRIPTION
 *      The lang_m4_scan function is used to scan a file looking
 *      for nclude files.  It does not walk the children.
 *      The names of any include files encountered are appended
 *      to the appropriate list.
 *
 * ARGUMENTS
 *      fp      - file stream to scan
 *      type1   - list of <filenames>
 *      type2   - list of "filenames"
 *
 * RETURNS
 *      int;    0 on success
 *              -1 on file errors
 */

static int
lang_m4_scan(input_ty *fp, string_list_ty *type1, string_list_ty *type2)
{
    size_t          pos;
    size_t          max;
    char            *line;
    int             result;
    int             c;

    trace(("lang_m4_scan(fp = %08lX, type1 = %08lX, type2 = %08lX)\n{\n", fp,
        type1, type2));
    pos = 0;
    max = 100;
    line = mem_alloc(max);
    result = 0;
    for (;;)
    {
        if (pos >= max)
        {
            max += 80;
            line = mem_change_size(line, max);
        }
        c = input_getc(fp);
        switch (c)
        {
        case INPUT_EOF:
            if (!pos)
                break;
            /* fall through... */

        case '\n':
            line[pos] = 0;
            pos = 0;

            /*
             * see if it is an include line
             */
            directive(line, type1, type2);
            continue;

        default:
            line[pos++] = c;
            continue;
        }
        break;
    }
    mem_free(line);
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}


static void
lang_m4_prepare(void)
{
    trace(("lang_m4_prepare()\n{\n"));
    if (sniff_include_count() == 0)
        sniff_include(".");
    trace(("}\n"));
}


sniff_ty lang_m4 =
{
    lang_m4_scan,
    lang_m4_prepare,
};
