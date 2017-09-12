/*
 *      cook - file construction tool
 *      Copyright (C) 2001, 2002, 2005-2007 Peter Miller;
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
#include <common/ac/stdio.h>
#include <common/ac/string.h>

#include <file_check/file_check.h>
#include <common/error_intl.h>

int             warning;
int             limit;
static int      number_of_blank_lines;
static int      number_of_errors;
static int      line_number;
static FILE     *fp;
static string_ty *fn;


static int
check_one_line(void)
{
    int             unprintable;
    int             white_space;
    int             pos;
    sub_context_ty  *scp;
    int             line_contains_white_space;

    ++line_number;
    pos = 0;
    unprintable = 0;
    white_space = 0;
    line_contains_white_space = 0;
    for (;;)
    {
        int             c;

        c = getc(fp);
        if (c == EOF)
        {
            if (ferror(fp))
                fatal_intl_read(fn->str_text);
            if (pos)
            {
                scp = sub_context_new();
                sub_var_set_string(scp, "File_Name", fn);
                sub_var_set_long(scp, "Number", line_number);
                error_intl
                (
                    scp,
                    i18n("$filename: $number: last line has no newline")
                );
                ++number_of_errors;
                sub_context_delete(scp);
                goto done;
            }
            return 0;
        }
        switch (c)
        {
        case '\f':
            ++pos;
            break;

        case '\n':
            done:
            if (unprintable)
            {
                scp = sub_context_new();
                sub_var_set_string(scp, "File_Name", fn);
                sub_var_set_long(scp, "Number", line_number);
                sub_var_set_long(scp, "Excess", unprintable);
                error_intl
                (
                    scp,
                   i18n("$filename: $number: line contains $excess unprintable")
                );
                ++number_of_errors;
                sub_context_delete(scp);
            }
            if (white_space)
            {
                scp = sub_context_new();
                sub_var_set_string(scp, "File_Name", fn);
                sub_var_set_long(scp, "Number", line_number);
                error_intl
                (
                    scp,
                    i18n("$filename: $number: white space at end of line")
                );
                ++number_of_errors;
                sub_context_delete(scp);
            }
            if (pos > limit && line_contains_white_space)
            {
                scp = sub_context_new();
                sub_var_set_string(scp, "File_Name", fn);
                sub_var_set_long(scp, "Number", line_number);
                sub_var_set_long(scp, "Excess", pos - limit);
                error_intl
                (
                    scp,
                    i18n("$filename: $number: line too long (by $excess)")
                );
                ++number_of_errors;
                sub_context_delete(scp);
            }
            if (pos)
                number_of_blank_lines = 0;
            else
                ++number_of_blank_lines;
            return 1;

        case '\t':
            pos = (pos + 8) & ~7;
            ++white_space;
            ++line_contains_white_space;
            break;

        case ' ':
            ++pos;
            ++white_space;
            ++line_contains_white_space;
            break;

        default:
            if (!isprint(c))
                ++unprintable;
            ++pos;
            white_space = 0;
            break;
        }
    }
}


static int
begins_with(const char *haystack, const char *needle)
{
    size_t          hlen;
    size_t          nlen;

    hlen = strlen(haystack);
    nlen = strlen(needle);
    return (hlen >= nlen && 0 == memcmp(haystack, needle, nlen));
}


static int
ends_with(const char *haystack, const char *needle)
{
    size_t          hlen;
    size_t          nlen;

    hlen = strlen(haystack);
    nlen = strlen(needle);
    return (hlen >= nlen && 0 == memcmp(haystack + hlen - nlen, needle, nlen));
}


void
file_check(string_ty *file_name)
{
    sub_context_ty  *scp;
    const char      *sfn;

    fn = file_name;
    fp = fopen_and_check(fn->str_text, "r");
    limit = 80;

    sfn = file_name->str_text;
    while (sfn[0] == 'b' && sfn[1] == 'l')
        sfn += 2;
    if (*sfn == '/')
        ++sfn;
    else
        sfn = file_name->str_text;

    if (begins_with(sfn, "test/") && ends_with(sfn, ".sh"))
        limit = 510;

    number_of_errors = 0;
    number_of_blank_lines = 0;
    line_number = 0;
    while (check_one_line())
        ;
    if (number_of_blank_lines)
    {
        scp = sub_context_new();
        sub_var_set_string(scp, "File_Name", fn);
        sub_var_set_long(scp, "Excess", number_of_blank_lines);
        error_intl(scp, i18n("$filename: found $excess blank lines at eof"));
        ++number_of_errors;
        sub_context_delete(scp);
    }
    if (number_of_errors && !warning)
    {
        scp = sub_context_new();
        sub_var_set_string(scp, "File_Name", fn);
        sub_var_set_long(scp, "Number", number_of_errors);
        fatal_intl(scp, i18n("$filename: found $number fatal errors"));
    }
    fclose_and_check(fp, fn->str_text);
    fn = 0;
    fp = 0;
}
