/*
 *	cook - file construction tool
 *	Copyright (C) 1993-1995, 1997-1999, 2001, 2004 Peter Miller;
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
 * MANIFEST: interface definition for cook/os.c
 *
 * This file was originally called cook/os.h, but <os.h> is a system
 * include file on some systems, and this caused portability problems.
 */

#ifndef COOK_OS_INTERFACE_H
#define COOK_OS_INTERFACE_H

#include <ac/time.h>
#include <str.h>
#include <str_list.h>

time_t os_mtime_oldest _((string_ty *));
time_t os_mtime_newest _((string_ty *));
int os_mtime_adjust _((string_ty *, time_t));
int os_touch _((string_ty *));

/**
  * The os_execute_magic_characters function is used to determine if the
  * given string contains any characters "magic" to the shell.
  *
  * \returns
  *     zero if there are no magic characters, non-zero if there is at
  *     least one magic character
  */
int os_execute_magic_characters _((string_ty *));

/**
  * The os_execute_magic_characters_list function is used to determine if the
  * given list of string contains any characters "magic" to the shell.
  *
  * \returns
  *     zero if there are no magic characters, non-zero if there is at
  *     least one magic character
  */
int os_execute_magic_characters_list _((string_list_ty *));

int os_execute _((string_list_ty *cmd, string_ty *input, int errok));
int os_exists _((string_ty *));
int os_exists_symlink _((string_ty *));
int os_exists_dir _((string_ty *));

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
int os_executable _((string_ty *path));

string_ty *os_accdir _((void));
string_ty *os_curdir _((void));
string_ty *os_dirname _((string_ty *));
string_ty *os_dirname_relative _((string_ty *));
string_ty *os_entryname _((string_ty *));
string_ty *os_pathname _((string_ty *));
int os_legal_path _((string_ty *));
int os_delete _((string_ty *path, int echo));
int os_clear_stat _((string_ty *));
int exit_status _((char *cmd, int status, int errok));
int os_mkdir _((string_ty *path, int echo));
string_ty *os_path_cat _((string_ty *, string_ty *));

#endif /* COOK_OS_INTERFACE_H */
