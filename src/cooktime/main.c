/*
 *      cook - file construction tool
 *      Copyright (C) 1992-1994, 1997-1999, 2001, 2006-2008 Peter Miller
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

#include <common/ac/stddef.h>
#include <common/ac/stdio.h>
#include <common/ac/string.h>
#include <common/ac/stdlib.h>

#include <sys/types.h>
#include <common/ac/utime.h>
#include <sys/stat.h>

#include <common/arglex.h>
#include <cooktime/date.h>
#include <common/error_intl.h>
#include <common/help.h>
#include <common/main.h>
#include <common/progname.h>
#include <common/str.h>
#include <common/ts.h>
#include <common/version.h>
#include <common/str_list.h>


static void
usage(void)
{
    char            *progname;

    progname = progname_get();
    fprintf(stderr, "usage: %s [ <option>... ] <filename>\n", progname);
    fprintf(stderr, "       %s -Help\n", progname);
    fprintf(stderr, "       %s -VERsion\n", progname);
    exit(1);
}


enum
{
    arglex_token_access,
    arglex_token_modify,
    arglex_token_report,
    arglex_token_tsg
};

static arglex_table_ty argtab[] =
{
    { "-Access", (arglex_token_ty) arglex_token_access },
    { "-Modify", (arglex_token_ty) arglex_token_modify },
    { "-Report", (arglex_token_ty) arglex_token_report },
    { "-Time_Stamp_Granularity", (arglex_token_ty) arglex_token_tsg },
    { 0, (arglex_token_ty)0 }, /* end marker */
};


static time_t
round_up(time_t t, time_t tsg)
{
    if (tsg <= 1)
    {
        return t;
    }
    else
    {
        if (t == (time_t)(-1))
            return t;
        t = (t + tsg - 1) / tsg;
        return (t * tsg);
    }
}


int
main(int argc, char **argv)
{
    string_list_ty  filename;
    string_ty       *s;
    size_t          j;
    int             mtime_set;
    long            mtime;
    int             atime_set;
    long            atime;
    int             report;
    int             tsg_set;
    time_t          tsg;

    arglex_init(argc, argv, argtab);
    str_initialize();
    switch (arglex())
    {
    case arglex_token_help:
        help((char *)0, usage);
        exit(0);

    case arglex_token_version:
        version();
        exit(0);

    default:
        break;
    }

    report = 0;
    atime_set = 0;
    atime = 0;
    mtime_set = 0;
    mtime = 0;
    tsg_set = 0;
    tsg = 1;
    string_list_constructor(&filename);
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(usage);
            continue;

        case arglex_token_string:
            s = str_from_c(arglex_value.alv_string);
            string_list_append(&filename, s);
            str_free(s);
            break;

        case arglex_token_tsg:
            if (tsg_set)
                goto duplicate;
            tsg_set = 1;
            if (arglex() != arglex_token_number)
            {
                arg_needs_number(arglex_token_tsg, usage);
                /* NOTREACHED */
            }
            tsg = (time_t)arglex_value.alv_number;

            break;

        case arglex_token_report:
            if (report)
            {
              duplicate:
                arg_duplicate_cur(usage);
                /* NOTREACHED */
            }
            report++;
            break;

        case arglex_token_access:
            if (atime_set)
                goto duplicate;
            if (arglex() != arglex_token_string)
            {
                arg_needs_string(arglex_token_access, usage);
                /* NOTREACHED */
            }
            atime_set = 1;
            atime = date_scan(arglex_value.alv_string);
            break;

        case arglex_token_modify:
            if (mtime_set)
                goto duplicate;
            if (arglex() != arglex_token_string)
            {
                arg_needs_string(arglex_token_modify, usage);
                /* NOTREACHED */
            }
            mtime_set = 1;
            mtime = date_scan(arglex_value.alv_string);
            break;
        }
        arglex();
    }
    if (!filename.nstrings)
        fatal_intl(0, i18n("no files named"));

    if (atime_set)
    {
        atime = round_up(atime, tsg);
        if (atime < 0)
        {
            sub_context_ty  *scp;

            scp = sub_context_new();
            sub_var_set(scp, "Value", "%s", arglex_value.alv_string);
            fatal_intl
                (
                    scp,
                    i18n("the string \"$value\" is not a valid date")
                );
        }

    }

    if (mtime_set)
    {
        mtime_set = 1;
        mtime = round_up(mtime, tsg);
        if (mtime < 0)
        {
            sub_context_ty  *scp;

            scp = sub_context_new();
            sub_var_set(scp, "Value", "%s", arglex_value.alv_string);
            fatal_intl
                (
                    scp,
                    i18n("the string \"$value\" is not a valid date")
                );
        }

    }

    if (!report && !mtime_set && !atime_set)
    {
        mtime_set = 1;
        mtime = round_up(date_scan("now"), tsg);
    }

    for (j = 0; j < filename.nstrings; ++j)
    {
        struct stat     st;
        struct utimbuf  ut;

        s = filename.string[j];
        if (stat(s->str_text, &st))
            fatal_intl_stat(s->str_text);
        if (mtime_set)
            ut.modtime = mtime;
        else
            ut.modtime = st.st_mtime;
        if (atime_set)
            ut.actime = atime;
        else
            ut.actime = st.st_atime;
        if ((atime_set || mtime_set) && utime(s->str_text, &ut))
        {
            sub_context_ty  *scp;

            scp = sub_context_new();
            sub_errno_set(scp);
            sub_var_set_string(scp, "File_Name", s);
            fatal_intl(scp, i18n("utime $filename: $errno"));
        }
        if (report)
        {
            printf("%s\n", s->str_text);
            printf("    access %s\n", date_string(ut.actime));
            printf("    modify %s\n", date_string(ut.modtime));
        }
    }
#if defined(sun) || defined(__sun__)
    /*
     * Solaris is brain dead.  The inode times are not updated until
     * the next sync.  So we'll help it along.
     */
    sync();
#endif
    exit(0);
    return 0;
}
