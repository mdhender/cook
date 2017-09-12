/*
 *      cook - file construction tool
 *      Copyright (C) 1992-1994, 1997, 1998, 2001, 2006-2009 Peter Miller
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
#include <c_incl/lang_roff.h>
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
directive(const char *s, string_list_ty *type1, string_list_ty *type2)
{
    string_list_ty  args;

    /*
     * Dismantle the directive.
     */
    trace(("directive(s = \"%s\", type1 = %p, type2 = %p)\n{\n", s, type1,
        type2));
    (void)type2;
    string_list_constructor(&args);
    assert(*s == '.');
    s++;
    for (;;)
    {
        unsigned char   c;
        const char      *start;

        for (;;)
        {
            c = *s;
            if (!c)
                break;
            if (!isspace(c))
                break;
            ++s;
        }
        if (!c)
            break;
        start = s;
        ++s;
        for (;;)
        {
            c = *s;
            if (!c)
                break;
            if (isspace(c))
                break;
            ++s;
        }
        string_list_append(&args, str_n_from_c(start, s - start));
    }

    /*
     * see if it is a .so directive
     */
    if (args.nstrings >= 2 && 0 == strcmp("so", args.string[0]->str_text))
    {
        string_list_append_unique(type1, args.string[1]);
    }
    else if
    (
        args.nstrings >= 2
    &&
        0 == strcmp("PSPIC", args.string[0]->str_text)
    )
    {
        size_t          j;

        j = 1;
        if
        (
            j < args.nstrings
        &&
            (
                0 == strcmp("-L", args.string[j]->str_text)
            ||
                0 == strcmp("-R", args.string[j]->str_text)
            )
        )
            ++j;
        else if
        (
            j < args.nstrings
        &&
            0 == strcmp("-I", args.string[j]->str_text)
        )
            j += 2;

        if (j < args.nstrings)
            string_list_append_unique(type1, args.string[j]);
    }

    string_list_destructor(&args);
    trace(("}\n"));
}


/*
 * NAME
 *      lang_roff_scan
 *
 * SYNOPSIS
 *      int lang_roff_scan(input_ty *fp, string_list_ty *type1,
 *              string_list_ty *type2);
 *
 * DESCRIPTION
 *      The lang_roff_scan function is used to scan a file looking
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
lang_roff_scan(input_ty *fp, string_list_ty *type1, string_list_ty *type2)
{
    size_t          pos;
    size_t          max;
    char            *line;
    int             result;
    int             c;

    trace(("lang_roff_scan(fp = %p, type1 = %p, type2 = %p)\n{\n", fp, type1,
        type2));
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
             * see if it is a control line
             */
            if (line[0] == '.')
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
lang_roff_prepare(void)
{
    trace(("lang_roff_prepare()\n{\n"));
    if (sniff_include_count() == 0)
        sniff_include(".");
    trace(("}\n"));
}


sniff_ty lang_roff =
{
    lang_roff_scan,
    lang_roff_prepare,
};
