/*
 *      cook - file construction tool
 *      Copyright (C) 1993-2007 Peter Miller
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
 *
 * This file contains the lexical analyser for the cookbook parser.
 * A known bug is that it processes # control lines even within comments.
 * Another bug is that comments vanish: they should be replaced by a single
 * space.
 */

#include <common/ac/ctype.h>
#include <common/ac/stddef.h>
#include <common/ac/string.h>
#include <common/ac/stdlib.h>
#include <common/ac/stdarg.h>

#include <common/error.h>
#include <common/error_intl.h>
#include <common/input/file_text.h>
#include <common/mem.h>
#include <common/star.h>
#include <common/str_list.h>
#include <common/stracc.h>
#include <common/symtab.h>
#include <common/trace.h>
#include <cook/expr.h>
#include <cook/expr/list.h>     /* for parse.gen.h */
#include <cook/hashline.h>
#include <cook/lex.h>
#include <cook/lex/filename.h>
#include <cook/lex/filenamelist.h>
#include <cook/option.h>
#include <cook/stmt.h>
#include <cook/stmt/list.h>     /* for parse.gen.h */
#include <cook/parse.gen.h>     /* must be last */
#include <cook/hashline.gen.h>  /* must be last */


/*
 * Meta types
 */
#define NORMAL  1
#define SLOSHED 2
#define WHITE   3

/*
 */
#define BEFORE  (1<<0)
#define AFTER   (1<<1)
#define SINGLE  (1<<2)

#define HASHLINE_ESCAPE ('#' ^ 0x80)
#define UNQUOTED_WORD 32767
#define EOLN 32766

enum internal_token_ty
{
    internal_token_eof,
    internal_token_eoln,
    internal_token_word,
    internal_token_unquoted_word,
    internal_token_data,
    internal_token_dataend,
    internal_token_catenate,
    internal_token_file_boundary,
};
typedef enum internal_token_ty internal_token_ty;


#ifdef DEBUG

static const char *
internal_token_name(internal_token_ty tok)
{
    static char     buffer[16];

    switch (tok)
    {
    case internal_token_eof:
        return "internal_token_eof";

    case internal_token_eoln:
        return "internal_token_eoln";

    case internal_token_word:
        return "internal_token_word";

    case internal_token_unquoted_word:
        return "internal_token_unquoted_word";

    case internal_token_data:
        return "internal_token_data";

    case internal_token_dataend:
        return "internal_token_dataend";

    case internal_token_catenate:
        return "internal_token_catenate";

    case internal_token_file_boundary:
        return "internal_token_file_boundary";
    }
    sprintf(buffer, "%d", tok);
    return buffer;
}

#endif


typedef struct meta_ty meta_ty;
struct meta_ty
{
    int             m_type;     /* meta character "type" info   */
    int             m_char;     /* meta character actual value  */
    int             m_flag;     /* meta character flags         */
};

typedef struct lex_ty lex_ty;
struct lex_ty
{
    lex_filename_ty filename;   /* name of file being analysed  */
    input_ty        *l_file;    /* open file structure of the file */
    long            l_line;     /* the line number we are up to */
    meta_ty         l_mback;    /* backup for meta chars        */
    lex_ty          *l_chain;   /* file this one is an insert from */
    short           l_bol;
    lex_filename_list_ty pending_include_list;
    int             have_said_eof_token;
};

static lex_ty   *root;          /* root of insert list          */
static stracc   saroot;         /* root of string accum list    */
static int      errcnt;         /* count of errors to date      */
static lex_mode_ty mode;        /* what lex mode we are in      */
static int      state;
static int      passing;
static int      catted;
static symtab_ty *parse_symtab;
static symtab_ty *hash_symtab;
static symtab_ty *hash_directive_symtab;


/*
 * NAME
 *      lex_initialize - look for keywords
 *
 * SYNOPSIS
 *      int lex_initialize(void);
 *
 * DESCRIPTION
 *      The lex_initialize function adds all the keywords to the symbol table.
 *
 * CAVEAT
 *      The keywords are intentionally case sensitive.
 *      Assumes that str_initialize has already been called.
 */

