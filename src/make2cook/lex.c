/*
 *      cook - file construction tool
 *      Copyright (C) 1994, 1997-1999, 2001, 2006, 2007 Peter Miller;
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
#include <common/ac/stdarg.h>

#include <common/error_intl.h>
#include <common/input/file_text.h>
#include <common/input/stdin.h>
#include <common/mem.h>
#include <common/symtab.h>
#include <common/trace.h>
#include <make2cook/blob.h>
#include <make2cook/lex.h>
#include <make2cook/stmt.h>
#include <make2cook/gram.gen.h> /* must be last */

static input_ty *input;
static long     line_number;
static int      error_count;
static int      bol;
static int      first;
static int      colon_special;
static int      within_define;
static long     sa_data_length;
static long     sa_data_max;
static char     *sa_data;


static void
notify(void)
{
    if (!input)
        return;
    if (++error_count >= 20)
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        sub_var_set_string(scp, "File_Name", input_filename(input));
        fatal_intl(scp, i18n("$filename: too many fatal errors"));
    }
}


void
lex_open(char *s)
{
    trace(("lex_open()\n{\n"));
    assert(!input);
    blob_error_notify(notify);
    if (s)
    {
        string_ty       *filename;

        filename = str_from_c(s);
        input = input_file_text_open(filename);
        str_free(filename);
    }
    else
    {
        input = input_file_text_open((string_ty *) 0);
    }
    line_number = 1;
    bol = 1;
    first = 1;
    colon_special = 1;
    error_count = 0;
    trace(("}\n"));
}


void
lex_close(void)
{
    trace(("lex_close()\n{\n"));
    assert(input);
    if (error_count)
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        sub_var_set_string(scp, "File_Name", input_filename(input));
        sub_var_set_long(scp, "Number", error_count);
        sub_var_optional(scp, "Number");
        fatal_intl(scp, i18n("$filename: found $number fatal errors"));
    }
    input_delete(input);
    input = 0;
    line_number = 0;
    bol = 0;
    first = 0;
    trace(("}\n"));
}


void
gram_error(char *s)
{
    lex_error(0, s);
}


void
lex_error(sub_context_ty *scp, char *s)
{
    string_ty       *buffer;
    int             len;
    int             need_to_delete;

    if (scp)
        need_to_delete = 0;
    else
    {
        scp = sub_context_new();
        need_to_delete = 1;
    }

    buffer = subst_intl(scp, s);
    len = buffer->str_length;
    while (len > 0 && isspace(buffer->str_text[len - 1]))
        --len;
    /* re-use substitution context */
    sub_var_set_string(scp, "File_Name", input_filename(input));
    sub_var_set_long(scp, "Number", line_number);
    sub_var_set(scp, "MeSsaGe", "%.*s", len, buffer->str_text);
    str_free(buffer);
    error_intl(scp, i18n("$filename: $number: $message"));
    notify();

    if (need_to_delete)
        sub_context_delete(scp);
}


static int
byte(void)
{
    int             c;

    assert(input);
    for (;;)
    {
        c = input_getc(input);
        switch (c)
        {
        case INPUT_EOF:
            bol = 1;
            break;

        case '\n':
            ++line_number;
            bol = 1;
            first = 1;
            colon_special = 1;
            break;

        case ' ':
        case '\t':
        case '\f':
#if __STDC__ >= 1
        case '\v':
#endif
            bol = 0;
            break;

        case '\\':
            c = input_getc(input);
            if (c == '\n')
            {
                ++line_number;
                continue;
            }
            if (c != INPUT_EOF)
                input_ungetc(input, c);
            c = '\\';
            /* fall through... */

        default:
            bol = 0;
            first = 0;
            break;
        }
        return c;
    }
}


static void
byte_undo(int c)
{
    switch (c)
    {
    case INPUT_EOF:
        break;

    case '\n':
        --line_number;
        /* fall through... */

    default:
        input_ungetc(input, c);
        break;
    }
}


/*
 * NAME
 *      lex_trace - debug output
 *
 * SYNOPSIS
 *      void lex_trace(char *s, ...);
 *
 * DESCRIPTION
 *      The lex_trace function is used to format and output yyparse's trace
 *      output.  The printf's in yyparse are #define'd into lex_trace calls.
 *      Unfortunately yacc's designer did not take trace output redirection
 *      into account, to this function is a little tedious.
 *
 * RETURNS
 *      void
 *
 * CAVEAT
 *      This function is only available when the DEBUG symbol is #define'd.
 */

#ifdef DEBUG

