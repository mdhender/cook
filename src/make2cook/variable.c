/*
 *      cook - file construction tool
 *      Copyright (C) 1994, 1997, 1998, 2001, 2002, 2006-2010 Peter Miller
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
#include <common/ac/stdarg.h>
#include <common/ac/string.h>

#include <common/error_intl.h>
#include <common/symtab.h>
#include <common/trace.h>
#include <make2cook/vargram.h>
#include <make2cook/variable.h>
#include <make2cook/vargram.yacc.h>      /* must be last */


static blob_ty *source;
static char    *lexpos;
static int      lex_after_dollars;
static int      lex_depth;
static int      lex_quote_spaces;
static int      lex_quote_quotes;
static string_list_ty *reference;
static symtab_ty *symtab;
static symtab_ty *special;
static string_list_ty result;
static int      allow_archive_parens;


static void
init(void)
{
    typedef struct table_ty table_ty;
    struct table_ty
    {
        char            *name;
        char            *value;
    };

    static table_ty table[] =
    {
        { "%", "" },
        { "%D", "" },
        { "%F", "" },
        { "*", "%0%" },
        { "*D", "[dirname %0%]" },
        { "*F", "[entryname %0%]" },
        { "<", "[resolve [head [need]]]" },
        { "<D", "[dirname [resolve [head [need]]]]" },
        { "<F", "[entryname [resolve [head [need]]]]" },
        { "?", "[resolve [younger]]" },
        { "?D", "[dirname [resolve [younger]]]" },
        { "?F", "[entryname [resolve [younger]]]" },
        { "@", "[target]" },
        { "@D", "[dirname [target]]" },
        { "@F", "[entryname [target]]" },
        { "^", "[resolve [need]]" },
        { "^D", "[dirname [resolve [need]]]" },
        { "^F", "[entryname [resolve [need]]]" },
        { "MAKECMDGOALS", "[command-line-goals]" },
        { "MAKE", "[self]" },
        { "MAKEFLAGS", "[options]" },
        { "MFLAGS", "[options]" },
        { "VERSION", "[version]" },
        { "VPATH", "[search_list]" },
    };

    table_ty        *tp;
    string_ty       *name;
    string_ty       *value;

    if (symtab)
        return;
    trace(("init()\n{\n"));
    symtab = symtab_alloc(SIZEOF(table));
    for (tp = table; tp < ENDOF(table); ++tp)
    {
        name = str_from_c(tp->name);
        value = str_from_c(tp->value);
        symtab_assign(symtab, name, value);
        str_free(name);
    }
    trace(("}\n"));
}


string_ty *
variable_mangle_lookup(string_ty *name)
{
    string_ty       *data;
    string_ty       *retval;

    trace(("variable_mangle_lookup(\"%s\")\n{\n", name->str_text));
    assert(symtab);
    data = 0;
    if (special)
        data = symtab_query(special, name);
    if (!data)
        data = symtab_query(symtab, name);
    if (data)
        retval = str_copy(data);
    else
    {
        string_list_append_unique(reference, name);
        retval = str_format("[%s]", name->str_text);
    }
    trace(("return \"%s\";\n", retval->str_text));
    trace(("}\n"));
    return retval;
}


void
variable_mangle_forget(string_ty *name)
{
    string_list_remove(reference, name);
}


#ifdef DEBUG

static char *
unctrl(int c)
{
    static char     buf[5];

    if (c == '\n')
        return "\\n";
    if (c == '^' || c == '\\' || c == '\'')
    {
        buf[0] = '\\';
        buf[1] = c;
        buf[2] = 0;
        return buf;
    }
    if (isprint(c))
    {
        buf[0] = c;
        buf[1] = 0;
        return buf;
    }
    c ^= 0x40;
    if (isprint(c))
    {
        buf[0] = '^';
        buf[1] = c;
        buf[2] = 0;
        return buf;
    }
    c ^= 0x40;
    buf[0] = '\\';
    buf[1] = '0' + ((c >> 6) & 3);
    buf[2] = '0' + ((c >> 3) & 7);
    buf[3] = '0' + (c & 7);
    buf[4] = 0;
    return buf;
}

#endif