void
lex_initialize(void)
{
    typedef struct keyword_ty keyword_ty;
    struct keyword_ty
    {
        char            *k_name;
        int             k_token;
    };

    static keyword_ty parse_keyword[] =
    {
        { "+=", PLUS_EQUALS },
        { ":", COLON },
        { "::", COLON2 },
        { ";", SEMICOLON },
        { "=", EQUALS },
        { "[", LBRAK },
        { "]", RBRAK },
        { "data", DATA },
        { "else", ELSE },
        { "fail", FAIL },
        { "function", FUNCTION },
        { "host-binding", HOST_BINDING },
        { "if", IF },
        { "loop", LOOP },
        { "loopstop", LOOPSTOP },
        { "return", RETURN },
        { "set", SET },
        { "single-thread", SINGLE_THREAD },
        { "then", THEN },
        { "unsetenv", UNSETENV },
        { "{", LBRACE },
        { "}", RBRACE },
    };
    static keyword_ty hash_directive[] =
    {
        { "elif", HASH_ELIF },
        { "else", HASH_ELSE },
        { "endif", HASH_ENDIF },
        { "if", HASH_IF },
        { "ifdef", HASH_IFDEF },
        { "ifndef", HASH_IFNDEF },
        { "include", HASH_INCLUDE },
        { "include-cooked", HASH_INCLUDE_COOKED },
        { "include-cooked-nowarn", HASH_INCLUDE_COOKED2 },
        { "line", HASH_LINE },
        { "pragma", HASH_PRAGMA },
    };
    static keyword_ty hash_keyword[] =
    {
        { "[", HASHLINE_LBRAK },
        { "]", HASHLINE_RBRAK },
    };
    keyword_ty      *kp;

    trace(("init_reserved()\n{\n"));
    passing = 1;

    if (!parse_symtab)
        parse_symtab = symtab_alloc(SIZEOF(parse_keyword));
    for (kp = parse_keyword; kp < ENDOF(parse_keyword); ++kp)
    {
        string_ty       *s;

        s = str_from_c(kp->k_name);
        symtab_assign(parse_symtab, s, &kp->k_token);
        str_free(s);
    }

    if (!hash_symtab)
        hash_symtab = symtab_alloc(SIZEOF(hash_keyword));
    for (kp = hash_keyword; kp < ENDOF(hash_keyword); ++kp)
    {
        string_ty       *s;

        s = str_from_c(kp->k_name);
        symtab_assign(hash_symtab, s, &kp->k_token);
        str_free(s);
    }

    if (!hash_directive_symtab)
        hash_directive_symtab = symtab_alloc(SIZEOF(hash_directive));
    for (kp = hash_directive; kp < ENDOF(hash_directive); ++kp)
    {
        string_ty       *s;

        s = str_from_c(kp->k_name);
        symtab_assign(hash_directive_symtab, s, &kp->k_token);
        str_free(s);
    }
    trace(("}\n"));
}


/*
 * NAME
 *      lex_open - open a file for lexical analysis
 *
 * SYNOPSIS
 *      void lex_open(string_ty *filename);
 *
 * DESCRIPTION
 *      Lex_open opens a file for lexical analysis
 *      (it is also used to open include files).
 *
 * RETURNS
 *      void
 */

void
lex_open(string_ty *logical, string_ty *physical)
{
    lex_ty          *new;

    trace(("lex_open(filename = %08lX)\n{\n", logical));
    trace_string(logical->str_text);
    if (!physical)
        physical = logical;
    if (!root)
        hashline_reset();
    for (new = root; new; new = new->l_chain)
    {
        if (str_equal(physical, new->filename.physical))
        {
            sub_context_ty  *scp;

            scp = sub_context_new();
            sub_var_set_string(scp, "File_Name", physical);
            lex_error(scp, i18n("$filename: recursive include"));
            sub_context_delete(scp);
            return;
        }
    }
    new = mem_alloc_clear(sizeof(lex_ty));
    lex_filename_constructor(&new->filename, logical, physical);
    new->l_file = input_file_text_open(physical);
    new->l_line = 1;
    new->l_bol = 1;
    new->l_chain = root;
    lex_filename_list_constructor(&new->pending_include_list);
    new->have_said_eof_token = 0;
    root = new;
    trace(("}\n"));
}


void
lex_open_include(string_ty *logical, string_ty *physical)
{
    assert(root);
    lex_filename_list_push_back
    (
        &root->pending_include_list,
        lex_filename_new(logical, physical)
    );
}


/*
 * NAME
 *      lex_close - closes a file which was open for lexical analysis
 *
 * SYNOPSIS
 *      void lex_close(void);
 *
 * DESCRIPTION
 *      Lex_close closes a file previously opened for lexical analysis.
 *
 * CAVEAT
 *      Lex_open must have been previously used to open the file.
 */

void
lex_close(void)
{
    lex_ty          *old;

    trace(("lex_close()\n{\n"));
    assert(root);
    old = root;
    input_delete(root->l_file);
    root = old->l_chain;
    if (!root && errcnt)
    {
        sub_context_ty *scp;

        scp = sub_context_new();
        sub_var_set_string(scp, "File_Name", old->filename.logical);
        sub_var_set_long(scp, "Number", errcnt);
        sub_var_optional(scp, "Number");
        fatal_intl(scp, i18n("$filename: found $number fatal errors"));
        /* NOTREACHED */
    }
    lex_filename_destructor(&old->filename);
    lex_filename_list_destructor(&old->pending_include_list);
    free(old);
    trace(("}\n"));
}


/*
 * NAME
 *      illeof - conplain about an illegal end-of-file
 *
 * SYNOPSIS
 *      void illeof(void);
 *
 * DESCRIPTION
 *      Illeof is used to complain about illegal end-of-file.
 *      This may occu in several places.
 *
 * CAVEAT
 *      It does not return.
 */

