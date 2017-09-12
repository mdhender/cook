/*
 *      cook - file construction tool
 *      Copyright (C) 1994, 1995, 1997-2002, 2006, 2007 Peter Miller;
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
#include <common/ac/stddef.h>
#include <common/ac/string.h>
#include <common/ac/errno.h>
#include <common/ac/signal.h>

#include <common/error_intl.h>
#include <common/input/file_text.h>
#include <common/input/stdin.h>
#include <common/mem.h>
#include <common/os_path_cat.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <c_incl/cache.h>
#include <c_incl/flatten.h>
#include <c_incl/os_interface.h>
#include <c_incl/sniff.h>
#include <c_incl/stripdot.h>

typedef struct sub_ty sub_ty;
struct sub_ty
{
    string_ty       *from;
    string_ty       *to;
};

        option_ty       option;
static  string_list_ty  prefix;
static  char            *suffix;
static  long            pcount;
static  string_list_ty  srl1;
static  string_list_ty  srl2;
static  string_list_ty  use_these;
static  string_list_ty  visited;
static  string_list_ty  remove_path;
static  string_list_ty  exclude;
static  sniff_ty        *lang;
static  int             has_been_cut;
static  int             interrupted;
static  size_t          nsubs;
static  size_t          nsubs_max;
static  sub_ty          *sub;


static RETSIGTYPE
interrupt(int n)
{
    ++interrupted;
    signal(n, interrupt);
}


void
sniff_language(sniff_ty *lp)
{
    trace(("sniff_language(lp = %08lX)\n{\n", lp));
    assert(lp);
    lang = lp;
    trace(("}\n"));
}


/*
 * NAME
 *      sniff_include
 *
 * SYNOPSIS
 *      void sniff_include(string_ty *path);
 *
 * DESCRIPTION
 *      The sniff_include function is used to add to
 *      the standard include paths.
 *
 * ARGUMENTS
 *      path    - path to add
 */

void
sniff_include(char *path)
{
    string_ty       *s;

    assert(path);
    trace(("sniff_include(path = \"%s\")\n{\n", path));
    s = str_from_c(path);
    string_list_append_unique(&srl1, s);
    string_list_append_unique(&srl2, s);
    str_free(s);
    trace(("}\n"));
}


void
sniff_include_cut(void)
{
    trace(("sniff_include_cut()\n{\n"));
    string_list_destructor(&srl2);
    trace(("}\n"));
}


long
sniff_include_count(void)
{
    trace(("sniff_include_count()\n{\n"));
    trace(("return %ld;\n", (long)srl1.nstrings));
    trace(("}\n"));
    return srl1.nstrings;
}

/*
 * NAME
 *      sniff_use_this
 *
 * SYNOPSIS
 *      void sniff_use_this(string_ty *path);
 *
 * DESCRIPTION
 *      The sniff_use_this function is used to add file names
 *      to a list of additional pathnames to use if a file can't
 *      be found (probably because it needs to be generated)
 *
 * ARGUMENTS
 *      path    - path to add
 */

void
sniff_use_this(char *path)
{
    string_ty       *s;

    assert(path);
    trace(("sniff_use_this(path = \"%s\")\n{\n", path));
    s = str_from_c(path);
    string_list_append_unique(&use_these, s);
    str_free(s);
    trace(("}\n"));
}


void
sniff_use_this_cut(void)
{
    trace(("sniff_use_this_cut()\n{\n"));
    string_list_destructor(&use_these);
    trace(("}\n"));
}


long
sniff_use_this_count(void)
{
    trace(("sniff_use_this_count()\n{\n"));
    trace(("return %ld;\n", (long)use_these.nstrings));
    trace(("}\n"));
    return use_these.nstrings;
}


/*
 * NAME
 *      sniff_prepare
 *
 * SYNOPSIS
 *      void sniff_prepare(void);
 *
 * DESCRIPTION
 *      The sniff_prepare function is used to append the standard
 *      search paths after the user-supplied search paths.
 */

void
sniff_prepare(void)
{
    trace(("sniff_prepare()\n{\n"));
    assert(lang);
    lang->prepare();
    trace(("}\n"));
}


