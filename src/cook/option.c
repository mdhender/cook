/*
 *      cook - file construction tool
 *      Copyright (C) 1993-2007 Peter Miller;
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
 *
 * The options may be set at various levels.  The level with the highest
 * precedence which has actually been set is used to determine the option
 * value at any given time.
 *
 * Each level of an option is represented by 2 bits in the flag word.  One bit
 * is used to indicate that the option has been set for that level, the other
 * bit indicates the state.  Determining the least set bit in an expression is
 * cheap (x&-x) so highest priority is the lowest numbered level.
 *
 * The COOK enviroment variable is basically a replacement for the defaults,
 * so that users can change the default behaviour.  The command line overrides
 * almost everything.  The error level is the only level with higher
 * precedence than the command line, and it is used to prevent disasters
 * after parse errors or interrupts have happened.
 *
 * -------------------------------------------------------------------------
 *
 *
 * If you are going to add a new recipe flag (set by the "set" statement,
 * or the "set" clause of a recipe) you need to change all of the
 * following places:
 *
 * cook/option.h
 *     to define the OPTION_ value
 * cook/option.c
 *     option_tidy_up()
 *         if the option defaults to true
 *     option_set_errors()
 *         if the option should be turned off once cookbook errors
 *         are encountered.
 *     option_number_name()
 *         for the name of the option
 * cook/flag.h
 *     to define the RF_ values (RF stands for Recipe Flag)
 * cook/flag.c
 *     to define the RF_ names
 * langu.flags.so
 *     to document the recipe flag
 *
 * If you choose to make it a command line option,
 * you must also update these files:
 *
 * cook/main.c
 *     to define the new command line option and process it
 *     (only if it should also be a command line option)
 * cook/builtin/options.c
 *     to access the option from within the cookbook (typically used
 *     for recursive cook invokations)
 * lib/en/man1/cook.1
 *     to document it, if you added a new command line option
 */

#include <common/ac/ctype.h>
#include <common/ac/limits.h>
#include <common/ac/stddef.h>
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>
#include <common/ac/time.h>

#include <common/libdir.h>
#include <common/mem.h>
#include <common/progname.h>
#include <common/trace.h>
#include <cook/option.h>
#include <cook/os_interface.h>


typedef struct flag_ty flag_ty;
struct flag_ty
{
    unsigned long   o_flag[OPTION_max];
};

option_ty       option;
static flag_ty  flag;


/*
 * NAME
 *      option_set - set an option
 *
 * SYNOPSIS
 *      void option_set(option_number_ty num, option_level_ty lvl, int state);
 *
 * DESCRIPTION
 *      The option_set function is used to set the given option at the given
 *      level to the given state.
 *
 * RETURNS
 *      void
 */

void
option_set(option_number_ty o, option_level_ty level, int state)
{
    trace(("option_set(o = %s, level = %s, state = %d)\n{\n",
        option_number_name(o), option_level_name(level), state));
    assert((int)o >= 0 && (int)o < (int)OPTION_max);
    flag.o_flag[(size_t) o] &= ~(3L << (2 * (int)level));
    if (state)
        flag.o_flag[(size_t) o] |= 3L << (2 * (int)level);
    else
        flag.o_flag[(size_t) o] |= 1L << (2 * (int)level);
    trace(("}\n"));
}


/*
 * NAME
 *      option_already - see if an option is already set
 *
 * SYNOPSIS
 *      int option_already(option_number_ty num, option_level_ty lvl);
 *
 * DESCRIPTION
 *      The option_already function is used to test if a given option at a
 *      given level has been set.
 *
 * RETURNS
 *      int: zero if the option has not been set, nonzero if it has.
 */

int
option_already(option_number_ty o, option_level_ty level)
{
    int             result;

    trace(("option_already(o = %s, level = %s)\n{\n",
            option_number_name(o), option_level_name(level)));
    assert((int)o >= 0 && (int)o < (int)OPTION_max);
    result = (((flag.o_flag[(size_t) o] >> (2 * (int)level)) & 1) != 0);
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}


/*
 * NAME
 *      option_undo - remove option setting
 *
 * SYNOPSIS
 *      void option_undo(option_number_ty num, option_level_ty lvl);
 *
 * DESCRIPTION
 *      The option_undo function is used to is used to remove the option
 *      setting for the given option at the given level.
 *
 * RETURNS
 *      void
 */

void
option_undo(option_number_ty o, option_level_ty level)
{
    trace(("option_undo(o = %s, level = %s)\n{\n", option_number_name(o),
        option_level_name(level)));
    assert((int)o >= 0 && (int)o < (int)OPTION_max);
    flag.o_flag[(size_t) o] &= ~(3L << (2 * (int)level));
    trace(("}\n"));
}


/*
 * NAME
 *      option_undo_level - remove options settings
 *
 * SYNOPSIS
 *      void option_undo_level(option_level_ty lvl);
 *
 * DESCRIPTION
 *      The option_undo_level function is used to remove the settings for all
 *      options at a given level.
 *
 * RETURNS
 *      void
 */