static void
illeof(void)
{
    lex_error(0, i18n("unexpected end of file"));
    exit(1);
}


/*
 * NAME
 *      byte - get a byte from the input stream
 *
 * SYNOPSIS
 *      int byte(void);
 *
 * DESCRIPTION
 *      Byte return the next byte from the input stream, or EOF
 *      If the end of the outermost file is reached.
 *      It detechs the end of include files and closes them transparently.
 *
 * CAVEAT
 *      Lex_open must have been called previously.
 */

static int
byte(void)
{
    int             c;

    trace(("byte()\n{\n"));
    for (;;)
    {
        /*
         * leap into include file if possible
         */
        assert(root);
        while (root->pending_include_list.length)
        {
            lex_filename_ty *fnp;

            fnp = lex_filename_list_pop_front(&root->pending_include_list);
            lex_open(fnp->logical, fnp->physical);
            lex_filename_delete(fnp);
            star_as_specified('+');
        }

        /*
         * fetch the next input character
         */
        c = input_getc(root->l_file);
        if (c != EOF)
            break;
        if (!root->l_chain || !root->have_said_eof_token)
            break;
        lex_close();
    }
    if (mode != LM_COMMENT && !isprint(c) && c != EOF && !strchr("\t\n\f", c))
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        sub_var_set(scp, "Name", "\\%o", (unsigned char)c);
        lex_error(scp, i18n("illegal '$name' character"));
        sub_context_delete(scp);
        c = ' ';
    }
    if (mode != LM_DATA && root->l_bol && c == '#')
        c = HASHLINE_ESCAPE;
    if (c == '\n')
    {
        root->l_bol = 1;
        root->l_line++;
    }
    else
    {
        if (!strchr(" \t\f", c))
            root->l_bol = 0;
    }
    trace_short(root->l_bol);
    trace(("return '%c';\n", c));
    trace(("}\n"));
    return c;
}


/*
 * NAME
 *      byte_undo - push back a character
 *
 * SYNOPSIS
 *      void byte_undo(int c);
 *
 * Description
 *      Byte_undo pushes a byte back onto the input stream.
 *
 * CAVEAT
 *      Only one byte may be pushed back at any one time.
 */

static void
byte_undo(int c)
{
    trace(("byte_undo(c = '%c')\n{\n", c));
    assert(root);
    switch (c)
    {
    case EOF:
        break;

    case '\n':
        root->l_line--;
        /* fall through... */

    default:
        input_ungetc(root->l_file, c);
        break;
    }
    trace(("}\n"));
}


/*
 * NAME
 *      meta_repn - mata_ty representation
 *
 * SYNOPSIS
 *      char *meta_repn(meta_ty*);
 *
 * DESCRIPTION
 *      The meta_repn function is used to produce a readable representation of
 *      a mata_ty value.
 *
 * RETURNS
 *      char* - a pointer to ta C string.
 *
 * CAVEAT
 *      This function is only available when the DEBUG symbol is #define'd.
 */

#ifdef DEBUG

static char *
meta_repn(meta_ty *val)
{
    static char *type[] =
    {
        "0",
        "NORMAL",
        "SLOSHED",
        "WHITE",
    };
    static char *flag[] =
    {
        "0",
        "BEFORE",
        "AFTER",
        "AFTER | BEFORE",
        "SINGLE",
        "SINGLE | BEFORE",
        "SINGLE | AFTER",
        "SINGLE | AFTER | BEFORE",
    };
    static char     buffer[100];
    char            buf2[20];
    char            buf3[10];

    if (val->m_type >= 0 && val->m_type < SIZEOF(type))
        strendcpy(buf2, type[val->m_type], buf2 + sizeof(buf2));
    else
        snprintf(buf2, sizeof(buf2), "%d", val->m_type);
    switch (val->m_char)
    {
    default:
        if (isprint(val->m_char))
        {
            buf3[0] = val->m_char;
            buf3[1] = 0;
        }
        else
            sprintf(buf3, "\\%03o", (unsigned char)val->m_char);
        break;

    case '\'':
    case '\\':
        buf3[0] = '\\';
        buf3[1] = val->m_char;
        buf3[2] = 0;
        break;
    }
    snprintf
    (
        buffer,
        sizeof(buffer),
        "{m_char = '%s', m_type = %s, m_flag = %s}",
        buf3,
        buf2,
        flag[val->m_flag]
    );
    return buffer;
}

#endif


/*
 * NAME
 *      meta - fetch and classify a character
 *
 * SYNOPSIS
 *      void meta(meta_ty*);
 *
 * DESCRIPTION
 *      Meta fetches and classifies a character from the input stream.
 *      The classification may be 'mode' dependent.
 *
 *      This is the point at which escaped newlines and comments are replaced
 *      by a single space.
 *
 * CAVEAT
 *      Lex_open must have been called previously.
 */