/*
 * NAME
 *      resolve
 *
 * SYNOPSIS
 *      string_ty *resolve(string_ty *filename, string_ty *extra,
 *              string_list_ty *srl);
 *
 * DESCRIPTION
 *      The resolve function is used to resolve an include
 *      filename into the path of an existing file.
 *
 * ARGUMENTS
 *      filename - name to be resolved
 *      extra   - extra first search element, if not NULL
 *      srl     - search list
 *
 * RETURNS
 *      string_ty *; name of path, or NULL if unmentionable
 */


static string_ty *
resolve(string_ty *filename, string_ty *extra, string_list_ty *srl, int flags)
{
    string_ty       *s;
    size_t          j;
    string_ty       *result;
    sub_context_ty  *scp;

    /*
     * If the name is absolute, irrespecitive of
     * which style, we need look no further.
     */
    trace(("resolve(filename = \"%s\", extra = \"%s\")\n{\n",
        filename->str_text, extra ? extra->str_text : "NULL"));
    if (filename->str_text[0] == '/')
    {
        result = flatten(filename);
        if (os_exists(result->str_text))
            goto done;
        if (string_list_member(&use_these, result))
            goto done;
        goto dilema;
    }

    /*
     * Includes of the form "filename" look in the directory
     * of the parent file, and then the <filename> places.
     */
    if (extra)
    {
        s = os_path_cat(extra, filename);
        result = flatten(s);
        str_free(s);
        scp = sub_context_new();
        sub_var_set_string(scp, "File_Name", result);
        verbose_intl(scp, i18n("may need to look at \"$filename\" file"));
        sub_context_delete(scp);
        if (os_exists(result->str_text))
            goto done;
        if (string_list_member(&use_these, result))
            goto done;
        str_free(result);
    }

    /*
     * look in all the standard places
     */
    for (j = 0; j < srl->nstrings; ++j)
    {
        s = os_path_cat(srl->string[j], filename);
        result = flatten(s);
        str_free(s);
        scp = sub_context_new();
        sub_var_set_string(scp, "File_Name", result);
        verbose_intl(scp, i18n("may need to look at \"$filename\" file"));
        sub_context_delete(scp);
        if (os_exists(result->str_text))
                goto done;
        if (string_list_member(&use_these, result))
                goto done;
        str_free(result);
    }

    /*
     * not found, must have been ifdef'ed out
     * or needs to be built
     */
    dilema:
    switch (flags)
    {
    default:
        scp = sub_context_new();
        sub_errno_setx(scp, ENOENT);
        sub_var_set_string(scp, "File_Name", filename);
        fatal_intl(scp, i18n("open $filename: $errno"));
        /* NOTREACHED */

    case absent_ignore:
        result = 0;
        break;

    case absent_mention:
        if (filename->str_text[0] == '/')
            result = str_copy(filename);
        else if (extra)
        {
            s = os_path_cat(extra, filename);
            result = flatten(s);
            str_free(s);
        }
        else if (srl->nstrings)
        {
            s = os_path_cat(srl->string[0], filename);
            result = flatten(s);
            str_free(s);
        }
        else
            result = flatten(filename);
        break;
    }

    /*
     * here for all exits
     */
    done:
    trace(("return \"%s\";\n", result ? result->str_text : "NULL"));
    trace(("}\n"));
    return result;
}


/*
 * NAME
 *      stat_equal - compare stat structures
 *
 * SYNOPSIS
 *      vint stat_equal(struct stat *, struct stat *);
 *
 * DESCRIPTION
 *      The stat_equal function is sued to compare two stat structures
 *      for equality.  Only this fields which would change if
 *      the file changed are examined.
 */

static int
stat_equal(struct stat *st1, struct stat *st2)
{
    return
    (
        st1->st_dev == st2->st_dev
    &&
        st1->st_ino == st2->st_ino
    &&
        st1->st_size == st2->st_size
    &&
        st1->st_mtime == st2->st_mtime
    &&
        st1->st_ctime == st2->st_ctime
    );
}