void
gram_trace(char *s, ...)
{
    va_list         ap;
    string_ty       *buffer;
    char            *cp;
    static char     line[1024];

    va_start(ap, s);
    buffer = str_vformat(s, ap);
    va_end(ap);
    cp = line + strlen(line);
    cp = strendcpy(cp, buffer->str_text, line + sizeof(line));
    str_free(buffer);
    if (cp > line && cp[-1] == '\n')
    {
        --cp;
        *cp = 0;
        trace_printf
        (
            "%s: %ld: %s\n",
            input_filename(input)->str_text,
            line_number,
            line
        );
        line[0] = 0;
    }
}


void
gram_trace2(void *garbage, char *s, ...)
{
    va_list         ap;
    string_ty       *buffer;
    char            *cp;
    static char     line[1024];

    va_start(ap, s);
    buffer = str_vformat(s, ap);
    va_end(ap);
    cp = line + strlen(line);
    cp = strendcpy(cp, buffer->str_text, line + sizeof(line));
    str_free(buffer);
    if (cp > line && cp[-1] == '\n')
    {
        --cp;
        *cp = 0;
        trace_printf
        (
            "%s: %ld: %s\n",
            input_filename(input)->str_text,
            line_number,
            line
        );
        line[0] = 0;
    }
}

#endif


static int
reserved(string_ty *s)
{
    typedef struct table_ty table_ty;
    struct table_ty
    {
        char            *name;
        int             token;
    };

    static table_ty table[] =
    {
        { "override", OVERRIDE },
        { "include", INCLUDE2 },
        { "-include", INCLUDE3 },
        { ".include", INCLUDE },
        { "vpath", VPATH },
        { "VPATH", VPATH2 },
        { "ifdef", IF },
        { "ifndef", IF },
        { "ifeq", IF },
        { "ifneq", IF },
        { "else", ELSE },
        { "endif", ENDIF },
        { "endef", ENDDEF },
        { "define", DEFINE },
        { "export", EXPORT },
        { "unexport", UNEXPORT },
    };

    static symtab_ty *symtab;
    int             *data;
    string_ty       *name;
    char            *cp;

    if (!symtab)
    {
        table_ty        *tp;

        symtab = symtab_alloc(SIZEOF(table));
        for (tp = table; tp < ENDOF(table); ++tp)
        {
            name = str_from_c(tp->name);
            symtab_assign(symtab, name, &tp->token);
            str_free(name);
        }
    }

    cp = strchr(s->str_text, '(');
    if (cp)
    {
        name = str_n_from_c(s->str_text, cp - s->str_text);
        data = symtab_query(symtab, name);
        str_free(name);
    }
    else
        data = symtab_query(symtab, s);
    if (data)
        return *data;
    return 0;
}


static void
sa_open(void)
{
    sa_data_length = 0;
}


static string_ty *
sa_close(void)
{
    string_ty       *s;

    s = str_n_from_c(sa_data, sa_data_length);
    sa_data_length = 0;
    return s;
}


static void
sa_char(int c)
{
    if (sa_data_length >= sa_data_max)
    {
        sa_data_max = sa_data_max * 2 + 16;
        sa_data = mem_change_size(sa_data, sa_data_max);
    }
    sa_data[sa_data_length++] = c;
}


