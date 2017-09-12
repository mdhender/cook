/*
 *      cook - file construction tool
 *      Copyright (C) 1997-1999, 2001-2003, 2005-2007 Peter Miller;
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
 *     to define the RF_ names (see table[], below)
 *     AND define the RF_ to OPTION_ mapping (see flag_set_options(), below)
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

#include <common/error_intl.h>
#include <cook/expr/position.h>
#include <cook/flag.h>
#include <common/itab.h>
#include <common/mem.h>
#include <cook/option.h>
#include <common/str_list.h>
#include <common/symtab.h>
#include <common/trace.h>


static symtab_ty *flags_symtab;
static itab_ty *flags_itab;


typedef struct table_ty table_ty;
struct table_ty
{
    char            *name;
    flag_value_ty   value;
    flag_value_ty   opposite;
};

static table_ty table[] =
{
    { "cascade", RF_CASCADE, RF_CASCADE_OFF },
    { "no-cascade", RF_CASCADE_OFF, RF_CASCADE },
    { "nocascade", RF_CASCADE_OFF, RF_CASCADE },
    { "clearstat", RF_CLEARSTAT, RF_CLEARSTAT_OFF },
    { "no-clearstat", RF_CLEARSTAT_OFF, RF_CLEARSTAT },
    { "noclearstat", RF_CLEARSTAT_OFF, RF_CLEARSTAT },
    { "default", RF_DEFAULT, RF_DEFAULT_OFF },
    { "no-default", RF_DEFAULT_OFF, RF_DEFAULT },
    { "nodefault", RF_DEFAULT_OFF, RF_DEFAULT },
    { "ignore-error", RF_ERROK, RF_ERROK_OFF },
    { "errok", RF_ERROK, RF_ERROK_OFF },
    { "no-ignore-error", RF_ERROK_OFF, RF_ERROK },
    { "no-errok", RF_ERROK_OFF, RF_ERROK },
    { "noerrok", RF_ERROK_OFF, RF_ERROK },
    { "fingerprint", RF_FINGERPRINT, RF_FINGERPRINT_OFF },
    { "fingerprints", RF_FINGERPRINT, RF_FINGERPRINT_OFF },
    { "fingerprinting", RF_FINGERPRINT, RF_FINGERPRINT_OFF },
    { "no-fingerprint", RF_FINGERPRINT_OFF, RF_FINGERPRINT },
    { "no-fingerprints", RF_FINGERPRINT_OFF, RF_FINGERPRINT },
    { "nofingerprint", RF_FINGERPRINT_OFF, RF_FINGERPRINT },
    { "no-fingerprinting", RF_FINGERPRINT_OFF, RF_FINGERPRINT },
    { "nofingerprinting", RF_FINGERPRINT_OFF, RF_FINGERPRINT },
    { "fingerprint-nowrite", RF_FINGERPRINT_NOWRITE, RF_FINGERPRINT_OFF },
    { "force", RF_FORCE, RF_FORCE_OFF },
    { "forced", RF_FORCE, RF_FORCE_OFF },
    { "no-force", RF_FORCE_OFF, RF_FORCE },
    { "noforce", RF_FORCE_OFF, RF_FORCE },
    { "no-forced", RF_FORCE_OFF, RF_FORCE },
    { "noforced", RF_FORCE_OFF, RF_FORCE },
    { "gate-before-ingredients", RF_GATEFIRST, RF_GATEFIRST_OFF },

    /* undocumented, for compatibility only */
    { "gate-first", RF_GATEFIRST, RF_GATEFIRST_OFF },
    /* undocumented, for compatibility only */
    { "no-gate-first", RF_GATEFIRST_OFF, RF_GATEFIRST },

    { "gate-after-ingredients", RF_GATEFIRST_OFF, RF_GATEFIRST },
    { "implicit-ingredients", RF_IMPLICIT_ALLOWED, RF_IMPLICIT_ALLOWED_OFF },
    { "explicit-ingredients", RF_IMPLICIT_ALLOWED_OFF, RF_IMPLICIT_ALLOWED },
    { "implicit-allowed", RF_IMPLICIT_ALLOWED, RF_IMPLICIT_ALLOWED_OFF },
    { "no-implicit-ingredients", RF_IMPLICIT_ALLOWED_OFF, RF_IMPLICIT_ALLOWED },
    { "no-implicit-allowed", RF_IMPLICIT_ALLOWED_OFF, RF_IMPLICIT_ALLOWED },
    { "explicit-required", RF_IMPLICIT_ALLOWED_OFF, RF_IMPLICIT_ALLOWED },
    { "include-cooked-warning", RF_INCLUDE_COOKED_WARNING,
        RF_INCLUDE_COOKED_WARNING_OFF },
    { "no-include-cooked-warning", RF_INCLUDE_COOKED_WARNING_OFF,
        RF_INCLUDE_COOKED_WARNING },
    { "ingredients-fingerprint", RF_INGREDIENTS_FINGERPRINT,
        RF_INGREDIENTS_FINGERPRINT_OFF },
    { "no-ingredients-fingerprint", RF_INGREDIENTS_FINGERPRINT_OFF,
        RF_INGREDIENTS_FINGERPRINT },
    { "match-mode-cook", RF_MATCH_MODE_COOK, RF_MATCH_MODE_REGEX },
    { "match-mode-regex", RF_MATCH_MODE_REGEX, RF_MATCH_MODE_COOK },
    { "meter", RF_METER, RF_METER_OFF },
    { "no-meter", RF_METER_OFF, RF_METER },
    { "nometer", RF_METER_OFF, RF_METER },
    { "mkdir", RF_MKDIR, RF_MKDIR_OFF },
    { "no-mkdir", RF_MKDIR_OFF, RF_MKDIR },
    { "nomkdir", RF_MKDIR_OFF, RF_MKDIR },
    { "precious", RF_PRECIOUS, RF_PRECIOUS_OFF },
    { "no-precious", RF_PRECIOUS_OFF, RF_PRECIOUS },
    { "noprecious", RF_PRECIOUS_OFF, RF_PRECIOUS },
    { "recurse", RF_RECURSE, RF_RECURSE_OFF },
    { "no-recurse", RF_RECURSE_OFF, RF_RECURSE },
    { "norecurse", RF_RECURSE_OFF, RF_RECURSE },
    { "shallow", RF_SHALLOW, RF_SHALLOW_OFF },
    { "no-shallow", RF_SHALLOW_OFF, RF_SHALLOW },
    { "noshallow", RF_SHALLOW_OFF, RF_SHALLOW },
    { "silent", RF_SILENT, RF_SILENT_OFF },
    { "no-silent", RF_SILENT_OFF, RF_SILENT },
    { "nosilent", RF_SILENT_OFF, RF_SILENT },
    { "stripdot", RF_STRIPDOT, RF_STRIPDOT_OFF },
    { "no-stripdot", RF_STRIPDOT_OFF, RF_STRIPDOT },
    { "nostripdot", RF_STRIPDOT_OFF, RF_STRIPDOT },
    { "symlink-ingredients", RF_SYMLINK_INGREDIENTS,
        RF_SYMLINK_INGREDIENTS_OFF },
    { "symlinkingredients", RF_SYMLINK_INGREDIENTS,
        RF_SYMLINK_INGREDIENTS_OFF },
    { "no-symlink-ingredients", RF_SYMLINK_INGREDIENTS_OFF,
        RF_SYMLINK_INGREDIENTS },
    { "no-symlinkingredients", RF_SYMLINK_INGREDIENTS_OFF,
        RF_SYMLINK_INGREDIENTS },
    { "nosymlink-ingredients", RF_SYMLINK_INGREDIENTS_OFF,
        RF_SYMLINK_INGREDIENTS },
    { "nosymlinkingredients", RF_SYMLINK_INGREDIENTS_OFF,
        RF_SYMLINK_INGREDIENTS },
    { "unlink", RF_UNLINK, RF_UNLINK_OFF },
    { "no-unlink", RF_UNLINK_OFF, RF_UNLINK },
    { "nounlink", RF_UNLINK_OFF, RF_UNLINK },
    { "tell-position", RF_TELL_POSITION, RF_TELL_POSITION_OFF },
    { "no-tell-position", RF_TELL_POSITION_OFF, RF_TELL_POSITION },
    { "time-adjust", RF_UPDATE, RF_UPDATE_OFF },
    { "timeadjust", RF_UPDATE, RF_UPDATE_OFF },
    { "update", RF_UPDATE, RF_UPDATE_OFF },
    { "no-time-adjust", RF_UPDATE_OFF, RF_UPDATE },
    { "notimeadjust", RF_UPDATE_OFF, RF_UPDATE },
    { "no-update", RF_UPDATE_OFF, RF_UPDATE },
    { "noupdate", RF_UPDATE_OFF, RF_UPDATE },
    { "time-adjust-back", RF_UPDATE_MAX, RF_UPDATE_OFF },
};


