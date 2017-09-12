/*
 *      cook - file construction tool
 *      Copyright (C) 1993, 1994, 1996-1999, 2001, 2006, 2007 Peter Miller;
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

#include <common/ac/errno.h>
#include <common/ac/stddef.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <common/ac/dirent.h>

#include <cook/builtin/glob.h>
#include <common/error_intl.h>
#include <cook/expr/position.h>
#include <common/gmatch.h>
#include <common/mem.h>
#include <common/stracc.h>
#include <common/str_list.h>
#include <common/trace.h>


static string_list_ty *where;
static stracc   temp;


static void
error_handler(void *p, string_ty *message)
{
    const expr_position_ty *pp;
    sub_context_ty  *scp;

    pp = p;
    scp = sub_context_new();
    sub_var_set_string(scp, "MeSsaGe", message);
    error_with_position(pp, scp, i18n("$message"));
    sub_context_delete(scp);
}


/*
 * NAME
 *      globber - file name expander
 *
 * SYNOPSIS
 *      int globber(char *formal);
 *
 * DESCRIPTION
 *      The globber function is used to generate a list of file names from the
 *      given `formal' pattern.  Results are appended to the word list pointed
 *      to the global `where' variable.
 *
 * RETURNS
 *      int; 0 on success, -1 on error.
 *
 * CAVEAT
 *      Assumes that the `where' global variable has been initialized.
 */

static int
globber(char *formal, const expr_position_ty * pp)
{
    char            *formal_end;
    char            *cp;
    int             retval;
    sub_context_ty  *scp;

    trace(("globber(formal = %8.8lX)\n{\n", formal));
    trace_string(formal);
    retval = 0;
    gmatch_error_handler(error_handler, (void *)pp);
    for (;;)
    {
        while (*formal == '/')
            sa_char(&temp, *formal++);
        formal_end = strchr(formal, '/');
        if (!formal_end)
            formal_end = formal + strlen(formal);
        for (cp = formal; cp < formal_end; ++cp)
            if (strchr("[?*", *cp))
                break;
        if (cp >= formal_end)
        {
            struct stat     st;
            size_t          n;

            /* nothing special */
            trace(("ordinary = \"%.*s\";\n", formal_end - formal, formal));
            for (cp = formal; cp < formal_end; ++cp)
                sa_char(&temp, *cp);
            n = sa_mark(&temp);
            sa_char(&temp, 0);
            trace(("generated = \"%s\"\n", temp.sa_buf));

            /*
             * Need to confirm that it exists.  We are a
             * file name matcher, not a file name generator.
             */
            if (stat(temp.sa_buf, &st) < 0)
            {
                if (errno != ENOENT && errno != ENOTDIR)
                {
                    scp = sub_context_new();
                    sub_errno_set(scp);
                    sub_var_set_charstar(scp, "File_Name", temp.sa_buf);
                    error_with_position
                    (
                        pp,
                        scp,
                        i18n("stat $filename: $errno")
                    );
                    sub_context_delete(scp);
                    retval = -1;
                    goto ret;
                }

                /* fail the match */
                break;
            }
            sa_goto(&temp, n);

            if (!*cp)
            {
                string_ty       *s;

                s = sa_close(&temp);
                /* defeat assert protection */
                temp.sa_inuse = 1;
                string_list_append(where, s);
                str_free(s);
                break;
            }
            formal = formal_end;
        }
        else
        {
            size_t          n;
            DIR             *dp;
            struct dirent   *dep;

            /* need to expand wild characters */
            trace(("expand = \"%.*s\";\n", formal_end - formal, formal));
            n = sa_mark(&temp);
            sa_char(&temp, 0);
            dp = opendir(n ? temp.sa_buf : ".");
            if (!dp)
            {
                if (errno == ENOTDIR)
                    break;
                scp = sub_context_new();
                sub_errno_set(scp);
                sub_var_set_charstar(scp, "File_Name", temp.sa_buf);
                error_with_position(pp, scp, i18n("opendir $filename: $errno"));
                sub_context_delete(scp);
                retval = -1;
                goto ret;
            }
            sa_goto(&temp, n);
            for (;;)
            {
                char            *np;

                dep = readdir(dp);
                if (!dep)
                    break;
                np = dep->d_name;
                if (np[0] == '.' && (!np[1] || (np[1] == '.' && !np[2])))
                    continue;
                trace(("filename = \"%s\"\n", np));
                switch (gmatch2(formal, formal_end, np))
                {
                case 0:
                    continue;

                case -1:
                    retval = -1;
                    goto ret;
                }
                for (cp = np; *cp; ++cp)
                    sa_char(&temp, *cp);
                if (!*formal_end)
                {
                    string_ty       *s;

                    s = sa_close(&temp);
                    /* defeat assert protection */
                    temp.sa_inuse = 1;
                    string_list_append(where, s);
                    str_free(s);
                }
                else
                {
                    sa_char(&temp, '/');
                    if (globber(formal_end + 1, pp))
                    {
                        closedir(dp);
                        retval = -1;
                        goto ret;
                    }
                }
                sa_goto(&temp, n);
                temp.sa_inuse = 1;
            }
            closedir(dp);
            break;
        }
    }
    ret:
    gmatch_error_handler(0, 0);
    trace(("return %d;\n", retval));
    trace(("}\n"));
    return retval;
}


/*
 * NAME
 *      cmp - compare strings
 *
 * SYNOPSIS
 *      int cmp(string_ty **, string_ty **);
 *
 * DESCRIPTION
 *      The cmp function is used to compare two strings.
 *
 * RETURNS
 *      int; <0 if a<b, 0 if a==b, >0 if a>b
 *
 * CAVEAT
 *      Intended for use by qsort.
 */

static int
cmp(const void *va, const void *vb)
{
    string_ty       *a;
    string_ty       *b;

    a = *(string_ty **)va;
    b = *(string_ty **)vb;
    return strcmp(a->str_text, b->str_text);
}


/*
 * NAME
 *      builtin_glob - builtin function for expanding file names
 *
 * SYNOPSIS
 *      int builtin_glob(string_list_ty *result, string_list_ty *args);
 *
 * DESCRIPTION
 *      The builtin_glob function is used to implement the "glob" builtin
 *      function of cook to expand file name patterns.
 *
 * RETURNS
 *      int; 0 on success, -1 on any error
 *
 * CAVEAT
 *      This function is designed to be used as a "builtin" function.
 */

static int
interpret(string_list_ty *result, const string_list_ty *args,
    const expr_position_ty *pp, const struct opcode_context_ty *ocp)
{
    size_t          j;
    size_t          start;
    int             retval;

    trace(("glob(result = %08X, args = %08X)\n{\n", result, args));
    (void)ocp;
    retval = 0;
    where = result;
    for (j = 1; j < args->nstrings; ++j)
    {
        temp.sa_inuse = 0;      /* bypass assert protection */
        sa_open(&temp);
        start = result->nstrings;
        if (globber(args->string[j]->str_text, pp))
        {
            retval = -1;
            break;
        }
        qsort
        (
            result->string + start,
            result->nstrings - start,
            sizeof(string_ty *),
            cmp
        );
    }
    trace(("return %d;\n", retval));
    trace(("}\n"));
    return retval;
}


builtin_ty builtin_glob =
{
    "glob",
    interpret,
    interpret,                  /* script */
};


builtin_ty builtin_wildcard =
{
    "wildcard",
    interpret,
    interpret,                  /* script */
};
