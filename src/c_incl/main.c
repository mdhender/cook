/*
 *      cook - file construction tool
 *      Copyright (C) 1994, 1995, 1997-2002, 2006-2008 Peter Miller
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

#include <common/arglex.h>
#include <common/error_intl.h>
#include <common/help.h>
#include <common/progname.h>
#include <common/str.h>
#include <common/verbose.h>
#include <common/version.h>
#include <c_incl/cache.h>
#include <c_incl/lang.h>
#include <c_incl/sniff.h>


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
    arglex_token_absent_local_error,
    arglex_token_absent_local_ignore,
    arglex_token_absent_local_mention,
    arglex_token_absent_program_error,
    arglex_token_absent_program_ignore,
    arglex_token_absent_system_error,
    arglex_token_absent_system_ignore,
    arglex_token_absent_system_mention,
    arglex_token_absent_use_this,
    arglex_token_escape_newline,
    arglex_token_exclude,
    arglex_token_include,
    arglex_token_language,
    arglex_token_lang_c,
    arglex_token_lang_roff,
    arglex_token_absolute,
    arglex_token_no_absolute,
    arglex_token_cache,
    arglex_token_cache_not,
    arglex_token_no_source_relative_includes,
    arglex_token_no_system,
    arglex_token_output,
    arglex_token_prefix,
    arglex_token_quote_filenames,
    arglex_token_recursive,
    arglex_token_recursive_not,
    arglex_token_remove_leading_path,
    arglex_token_stripdot,
    arglex_token_stripdot_not,
    arglex_token_substitute_leading_path,
    arglex_token_suffix
};

static arglex_table_ty argtab[] =
{
    {
        "-Absent",
        (arglex_token_ty)arglex_token_absent_local_ignore
    },
    {
        "-Absolute_Paths",
        (arglex_token_ty)arglex_token_absolute
    },
    {
        "-Absent_Local_Error",
        (arglex_token_ty)arglex_token_absent_local_error
    },
    {
        "-Absent_Local_Ignore",
        (arglex_token_ty)arglex_token_absent_local_ignore
    },
    {
        "-Absent_Local_Mention",
        (arglex_token_ty)arglex_token_absent_local_mention
    },
    {
        "-Absent_Program_Error",
        (arglex_token_ty)arglex_token_absent_program_error
    },
    {
        "-Absent_Program_Ignore",
        (arglex_token_ty)arglex_token_absent_program_ignore
    },
    {
        "-Absent_System_Error",
        (arglex_token_ty)arglex_token_absent_system_error
    },
    {
        "-Absent_System_Ignore",
        (arglex_token_ty)arglex_token_absent_system_ignore
    },
    {
        "-Absent_System_Mention",
            (arglex_token_ty)arglex_token_absent_system_mention
    },
    {
        "-Absent_Use_This",
        (arglex_token_ty)arglex_token_absent_use_this
    },
    {
        "-Generated_Files",
        (arglex_token_ty)arglex_token_absent_use_this
    },
    {
        "-Interior_Files",
        (arglex_token_ty)arglex_token_absent_use_this
    },
    {
        "-Derived_Files",
        (arglex_token_ty)arglex_token_absent_use_this
    },
    {
        "-\\U*",
        (arglex_token_ty)arglex_token_absent_use_this
    },
    {
        "-C",
        (arglex_token_ty)arglex_token_lang_c
    },
    {
        "-Empty_If_Absent",
        (arglex_token_ty)arglex_token_absent_program_ignore
    },
    {
        "-Escape_Newlines",
        (arglex_token_ty)arglex_token_escape_newline
    },
    {
        "-EXclude",
        (arglex_token_ty)arglex_token_exclude
    },
    {
        "-\\I*",
        (arglex_token_ty)arglex_token_include
    },
    {
        "-Include",
        (arglex_token_ty)arglex_token_include
    },
    {
        "-Language",
        (arglex_token_ty)arglex_token_language
    },
    {
        "-No_Absolute_Paths",
        (arglex_token_ty)arglex_token_no_absolute
    },
    {
        "-CAche",
        (arglex_token_ty)arglex_token_cache
    },
    {
        "-No_Cache",
        (arglex_token_ty)arglex_token_cache_not
    },
    {
        "-No_Source_Relative_Includes",
        (arglex_token_ty)arglex_token_no_source_relative_includes
    },
    {
        "-No_System",
        (arglex_token_ty)arglex_token_no_system
    },
    {
        "-Output",
        (arglex_token_ty)arglex_token_output
    },
    {
        "-PREfix",
        (arglex_token_ty)arglex_token_prefix
    },
    {
        "-Quote_FileNames",
        (arglex_token_ty)arglex_token_quote_filenames
    },
    {
        "-RECursion",
        (arglex_token_ty)arglex_token_recursive
    },
    {
        "-No_RECursion",
        (arglex_token_ty)arglex_token_recursive_not
    },
    {
        "-Remove_Leading_Path",
        (arglex_token_ty)arglex_token_remove_leading_path
    },
    {
        "-Roff",
        (arglex_token_ty)arglex_token_lang_roff
    },
    {
        "-STripdot",
        (arglex_token_ty)arglex_token_stripdot
    },
    {
        "-Not_STripdot",
        (arglex_token_ty)arglex_token_stripdot_not
    },
    {
        "-System_absent",
        (arglex_token_ty)arglex_token_absent_system_ignore
    },
    {
        "-Substitute_Leading_Path",
        (arglex_token_ty)arglex_token_substitute_leading_path
    },
    {
        "-SUFfix",
        (arglex_token_ty)arglex_token_suffix
    },

    /* end marker */
    { 0, (arglex_token_ty)0 },
};


