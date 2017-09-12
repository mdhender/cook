/*
 *	cook - file construction tool
 *	Copyright (C) 1999, 2001 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to find the home directory
 */

#include <ac/stddef.h>
#include <ac/stdlib.h>
#include <ac/unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include <env.h>
#include <home_directo.h>
#include <mem.h>


const char *
home_directory()
{
	static const char *result;
	struct passwd	*pw;

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
