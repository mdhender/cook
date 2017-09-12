/*
 *      cook - file construction tool
 *      Copyright (C) 1994, 1997-1999, 2001, 2006-2008 Peter Miller
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

#include <make2cook/emit.h>
#include <common/error_intl.h>
#include <common/trace.h>

static string_ty *filename;
static FILE     *output;
int             emit_line_numbers;
static int      icol;
static int      ocol;
static long     old_ln;
static string_ty *old_fn;
static int      depth;
static int      back_slash_seen;


void
emit_open(char *s)
{
    sub_context_ty  *scp;

    trace(("open\n"));
    assert(!output);
    if (s)
    {
        output = fopen_and_check(s, "w");
        filename = str_from_c(s);
    }
    else
    {
        scp = sub_context_new();
        filename = subst_intl(scp, i18n("standard output"));
        sub_context_delete(scp);
        output = stdout;
    }
    icol = 0;
    ocol = 0;
    back_slash_seen = 0;
}


void
emit_close(void)
{
    assert(output);
    emit_bol();
    fflush_and_check(output, filename->str_text);
    if (output != stdout)
        fclose_and_check(output, filename->str_text);
    str_free(filename);
    filename = 0;
    output = 0;
    icol = 0;
    ocol = 0;
    back_slash_seen = 0;
}


void
emit_bol(void)
{
    if (ocol)
        emit_char('\n');
}


void
emit_char(int c)
{
    if (!icol && !ocol)
        icol = 8 * depth;
    if (back_slash_seen && (c == ' ' || c == '\\'))
    {
        assert(icol == ocol);
        back_slash_seen = 0;
        goto normal;
    }
    back_slash_seen = (c == '\\');
    switch (c)
    {
    case ' ':
        ++icol;
        return;

    case '\t':
        icol = (icol + 8) & ~7;
        return;

    case '\n':
        if (putc(c, output) == EOF)
            goto bomb;
        icol = 0;
        ocol = 0;
        ++old_ln;
        return;
    }
  normal:
    while (ocol < icol)
    {
        if (putc(' ', output) == EOF)
            goto bomb;
        ++ocol;
    }
    if (putc(c, output) == EOF)
    {
      bomb:
        fatal_intl_write(filename->str_text);
    }
    ++icol;
    ++ocol;
}


void
emit_str(char *s)
{
    while (*s)
        emit_char(*s++);
}


void
emit_string(string_ty *s)
{
    emit_str(s->str_text);
}


void
emit_line_number(long lino, string_ty *fn)
{
    static string_ty *builtin;

    assert(output);
    if (!builtin)
        builtin = str_from_c("builtin");
    if (str_equal(fn, builtin))
        return;
    if (old_ln == lino && str_equal(old_fn, fn))
        return;

    /*
     * for short distances in the same file,
     * just throw newlines
     */
    if (str_equal(old_fn, fn) && lino > old_ln && lino - old_ln < 10)
    {
        while (old_ln < lino)
            emit_char('\n');
        return;
    }

    if (emit_line_numbers)
    {
        emit_bol();
        fprintf(output, "#line %ld \"%s\"\n", lino, fn->str_text);
        fflush_and_check(output, filename->str_text);
    }
    else if (!str_equal(old_fn, fn) || lino > (old_ln + (ocol != 0)))
    {
        emit_bol();
        emit_char('\n');
    }

    old_ln = lino;
    if (old_fn)
        str_free(old_fn);
    old_fn = str_copy(fn);
}


void
emit_set_file(string_ty *fn)
{
    if (str_equal(old_fn, fn))
        return;
    emit_bol();
    if (old_fn)
    {
        str_free(old_fn);
        emit_char('\n');
    }
    old_fn = str_copy(fn);
    old_ln = 32767;
}


void
emit_indent_more(void)
{
    ++depth;
}


void
emit_indent_less(void)
{
    --depth;
}