static void
meta(meta_ty *val)
{
    trace(("meta(val = %08lX)\n{\n", val));
    assert(root);
    if (root->l_mback.m_type)
    {
        *val = root->l_mback;
        root->l_mback.m_type = 0;
    }
    else
    {
        switch (val->m_char = byte())
        {
        default:
            val->m_type = NORMAL;
            val->m_flag = BEFORE | AFTER;
            break;

        case EOF:
            val->m_type = EOF;
            val->m_flag = SINGLE;
            break;

        case '+':
            /*
             * processing for += token
             */
            val->m_type = NORMAL;
            val->m_flag = BEFORE | AFTER;
            switch (mode)
            {
                int             c;

            case LM_NORMAL:
                c = byte();
                byte_undo(c);
                if (c == '=')
                {
                    val->m_type = NORMAL;
                    val->m_flag = SINGLE;
                }
                break;

            case LM_DATA:
            case LM_SQUOTE:
            case LM_DQUOTE:
            case LM_COMMENT:
                /*
                 * I haven't used "default:" so that
                 * GCC will barf if a new mode is added,
                 * but not handled by this switch.
                 */
                break;
            }
            break;

        case '=':
        case ';':
        case ':':
        case '{':
        case '}':
            val->m_type = NORMAL;
            val->m_flag = BEFORE | AFTER;
            switch (mode)
            {
            case LM_NORMAL:
                val->m_flag = SINGLE;
                break;

            case LM_DATA:
            case LM_SQUOTE:
            case LM_DQUOTE:
            case LM_COMMENT:
                /*
                 * I haven't used "default:" so that
                 * GCC will barf if a new mode is added,
                 * but not handled by this switch.
                 */
                break;
            }
            break;

        case '[':
            val->m_type = NORMAL;
            val->m_flag = BEFORE | AFTER;
            switch (mode)
            {
            case LM_NORMAL:
            case LM_DATA:
                val->m_flag = BEFORE | SINGLE;
                break;

            case LM_SQUOTE:
            case LM_DQUOTE:
            case LM_COMMENT:
                /*
                 * I haven't used "default:" so that
                 * GCC will barf if a new mode is added,
                 * but not handled by this switch.
                 */
                break;
            }
            break;

        case ']':
            val->m_type = NORMAL;
            val->m_flag = BEFORE | AFTER;
            switch (mode)
            {
            case LM_NORMAL:
                val->m_flag = AFTER | SINGLE;
                break;

            case LM_DATA:
            case LM_SQUOTE:
            case LM_DQUOTE:
            case LM_COMMENT:
                /*
                 * I haven't used "default:" so that
                 * GCC will barf if a new mode is added,
                 * but not handled by this switch.
                 */
                break;
            }
            break;

        case ' ':
        case '\t':
        case '\n':
        case '\f':
            val->m_type = NORMAL;
            val->m_flag = BEFORE | AFTER;
            switch (mode)
            {
            case LM_NORMAL:
                val->m_type = WHITE;
                val->m_flag = SINGLE;
                break;

            case LM_SQUOTE:
            case LM_DQUOTE:
            case LM_DATA:
            case LM_COMMENT:
                /*
                 * I haven't used "default:" so that
                 * GCC will barf if a new mode is added,
                 * but not handled by this switch.
                 */
                break;
            }
            break;

        case '\\':
            switch (val->m_char = byte())
            {
            case EOF:
                illeof();

            case '\n':
                val->m_char = ' ';
                val->m_type = WHITE;
                val->m_flag = SINGLE;
                break;

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            {
                int             count;
                int             c;

                c = val->m_char;
                val->m_char = 0;
                for (count = 0; count < 3; count++)
                {
                    val->m_char = val->m_char * 8 + c - '0';
                    c = byte();
                    if (c < '0' || c > '7')
                    {
                        byte_undo(c);
                        break;
                    }
                }
                val->m_type = SLOSHED;
                val->m_flag = BEFORE | AFTER;
            }
                break;

            default:
            {
                char            *cp;

                cp = strchr("b\bf\fn\nr\rt\t", val->m_char);
                if (cp)
                    val->m_char = cp[1];
                val->m_type = SLOSHED;
                val->m_flag = BEFORE | AFTER;
            }
                break;
            }
            break;

        case '/':
            val->m_type = NORMAL;
            val->m_flag = BEFORE | AFTER;
            switch (mode)
            {
            case LM_NORMAL:
            {
                int             c;
                int             gate;
                int             count;

                c = byte();
                if (c != '*')
                {
                    byte_undo(c);
                    val->m_type = NORMAL;
                    val->m_flag = BEFORE | AFTER;
                    break;
                }
                gate = 0;
                count = 1;
                mode = LM_COMMENT;
                while (count)
                {
                    switch (c = byte())
                    {
                    default:
                    case '\n':
                    case '\t':
                    case '\f':
                        gate = 0;
                        break;

                    case EOF:
                        illeof();

                    case '*':
                        if (gate == 1)
                        {
                            /*
                             * nested comment start
                             *
                             * should a warning be issued?
                             */
                            count++;
                            gate = 0;
                        }
                        else
                            gate = 2;
                        break;

                    case '/':
                        if (gate == 2)
                        {
                            count--;
                            gate = 0;
                        }
                        else
                            gate = 1;
                        break;
                    }
                }
                mode = LM_NORMAL;
                val->m_char = ' ';
                val->m_type = WHITE;
                val->m_flag = SINGLE;
            }
                break;

            case LM_DATA:
            case LM_SQUOTE:
            case LM_DQUOTE:
            case LM_COMMENT:
                /*
                 * I haven't used "default:" so that
                 * GCC will barf if a new mode is added,
                 * but not handled by this switch.
                 */
                break;
            }
            break;
        }
    }
    trace(("*val = %s;\n", meta_repn(val)));
    trace(("}\n"));
}