int
vargram_lex(void)
{
    int             c;
    int             token;
    int             quote_spaces;

    trace(("vargram_lex()\n{\n"));
    quote_spaces = (lex_quote_spaces && (lex_depth <= 0));
    c = (unsigned char)*lexpos++;
    trace(("c = '%s';\n", unctrl(c)));
    if (lex_after_dollars)
    {
        /*
         * Characters immediately following a $ symbol.
         */
        lex_after_dollars = 0;
        switch (c)
        {
        case 0:
            --lexpos;
            token = 0;
            break;

        case '$':
            token = DOLLAR;
            break;

        case '(':
            ++lex_depth;
            token = LP;
            break;

        case '{':
            ++lex_depth;
            token = LB;
            break;

        default:
            goto normal;
        }
    }
    else if (lex_depth <= 0)
    {
        int             len;
        char            buf[5];

        /*
         * Characters at the outermost level of interpretation,
         * not within any other sequence.
         */
      normal:
        token = PLAIN;
        len = 0;
        switch (c)
        {
        case 0:
            --lexpos;
            token = 0;
            goto done;

        case '$':
            token = DOLLAR;
            lex_after_dollars = 1;
            goto done;

        case ' ':
            if (!quote_spaces)
            {
                token = SPACE;
                goto done;
            }
            buf[len++] = '\\';
            buf[len++] = c;
            break;

        case '\b':
            buf[len++] = '\\';
            buf[len++] = 'b';
            break;

        case '\f':
            if (!quote_spaces)
            {
                token = SPACE;
                goto done;
            }
            buf[len++] = '\\';
            buf[len++] = 'f';
            break;

        case '\n':
            if (!quote_spaces)
            {
                token = SPACE;
                goto done;
            }
            buf[len++] = '\\';
            buf[len++] = 'n';
            break;

        case '\r':
            buf[len++] = '\\';
            buf[len++] = 'r';
            break;

        case '\t':
            if (!quote_spaces)
            {
                token = SPACE;
                goto done;
            }
            buf[len++] = '\\';
            buf[len++] = 't';
            break;

#if __STDC__ >= 1
        case '\v':
            buf[len++] = '\\';
            buf[len++] = 'v';
            break;
#endif

        case '"':
        case '\'':
            /*
             * characters with special meaning to cook,
             * with special treatment for ifeq and ifneq.
             */
            if (lex_quote_quotes)
                buf[len++] = '\\';
            buf[len++] = c;
            break;

        case ';':
        case ':':
        case '=':
        case '\\':
        case '{':
        case '}':
        case '[':
        case ']':
            /* characters with special meaning to cook */
            buf[len++] = '\\';
            buf[len++] = c;
            break;

        case '(':
            if (allow_archive_parens && (lex_depth <= 0))
            {
                token = LP;
                ++lex_depth;
                goto done;
            }
            buf[len++] = c;
            break;

        default:
            if (!isprint(c))
            {
                buf[len++] = '\\';
                if (isdigit((unsigned char)*lexpos) || (c & 0300))
                    buf[len++] = '0' + ((c >> 6) & 3);
                if (isdigit((unsigned char)*lexpos) || (c & 0370))
                    buf[len++] = '0' + ((c >> 3) & 7);
                buf[len++] = '0' + (c & 7);
            }
            else
                buf[len++] = c;
            break;
        }
        vargram_lval.lv_string = str_n_from_c(buf, len);
        trace(("value = \"%.*s\";\n", len, buf));
    }
    else
    {
        /*
         * Characters within a $(...) or ${...} sequence.
         * Subject to lots of interpretation.
         */
        switch (c)
        {
        case 0:
            --lexpos;
            token = 0;
            break;

        case '$':
            token = DOLLAR;
            lex_after_dollars = 1;
            break;

        case ':':
            token = COLON;
            break;

        case '=':
            token = EQU;
            break;

        case ',':
            token = COMMA;
            break;

        case ' ':
        case '\f':
        case '\n':
        case '\t':
#if __STDC__ >= 1
        case '\v':
#endif
            for (;;)
            {
                switch (*lexpos)
                {
                case ' ':
                case '\f':
                case '\n':
                case '\t':
#if __STDC__ >= 1
                case '\v':
#endif
                    ++lexpos;
                    continue;

                default:
                    break;
                }
                break;
            }
            token = SPACE;
            break;

        case '(':
            ++lex_depth;
            token = LP;
            break;

        case ')':
            --lex_depth;
            token = RP;
            break;

        case '{':
            token = LB;
            --lex_depth;
            break;

        case '}':
            token = RB;
            --lex_depth;
            break;

        default:
            goto normal;
        }
    }

  done:
    trace(("return %d;\n", token));
    trace(("}\n"));
    return token;
}


