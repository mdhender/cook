/*
 *      cook - file construction tool
 *      Copyright (C) 1999, 2006-2008 Peter Miller
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

#include <cook/match/new_by_recip.h>
#include <cook/option.h>
#include <cook/recipe.h>


match_ty *
match_new_by_recipe(recipe_ty *rp)
{
    void            *hold;
    match_ty        *mp;

    hold = option_flag_state_get();
    if (rp)
        recipe_flags_set(rp);
    else
        option_undo_level(OPTION_LEVEL_RECIPE);
    mp = match_new();
    option_flag_state_set(hold);
    return mp;
}
