/*
 *      cook - a program construction tool
 *      Copyright (C) 1991-1995, 1997, 1998, 2006-2008, 2010 Peter Miller
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

#include <common/ac/stdlib.h>
#include <common/ac/unistd.h>

#include <common/arglex.h>
#include <common/env.h>
#include <common/error.h>
#include <common/error_intl.h>
#include <common/help.h>
#include <common/libdir.h>
#include <common/os_path_cat.h>
#include <common/page.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/trace.h>
#include <common/verbose.h>
#include <common/str_list.h>


static void
help_env(void)
{
    char            *cp;
    string_list_ty  manpath;
    string_ty       *s;
    string_list_ty  lib;
    string_list_ty  lang;
    size_t          j;
    size_t          k;
    string_list_ty  wl;

    /*
     * Honour any existing MANPATH setting by appending only.
     * Read the MANPATH to set the initial path.
     */
    cp = getenv("MANPATH");
    if (cp)
    {
        s = str_from_c(cp);
        str2wl(&manpath, s, ":", 0);
        str_free(s);
    }
    else
    {
        string_list_constructor(&manpath);
        s = str_from_c("/usr/man");
        string_list_append(&manpath, s);
        str_free(s);

        s = str_from_c("/usr/share/man");
        string_list_append(&manpath, s);
        str_free(s);
    }

    /*
     * The list of library directories to look in.
     */
    string_list_constructor(&lib);
    s = str_from_c(data_directory_get());
    string_list_append_unique(&lib, s);
    str_free(s);
    cp = getenv("COOK_MESSAGE_LIBRARY");
    if (cp)
    {
        s = str_from_c(cp);
        str2wl(&wl, s, ":", 0);
        for (j = 0; j < wl.nstrings; ++j)
            string_list_append_unique(&lib, wl.string[j]);
        string_list_destructor(&wl);
    }

    /*
     * Use the LANGUAGE (or LANG) environment
     * variables to know which languages to add.
     * Default to "en" if not set.
     */
    string_list_constructor(&lang);
    cp = getenv("LANGUAGE");
    if (cp)
    {
        s = str_from_c(cp);
        str2wl(&wl, s, ":", 0);
        for (j = 0; j < wl.nstrings; ++j)
            string_list_append_unique(&lang, wl.string[j]);
        string_list_destructor(&wl);
    }
    cp = getenv("LANG");
    if (cp)
    {
        s = str_from_c(cp);
        str2wl(&wl, s, ":", 0);
        for (j = 0; j < wl.nstrings; ++j)
            string_list_append_unique(&lang, wl.string[j]);
        string_list_destructor(&wl);
    }
    if (!lang.nstrings)
    {
        s = str_from_c("en");
        string_list_append(&lang, s);
        str_free(s);
    }

    /*
     * convolve the lib and lang lists and append them to the manpath
     */
    for (j = 0; j < lib.nstrings; ++j)
    {
        string_list_append_unique(&manpath, lib.string[j]);
        for (k = 0; k < lang.nstrings; ++k)
        {
            s = os_path_cat(lib.string[j], lang.string[k]);
            string_list_append_unique(&manpath, s);
            str_free(s);
        }
    }
    string_list_destructor(&lib);
    string_list_destructor(&lang);

    s = str_from_c(manual_directory_get());
    string_list_append_unique(&lib, s);
    str_free(s);

    /*
     * set the MANPATH environment variable
     */
    s = wl2str_respect_empty(&manpath, 0, manpath.nstrings, ":", 1);
    string_list_destructor(&manpath);
    env_set("MANPATH", s->str_text);
    str_free(s);
}


void
help(char *progname, void (*usage)(void))
{
    char            *argv[3];
    sub_context_ty  *scp;

    /*
     * collect the rest of the command line,
     * if necessary
     */
    if (usage)
    {
        arglex();
        while (arglex_token != arglex_token_eoln)
            generic_argument(usage);
    }
    if (!progname)
        progname = progname_get();

    /*
     * set the MANPATH environment variable
     * to point into the libraries
     */
    help_env();

    /*
     * Invoke the appropriate ``man'' command.  This will find the
     * right language on the search path, and it already knows how
     * to translate the *roff into text.
     */
    argv[0] = "man";
    argv[1] = progname;
    argv[2] = 0;
    error_raw("%s %s", argv[0], argv[1]);
    execvp(argv[0], argv);
    scp = sub_context_new();
    sub_errno_set(scp);
    sub_var_set_charstar(scp, "File_Name", argv[0]);
    fatal_intl(scp, i18n("exec $filename: $errno"));
    /* NOTREACHED */
}


