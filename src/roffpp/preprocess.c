/*
 *      cook - file construction tool
 *      Copyright (C) 1992-1994, 1997-1999, 2006-2008 Peter Miller
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
#include <common/ac/stdlib.h>
#include <common/ac/unistd.h>

#include <common/error_intl.h>
#include <common/input/file_text.h>
#include <common/mem.h>
#include <common/os_path_cat.h>
#include <common/str.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <roffpp/preprocess.h>

static string_list_ty search;
static string_ty *ofn;
static FILE    *ofp;


/*
 * NAME
 *      preprocess_include
 *
 * SYNOPSIS
 *      void preprocess_include(char *path);
 *
 * DESCRIPTION
 *      The preprocess_include function is used to append
 *      to the include search path.
 *
 * ARGUMENTS
 *      path    - path to append
 */

void
preprocess_include(char *path)
{
    string_ty       *s;

    trace(("preprocess_include(path = \"%s\")\n{\n", path));
    s = str_from_c(path);
    string_list_append_unique(&search, s);
    str_free(s);
    trace(("}\n"));
}


/*
 * NAME
 *      source
 *
 * SYNOPSIS
 *      int source(char *line);
 *
 * DESCRIPTION
 *      The source function is used to test if a line of tect is
 *      a .so directive, and to insert the contents of the sourced
 *      file at this point.
 *
 * ARGUMENTS
 *      line    - pointer to line of text
 *
 * RETURNS
 *      int;    zero if is not a .so directive,
 *              non-zero if it is a .so directive
 */

static void     scan(string_ty *);      /* forward */

static int
source(char *line)
{
    string_ty       *filename;
    size_t          j;
    int             result;
    char            *ep;

    /*
     * see if this is a .so directive
     */
    if (*line != '.')
        return 0;
    trace(("source(line = \"%s\")\n{\n", line));
    result = 0;
    line++;
    while (isspace(*line))
        line++;
    if (line[0] != 's' || line[1] != 'o' || !isspace(line[2]))
        goto ret;
    line += 3;
    while (isspace(*line))
        line++;
    if (!*line)
        goto ret;

    /*
     * find the end of the argument
     */
    for (ep = line + 1; *ep && !isspace(*ep); ++ep)
        ;
    filename = str_n_from_c(line, ep - line);

    /*
     * no need to search when it's an absolute path
     */
    if (*line == '/')
    {
        scan(filename);
        str_free(filename);
        result = 1;
        goto ret;
    }

    /*
     * search for the name in the search list
     */
    for (j = 0; j < search.nstrings; ++j)
    {
        string_ty      *s;
        string_ty      *dir;

        dir = search.string[j];
        s = os_path_cat(dir, filename);
        if (access(s->str_text, F_OK) == 0)
        {
            str_free(filename);
            scan(s);
            str_free(s);
            result = 1;
            goto ret;
        }
        str_free(s);
    }
    str_free(filename);

    /*
     * let {ps,n,t,dit,pt}roff bomb later
     */
    result = 0;

    /*
     * here for all exits
     */
  ret:
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}

/*
 * NAME
 *      lf_directive
 *
 * SYNOPSIS
 *      int lf_directive(char *line, string_ty **ifn, long *lino);
 *
 * DESCRIPTION
 *      The lf_directive function is used to test if a line of text is
 *      a .lf directive, and to adjust the file position to this point.
 *
 * ARGUMENTS
 *      line    - pointer to line of text
 *      ifn     - file name (ptr) if needs changing
 *      lino    - line number (ptr) if needs changing
 *
 * RETURNS
 *      int;    zero if is not a .so directive,
 *              non-zero if it is a .so directive
 */

static int
lf_directive(char *line, string_ty **ifn, long *lino)
{
    int             result;
    long            n;
    string_ty       *s;
    char            *ep;

    /*
     * see if this is a .so directive
     */
    if (*line != '.')
        return 0;
    trace(("source(line = \"%s\")\n{\n", line));
    result = 0;
    line++;
    while (isspace(*line))
        line++;
    if (line[0] != 'l' || line[1] != 'f' || !isspace(line[2]))
        goto ret;
    line += 3;
    while (isspace(*line))
        line++;
    if (!*line)
        goto ret;

    /*
     * find the line number
     */
    for (ep = line + 1; *ep && !isspace(*ep); ++ep)
        ;
    s = str_n_from_c(line, ep - line);
    n = atol(s->str_text);
    str_free(s);
    if (n <= 0)
        goto ret;
    *lino = n - 1;
    result = 1;

    /*
     * find the file name
     */
    line = ep;
    while (*line && isspace(*line))
        line++;
    if (!*line)
        goto ret;
    for (ep = line + 1; *ep && !isspace(*ep); ++ep)
        ;
    s = str_n_from_c(line, ep - line);
    if (*ifn)
        str_free(*ifn);
    *ifn = s;

    /*
     * here for all exits
     */
    ret:
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}


