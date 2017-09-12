/*
 *	cook - file construction tool
 *	Copyright (C) 1993-1995, 1997-1999, 2001, 2002 Peter Miller;
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
 * MANIFEST: interface definition for c_incl/sniff.c
 */

#ifndef C_INCL_SNIFF_H
#define C_INCL_SNIFF_H

#include <main.h>

enum
{
	absent_ignore,
	absent_mention,
	absent_error
};

typedef struct option_ty option_ty;
struct option_ty
{
	int	o_verbose;
	int	o_absent_local;
	int	o_absent_system;
	int	o_absent_program;
	int	no_src_rel_inc;
	int	absolute;
	int	recursive;
	int	stripdot;
	int	escape_newline;
	int	quote_filenames;
	char	*output;
};
extern option_ty option;

struct input_ty;
struct string_list_ty;

typedef struct sniff_ty sniff_ty;
struct sniff_ty
{
	int (*scan)_((struct input_ty *, struct string_list_ty *,
		struct string_list_ty *));
	void (*prepare)_((void));
};

void sniff _((char *));
void sniff_include _((char *));
void sniff_include_cut _((void));
long sniff_include_count _((void));
void sniff_use_this _((char *));
void sniff_use_this_cut _((void));
long sniff_use_this_count _((void));
void sniff_prepare _((void));
void sniff_language _((sniff_ty *));

void sniff_remove_leading_path _((char *));
void sniff_substitute_leading_path _((char *, char *));

int absolute_filename_test _((char *));

void sniff_prefix_set _((char *));
void sniff_suffix_set _((char *));
void sniff_exclude _((char *));

#endif /* C_INCL_SNIFF_H */