static void
print_without_prefix(string_ty *s, string_list_ty *result)
{
    size_t          j;
    int             changed;
    string_ty       *tmp;
    string_ty       *p;

    s = str_copy(s);
    for (;;)
    {
        changed = 0;
        for (j = 0; j < remove_path.nstrings; ++j)
        {
            p = remove_path.string[j];
            if
            (
                s->str_length > p->str_length
            &&
                !memcmp(s->str_text, p->str_text, p->str_length)
            &&
                s->str_text[p->str_length] == '/'
            )
            {
                tmp =
                    str_n_from_c
                    (
                        s->str_text + p->str_length+1,
                        s->str_length-p->str_length-1
                    );
                str_free(s);
                s = tmp;
                ++changed;
            }
        }
        if (!changed)
            break;
    }

    for (j = 0; j < nsubs; ++j)
    {
        sub_ty          *sp;

        sp = &sub[j];
        p = sp->from;
        if
        (
            s->str_length > p->str_length
        &&
            !memcmp(s->str_text, p->str_text, p->str_length)
        &&
            s->str_text[p->str_length] == '/'
        )
        {
            tmp =
                str_format
                (
                    "%s%s",
                    sp->to->str_text,
                    s->str_text + p->str_length
                );
            str_free(s);
            s = tmp;
        }
    }

    string_list_append_unique(result, s);
    str_free(s);
}


static void
exclude_error(string_ty *includer, string_ty *includee)
{
    sub_context_ty  *scp;

    scp = sub_context_new();
    sub_var_set_string(scp, "File_Name1", includer);
    sub_var_set_string(scp, "File_Name2", includee);
    fatal_intl(scp, i18n("$filename1: excluded $filename2 referenced"));
}


/*
 * NAME
 *      sniffer - search file for include dependencies
 *
 * SYNOPSIS
 *      void sniffer(string_ty *pathname);
 *
 * DESCRIPTION
 *      The sniffer function is used to walk a file looking
 *      for any files which it includes, and walking then also.
 *      The names of any include files encountered are printed onto
 *      the standard output.
 *
 * ARGUMENTS
 *      pathname        - pathname to read
 *
 * CAVEATS
 *      Uses the cache where possible to speed things up.
 */

static void
sniffer(string_ty *filename, int prnam, string_list_ty *result)
{
    input_ty        *fp;
    cache_ty        *cp;
    struct stat     st;
    size_t          j;
    sub_context_ty  *scp;
    static string_ty *dash;

    trace(("sniffer(filename = \"%s\")\n{\n", filename->str_text));
    if (!dash)
        dash = str_from_c("-");
    if (prnam)
    {
        print_without_prefix(filename, result);
        ++pcount;
    }

    /*
     * find the file in the cache
     * (will be created if not already there)
     */
    cp = cache_search(filename);
    assert(cp);
    if (str_equal(filename, dash))
        memset(&st, -1, sizeof(st));
    else if (stat(filename->str_text, &st) < 0)
    {
        /*
         * here for failure to open/find a file
         */
        absent:
        switch (errno)
        {
        case ENOENT:
            break;

        case ENOTDIR:
        case EACCES:
            scp = sub_context_new();
            sub_errno_set(scp);
            sub_var_set_string(scp, "File_Name", filename);
            error_intl(scp, i18n("warning: stat $filename: $errno"));
            sub_context_delete(scp);
            break;

        default:
            fatal_intl_stat(filename->str_text);
            /* NOTREACHED */
            break;
        }

        /*
         * zap the stat info,
         * and pretend the file was empty
         */
        memset(&cp->st, 0, sizeof(cp->st));
        string_list_destructor(&cp->ingredients);
        cache_update_notify();
        scp = sub_context_new();
        sub_var_set_string(scp, "File_Name", filename);
        verbose_intl(scp, i18n("bogus empty \"$filename\" file"));
        sub_context_delete(scp);
        goto done;
    }

    /*
     * if the stat in the cache is not the same
     * as the state just obtained, reread the file.
     */
    if (!stat_equal(&st, &cp->st))
    {
        string_list_ty  type1;
        string_list_ty  type2;

        cp->st = st;
        string_list_destructor(&cp->ingredients);
        cache_update_notify();
        scp = sub_context_new();
        sub_var_set_string(scp, "File_Name", filename);
        verbose_intl(scp, i18n("cache miss for \"$filename\" file"));
        sub_context_delete(scp);

        if (str_equal(dash, filename))
            fp = input_stdin();
        else
        {
            if (!os_exists(filename->str_text))
                goto absent;
            fp = input_file_text_open(filename);
        }
        string_list_constructor(&type1);
        string_list_constructor(&type2);
        if (lang->scan(fp, &type1, &type2))
            fatal_intl_read(filename->str_text);
        input_delete(fp);

        /*
         * type2 names have an implicit first element of the search path
         * which is the directory of the including file
         */
        if (type2.nstrings)
        {
            string_ty       *parent;

            if (option.no_src_rel_inc)
            {
                scp = sub_context_new();
                sub_var_set_string(scp, "File_Name", filename);
                sub_var_set_long(scp, "Number", type2.nstrings);
                sub_var_optional(scp, "Number");
                fatal_intl(scp, i18n("$filename has source relative includes"));
                /* NOTREACHED */
            }
            if (!has_been_cut)
            {
                char            *ep;
                char            *sp;

                /*
                 * The -I- option has not been used.
                 */
                sp = filename->str_text;
                ep = strrchr(sp, '/');
                if (ep)
                    parent = str_n_from_c(sp, ep - sp);
                else
                    parent = str_from_c(".");
            }
            else
                parent = 0;
            for (j = 0; j < type2.nstrings; ++j)
            {
                string_ty       *path;

                path = type2.string[j];
                if (string_list_member(&exclude, path))
                    exclude_error(filename, path);
                if (!option.absolute && absolute_filename_test(path->str_text))
                    continue;
                path = resolve(path, parent, &srl1, option.o_absent_local);
                if (path)
                {
                    string_list_append_unique(&cp->ingredients, path);
                    str_free(path);
                }
            }
            if (parent)
                str_free(parent);
        }

        /*
         * type1 names scan the search path
         */
        for (j = 0; j < type1.nstrings; ++j)
        {
            string_ty       *path;

            path = type1.string[j];
            if (string_list_member(&exclude, path))
                exclude_error(filename, path);
            if (!option.absolute && absolute_filename_test(path->str_text))
                continue;
            path = resolve(path, (string_ty *)0, &srl2, option.o_absent_system);
            if (path)
            {
                string_list_append_unique(&cp->ingredients, path);
                str_free(path);
            }
        }

        /*
         * let the lists go
         */
        string_list_destructor(&type1);
        string_list_destructor(&type2);
    }

    /*
     * work down the ingredients list
     * to see if there are more dependencies
     */
    string_list_append_unique(&visited, filename);
    for (j = 0; j < cp->ingredients.nstrings && !interrupted; ++j)
    {
        string_ty       *s;

        s = cp->ingredients.string[j];
        if (!string_list_member(&visited, s))
        {
            if (option.recursive)
                sniffer(s, 1, result);
            else
            {
                print_without_prefix(s, result);
                ++pcount;
            }
        }
    }

    /*
     * here for all exits
     */
    done:
    trace(("}\n"));
}