/*
 * NAME
 *      resync
 *
 * SYNOPSOS
 *      void resync(FILE *fp, char *name, long line);
 *
 * DESCRIPTION
 *      The resync function is used to emit appropriate
 *      *roff requests to resynchronize the *roff engine
 *      to the correct file name and line numner,
 *      so that error messages, etc al, are meaningful.
 *
 * ARGUMENTS
 *      fp      - file stream to print on
 *      name    - name of input file
 *      line    - num number in the input file
 */

static void
resync(FILE *fp, string_ty *file, long line)
{
    fprintf(fp, ".lf %ld %s\n", line, file->str_text);
}


/*
 * NAME
 *      scan
 *
 * SYNOPSIS
 *      void scan(char *path);
 *
 * DESCRIPTION
 *      The scan function is used to can a file, copying its contents
 *      to the output, replacing .so directives with the contents of
 *      the included files.
 *
 * ARGUMENTS
 *      path    - name of file to scan
 */

static void
scan(string_ty *ifn)
{
    input_ty        *ifp;
    size_t          pos;
    static size_t   max;
    static char     *line;
    long            lino;
    int             c;
    string_ty       *ifn2;

    trace(("scan(ifn = \"%s\")\n{\n", ifn ? ifn->str_text : "-"));
    ifp = input_file_text_open(ifn);
    ifn2 = str_copy(input_filename(ifp));

    lino = 1;
    resync(ofp, ifn2, lino);

    pos = 0;
    for (;;)
    {
        if (pos >= max)
        {
            max += 100;
            line = mem_change_size(line, max);
        }
        c = input_getc(ifp);
        switch (c)
        {
        case INPUT_EOF:
            if (!pos)
                break;
            /* fall through... */

        case '\n':
            line[pos] = 0;
            if (source(line) || lf_directive(line, &ifn2, &lino))
                resync(ofp, ifn2, lino + 1);
            else
            {
                fputs(line, ofp);
                putc('\n', ofp);
            }
            fflush_and_check(ofp, ofn->str_text);
            lino++;
            pos = 0;
            continue;

        default:
            line[pos++] = c;
            continue;
        }
        break;
    }

    input_delete(ifp);
    str_free(ifn2);
    trace(("}\n"));
}


/*
 * NAME
 *      preprocess
 *
 * SYNOPSIS
 *      void preprocess(char *infile, char *outfile);
 *
 * DESCRIPTION
 *      The preprocess function is used to process an *roff file and
 *      eliminate the .so directives, replacing them with the contents
 *      of the included files.
 *
 * ARGUMENTS
 *      infile  - name of file to scan, NULL means stdin
 *      outfile - name of file to hold result, NULL means stdout
 */

void
preprocess(char *ifile, char *ofile)
{
    string_ty       *s;
    sub_context_ty  *scp;

    /*
     * default the search path iff the user specified nothing
     */
    trace(("preprocess(ifile = \"%s\", ofile = \"%s\")\n{\n",
            ifile ? ifile : "-", ofile ? ofile : "-"));
    if (!search.nstrings)
        preprocess_include(".");

    /*
     * open the output file
     */
    if (ofile)
    {
        ofn = str_from_c(ofile);
        ofp = fopen_and_check(ofn->str_text, "w");
    }
    else
    {
        scp = sub_context_new();
        ofn = subst_intl(scp, i18n("standard output"));
        sub_context_delete(scp);
        ofp = stdout;
    }

    /*
     * scan the input
     */
    if (ifile)
        s = str_from_c(ifile);
    else
        s = 0;
    scan(s);
    if (s)
        str_free(s);

    /*
     * close up and go home
     */
    fflush_and_check(ofp, ofn->str_text);
    if (ofp != stdout)
        fclose_and_check(ofp, ofn->str_text);
    str_free(ofn);
    ofp = 0;
    ofn = 0;
    trace(("}\n"));
}
