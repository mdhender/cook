/*
 *      cook - file construction tool
 *      Copyright (C) 1997-1999, 2001, 2003, 2006-2009 Peter Miller
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
 * lib/en/user-guide/langu.flags.so
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

#ifndef COOK_FLAG_H
#define COOK_FLAG_H

#include <common/main.h>

struct expr_position_ty; /* existence */
struct string_list_ty; /* existence */

enum flag_value_ty
{
        RF_CASCADE,
        RF_CASCADE_OFF,
        RF_CLEARSTAT,
        RF_CLEARSTAT_OFF,
        RF_CTIME,
        RF_CTIME_OFF,
        RF_DEFAULT,
        RF_DEFAULT_OFF,
        RF_ERROK,
        RF_ERROK_OFF,
        RF_FILE_SIZE_STATS,
        RF_FILE_SIZE_STATS_OFF,
        RF_FINGERPRINT,
        RF_FINGERPRINT_NOWRITE,
        RF_FINGERPRINT_OFF,
        RF_FORCE,
        RF_FORCE_OFF,
        RF_GATEFIRST,
        RF_GATEFIRST_OFF,
        RF_IMPLICIT_ALLOWED,
        RF_IMPLICIT_ALLOWED_OFF,
        RF_INCLUDE_COOKED_WARNING,
        RF_INCLUDE_COOKED_WARNING_OFF,
        RF_INGREDIENTS_FINGERPRINT,
        RF_INGREDIENTS_FINGERPRINT_OFF,
        RF_MATCH_MODE_COOK,
        RF_MATCH_MODE_REGEX,
        RF_METER,
        RF_METER_OFF,
        RF_MKDIR,
        RF_MKDIR_OFF,
        RF_PRECIOUS,
        RF_PRECIOUS_OFF,
        RF_RECURSE,
        RF_RECURSE_OFF,
        RF_SHALLOW,
        RF_SHALLOW_OFF,
        RF_SILENT,
        RF_SILENT_OFF,
        RF_STAR,
        RF_STAR_OFF,
        RF_STRIPDOT,
        RF_STRIPDOT_OFF,
        RF_SYMLINK_INGREDIENTS,
        RF_SYMLINK_INGREDIENTS_OFF,
        RF_TELL_POSITION,
        RF_TELL_POSITION_OFF,
        RF_UNLINK,
        RF_UNLINK_OFF,
        RF_UPDATE,
        RF_UPDATE_MAX,
        RF_UPDATE_OFF,
        RF_max  /* MUST be last */
};
typedef enum flag_value_ty flag_value_ty;

typedef struct flag_ty flag_ty;
struct flag_ty
{
        unsigned char flag[RF_max];
};


flag_ty *flag_new(void);
flag_ty *flag_copy(const flag_ty *);
void flag_delete(flag_ty *);
void flag_union(flag_ty *, const flag_ty *);
flag_ty *flag_recognize(const struct string_list_ty *,
        const struct expr_position_ty *);
void flag_set_options(const flag_ty *, int);
int flag_query(const flag_ty *, flag_value_ty);

#endif /* COOK_FLAG_H */