void
option_undo_level(option_level_ty level)
{
    int             o;

    trace(("option_undo_level(level = %s)\n{\n", option_level_name(level)));
    for (o = 0; o < (int)OPTION_max; ++o)
        option_undo((option_number_ty) o, level);
    trace(("}\n"));
}


/*
 * NAME
 *      option_test - test an option
 *
 * SYNOPSIS
 *      int option_test(option_number_ty num);
 *
 * DESCRIPTION
 *      The option_test function is used to test the setting of an option.
 *      The level of highest precedence which hash been set is used.
 *
 * RETURNS
 *      int: zero if the option is off, nonzero if the option is on.
 */

int
option_test(option_number_ty o)
{
    unsigned long   *op;
    unsigned long   mask;
    int             result;

    trace(("option_test(o = %s)\n{\n", option_number_name(o)));
    assert((int)o >= 0 && (int)o < (int)OPTION_max);
    op = &flag.o_flag[(size_t) o];
    mask = *op & 0x55555555;
    mask &= -mask; /* get LSB */
    result = ((*op & (mask << 1)) != 0);
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}


static string_ty *
Capitalize(string_ty *s)
{
    if (s->str_length < 1 || !islower((unsigned char)s->str_text[0]))
        return str_copy(s);
    return
        str_format
        (
            "%c%s",
            toupper((unsigned char)s->str_text[0]),
            s->str_text + 1
        );
}


/*
 * NAME
 *      option_tidy_up - mother hen
 *
 * SYNOPSIS
 *      void option_tidy_up(void);
 *
 * DESCRIPTION
 *      The option_tidy_up function is used to set a few defaults, and tidy up
 *      after the command line.
 *
 * RETURNS
 *      void
 *
 * CAVEAT
 *      Must be called after the command line has been parsed.
 */

void
option_tidy_up(void)
{
    string_ty       *s;
    string_ty       *s1;

    /*
     * Set the defaults before we do anything else,
     * the rest of tidy_up depends on them.
     */
    trace(("option_tidy_up()\n{\n"));
    option_set(OPTION_ACTION, OPTION_LEVEL_DEFAULT, 1);
    option_set(OPTION_CASCADE, OPTION_LEVEL_DEFAULT, 1);
    option_set(OPTION_CTIME, OPTION_LEVEL_DEFAULT, 1);
    option_set(OPTION_FINGERPRINT_WRITE, OPTION_LEVEL_DEFAULT, 1);
    option_set(OPTION_INCLUDE_COOKED, OPTION_LEVEL_DEFAULT, 1);
    option_set(OPTION_INCLUDE_COOKED_WARNING, OPTION_LEVEL_DEFAULT, 1);
    option_set(OPTION_LOGGING, OPTION_LEVEL_DEFAULT, 1);
    option_set(OPTION_STRIP_DOT, OPTION_LEVEL_DEFAULT, 1);
    option_set(OPTION_TERMINAL, OPTION_LEVEL_DEFAULT, 1);

    /*
     * user's library
     */
    s = os_accdir();
    assert(s);
    s1 = str_format("%s/.%s", s->str_text, progname_get());
    str_free(s);
    string_list_append_unique(&option.o_search_path, s1);
    str_free(s1);

    /*
     * cook's library
     *      architecture-specific, then architecture-neutral
     */
    s = str_from_c(library_directory_get());
    string_list_append_unique(&option.o_search_path, s);
    str_free(s);
    s = str_from_c(data_directory_get());
    string_list_append_unique(&option.o_search_path, s);
    str_free(s);

    if (!option.o_book)
    {
        static char *name[] =
        {
            ".how.to.%s",
            ".howto.%s",
            "how.to.%s",
            "howto.%s",
            "%s.file",
            "%sfile",
            "%s.book",
            "%sbook",
            ".%s.rc",
            ".%src",
        };
        size_t          j;

        /*
         * A huge range of alternative default names is given.
         * The first found will be used.
         */
        for (j = 0; j < SIZEOF(name); j++)
        {
            s = str_format(name[j], progname_get());
            switch (os_exists(s))
            {
            case -1:
                exit(1);

            case 0:
                s1 = Capitalize(s);
                str_free(s);
                s = s1;
                switch (os_exists(s))
                {
                case -1:
                    exit(1);

                case 0:
                    str_free(s);
                    continue;

                case 1:
                    option.o_book = s;
                    break;
                }
                break;

            case 1:
                option.o_book = s;
                break;
            }
            break;
        }
    }

    if (!option.o_logfile && option.o_book)
    {
        char            *sp;
        char            *cp;

        sp = option.o_book->str_text;
        /* skip first char in case it's a '.' */
        cp = strrchr(sp + 1, '.');
        if (cp)
            s = str_n_from_c(sp, cp - sp);
        else
            s = str_copy(option.o_book);
        sp = (option_test(OPTION_CMDFILE) ? "sh" : "list");
        option.o_logfile = str_format("%s.%s", s->str_text, sp);
        str_free(s);
    }
    trace(("}\n"));
}


/*
 * NAME
 *      option_set_errors - set error flags
 *
 * SYNOPSIS
 *      void option_set_errors(void);
 *
 * DESCRIPTION
 *      The option_set_errors function is used to set the appropriate options
 *      to prevent undesirable side effects when errors occur.
 *
 * RETURNS
 *      void
 */

