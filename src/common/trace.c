/*
 *      cook - file construction tool
 *      Copyright (C) 1992-2007 Peter Miller
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
#include <common/ac/stddef.h>
#include <common/ac/stdio.h>
#include <common/ac/string.h>
#include <common/ac/stdarg.h>

#include <common/error.h>
#include <common/fflush_slow.h>
#include <common/mem.h>
#include <common/progname.h>
#include <common/str.h>
#include <common/trace.h>


#define INDENT 2
#define PAGE_WIDTH 79

typedef struct known_ty known_ty;
struct known_ty
{
    string_ty       *filename;
    int             flag;
    int             *flag_p;
    known_ty        *next;
};

static string_ty *file_name;
static int      line_number;
static int      page_width;
static known_ty *known;


static string_ty *
base_name(char *file)
{
    char            *cp1;
    char            *cp2;

    cp1 = strrchr(file, '/');
    if (cp1)
        ++cp1;
    else
        cp1 = file;
    cp2 = strrchr(cp1, '.');
    if (!cp2)
        cp2 = cp1 + strlen(cp1);
    if (cp2 > cp1 + 6)
        return str_n_from_c(cp1, 6);
    return str_n_from_c(cp1, cp2 - cp1);
}


int
trace_pretest(char *file, int *result)
{
    string_ty       *s;
    known_ty        *kp;
    int             err;

    err = errno;
    s = base_name(file);
    for (kp = known; kp; kp = kp->next)
    {
        if (str_equal(s, kp->filename))
        {
            str_free(s);
            break;
        }
    }
    if (!kp)
    {
        kp = (known_ty *) mem_alloc(sizeof(known_ty));
        kp->filename = s;
        kp->next = known;
        kp->flag = 2;   /* disabled */
        known = kp;
    }
    kp->flag_p = result;
    *result = kp->flag;
    errno = err;
    return *result;
}


void
trace_where(char *file, int line)
{
    string_ty       *s;
    int             err;

    /*
     * take new name fist, because will probably be same as last
     * thus saving a free and a malloc (which are slow)
     */
    err = errno;
    s = base_name(file);
    if (file_name)
        str_free(file_name);
    file_name = s;
    line_number = line;
    errno = err;
}


static void
trace_putchar(int c)
{
    static int      depth;
    static char     buffer[PAGE_WIDTH + 2];
    static char     *buf_end = buffer + sizeof(buffer);
    static char     *cp;
    static int      in_col;
    static int      out_col;
    char            *progname;

    progname = progname_get();
    if (!page_width)
    {
        page_width = PAGE_WIDTH - 24;
        if (page_width < 8)
            page_width = 8;
    }
    if (!cp)
    {
        strendcpy(buffer, progname, buf_end);
        cp = buffer + strlen(buffer);
        if (cp > buffer + 6)
            cp = buffer + 6;
        *cp++ = ':';
        *cp++ = '\t';
        cp = strendcpy(cp, file_name->str_text, buf_end);
        *cp++ = ':';
        *cp++ = '\t';
        snprintf(cp, buf_end - cp, "%d:\t", line_number);
        cp += strlen(cp);
        in_col = 0;
        out_col = 0;
    }
    switch (c)
    {
    case '\n':
        *cp++ = '\n';
        *cp = 0;
        fflush_slowly(stdout);
        fputs(buffer, stderr);
        if (fflush_slowly(stderr))
            nfatal_raw("standard error");
        cp = 0;
        break;

    case ' ':
        if (out_col)
            ++in_col;
        break;

    case '\t':
        if (out_col)
            in_col = (in_col / INDENT + 1) * INDENT;
        break;

    case '}':
    case ')':
    case ']':
        if (depth > 0)
            --depth;
        /* fall through */

    default:
        if (!out_col)
        {
            if (c != '#')
                /* modulo so never too long */
                in_col = (INDENT * depth) % page_width;
            else
                in_col = 0;
        }
        if (in_col >= page_width)
        {
            trace_putchar('\n');
            trace_putchar(c);
            return;
        }
        while (((out_col + 8) & -8) <= in_col && out_col + 1 < in_col)
        {
            *cp++ = '\t';
            out_col = (out_col + 8) & -8;
        }
        while (out_col < in_col)
        {
            *cp++ = ' ';
            ++out_col;
        }
        if (c == '{' || c == '(' || c == '[')
            ++depth;
        *cp++ = c;
        in_col++;
        out_col++;
        break;
    }
}


void
trace_printf(const char *s, ...)
{
    string_ty       *buffer;
    va_list         ap;
    int             err;

    err = errno;
    va_start(ap, s);
    buffer = str_vformat(s, ap);
    va_end(ap);
    for (s = buffer->str_text; *s; ++s)
        trace_putchar(*s);
    str_free(buffer);
    errno = err;
}


