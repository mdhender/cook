/*
 *	cook - a program construction tool
 *	Copyright (C) 1991, 1992, 1993, 1994, 1997, 1998 Peter Miller;
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
 * MANIFEST: interface definition for comon/help.c
 */

#ifndef HELP_H
#define HELP_H

#include <main.h>

void help _((char *, void (*)(void)));
void generic_argument _((void(*)(void)));
void bad_argument _((void(*)(void)));

void arg_duplicate _((int, void(*)(void)));
void arg_duplicate_cur _((void(*)(void)));
void arg_needs_string _((int, void(*)(void)));
void arg_needs_number _((int, void(*)(void)));

#endif /* HELP_H */
