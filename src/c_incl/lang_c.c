/*
 *      cook - file construction tool
 *      Copyright (C) 1992-1994, 1997, 1998, 2001, 2006, 2007 Peter Miller;
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
#include <c_incl/lang_c.h>
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
 *      The directive function is used to scan a # control line for an
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
directive(char *s, string_list_ty * type1, string_list_ty * type2)
{
    int             right;
    char            *filename;
    string_ty       *path;

    /*
     * see if it is a #include directive
     */
    trace(("directive(s = \"%s\", type1 = %08lX, type2 = %08lX)\n{\n",
            s, type1, type2));
    assert(*s == '#');
    s++;
    while (isspace(*s))
        ++s;
    if (memcmp(s, "include", 7))
        goto done;
    s += 7;
    while (isspace(*s))
        ++s;

    /*
     * figure which type
     */
    switch (*s++)
    {
    default:
        goto done;

    case '"':
        right = '"';
        break;

    case '<':
        right = '>';
        break;
    }

    /*
     * find the end of the filename
     *      (ignore anything on the end of the line)
     */
    filename = s;
    while (*s != right)
    {
        if (!*s)
            goto done;
        ++s;
    }

    /*
     * extract the path
     */
    if (s == filename)
        goto done;
    path = str_n_from_c(filename, s - filename);

    /*
     * dispatch the path to the appropriate list
     */
    if (right != '"' || absolute_filename_test(path->str_text))
    {
        trace(("type1 %s\n", path->str_text));
        string_list_append_unique(type1, path);
    }
    else
    {
        trace(("type2 %s\n", path->str_text));
        string_list_append_unique(type2, path);
    }
    str_free(path);

    /*
     * here for all exits
     */
  done:
    trace(("}\n"));
}


/*
 * NAME
 *      lang_c_scan
 *
 * SYNOPSIS
 *      int lang_c_scan(input_ty *fp, string_list_ty *type1,
 *              string_list_ty *type2);
 *
 * DESCRIPTION
 *      The lang_c_scan function is used to scan a file looking
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
lang_c_scan(input_ty *fp, string_list_ty *type1, string_list_ty *type2)
{
    size_t          pos;
    size_t          max;
    char            *line;
    int             result;
    int             c;
    char            *cp;

    trace(("lang_c_scan(fp = %08lX, type1 = %08lX, type2 = %08lX)\n{\n", fp,
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
             * see if it is a hash line
             */
            for (cp = line; isspace(*cp); ++cp)
                ;
            if (*cp == '#')
                directive(cp, type1, type2);
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
lang_c_prepare(void)
{
    trace(("lang_c_prepare()\n{\n"));
    sniff_include("/usr/include");
    trace(("}\n"));
}


sniff_ty lang_c =
{
    lang_c_scan,
    lang_c_prepare,
};
