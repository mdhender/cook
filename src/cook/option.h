/*
 *      cook - file construction tool
 *      Copyright (C) 1993-1997, 1999, 2001, 2003, 2006, 2007 Peter Miller;
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
 *
 * If you are going to add a new recipe flag (set by the "set" statement,
 * or the "set" clause of a recipe) you need to change all of the
 * following places:
 *
 * cook/option.h
 *     to define the OPTION_ value
 * cook/option.c
 *     option_tidyup()
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

#ifndef OPTION_H
#define OPTION_H

#include <common/main.h>

#include <common/str.h>
#include <common/str_list.h>

/*
 * option levels, highest to lowest
 * (room for 16 levels in a 32 bit unsigned)
 */
enum option_level_ty
{
        OPTION_LEVEL_ERROR,
        OPTION_LEVEL_AUTO,
        OPTION_LEVEL_COMMAND_LINE,
        OPTION_LEVEL_EXECUTE,
        OPTION_LEVEL_RECIPE,
        OPTION_LEVEL_COOKBOOK,
        OPTION_LEVEL_ENVIRONMENT,
        OPTION_LEVEL_DEFAULT
};
typedef enum option_level_ty option_level_ty;

enum option_number_ty
{
        OPTION_ACTION,          /* do not execute the command */
        OPTION_BOOK,
        OPTION_CASCADE,         /* do (not) cascade ingredients */
        OPTION_CMDFILE,         /* generate a command file */
        OPTION_DISASSEMBLE,     /* undocumented: disassemble opcode lists after
                                   compilation */
        OPTION_ERROK,           /* ignore error returns from commands */
        OPTION_FINGERPRINT,     /* remember file fingerprints */
        OPTION_FINGERPRINT_WRITE, /* preserve fingerprints if taken */
        OPTION_FORCE,           /* always execute the commands */
        OPTION_GATEFIRST,       /* check the gate conditions on a recipe before
                                   evaluating the ingredients */
        OPTION_IMPLICIT_ALLOWED, /* implicit recipes may be used */
        OPTION_INCLUDE_COOKED,  /* cook the include-cooked include files */
        OPTION_INCLUDE_COOKED_WARNING,  /* warn of include-cooked problems */
        OPTION_INGREDIENTS_FINGERPRINT, /* use ingredients fingerprints */
        OPTION_INVALIDATE_STAT_CACHE,
        OPTION_LOGGING,
        OPTION_METER,           /* meter each command */
        OPTION_MKDIR,           /* make directories of targets */
        OPTION_PERSEVERE,       /* keep trying if have errors */
        OPTION_PRECIOUS,        /* do not delete failed targets */
        OPTION_REASON,          /* emit inference debugging commentary */
        OPTION_RECURSE,         /* allow target recursion loops */
        OPTION_SHALLOW,         /* recipe targets are to be shallow on
                                   search_list */
        OPTION_SILENT,          /* do not echo any command */
        OPTION_STAR,            /* emit progress stars */
        OPTION_STRIP_DOT,       /* strip leading ./ from paths */
        OPTION_SYMLINK_INGREDIENTS, /* make symlink for non-top-level ingredi */
        OPTION_TERMINAL,        /* enable tty output when logging */
        OPTION_TOUCH,           /* do not execute the command, just touch */
        OPTION_UNLINK,          /* remove targets before running rule body */
        OPTION_UPDATE,          /* update utime for consistency */
        OPTION_UPDATE_MAX,      /* update utime for consistency - backwards! */
        OPTION_MATCH_MODE_REGEX, /* regex pattern matching (as opp native) */
        OPTION_TELL_POSITION,   /* add file and line when echoing commands */
        OPTION_CTIME,           /* use both st_ctime and st_mtime */

        /*
         * If you add to this list, make sure you also add the option to
         * the list in cook/builtin/options.c
         */
        OPTION_max
};
typedef enum option_number_ty option_number_ty;

typedef struct option_ty option_ty;
struct option_ty
{
        string_list_ty  o_target;
        string_ty       *o_book;
        string_ty       *o_logfile;
        string_list_ty  o_search_path;
        string_list_ty  o_vardef;
        int             pairs;
        int             script;
        int             web;
        int             fingerprint_update;
};

extern  option_ty       option;

int option_already(option_number_ty, option_level_ty);
int option_test(option_number_ty);
void option_set(option_number_ty, option_level_ty, int);
void option_undo(option_number_ty, option_level_ty);
void option_undo_level(option_level_ty);
void option_set_errors(void);
void option_tidy_up(void);

void *option_flag_state_get(void);
void option_flag_state_set(void *);

const char *option_number_name(option_number_ty);
const char *option_level_name(option_level_ty);

#endif /* OPTION_H */
