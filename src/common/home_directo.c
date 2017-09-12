/*
 *      cook - file construction tool
 *      Copyright (C) 1999, 2001, 2006, 2007 Peter Miller;
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

#include <common/ac/stddef.h>
#include <common/ac/stdlib.h>
#include <common/ac/unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include <common/env.h>
#include <common/home_directo.h>
#include <common/mem.h>


const char *
home_directory(void)
{
    static const char *result;
    struct passwd   *pw;

    /*
     * If we already worked this out,
     * just use the old answer.
     */
    if (result)
        return result;

    /*
     * If the HOME environment variable is set,
     * use that.
     */
    result = getenv("HOME");
    if (result)
        return result;

    /*
     * Otherwise, read the passwd file for our current user ID.
     * And if that fails, just use root.
     */
    setpwent();
    pw = getpwuid(geteuid());
    result = (pw && pw->pw_dir) ? mem_copy_string(pw->pw_dir) : "/";
    endpwent();

    /*
     * Export this HOME environment variable to child processes.
     */
    env_set("HOME", result);
    return result;
}