void
option_set_errors(void)
{
    trace(("option_set_errors()\n{\n"));
    option_set(OPTION_SILENT, OPTION_LEVEL_ERROR, 0);
    option_set(OPTION_ACTION, OPTION_LEVEL_ERROR, 0);
    option_set(OPTION_ERROK, OPTION_LEVEL_ERROR, 0);
    option_set(OPTION_METER, OPTION_LEVEL_ERROR, 0);
    option_set(OPTION_PERSEVERE, OPTION_LEVEL_ERROR, 0);
    trace(("}\n"));
}


void *
option_flag_state_get(void)
{
    flag_ty         *fp;

    fp = mem_alloc(sizeof(flag_ty));
    *fp = flag;
    return fp;
}


void
option_flag_state_set(void *p)
{
    flag_ty         *fp;

    fp = p;
    flag = *fp;
    mem_free(p);
}


const char *
option_level_name(option_level_ty lvl)
{
    switch (lvl)
    {
    case OPTION_LEVEL_ERROR:
        return "OPTION_LEVEL_ERROR";

    case OPTION_LEVEL_AUTO:
        return "OPTION_LEVEL_AUTO";

    case OPTION_LEVEL_COMMAND_LINE:
        return "OPTION_LEVEL_COMMAND_LINE";

    case OPTION_LEVEL_EXECUTE:
        return "OPTION_LEVEL_EXECUTE";

    case OPTION_LEVEL_RECIPE:
        return "OPTION_LEVEL_RECIPE";

    case OPTION_LEVEL_COOKBOOK:
        return "OPTION_LEVEL_COOKBOOK";

    case OPTION_LEVEL_ENVIRONMENT:
        return "OPTION_LEVEL_ENVIRONMENT";

    case OPTION_LEVEL_DEFAULT:
        return "OPTION_LEVEL_DEFAULT";
    }
    return "option level unknown";
}


const char *
option_number_name(option_number_ty o)
{
    switch (o)
    {
    case OPTION_ACTION:
        return "OPTION_ACTION";

    case OPTION_BOOK:
        return "OPTION_BOOK";

    case OPTION_CASCADE:
        return "OPTION_CASCADE";

    case OPTION_CMDFILE:
        return "OPTION_CMDFILE";

    case OPTION_CTIME:
        return "OPTION_CTIME";

    case OPTION_DISASSEMBLE:
        return "OPTION_DISASSEMBLE";

    case OPTION_ERROK:
        return "OPTION_ERROK";

    case OPTION_FINGERPRINT:
        return "OPTION_FINGERPRINT";

    case OPTION_FINGERPRINT_WRITE:
        return "OPTION_FINGERPRINT_WRITE";

    case OPTION_FORCE:
        return "OPTION_FORCE";

    case OPTION_GATEFIRST:
        return "OPTION_GATEFIRST";

    case OPTION_IMPLICIT_ALLOWED:
        return "OPTION_IMPLICIT_ALLOWED";

    case OPTION_INCLUDE_COOKED:
        return "OPTION_INCLUDE_COOKED";

    case OPTION_INCLUDE_COOKED_WARNING:
        return "OPTION_INCLUDE_COOKED_WARNING";

    case OPTION_INGREDIENTS_FINGERPRINT:
        return "OPTION_INGREDIENTS_FINGERPRINT";

    case OPTION_INVALIDATE_STAT_CACHE:
        return "OPTION_INVALIDATE_STAT_CACHE";

    case OPTION_LOGGING:
        return "OPTION_LOGGING";

    case OPTION_METER:
        return "OPTION_METER";

    case OPTION_MKDIR:
        return "OPTION_MKDIR";

    case OPTION_PERSEVERE:
        return "OPTION_PERSEVERE";

    case OPTION_PRECIOUS:
        return "OPTION_PRECIOUS";

    case OPTION_REASON:
        return "OPTION_REASON";

    case OPTION_RECURSE:
        return "OPTION_RECURSE";

    case OPTION_SHALLOW:
        return "OPTION_SHALLOW";

    case OPTION_SILENT:
        return "OPTION_SILENT";

    case OPTION_STAR:
        return "OPTION_STAR";

    case OPTION_STRIP_DOT:
        return "OPTION_STRIP_DOT";

    case OPTION_SYMLINK_INGREDIENTS:
        return "OPTION_SYMLINK_INGREDIENTS";

    case OPTION_TERMINAL:
        return "OPTION_TERMINAL";

    case OPTION_TOUCH:
        return "OPTION_TOUCH";

    case OPTION_UNLINK:
        return "OPTION_UNLINK";

    case OPTION_UPDATE:
        return "OPTION_UPDATE";

    case OPTION_UPDATE_MAX:
        return "OPTION_UPDATE_MAX";

    case OPTION_MATCH_MODE_REGEX:
        return "OPTION_MATCH_MODE_REGEX";

    case OPTION_TELL_POSITION:
        return "OPTION_TELL_POSITION";

    case OPTION_max:
        break;
    }
    return "option number unknown";
}
