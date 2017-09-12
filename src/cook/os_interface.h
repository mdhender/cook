/*
 *      cook - file construction tool
 *      Copyright (C) 1993-1995, 1997-1999, 2001, 2004, 2006, 2007 Peter Miller;
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
 * This file was originally called cook/os.h, but <os.h> is a system
 * include file on some systems, and this caused portability problems.
 */

#ifndef COOK_OS_INTERFACE_H
#define COOK_OS_INTERFACE_H

#include <common/ac/time.h>
#include <common/str.h>
#include <common/str_list.h>

time_t os_mtime_oldest(string_ty *);
time_t os_mtime_newest(string_ty *);
int os_mtime_adjust(string_ty *, time_t);
int os_touch(string_ty *);

/**
  * The os_execute_magic_characters function is used to determine if the
  * given string contains any characters "magic" to the shell.
  *
  * \returns
  *     zero if there are no magic characters, non-zero if there is at
  *     least one magic character
  */
int os_execute_magic_characters(string_ty *);

/**
  * The os_execute_magic_characters_list function is used to determine if the
  * given list of string contains any characters "magic" to the shell.
  *
  * \returns
  *     zero if there are no magic characters, non-zero if there is at
  *     least one magic character
  */
int os_execute_magic_characters_list(string_list_ty *);

int os_execute(string_list_ty *cmd, string_ty *input, int errok);
int os_exists(string_ty *);
int os_exists_symlink(string_ty *);
int os_exists_dir(string_ty *);

/**
  * The os_executable function is used to test if the given path exists
  * and is executable.
  *
  * \param path
  *     The path to test for existence and executablity.
  * \returns
  *     1 if exists and is executable, 0 if noes not exist, -1 on any
  *     other error (the error has already been printed).
  */
int os_executable(string_ty *path);

string_ty *os_accdir(void);
string_ty *os_curdir(void);
string_ty *os_dirname(string_ty *);
string_ty *os_dirname_relative(string_ty *);
string_ty *os_entryname(string_ty *);
string_ty *os_pathname(string_ty *);
int os_legal_path(string_ty *);
int os_delete(string_ty *path, int echo);
int os_clear_stat(string_ty *);
int exit_status(char *cmd, int status, int errok);
int os_mkdir(string_ty *path, int echo);

/**
  * The os_symlink function may be used to create symbolic links.
  *
  * @param from
  *     The existing file which is to form the content of the symbolic link
  * @param to
  *     The new file to be created as a symbolic link.
  * @returns
  *     zero on success, -1 on failure
  */
int os_symlink(string_ty *from, string_ty *to, int echo);

#endif /* COOK_OS_INTERFACE_H */