static void
quote_filename(FILE *fp, string_ty *s)
{
    const char      *cp;

    putc('"', fp);
    for (cp = s->str_text; *cp; ++cp)
    {
        int c = (unsigned char)*cp;
        switch (c)
        {
        default:
            if (isprint(c))
                putc(c, fp);
            else
                fprintf(fp, "\\%03o", c);
            break;

        case '"':
        case '\\':
            putc('\\', fp);
            putc(c, fp);
            break;
        }
    }
    putc('"', fp);
}


static void
maybe_quote_filename(FILE *fp, string_ty *s)
{
    if (option.quote_filenames)
        quote_filename(fp, s);
    else
        fputs(s->str_text, fp);
}


static void
maybe_escape_newline(FILE *fp)
{
    if (option.escape_newline)
    {
        putc(' ', fp);
        putc('\\', fp);
    }
    putc('\n', fp);
}


static void
print_the_list(FILE *fp, string_ty *pfx, string_list_ty *result)
{
    size_t          j;
    int             need_newline;

    need_newline = 0;
    if (pfx && pfx->str_length)
    {
        fputs(pfx->str_text, fp);
        need_newline = 1;
    }
    for (j = 0; j < result->nstrings; ++j)
    {
        if (need_newline)
            maybe_escape_newline(fp);
        maybe_quote_filename(fp, result->string[j]);
        need_newline = 1;
    }
    if (suffix && *suffix)
    {
        if (need_newline)
            maybe_escape_newline(fp);
        fputs(suffix, fp);
    }
    fputc('\n', fp);
}


/*
 * NAME
 *      sniff - search file for include dependencies
 *
 * SYNOPSIS
 *      void sniff(char *pathname);
 *
 * DESCRIPTION
 *      The sniff function is used to walk a file looking
 *      for any files which it includes, and walking then also.
 *      The names of any include files encountered are printed onto
 *      the standard output.
 *
 * ARGUMENTS
 *      pathname        - pathname to read
 */