/*
 * NAME
 *      meta_undo - push back a character
 *
 * SYNOPSIS
 *      void meta_undo(meta_ty*);
 *
 * DESCRIPTION
 *      Meta_undo is used to temporarily push back a character
 *      from the input stream.
 *
 * CAVEAT
 *      Only one character may be pushed back at any one time.
 */

static void
meta_undo(meta_ty *val)
{
    assert(root);
    assert(!root->l_mback.m_type);
    root->l_mback = *val;
}


/*
 * NAME
 *      lex_error - print a file location specific error message
 *
 * SYNOPSIS
 *      void lex_error(char *s);
 *
 * DESCRIPTION
 *      Lex_error prints a file location specific error message.
 *      If too many errors pass through here, cook will give up.
 */

void
lex_error(sub_context_ty *scp, char *s)
{
    string_ty       *buffer;
    int             need_to_delete;

    if (scp)
        need_to_delete = 0;
    else
    {
        scp = sub_context_new();
        need_to_delete = 1;
    }

    assert(root);
    buffer = subst_intl(scp, s);

    /* re-use the substitution context */
    sub_var_set_string(scp, "File_Name", root->filename.logical);
    sub_var_set_long(scp, "Number", root->l_line);
    sub_var_set_string(scp, "MeSsaGe", buffer);
    error_intl(scp, i18n("$filename: $number: $message"));
    str_free(buffer);
    if (++errcnt > 20)
    {
        /* re-use the substitution context */
        sub_var_set_string(scp, "File_Name", root->filename.logical);
        fatal_intl(scp, i18n("$filename: too many fatal errors"));
    }
    option_set_errors();

    if (need_to_delete)
        sub_context_delete(scp);
}


void
parse_error(char *s)
{
    lex_error(0, s);
}


/*
 * NAME
 *      lex_warning - print a file location specific warning message
 *
 * SYNOPSIS
 *      void lex_warning(char *s);
 *
 * DESCRIPTION
 *      Lex_warning prints a file location specific warning message.
 */

void
lex_warning(sub_context_ty *scp, char *s)
{
    string_ty       *buffer;
    int             need_to_delete;

    if (scp)
        need_to_delete = 0;
    else
    {
        scp = sub_context_new();
        need_to_delete = 1;
    }

    assert(root);
    buffer = subst_intl(scp, s);

    /* re-use the substitution context */
    sub_var_set_string(scp, "File_Name", root->filename.logical);
    sub_var_set_long(scp, "Number", root->l_line);
    sub_var_set_string(scp, "MeSsaGe", buffer);
    error_intl(scp, i18n("$filname: $number: warning: $message"));
    str_free(buffer);

    if (need_to_delete)
        sub_context_delete(scp);
}


/*
 * NAME
 *      lex_mode - change the mode of lexical analysis
 *
 * SYNOPSIS
 *      int lex_mode(int n);
 *
 * DESCRIPTION
 *      Lex_mode changes the mode of lexical analysis to the one given,
 *      and returns the previous mode.
 *
 * CAVEAT
 *      Use the #define's in lex.h.
 */

lex_mode_ty
lex_mode(lex_mode_ty n)
{
    lex_mode_ty     old;

    old = mode;
    mode = n;
    return (old);
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
lex_trace(char *s, ...)
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
            "%s: %d: %s\n",
            root->filename.logical->str_text,
            root->l_line,
            line
        );
        line[0] = 0;
    }
}


static char *
mode_repn(lex_mode_ty n)
{
    static char *name[] =
    {
        "NORMAL",
        "DATA",
        "SQUOTE",
        "DQUOTE",
        "COMMENT",
    };
    static char     buffer[12];

    if (n >= 0 && n < SIZEOF(name))
        return name[n];
    sprintf(buffer, "%d", n);
    return buffer;
}

#endif