void
vargram_error(char *fmt)
{
    string_ty       *s;
    sub_context_ty  *scp;

    scp = sub_context_new();
    s = subst_intl(scp, fmt);
    /* re-use substitution context */
    sub_var_set_string(scp, "MeSsaGe", s);
    blob_error(source, scp, i18n("variable reference: $message"));
    sub_context_delete(scp);
    str_free(s);
}


void
variable_mangle_result(string_ty *s)
{
    static string_ty *t1;
    static string_ty *t2;
    string_ty       *tmp;

    trace(("variable_mangle_result(\"%s\")\n{\n", s->str_text));
    if (!s->str_length)
    {
        str_free(s);
        s = str_from_c("\"\"");
    }
    if (!t1)
    {
        t1 = str_from_c("/*");
        t2 = str_from_c("/\\*");
    }
    tmp = str_substitute(t1, t2, s);
    str_free(s);
    string_list_append(&result, tmp);
    str_free(tmp);
    trace(("}\n"));
}


void
variable_rename(blob_ty *in, blob_list_ty *out, string_list_ty *ref, int flags)
{
    size_t          j;

    trace(("variable_rename(in = %p, out = %p, ref = %p)\n{\n", in, out, ref));
    trace_string(in->text->str_text);
    init();
    source = in;
    lexpos = in->text->str_text;
    lex_after_dollars = 0;
    lex_depth = 0;
    lex_quote_spaces = (flags & VAREN_QUOTE_SPACES);
    lex_quote_quotes = !(flags & VAREN_NO_QUOQUO);
    allow_archive_parens = (flags & VAREN_KNOW_ARCHIVE);
    reference = ref;
    vargram_parse();
    for (j = 0; j < result.nstrings; ++j)
    {
        blob_list_append
        (
            out,
            blob_alloc
            (
                str_copy(result.string[j]),
                in->file_name,
                in->line_number
            )
        );
    }

    /*
     * Having no words as an answer is probably the result of a
     * syntax error, but also upsets things down the track.  Fake an
     * answer.
     */
    if (result.nstrings == 0)
    {
        blob_list_append
        (
            out,
            blob_alloc(str_from_c("bogus"), in->file_name, in->line_number)
        );
    }

    string_list_destructor(&result);
    if (special)
    {
        symtab_free(special);
        special = 0;
    }
    trace(("}\n"));
}


void
variable_rename_list(blob_list_ty *in, blob_list_ty *out, string_list_ty *ref,
    int quote_spaces)
{
    size_t          j;

    for (j = 0; j < in->length; ++j)
        variable_rename(in->list[j], out, ref, quote_spaces);
}


static void
reap(void *p)
{
    string_ty      *s;

    s = p;
    str_free(s);
}


void
variable_archive(string_ty *target, string_ty *member)
{
    string_ty       *name;
    string_ty       *value;

    assert(!special);
    special = symtab_alloc(6);
    special->reap = reap;

    name = str_from_c("%");
    value = str_copy(member);
    symtab_assign(special, name, value);
    str_free(name);

    name = str_from_c("%D");
    value = str_format("[dirname %s]", member->str_text);
    symtab_assign(special, name, value);
    str_free(name);

    name = str_from_c("%F");
    value = str_format("[entryname %s]", member->str_text);
    symtab_assign(special, name, value);
    str_free(name);

    name = str_from_c("@");
    value = str_copy(target);
    symtab_assign(special, name, value);
    str_free(name);

    name = str_from_c("@D");
    value = str_format("[dirname %s]", target->str_text);
    symtab_assign(special, name, value);
    str_free(name);

    name = str_from_c("@F");
    value = str_format("[entryname %s]", target->str_text);
    symtab_assign(special, name, value);
    str_free(name);
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
vargram_trace(char *s, ...)
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
        trace_printf("%s\n", line);
        line[0] = 0;
    }
}


void
vargram_trace2(void *garbage, char *s, ...)
{
    va_list         ap;
    string_ty       *buffer;
    char            *cp;
    static char     line[1024];

    (void)garbage;
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
        trace_printf("%s\n", line);
        line[0] = 0;
    }
}

#endif /* DEBUG */