void
sniff(char *filename)
{
    string_ty       *s;
    RETSIGTYPE      (*int_hold)(int);
    RETSIGTYPE      (*hup_hold)(int);
    RETSIGTYPE      (*term_hold)(int);
    string_list_ty  result;
    string_ty       *ofn;
    FILE            *ofp;
    sub_context_ty  *scp;

    stripdot_list(&srl1);
    stripdot_list(&srl2);
    stripdot_list(&use_these);
    if (option.output)
    {
        ofp = fopen_and_check(option.output, "w");
        ofn = str_from_c(option.output);
    }
    else
    {
        ofp = stdout;
        scp = sub_context_new();
        ofn = subst_intl(scp, i18n("standard output"));
        sub_context_delete(scp);
    }

    int_hold = signal(SIGINT, SIG_IGN);
    if (int_hold != SIG_IGN)
        signal(SIGINT, interrupt);
    hup_hold = signal(SIGHUP, SIG_IGN);
    if (hup_hold != SIG_IGN)
        signal(SIGHUP, interrupt);
    term_hold = signal(SIGTERM, SIG_IGN);
    if (term_hold != SIG_IGN)
        signal(SIGTERM, interrupt);

    if (0 == strcmp(filename, "-"))
    {
        s = str_from_c("-");
    }
    else
    {
        trace(("sniff(filename = \"%s\")\n{\n", filename));
        if (!os_exists(filename))
        {
            switch (option.o_absent_program)
            {
            case absent_error:
                scp = sub_context_new();
                sub_errno_setx(scp, ENOENT);
                sub_var_set_charstar(scp, "File_Name", filename);
                fatal_intl(scp, i18n("open $filename: $errno"));
                /* NOTREACHED */
                break;

            case absent_mention:
                scp = sub_context_new();
                sub_errno_setx(scp, ENOENT);
                sub_var_set_charstar(scp, "File_Name", filename);
                error_intl(scp, i18n("open $filename: $errno"));
                /* NOTREACHED */
                break;

            default:
                break;
            }
            goto done;
        }
        s = str_from_c(filename);
        if (option.stripdot)
        {
            string_ty       *s2;

            s2 = stripdot(s);
            str_free(s);
            s = s2;
        }
    }

    string_list_constructor(&result);
    sniffer(s, 0, &result);
    str_free(s);
    if (result.nstrings)
    {
        if (!prefix.nstrings)
            print_the_list(ofp, 0, &result);
        else
        {
            size_t          j;

            for (j = 0; j < prefix.nstrings; ++j)
                print_the_list(ofp, prefix.string[j], &result);
        }
    }
    string_list_destructor(&result);

    done:
    fflush_and_check(ofp, ofn->str_text);
    if (ofp != stdout)
        fclose_and_check(ofp, ofn->str_text);
    signal(SIGINT, int_hold);
    signal(SIGHUP, hup_hold);
    signal(SIGTERM, term_hold);
    trace(("}\n"));
}


static string_ty *
nuke_trailing_slash(char *path)
{
    size_t          len;

    len = strlen(path);
    while (len >= 2 && path[len - 1] == '/')
        --len;
    return str_n_from_c(path, len);
}


void
sniff_remove_leading_path(char *path)
{
    string_ty       *s;

    s = nuke_trailing_slash(path);
    string_list_append_unique(&remove_path, s);
    str_free(s);
}


void
sniff_substitute_leading_path(char *from, char *to)
{
    sub_ty          *sp;

    if (nsubs >= nsubs_max)
    {
        size_t          nbytes;

        nsubs_max = nsubs_max * 2 + 4;
        nbytes = nsubs_max * sizeof(sub[0]);
        sub = mem_change_size(sub, nbytes);
    }
    sp = &sub[nsubs++];
    sp->from = nuke_trailing_slash(from);
    sp->to = nuke_trailing_slash(to);
}


int
absolute_filename_test(char *path)
{
#ifdef DOS
    if (path[0] && isalpha((unsigned char)path[0]) && path[1] == ':')
        return 1;
#endif
    return (path[0] == '/' || path[0] == '\\');
}


void
sniff_suffix_set(char *text)
{
    suffix = text;
}


void
sniff_prefix_set(char *text)
{
    string_ty       *s;

    s = str_from_c(text);
    string_list_append_unique(&prefix, s);
    str_free(s);
}


void
sniff_exclude(char *text)
{
    string_ty       *s;

    s = str_from_c(text);
    string_list_append_unique(&exclude, s);
    str_free(s);
}