int
main(int argc, char **argv)
{
    char            *source;
    int             no_system;
    int             cache;
    sniff_ty        *language;
    char            *arg1;
    char            *arg2;

    arglex_init(argc, argv, argtab);
    str_initialize();
    cache_initialize();
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

    source = 0;
    no_system = 0;
    cache = -1;
    language = 0;
    option.o_absent_local = -1;
    option.o_absent_system = -1;
    option.o_absent_program = -1;
    option.absolute = -1;
    option.recursive = -1;
    option.stripdot = -1;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(usage);
            continue;

        case arglex_token_string:
            if (source)
            {
                too_many_sources:
                error_intl(0, i18n("too many filenames specified"));
                usage();
            }
            source = arglex_value.alv_string;
            break;

        case arglex_token_stdio:
            if (source)
                goto too_many_sources;
            source = "-";
            cache = 0;
            break;

        case arglex_token_absent_local_ignore:
            if (option.o_absent_local != -1)
            {
                duplicate:
                arg_duplicate_cur(usage);
                /* NOTREACHED */
            }
            option.o_absent_local = absent_ignore;
            break;

        case arglex_token_absent_local_mention:
            if (option.o_absent_local != -1)
                goto duplicate;
            option.o_absent_local = absent_mention;
            break;

        case arglex_token_absent_local_error:
            if (option.o_absent_local != -1)
                goto duplicate;
            option.o_absent_local = absent_error;
            break;

        case arglex_token_absent_system_ignore:
            if (option.o_absent_system != -1)
                goto duplicate;
            option.o_absent_system = absent_ignore;
            break;

        case arglex_token_absent_system_mention:
            if (option.o_absent_system != -1)
                goto duplicate;
            option.o_absent_system = absent_mention;
            break;

        case arglex_token_absent_system_error:
            if (option.o_absent_system != -1)
                goto duplicate;
            option.o_absent_system = absent_error;
            break;

        case arglex_token_absent_program_ignore:
            if (option.o_absent_program != -1)
                goto duplicate;
            option.o_absent_program = absent_ignore;
            break;

        case arglex_token_absent_program_error:
            if (option.o_absent_program != -1)
                goto duplicate;
            option.o_absent_program = absent_error;
            break;

        case arglex_token_absent_use_this:
            for (;;)
            {
                switch (arglex())
                {
                default:
                    break;

                case arglex_token_string:
                    sniff_use_this(arglex_value.alv_string);
                    continue;

                case arglex_token_stdio:
                    sniff_use_this_cut();
                    continue;
                }
                break;
            }
            continue;

        case arglex_token_include:
            switch (arglex())
            {
            default:
                arg_needs_string(arglex_token_include, usage);
                /* NOTREACHED */

            case arglex_token_string:
                sniff_include(arglex_value.alv_string);
                break;

            case arglex_token_stdio:
                sniff_include_cut();
                break;
            }
            break;

        case arglex_token_no_system:
            if (no_system)
                goto duplicate;
            no_system++;
            break;

        case arglex_token_cache:
            if (cache >= 0)
                goto duplicate;
            cache = 1;
            break;

        case arglex_token_cache_not:
            if (cache >= 0)
                goto duplicate;
            cache = 0;
            break;

        case arglex_token_no_source_relative_includes:
            if (option.no_src_rel_inc)
                goto duplicate;
            option.no_src_rel_inc++;
            break;

        case arglex_token_language:
            if (language)
                goto duplicate;
            if (arglex() != arglex_token_string)
            {
                arg_needs_string(arglex_token_language, usage);
                /* NOTREACHED */
            }
            language = lang_from_name(arglex_value.alv_string);
            break;

        case arglex_token_lang_c:
            if (language)
                goto duplicate;
            language = lang_from_name("c");
            break;

        case arglex_token_lang_roff:
            if (language)
                goto duplicate;
            language = lang_from_name("roff");
            break;

        case arglex_token_prefix:
            if (arglex() != arglex_token_string)
                arg_needs_string(arglex_token_prefix, usage);
            sniff_prefix_set(arglex_value.alv_string);
            break;

        case arglex_token_suffix:
            if (arglex() != arglex_token_string)
                arg_needs_string(arglex_token_suffix, usage);
            sniff_suffix_set(arglex_value.alv_string);
            break;

        case arglex_token_output:
            if (option.output)
                goto duplicate;
            if (arglex() != arglex_token_string)
                arg_needs_string(arglex_token_output, usage);
            option.output = arglex_value.alv_string;
            break;

        case arglex_token_remove_leading_path:
            if (arglex() != arglex_token_string)
            {
                arg_needs_string(arglex_token_remove_leading_path, usage);
            }
            sniff_remove_leading_path(arglex_value.alv_string);
            break;

        case arglex_token_substitute_leading_path:
            if (arglex() != arglex_token_string)
            {
                arg_needs_string(arglex_token_substitute_leading_path, usage);
            }
            arg1 = arglex_value.alv_string;
            if (arglex() != arglex_token_string)
            {
                arg_needs_string(arglex_token_substitute_leading_path, usage);
            }
            arg2 = arglex_value.alv_string;
            sniff_substitute_leading_path(arg1, arg2);
            break;

        case arglex_token_absolute:
            if (option.absolute != -1)
                goto duplicate;
            option.absolute = 1;
            break;

        case arglex_token_no_absolute:
            if (option.absolute != -1)
                goto duplicate;
            option.absolute = 0;
            break;

        case arglex_token_recursive:
            if (option.recursive != -1)
                goto duplicate;
            option.recursive = 1;
            break;

        case arglex_token_recursive_not:
            if (option.recursive != -1)
                goto duplicate;
            option.recursive = 0;
            break;

        case arglex_token_stripdot:
            if (option.stripdot != -1)
                goto duplicate;
            option.stripdot = 1;
            break;

        case arglex_token_stripdot_not:
            if (option.stripdot != -1)
                goto duplicate;
            option.stripdot = 0;
            break;

        case arglex_token_escape_newline:
            if (option.escape_newline)
                goto duplicate;
            option.escape_newline = 1;
            break;

        case arglex_token_quote_filenames:
            if (option.quote_filenames)
                goto duplicate;
            option.quote_filenames = 1;
            break;

        case arglex_token_exclude:
            arglex();
            if (arglex_token != arglex_token_string)
                arg_needs_string(arglex_token_exclude, usage);
            sniff_exclude(arglex_value.alv_string);
            break;
        }
        arglex();
    }
    if (option.o_absent_local == -1)
        option.o_absent_local = absent_mention;
    if (option.o_absent_system == -1)
        option.o_absent_system = absent_ignore;
    if (option.o_absent_program == -1)
        option.o_absent_program = absent_error;
    if (!source)
    {
        error_intl(0, i18n("no input file specified"));
        usage();
    }
    if (option.absolute < 0)
        option.absolute = !no_system;
    if (option.recursive == 0 && cache < 0)
        cache = 0;

    /*
     * set the language to be used
     */
    if (!language)
        language = lang_from_name("c");
    sniff_language(language);

    /*
     * apply any default or suffix search rules
     * or anything else defined by the language
     */
    if (!no_system)
        sniff_prepare();

    /*
     * read and analyze the file
     */
    if (cache)
        cache_read();
    sniff(source);
    if (cache)
        cache_write();
    exit(0);
    return 0;
}
