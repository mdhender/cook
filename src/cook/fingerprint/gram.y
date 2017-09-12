/*
 *	cook - file construction tool
 *	Copyright (C) 1994, 1995, 1997-1999, 2001, 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate the persistent fingerprint cache
 */

%{

#include <ac/stdlib.h>

#include <fingerprint/find.h>
#include <fingerprint/gram.h>
#include <fingerprint/lex.h>
#include <fingerprint/subdir.h>
#include <fingerprint/value.h>
#include <str.h>
#include <trace.h>

%}

%token	STRING
%token	JUNK
%token	NUMBER
%token	EQ
%token	LB
%token	RB

%union
{
	string_ty	*lv_string;
	long		lv_number;
	struct
	{
		long lhs;
		long rhs;
		long stat_mod_time;
	}
			lv_number_set;
	struct
	{
		string_ty *lhs;
		string_ty *rhs;
	}
			lv_string_pair;
}

%type	<lv_string>	STRING
%type	<lv_number>	NUMBER
%type	<lv_number_set> number_set
%type	<lv_string_pair> string_pair

%{

static fp_subdir_ty *subdir;


/*
 * NAME
 *	fp_gram
 *
 * SYNOPSIS
 *	void fp_gram(fp_subdir_ty *sdp, string_ty *filename);
 *
 * DESCRIPTION
 *	The fp_gram function is used to read the fingerprint cache of a
 *	directory.  Fingerprints are remembered ralative to the directory
 *	they are stored in, so recursive cook usage and search_list Cook
 *	usage, are all able to take advantage of the fingerprint caches.
 *
 *	The fp_find_update function is called by the grammar parser,
 *	to update both the specified sub-directory structure, but also
 *	the master symbol table.  (The master symbol table permits O(1)
 *	access to allpaths, once they are known.)
 */

void
fp_gram(sdp, filename)
	fp_subdir_ty	*sdp;
	string_ty	*filename;
{
	extern int	yyparse _((void));

	subdir = sdp;

	fingerprint_lex_open(filename);
	yyparse();
	fingerprint_lex_close();

	subdir = 0;
}

%}

%%

cache
	: /* empty */
	| cache entry
	;

entry
	: STRING EQ LB number_set string_pair RB
		{
			fp_value_ty	data;

			fp_value_constructor5
			(
				&data,
				$4.lhs,
				$4.rhs,
				$4.stat_mod_time,
				$5.lhs,
				$5.rhs
			);

			str_free($5.lhs);
			if ($5.rhs)
				str_free($5.rhs);

			fp_find_update(subdir, $1, &data);
			str_free($1);
			fp_value_destructor(&data);
		}
	| error
	;

number_set
	: NUMBER
		{
			$$.lhs = $1;
			$$.rhs = $1;
			$$.stat_mod_time = $1;
		}
	| NUMBER NUMBER
		{
			$$.lhs = $1;
			$$.rhs = $2;
			$$.stat_mod_time = $2;
		}
	| NUMBER NUMBER NUMBER
		{
			$$.lhs = $1;
			$$.rhs = $2;
			$$.stat_mod_time = $3;
		}
	;

string_pair
	: STRING
		{ $$.lhs = $1; $$.rhs = 0; }
	| STRING STRING
		{ $$.lhs = $1; $$.rhs = $2; }
	;