void
trace_enable(char *file)
{
    string_ty       *s;
    known_ty        *kp;

    s = base_name(file);
    for (kp = known; kp; kp = kp->next)
    {
        if (str_equal(s, kp->filename))
        {
            str_free(s);
            break;
        }
    }
    if (!kp)
    {
        kp = (known_ty *) mem_alloc(sizeof(known_ty));
        kp->filename = s;
        kp->flag_p = 0;
        kp->next = known;
        known = kp;
    }
    kp->flag = 3;       /* enabled */
    if (kp->flag_p)
        *kp->flag_p = kp->flag;

    /*
     * silence a warning
     */
#ifdef DEBUG
    trace_pretest_result = 0;
#endif
}


void
trace_char_real(char *name, char *vp)
{
    trace_printf("%s = '", name);
    if (*vp < ' ' || *vp > '~' || strchr("(){}[]", *vp))
    {
        char           *s;

        s = strchr("\bb\nn\tt\rr\ff", *vp);
        if (s)
        {
            trace_putchar('\\');
            trace_putchar(s[1]);
        }
        else
            trace_printf("\\%03o", (unsigned char)*vp);
    }
    else
    {
        if (strchr("'\\", *vp))
            trace_putchar('\\');
        trace_putchar(*vp);
    }
    trace_printf("'; /* 0x%02X, %d */\n", (unsigned char)*vp, *vp);
}


void
trace_char_unsigned_real(char *name, unsigned char *vp)
{
    trace_printf("%s = '", name);
    if (*vp < ' ' || *vp > '~' || strchr("(){}[]", *vp))
    {
        char           *s;

        s = strchr("\bb\nn\tt\rr\ff", *vp);
        if (s)
        {
            trace_putchar('\\');
            trace_putchar(s[1]);
        }
        else
            trace_printf("\\%03o", *vp);
    }
    else
    {
        if (strchr("'\\", *vp))
            trace_putchar('\\');
        trace_putchar(*vp);
    }
    trace_printf("'; /* 0x%02X, %d */\n", *vp, *vp);
}


void
trace_int_real(char *name, int *vp)
{
    trace_printf("%s = %d;\n", name, *vp);
}


void
trace_int_unsigned_real(char *name, unsigned int *vp)
{
    trace_printf("%s = %u;\n", name, *vp);
}


void
trace_long_real(char *name, long *vp)
{
    trace_printf("%s = %ld;\n", name, *vp);
}


void
trace_long_unsigned_real(char *name, unsigned long *vp)
{
    trace_printf("%s = %lu;\n", name, *vp);
}


void
trace_pointer_real(char *name, void *vptrptr)
{
    void            **ptr_ptr = vptrptr;
    void            *ptr;

    ptr = *ptr_ptr;
    if (!ptr)
        trace_printf("%s = NULL;\n", name);
    else
        trace_printf("%s = 0x%08lX;\n", name, (long)ptr);
}


void
trace_short_real(char *name, short *vp)
{
    trace_printf("%s = %hd;\n", name, *vp);
}


void
trace_short_unsigned_real(char *name, unsigned short *vp)
{
    trace_printf("%s = %hu;\n", name, *vp);
}


void
trace_string_real(const char *name, const char *vp)
{
    const char      *s;
    long            count;

    trace_printf("%s = ", name);
    if (!vp)
    {
        trace_printf("NULL;\n");
        return;
    }
    trace_printf("\"");
    count = 0;
    for (s = vp; *s; ++s)
    {
        switch (*s)
        {
        case '(':
        case '[':
        case '{':
            ++count;
            break;

        case ')':
        case ']':
        case '}':
            --count;
            break;
        }
    }
    if (count > 0)
        count = -count;
    else
        count = 0;
    for (s = vp; *s; ++s)
    {
        int             c;

        c = *s;
        if (c < ' ' || c > '~')
        {
            char            *cp;

            cp = strchr("\bb\ff\nn\rr\tt", c);
            if (cp)
                trace_printf("\\%c", cp[1]);
            else
            {
                escape:
                trace_printf("\\%03o", (unsigned char)c);
            }
        }
        else
        {
            switch (c)
            {
            case '(':
            case '[':
            case '{':
                ++count;
                if (count <= 0)
                    goto escape;
                break;

            case ')':
            case ']':
            case '}':
                --count;
                if (count < 0)
                    goto escape;
                break;

            case '\\':
            case '"':
                trace_printf("\\");
                break;
            }
            trace_printf("%c", c);
        }
    }
    trace_printf("\";\n");
}