int
gram_lex(void)
{
    static char     *paren;
    static long     paren_max;
    long            paren_depth;
    int             c;
    long            linum;
    int             bol_was;
    int             first_was;
    string_ty       *s;
    int             token;
    int             start_of_line = 0;

    trace(("gram_lex()\n{\n"));
    for (;;)
    {
        linum = line_number;
        bol_was = bol;
        first_was = first;
        c = byte();
        switch (c)
        {
        case INPUT_EOF:
            token = 0;
            goto done;

        case '\t':
            if (!bol_was || within_define)
                continue;
            sa_open();
            for (;;)
            {
                c = byte();
                switch (c)
                {
                case INPUT_EOF:
                case '\n':
                    break;

                case ' ':
                case '\t':
                case '\f':
#if __STDC__ >= 1
                case '\v':
#endif
                    if (sa_data_length)
                        sa_char(c);
                    continue;

                default:
                    sa_char(c);
                    continue;
                }
                break;
            }
            gram_lval.lv_line =
                blob_alloc(sa_close(), input_filename(input), linum);
            token = COMMAND;
            goto done;

        case '#':
            sa_open();
          more_comment:
            start_of_line = 1;
            for (;;)
            {
                c = byte();
                switch (c)
                {
                case INPUT_EOF:
                case '\n':
                    break;

                case '#':
                    if (!start_of_line)
                        sa_char(c);
                    continue;

                case ' ':
                case '\t':
                case '\f':
#if __STDC__ >= 1
                case '\v':
#endif
                    if (!start_of_line)
                        sa_char(' ');
                    continue;

                default:
                    sa_char(c);
                    start_of_line = 0;
                    continue;
                }
                break;
            }
            if (!first_was)
            {
                /*
                 * If the comment did not start at the
                 * beginning of the line, throw it away.
                 */
                byte_undo('\n');
                continue;
            }
            token = COMMENT;
            if (c == '\n')
            {
                /*
                 * Take a peek at the next character.
                 * If it is '#', we have more comment.
                 * If it is '\t', we have a code comment.
                 */
                c = byte();
                if (c == '#')
                {
                    sa_char('\n');
                    goto more_comment;
                }
                if (c == '\t')
                    token = COMMAND_COMMENT;
                byte_undo(c);

                /* need to restore this state, too */
                bol = 1;
                first = 1;
                colon_special = 1;
            }
            gram_lval.lv_line =
                blob_alloc(sa_close(), input_filename(input), linum);
            goto done;

        case ' ':
        case '\f':
#if __STDC__ >= 1
        case '\v':
#endif
            break;

        case '\n':
            token = EOLN;
            goto done;

        case ';':
            if (!colon_special)
                goto normal;
            byte_undo('\t');
            bol = 1;
            first = 1;
            colon_special = 1;
            token = EOLN;
            goto done;

        case ':':
            if (!colon_special)
                goto normal;
            c = byte();
            if (c == ':')
            {
                token = COLON_COLON;
                goto done;
            }
            if (c == '=')
            {
                token = COLON_EQUALS;
                colon_special = 0;
                goto done;
            }
            byte_undo(c);
            token = COLON;
            goto done;

        case '=':
            token = EQUALS;
            colon_special = 0;
            goto done;

        case '+':
            c = byte();
            if (c == '=')
            {
                token = PLUS_EQUALS;
                colon_special = 0;
                goto done;
            }
            byte_undo(c);
            c = '+';
            /* fall through... */

        default:
          normal:
            sa_open();
            paren_depth = 0;
            for (;;)
            {
                switch (c)
                {
                case INPUT_EOF:
                case '\n':
                    break;

                case ' ':
                case '\t':
                case '\f':
#if __STDC__ >= 1
                case '\v':
#endif
                    if (!within_define && !paren_depth)
                        break;
                    sa_char(c);
                    c = byte();
                    continue;

                case ';':
                case ':':
                case '=':
                    if (colon_special && !within_define && !paren_depth)
                        break;
                    sa_char(c);
                    c = byte();
                    continue;

                default:
                    sa_char(c);
                    c = byte();
                    continue;

                case '(':
                    sa_char(c);
                    if (paren_depth >= paren_max)
                    {
                        paren_max = paren_max * 2 + 16;
                        paren = mem_change_size(paren, paren_max);
                    }
                    paren[paren_depth++] = ')';
                    c = byte();
                    continue;

                case ')':
                case '}':
                    sa_char(c);
                    if (paren_depth && c == paren[paren_depth - 1])
                        --paren_depth;
                    c = byte();
                    continue;

                case '{':
                    sa_char(c);
                    if (paren_depth >= paren_max)
                    {
                        paren_max = paren_max * 2 + 16;
                        paren = mem_change_size(paren, paren_max);
                    }
                    paren[paren_depth++] = '}';
                    c = byte();
                    continue;
                }
                break;
            }
            byte_undo(c);
            s = sa_close();
            if (first_was && (token = reserved(s)) != 0)
            {
                switch (token)
                {
                case DEFINE:
                    str_free(s);
                    ++within_define;
                    break;

                case ENDDEF:
                    str_free(s);
                    --within_define;
                    break;

                case IF:
                    gram_lval.lv_line =
                        blob_alloc(s, input_filename(input), linum);
                    break;

                case VPATH:
                    colon_special = 0;
                    break;

                default:
                    str_free(s);
                    break;
                }
                goto done;
            }
            gram_lval.lv_line = blob_alloc(s, input_filename(input), linum);
            token = WORD;
            goto done;
        }
    }

    /*
     * here for all exits
     */
  done:
#ifdef DEBUG
    if (token == WORD || token == COMMENT || token == COMMAND)
        trace(("text = \"%s\";\n", gram_lval.lv_line->text->str_text));
#endif
    trace(("return %d;\n", token));
    trace(("}\n"));
    return token;
}


blob_ty *
lex_blob(string_ty *s)
{
    return blob_alloc(s, input_filename(input), line_number - first);
}