/*
 * NAME
 *      tokenize - lexical analyser
 *
 * SYNOPSIS
 *      int tokenize(void);
 *
 * DESCRIPTION
 *      The tokenize functionm is used to partition the input stream into
 *      discreet tokens.
 *
 * CAVEAT
 *      Lex_open must have been called previously.
 */

static internal_token_ty
tokenize(void)
{
    meta_ty         c;
    internal_token_ty tok;
    static meta_ty  last;
    static int      dataended;

    trace(("tokenize()\n{\n"));
    if (dataended)
    {
        dataended = 0;
        tok = internal_token_dataend;
        goto ret;
    }

    if (!catted && (last.m_flag & AFTER))
    {
        /*
         * For parts of words in expressions which are abutted
         * together, the user is implying catenation of the strings.
         * This implied zero length token happens here.
         */
        meta(&c);
        meta_undo(&c);
        if (c.m_flag & BEFORE)
        {
            catted = 1;
            tok = internal_token_catenate;
            goto ret;
        }
    }
    else
        catted = 0;

    for (;;)
    {
        int             mark;

        mark = 0;
        meta(&c);
        if (c.m_char == HASHLINE_ESCAPE)
        {
            state = 0;
            last.m_flag = 0;
            hashline();
            state = 0;
            last.m_flag = 0;
            continue;
        }
        switch (c.m_type)
        {
        default:
            fatal_raw
            (
                "%s: %d: illegal m_type %d (bug)",
                __FILE__,
                __LINE__,
                c.m_type
            );
            /* NOTREACHED */

        case EOF:
            if (!root->have_said_eof_token)
            {
                tok = internal_token_file_boundary;
                root->have_said_eof_token = 1;
            }
            else
                tok = internal_token_eof;
            goto ret;

        case NORMAL:
            if (c.m_char == ':')
            {
                meta_ty         hold;

                hold = c;
                meta(&c);
                if (c.m_char == ':' && c.m_type == NORMAL)
                {
                    sa_open(&saroot);
                    sa_char(&saroot, ':');
                    sa_char(&saroot, ':');
                    parse_lval.lv_word = sa_close(&saroot);
                    state = 1;
                    tok = internal_token_unquoted_word;
                    last = c;
                    goto ret;
                }
                meta_undo(&c);
                c = hold;
            }
            if (c.m_char == '+')
            {
                meta_ty         hold;

                hold = c;
                meta(&c);
                if (c.m_char == '=' && c.m_type == NORMAL)
                {
                    sa_open(&saroot);
                    sa_char(&saroot, '+');
                    sa_char(&saroot, '=');
                    parse_lval.lv_word = sa_close(&saroot);
                    state = 1;
                    tok = internal_token_unquoted_word;
                    last = c;
                    goto ret;
                }
                meta_undo(&c);
                c = hold;
            }
            /* fall through... */

        case SLOSHED:
        {
            int             sloshed;

            sloshed = 0;
            sa_open(&saroot);
            if (state == 0)
                mark = sa_mark(&saroot);
            for (;;)
            {
                trace
                ((
                    "%s: %d: state %d, mode %s;\n",
                    root->filename.logical->str_text,
                    root->l_line,
                    state,
                    mode_repn(mode)
                ));
                sa_char(&saroot, c.m_char);
                if (c.m_type == SLOSHED)
                {
                    sloshed = 1;
                    switch (state)
                    {
                    default:
                        state = 1;
                        break;

                    case 11:
                    case 12:
                        /* don't change state */
                        break;
                    }
                }
                else
                {
                    switch (c.m_char)
                    {
                    default:
                        switch (state)
                        {
                        default:
                            state = 1;
                            break;

                        case 11:
                        case 12:
                            break;
                        }
                        break;

                    case '\n':
                        switch (state)
                        {
                        case 9:
                        case 10:
                            sa_goto(&saroot, mark);
                            parse_lval.lv_word = sa_close(&saroot);
                            state = 0;
                            /* don't CAT after this */
                            last.m_flag = 0;
                            dataended = internal_token_dataend;
                            tok = internal_token_word;
                            goto ret;

                        case 11:
                        case 12:
                            /* don't change state */
                            lex_error(0, i18n("newline in quote"));
                            break;

                        default:
                            state = 0;
                            mark = sa_mark(&saroot);
                            break;
                        }
                        break;

                    case '\'':
                        if (state != 12)
                        {
                            static int      oldmode = LM_SQUOTE;

                            oldmode = lex_mode(oldmode);
                            sa_goto(&saroot, sa_mark(&saroot) - 1);
                            state = ((state == 11) ? 1 : 11);
                            sloshed = 1;
                        }
                        break;

                    case '"':
                        if (state != 11)
                        {
                            static int      oldmode = LM_DQUOTE;

                            oldmode = lex_mode(oldmode);
                            sa_goto(&saroot, sa_mark(&saroot) - 1);
                            state = ((state == 12) ? 1 : 12);
                            sloshed = 1;
                        }
                        break;

                    case ' ':
                    case '\t':
                        switch (state)
                        {
                        default:
                            state = 1;
                            break;

                        case 0:
                        case 2:
                            state = 2;
                            break;

                        case 9:
                        case 10:
                            state = 10;
                            break;

                        case 11:
                        case 12:
                            break;
                        }
                        break;

                    case 'a':
                        switch (state)
                        {
                        default:
                            state = 1;
                            break;

                        case 3:
                        case 5:
                            state++;
                            break;

                        case 11:
                        case 12:
                            break;
                        }
                        break;

                    case 'd':
                        switch (state)
                        {
                        default:
                            state = 1;
                            break;

                        case 0:
                        case 2:
                            state = 3;
                            break;

                        case 8:
                            state = 9;
                            break;

                        case 11:
                        case 12:
                            break;
                        }
                        break;

                    case 'e':
                        switch (state)
                        {
                        default:
                            state = 1;
                            break;

                        case 6:
                            state = 7;
                            break;

                        case 11:
                        case 12:
                            break;
                        }
                        break;

                    case 'n':
                        switch (state)
                        {
                        default:
                            state = 1;
                            break;

                        case 7:
                            state = 8;
                            break;

                        case 11:
                        case 12:
                            break;
                        }
                        break;

                    case 't':
                        switch (state)
                        {
                        default:
                            state = 1;
                            break;

                        case 4:
                            state = 5;
                            break;

                        case 11:
                        case 12:
                            break;
                        }
                        break;
                    }
                }
                last = c;
                if (last.m_flag & SINGLE)
                    break;
                meta(&c);
                if (c.m_flag & SINGLE)
                {
                    meta_undo(&c);
                    break;
                }
            }
            parse_lval.lv_word = sa_close(&saroot);
            if (sloshed)
                tok = internal_token_word;
            else
                tok = internal_token_unquoted_word;
        }
            goto ret;

        case WHITE:
            if (c.m_char == '\n')
            {
                last = c;
                state = 0;
                tok = internal_token_eoln;
                goto ret;
            }
            state = 1;
            break;
        }
    }

    ret:
    trace
    ((
        "%S: %d: state %d, mode %s;\n",
        root->filename.logical,
        root->l_line,
        state,
        mode_repn(mode)
    ));
    trace(("return %s;\n", internal_token_name(tok)));
    trace(("}\n"));
    return tok;
}