flag_ty *
flag_new(void)
{
    flag_ty         *fp;
    size_t          j;

    fp = mem_alloc(sizeof(flag_ty));
    for (j = 0; j < SIZEOF(fp->flag); ++j)
        fp->flag[j] = 0;
    return fp;
}


void
flag_delete(flag_ty *fp)
{
    mem_free(fp);
}


void
flag_union(flag_ty *this, const flag_ty *that)
{
    size_t          j;

    for (j = 0; j < SIZEOF(this->flag); ++j)
        this->flag[j] |= that->flag[j];
}


flag_ty *
flag_recognize(const string_list_ty *slp, const expr_position_ty *pp)
{
    flag_ty         *fp;
    size_t          j;
    int             nerrs;

    trace(("flag_recognize()\n{\n"));
    if (!flags_symtab)
    {
        table_ty        *tp;

        flags_symtab = symtab_alloc(SIZEOF(table));
        flags_itab = itab_alloc(SIZEOF(table));
        for (tp = table; tp < ENDOF(table); ++tp)
        {
            string_ty       *s;

            s = str_from_c(tp->name);
            symtab_assign(flags_symtab, s, tp);
            str_free(s);

            if (!itab_query(flags_itab, tp->value))
                itab_assign(flags_itab, tp->value, tp);
        }
    }

    fp = flag_new();
    nerrs = 0;
    for (j = 0; j < slp->nstrings; ++j)
    {
        string_ty       *name;
        table_ty        *data;

        assert(flags_symtab);
        name = slp->string[j];
        data = symtab_query(flags_symtab, name);
        if (data)
        {
          set_it:
            if (fp->flag[data->value])
            {
                sub_context_ty  *scp;

                scp = sub_context_new();
                sub_var_set_string(scp, "Name", name);
                error_with_position
                    (pp, scp, i18n("flag \"$name\" set more than once"));
                sub_context_delete(scp);
                ++nerrs;
            }
            if (fp->flag[data->opposite])
            {
                table_ty        *odata;
                sub_context_ty  *scp;

                odata = itab_query(flags_itab, data->opposite);
                assert(odata);
                scp = sub_context_new();
                sub_var_set_string(scp, "Name1", name);
                if (odata)
                {
                    sub_var_set(scp, "Name2", "%s", odata->name);
                }
                else
                {
                    sub_var_set(scp, "Name2", "no-%s", name->str_text);
                }
                error_with_position
                    (pp, scp, i18n("flags \"$name1\" and \"$name2\" both set"));
                sub_context_delete(scp);
                ++nerrs;
            }
            fp->flag[data->value] = 1;

            /* special cases */
            if (data->value == RF_UPDATE_MAX)
                fp->flag[RF_UPDATE] = 1;
            if (data->value == RF_FINGERPRINT_NOWRITE)
                fp->flag[RF_FINGERPRINT] = 1;
        }
        else
        {
            string_ty       *other;
            sub_context_ty  *scp;

            ++nerrs;
            data = symtab_query_fuzzy(flags_symtab, name, &other);
            if (data)
            {
                scp = sub_context_new();
                sub_var_set_string(scp, "Name", name);
                sub_var_set_string(scp, "Guess", other);
                error_with_position
                (
                    pp,
                    scp,
                    i18n("flag \"$name\" not understood, closest is the "
                        "\"$guess\" flag")
                );
                sub_context_delete(scp);
                /* DO NOT str_free guess */
                goto set_it;
            }
            scp = sub_context_new();
            sub_var_set_string(scp, "Name", name);
            error_with_position(pp, scp, i18n("flag \"$name\" not understood"));
            sub_context_delete(scp);
        }
    }

    if (nerrs)
    {
        flag_delete(fp);
        fp = 0;
    }
    trace(("return %08lX;\n", (long)fp));
    trace(("}\n"));
    return fp;
}