void
generic_argument(void (*usage)(void))
{
    sub_context_ty  *scp;

    trace(("generic_argument()\n{\n"));
    switch (arglex_token)
    {
    default:
        bad_argument(usage);
        /* NOTREACHED */

    case arglex_token_page_length:
        if (arglex() != arglex_token_number)
        {
            arg_needs_number(arglex_token_page_length, usage);
            /* NOTREACHED */
        }
        page_length_set(arglex_value.alv_number);
        arglex();
        break;

    case arglex_token_page_width:
        if (arglex() != arglex_token_number)
        {
            arg_needs_number(arglex_token_page_width, usage);
            /* NOTREACHED */
        }
        page_width_set(arglex_value.alv_number);
        arglex();
        break;

    case arglex_token_tracing:
        if (arglex() != arglex_token_string)
        {
            scp = sub_context_new();
            sub_var_set
            (
                scp,
                "Name",
                "%s",
                arglex_token_name(arglex_token_tracing)
            );
            error_intl(scp, i18n("$name needs files"));
            sub_context_delete(scp);
            usage();
        }
        for (;;)
        {
#ifdef DEBUG
            trace_enable(arglex_value.alv_string);
#endif
            if (arglex() != arglex_token_string)
                break;
        }
#ifndef DEBUG
        scp = sub_context_new();
        sub_var_set_charstar
        (
            scp,
            "Name",
            arglex_token_name(arglex_token_tracing)
        );
        error_intl(scp, i18n("$name needs DEBUG"));
        sub_context_delete(scp);
#endif
        break;

    case arglex_token_verbose:
        verbose_set();
        arglex();
        break;
    }
    trace(("}\n"));
}


void
bad_argument(void (*usage)(void))
{
    sub_context_ty  *scp;

    trace(("bad_argument()\n{\n"));
    switch (arglex_token)
    {
    case arglex_token_string:
        scp = sub_context_new();
        sub_var_set_charstar(scp, "File_Name", arglex_value.alv_string);
        error_intl(scp, i18n("misplaced file name (\"$filename\")"));
        sub_context_delete(scp);
        break;

    case arglex_token_number:
        scp = sub_context_new();
        sub_var_set_charstar(scp, "Number", arglex_value.alv_string);
        error_intl(scp, i18n("misplaced number ($number)"));
        sub_context_delete(scp);
        break;

    case arglex_token_option:
        scp = sub_context_new();
        sub_var_set_charstar(scp, "Name", arglex_value.alv_string);
        error_intl(scp, i18n("unknown \"$name\" option"));
        sub_context_delete(scp);
        break;

    case arglex_token_eoln:
        error_intl(0, i18n("command line too short"));
        break;

    default:
        scp = sub_context_new();
        sub_var_set_charstar(scp, "Name", arglex_value.alv_string);
        error_intl(scp, i18n("misplaced \"$name\" option"));
        sub_context_delete(scp);
        break;
    }
    usage();
    trace(("}\n"));
    quit(1);
    /* NOTREACHED */
}


void
arg_duplicate(int n, void (*usage)(void))
{
    sub_context_ty  *scp;

    scp = sub_context_new();
    sub_var_set_charstar(scp, "Name", arglex_token_name(n));
    if (!usage)
    {
        fatal_intl(scp, i18n("duplicate \"$name\" option"));
        /* NOTREACHED */
    }
    error_intl(scp, i18n("duplicate \"$name\" option"));
    sub_context_delete(scp);
    usage();
    /* NOTREACHED */
}


void
arg_duplicate_cur(void (*usage)(void))
{
    arg_duplicate(arglex_token, usage);
    /* NOTREACHED */
}


void
arg_needs_string(int n, void (*usage)(void))
{
    sub_context_ty  *scp;

    scp = sub_context_new();
    sub_var_set_charstar(scp, "Name", arglex_token_name(n));
    if (!usage)
    {
        fatal_intl(scp, i18n("$name needs string"));
        /* NOTREACHED */
    }
    error_intl(scp, i18n("$name needs string"));
    sub_context_delete(scp);
    usage();
    /* NOTREACHED */
}


void
arg_needs_number(int n, void (*usage)(void))
{
    sub_context_ty  *scp;

    scp = sub_context_new();
    sub_var_set_charstar(scp, "Name", arglex_token_name(n));
    if (!usage)
    {
        fatal_intl(scp, i18n("$name needs number"));
        /* NOTREACHED */
    }
    error_intl(scp, i18n("$name needs number"));
    sub_context_delete(scp);
    usage();
    /* NOTREACHED */
}