/*
 * NAME
 *      lex_cur_file - name current file
 *
 * SYNOPSIS
 *      string_ty *lex_cur_file(void);
 *
 * DESCRIPTION
 *      The lex_cur_file function is used to get the name of the current
 *      file being analyzed by lex.
 *
 * RETURNS
 *      string_ty* - pointer to string with name in it
 *
 * CAVEAT
 *      Does not take a copy, so don't use str_free()
 */

string_ty *
lex_cur_file(void)
{
    assert(root);
    return root->filename.logical;
}


string_ty *
lex_cur_physical_file(void)
{
    assert(root);
    return root->filename.physical;
}


int
lex_cur_line(void)
{
    assert(root);
    return root->l_line;
}


/*
 * NAME
 *      parse_lex - lexer for parse.y
 *
 * SYNOPSIS
 *      int parse_lex(void);
 *
 * DESCRIPTION
 *      The parse_lex function is used to turn the tokens generated
 *      by tokenize into parse.y tokens.
 *
 * RETURNS
 *      int - the token number
 *
 * CAVEAT
 *      Intended solely for use by the yacc-generated compiler
 */

int
parse_lex(void)
{
    internal_token_ty tok;
    int             *data;
    int             result;

    trace(("parse_lex()\n{\n"));
    result = JUNK;
    for (;;)
    {
        tok = tokenize();
        if (!passing)
        {
            if (tok == internal_token_eof)
            {
                lex_error(0, i18n("unterminated conditional"));
                break;
            }
            if
            (
                tok == internal_token_word
            ||
                tok == internal_token_unquoted_word
            )
            {
                str_free(parse_lval.lv_word);
            }
            continue;
        }
        switch (tok)
        {
        case internal_token_eof:
            result = 0;
            break;

        case internal_token_eoln:
            continue;

        case internal_token_unquoted_word:
            assert(parse_symtab);
            data = symtab_query(parse_symtab, parse_lval.lv_word);
            if (data)
            {
                result = *data;
                str_free(parse_lval.lv_word);
                parse_lval.lv_word = 0;
                if (result == COLON || result == COLON2)
                {
                    parse_lval.lv_position.pos_name =
                        str_copy(root->filename.logical);
                    parse_lval.lv_position.pos_line = root->l_line;
                    parse_lval.lv_position.multi = (result == COLON2);
                    result = COLON;
                }
                else if (result == SEMICOLON)
                {
                    parse_lval.lv_position.pos_name =
                        str_copy(root->filename.logical);
                    parse_lval.lv_position.pos_line = root->l_line;
                    parse_lval.lv_position.multi = 0;
                }
                else if (result == DATA)
                {
                    int             whinged;
                    meta_ty         last;
                    meta_ty         c;

                    /*
                     * This is a special.  Gobble the rest
                     * of the line before he starts sucking
                     * in the program data.
                     */
                    whinged = 0;
                    for (;;)
                    {
                        meta(&c);
                        last = c;
                        switch (c.m_type)
                        {
                        default:
                            fatal_raw
                            (
                                "%s: %d: illegal m_type %d (bug)",
                                __FILE__,
                                __LINE__,
                                c.m_type
                            );

                        case EOF:
                            illeof();

                        case NORMAL:
                        case SLOSHED:
                            if (!whinged)
                            {
                                lex_error
                                (
                                    0,
                         i18n("the 'data' keyword must be the last on the line")
                                );
                                whinged = 1;
                            }
                            continue;

                        case WHITE:
                            if (c.m_char != '\n')
                                continue;
                            state = 0;
                            break;
                        }
                        break;
                    }
                    catted = 1;
                }
            }
            else
                result = WORD;
            break;

        case internal_token_word:
            result = WORD;
            break;

        case internal_token_data:
            result = DATA;
            break;

        case internal_token_dataend:
            result = DATAEND;
            break;

        case internal_token_catenate:
            result = CATENATE;
            break;

        case internal_token_file_boundary:
            result = FILE_BOUNDARY;
            break;
        }
        break;
    }
    trace(("return %d;\n", tok));
    trace(("}\n"));
    return result;
}