/*
 * NAME
 *      flag_set_options - set them
 *
 * SYNOPSIS
 *      void cook_flags(int mask, option_levelk_ty level);
 *
 * DESCRIPTION
 *      The cook_flags function is used to take a flags variable and set the
 *      appropriate options at the given level.
 *
 * RETURNS
 *      void
 *
 * CAVEAT
 *      Use the option_undo_level function to remove the flag settings.
 */

void
flag_set_options(const flag_ty *fp, int level)
{
    trace(("flag_set_options(fp = 0x%08lX, level = %d)\n{\n", (long)fp, level));
    if (fp->flag[RF_CASCADE])
        option_set(OPTION_CASCADE, level, 1);
    if (fp->flag[RF_CASCADE_OFF])
        option_set(OPTION_CASCADE, level, 0);

    if (fp->flag[RF_CLEARSTAT])
        option_set(OPTION_INVALIDATE_STAT_CACHE, level, 1);
    if (fp->flag[RF_CLEARSTAT_OFF])
        option_set(OPTION_INVALIDATE_STAT_CACHE, level, 0);

    if (fp->flag[RF_ERROK])
        option_set(OPTION_ERROK, level, 1);
    if (fp->flag[RF_ERROK_OFF])
        option_set(OPTION_ERROK, level, 0);

    if (fp->flag[RF_FINGERPRINT])
        option_set(OPTION_FINGERPRINT, level, 1);
    if (fp->flag[RF_FINGERPRINT_NOWRITE])
        option_set(OPTION_FINGERPRINT_WRITE, level, 0);
    if (fp->flag[RF_FINGERPRINT_OFF])
        option_set(OPTION_FINGERPRINT, level, 0);

    if (fp->flag[RF_FORCE])
        option_set(OPTION_FORCE, level, 1);
    if (fp->flag[RF_FORCE_OFF])
        option_set(OPTION_FORCE, level, 0);

    if (fp->flag[RF_GATEFIRST])
        option_set(OPTION_GATEFIRST, level, 1);
    if (fp->flag[RF_GATEFIRST_OFF])
        option_set(OPTION_GATEFIRST, level, 0);

    if (fp->flag[RF_IMPLICIT_ALLOWED])
        option_set(OPTION_IMPLICIT_ALLOWED, level, 1);
    if (fp->flag[RF_IMPLICIT_ALLOWED_OFF])
        option_set(OPTION_IMPLICIT_ALLOWED, level, 0);

    if (fp->flag[RF_INCLUDE_COOKED_WARNING])
        option_set(OPTION_INCLUDE_COOKED_WARNING, level, 1);
    if (fp->flag[RF_INCLUDE_COOKED_WARNING_OFF])
        option_set(OPTION_INCLUDE_COOKED_WARNING, level, 0);

    if (fp->flag[RF_INGREDIENTS_FINGERPRINT])
        option_set(OPTION_INGREDIENTS_FINGERPRINT, level, 1);
    if (fp->flag[RF_INGREDIENTS_FINGERPRINT_OFF])
        option_set(OPTION_INGREDIENTS_FINGERPRINT, level, 0);

    if (fp->flag[RF_MATCH_MODE_REGEX])
        option_set(OPTION_MATCH_MODE_REGEX, level, 1);
    if (fp->flag[RF_MATCH_MODE_COOK])
        option_set(OPTION_MATCH_MODE_REGEX, level, 0);

    if (fp->flag[RF_METER])
        option_set(OPTION_METER, level, 1);
    if (fp->flag[RF_METER_OFF])
        option_set(OPTION_METER, level, 0);

    if (fp->flag[RF_MKDIR])
        option_set(OPTION_MKDIR, level, 1);
    if (fp->flag[RF_MKDIR_OFF])
        option_set(OPTION_MKDIR, level, 0);

    if (fp->flag[RF_PRECIOUS])
        option_set(OPTION_PRECIOUS, level, 1);
    if (fp->flag[RF_PRECIOUS_OFF])
        option_set(OPTION_PRECIOUS, level, 0);

    if (fp->flag[RF_SHALLOW])
        option_set(OPTION_SHALLOW, level, 1);
    if (fp->flag[RF_SHALLOW_OFF])
        option_set(OPTION_SHALLOW, level, 0);

    if (fp->flag[RF_SILENT])
        option_set(OPTION_SILENT, level, 1);
    if (fp->flag[RF_SILENT_OFF])
        option_set(OPTION_SILENT, level, 0);

    if (fp->flag[RF_STRIPDOT])
        option_set(OPTION_STRIP_DOT, level, 1);
    if (fp->flag[RF_STRIPDOT_OFF])
        option_set(OPTION_STRIP_DOT, level, 0);

    if (fp->flag[RF_SYMLINK_INGREDIENTS])
        option_set(OPTION_SYMLINK_INGREDIENTS, level, 1);
    if (fp->flag[RF_SYMLINK_INGREDIENTS_OFF])
        option_set(OPTION_SYMLINK_INGREDIENTS, level, 0);

    if (fp->flag[RF_UPDATE])
        option_set(OPTION_UPDATE, level, 1);
    if (fp->flag[RF_UPDATE_OFF])
        option_set(OPTION_UPDATE, level, 0);

    if (fp->flag[RF_UPDATE_MAX])
    {
        option_set(OPTION_UPDATE, level, 1);
        option_set(OPTION_UPDATE_MAX, level, 1);
    }

    if (fp->flag[RF_UNLINK])
        option_set(OPTION_UNLINK, level, 1);
    if (fp->flag[RF_UNLINK_OFF])
        option_set(OPTION_UNLINK, level, 0);

    if (fp->flag[RF_RECURSE])
        option_set(OPTION_RECURSE, level, 1);
    if (fp->flag[RF_RECURSE_OFF])
        option_set(OPTION_RECURSE, level, 0);

    if (fp->flag[RF_TELL_POSITION])
        option_set(OPTION_TELL_POSITION, level, 1);
    if (fp->flag[RF_TELL_POSITION_OFF])
        option_set(OPTION_TELL_POSITION, level, 0);
    trace(("}\n"));
}


int
flag_query(const flag_ty *fp, flag_value_ty n)
{
    assert((int)n >= 0);
    assert(n < RF_max);
    return fp->flag[n];
}


flag_ty *
flag_copy(const flag_ty *that)
{
    flag_ty         *this;
    size_t          j;

    this = mem_alloc(sizeof(flag_ty));
    for (j = 0; j < SIZEOF(this->flag); ++j)
        this->flag[j] = that->flag[j];
    return this;
}
