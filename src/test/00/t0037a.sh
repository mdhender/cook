#!/bin/sh
#
#	cook - file construction tool
#	Copyright (C) 1990, 1991, 1992, 1993, 1994, 1997, 1998 Peter Miller;
#	All rights reserved.
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 2 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program; if not, write to the Free Software
#	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
#
# MANIFEST: test the c_incl -no_cache option
#
tmp=${COOK_TMP:-/tmp}/$$
PAGER=cat
export PAGER
umask 022
unset COOK
here=`pwd`
if test $? -ne 0 ; then exit 1; fi

bin="$here/${1-.}/bin"

fail()
{
	echo 'FAILED test of the c_incl -No_Cache option' 1>&2
	cd $here
	rm -rf $tmp
	exit 1
}
pass()
{
	cd $here
	rm -rf $tmp
	exit 0
}
trap "fail" 1 2 3 15

mkdir $tmp $tmp/lib
if test $? -ne 0 ; then exit 1; fi
cd $tmp
if test $? -ne 0 ; then fail; fi

#
# Use the default error messages.  There is no other way to get
# predictable test behaviour on the unknown systems we will be tested on.
#
COOK_MESSAGE_LIBRARY=$work/no-such-dir
export COOK_MESSAGE_LIBRARY
unset LANG

cat > kees.h << 'foobar'
/* just a simple test */
foobar
if test $? -ne 0 ; then fail; fi

cat > main.c << 'foobar'
#include <stdio.h>
#include "kees.h"

void
main()
{
	printf("Hello World!\n");
	exit(0);
}
foobar
if test $? -ne 0 ; then fail; fi

$bin/c_incl main.c > test.tmp
if test $? -ne 0 ; then fail; fi

grep kees test.tmp > test.out
if test $? -ne 0 ; then fail; fi

cat > test.ok << 'foobar'
kees.h
foobar
if test $? -ne 0 ; then fail; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

$bin/c_incl -nc -ns main.c > test.out
if test $? -ne 0 ; then fail; fi

cat > test.ok << 'foobar'
kees.h
foobar
if test $? -ne 0 ; then fail; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi


# probably OK
pass