/*
 * NAME
 *      hashline_lex - lexer for hashline.y
 *
 * SYNOPSIS
 *      int hashline_lex(void);
 *
 * DESCRIPTION
 *      The hashline_lex function is used to turn the tokens generated
 *      by tokenize into hashline.y tokens.
 *
 * RETURNS
 *      int - the token number
 *
 * CAVEAT
 *      Intended solely for use by the yacc-generated compiler
 */

static int      hashline_lex_nwords;

int
hashline_lex(void)
{
    internal_token_ty tok;
    int             *data;
    string_ty       *guess;
    sub_context_ty  *scp;
    int             result;

    trace(("hashline_lex()\n{\n"));
    tok = tokenize();
    result = HASH_JUNK;
    hashline_lval.lv_word = parse_lval.lv_word;
    parse_lval.lv_word = 0;
    switch (tok)
    {
    case internal_token_eoln:
    case internal_token_eof:
        result = 0;
        break;

    case internal_token_unquoted_word:
        assert(hash_symtab);
        if (hashline_lex_nwords)
        {
            data = symtab_query(hash_symtab, hashline_lval.lv_word);
            if (data)
            {
                result = *data;
                str_free(hashline_lval.lv_word);
                hashline_lval.lv_word = 0;
            }
            else
                result = HASHLINE_WORD;
            break;
        }
        data = symtab_query(hash_directive_symtab, hashline_lval.lv_word);
        if (data)
        {
            result = *data;
            str_free(hashline_lval.lv_word);
            hashline_lval.lv_word = 0;
            break;
        }
        guess = 0;
        data =
            symtab_query_fuzzy
            (
                hash_directive_symtab,
                hashline_lval.lv_word,
                &guess
            );
        if (data)
        {
            scp = sub_context_new();
            assert(guess);
            sub_var_set_string(scp, "Name", hashline_lval.lv_word);
            sub_var_set_string(scp, "Guess", guess);
            lex_error(scp, i18n("unknown #$name directive, guess #$guess"));
            sub_context_delete(scp);
            result = *data;
            /* DO NOT str_free guess */
            str_free(hashline_lval.lv_word);
            hashline_lval.lv_word = 0;
            break;
        }
        scp = sub_context_new();
        sub_var_set_string(scp, "Name", hashline_lval.lv_word);
        lex_error(scp, i18n("unknown #$name directive"));
        sub_context_delete(scp);
        str_free(hashline_lval.lv_word);
        hashline_lval.lv_word = 0;
        result = HASH_JUNK;
        break;

    case internal_token_word:
        result = HASHLINE_WORD;
        break;

    case internal_token_data:
    case internal_token_dataend:
    case internal_token_file_boundary:
        result = HASH_JUNK;
        break;

    case internal_token_catenate:
        result = HASHLINE_CATENATE;
        break;
    }
    ++hashline_lex_nwords;
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}


void
hashline_lex_reset(void)
{
    hashline_lex_nwords = 0;
}


void
lex_passing(int n)
{
    passing = n;
}


void
lex_lino_set(string_ty *line, string_ty *file)
{
    long            n;

    assert(root);
    if (!root)
        return;

    n = atol(line->str_text);
    if (n <= 0)
    {
        lex_error(0, i18n("#line needs positive decimal line number"));
        return;
    }
    root->l_line = n;

    assert(root->filename.logical);
    if (file)
    {
        str_free(root->filename.logical);
        root->filename.logical = str_copy(file);
    }
}
