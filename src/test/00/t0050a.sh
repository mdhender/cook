#!/bin/sh
#
#	cook - file construction tool
#	Copyright (C) 1994, 1997, 1998, 2003 Peter Miller;
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
# MANIFEST: Test the include-cooked functionality
#
work=${COOK_TMP:-/tmp}/$$
PAGER=cat
export PAGER
umask 022
unset COOK
here=`pwd`
if test $? -ne 0 ; then exit 1; fi

SHELL=/bin/sh
export SHELL

bin="$here/${1-.}/bin"

fail()
{
	set +x
	echo 'FAILED test of the include-cooked functionality' 1>&2
	cd $here
	rm -rf $work
	exit 1
}
pass()
{
	set +x
	cd $here
	rm -rf $work
	exit 0
}
trap \"fail\" 1 2 3 15

mkdir $work $work/lib
if test $? -ne 0 ; then exit 1; fi
cd $work
if test $? -ne 0 ; then fail; fi

#
# Use the default error messages.  There is no other way to get
# predictable test behaviour on the unknown systems we will be tested on.
#
COOK_MESSAGE_LIBRARY=$work/no-such-dir
export COOK_MESSAGE_LIBRARY
unset LANG

PATH=${bin}:${PATH}
export PATH

#
# use clever C include dependencies
#
cat > Howto.cook << 'fubar'
cc = [getenv CC]; if [not [cc]] then
{ cc = [find_command gcc]; if [not [cc]] then cc = cc; }

obj = [fromto %.c %.o [glob *.c]];

test.out: test
{
	./test > test.out;
}

%.o: %.c
{
	[cc] -c %.c;
}

%.d: %.c
{
	c_incl -nc -ns -eia %.c -prefix "'%.o %.d: %.c'" -suffix "';'" > %.d;
}

#include-cooked [fromto %.o %.d [obj]]

test: [obj]
{
	[cc] -o test [obj];
}
fubar
if test $? -ne 0 ; then fail; fi

cat > test.c << 'fubar'
#include "guts.h"
test()
{
	printf("%s\n", GUTS);
}
fubar
if test $? -ne 0 ; then fail; fi

cat > test.h << 'fubar'
void test();
fubar
if test $? -ne 0 ; then fail; fi

cat > guts.h << 'fubar'
#define GUTS "wrong"
fubar
if test $? -ne 0 ; then fail; fi

cat > main.c << 'fubar'
#include "test.h"
main()
{
	test();
	exit(0);
}
fubar
if test $? -ne 0 ; then fail; fi

sleep 2

cook -nl > /dev/null 2>&1
if test $? -ne 0 ; then fail; fi

# let the clock tick over
sleep 2

cat > guts.h << 'fubar'
#define GUTS "right"
fubar
if test $? -ne 0 ; then fail; fi

cook -nl > /dev/null 2>&1
if test $? -ne 0 ; then fail; fi

cat > test.ok << 'fubar'
right
fubar
if test $? -ne 0 ; then fail; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
