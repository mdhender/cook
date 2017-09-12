#!/bin/sh
#
#	cook - file construction tool
#	Copyright (C) 1998 Peter Miller;
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
# MANIFEST: Test the [expr] functionality
#
work=${COOK_TMP:-/tmp}/$$
PAGER=cat
export PAGER
umask 022
unset COOK
here=`pwd`
if test $? -ne 0 ; then exit 2; fi
SHELL=/bin/sh
export SHELL

bin="$here/${1-.}/bin"

pass()
{
	set +x
	cd $here
	rm -rf $work
	exit 0
}
fail()
{
	set +x
	echo 'FAILED test of the [expr] functionality' 1>&2
	cd $here
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo 'NO RESULT for test of the [expr] functionality' 1>&2
	cd $here
	rm -rf $work
	exit 2
}
trap \"no_result\" 1 2 3 15

mkdir $work $work/lib
if test $? -ne 0 ; then no_result; fi
cd $work
if test $? -ne 0 ; then no_result; fi

#
# Use the default error messages.  There is no other way to get
# predictable test behaviour on the unknown systems we will be tested on.
#
COOK_MESSAGE_LIBRARY=$work/no-such-dir
export COOK_MESSAGE_LIBRARY
unset LANG
unset LANGUAGE

#
# test the [expr] functionality
#
cat > Howto.cook << 'fubar'
test:
{
	function print [expr 1 + 0];
	function print [expr 6 / 3];
	function print [expr 10 % 7];
	function print [expr 2 * 2];
	function print [expr 2 * 2 + 1];
	function print [expr -3 * - 2];
	function print [expr ~ -8];
	function print [expr ! -8];
	function print [expr 3 << 2];
	function print [expr 4095 >> 3];
	function print [expr 0x56 < 56];
	function print [expr 0x56 "<=" 56];
	function print [expr 0x56 > 56];
	function print [expr 0x56 ">=" 56];
	function print [expr 0x56 "==" 56];
	function print [expr 0x56 "!=" 56];
	function print [expr 0xFF & 0x80];
	function print [expr 0x02 ^ 0x82];
	function print [expr 0x02 | 0x80];
	function print [expr 2 && 3];
	function print [expr 2 || 3];
	function print [expr 1 ? 2 ":" 3];
}
fubar
if test $? -ne 0 ; then no_result; fi

$bin/cook -nl > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi

cat > test.ok << 'fubar'
cook: 1
cook: 2
cook: 3
cook: 4
cook: 5
cook: 6
cook: 7
cook: 0
cook: 12
cook: 511
cook: 0
cook: 0
cook: 1
cook: 1
cook: 0
cook: 1
cook: 128
cook: 128
cook: 130
cook: 1
cook: 1
cook: 2
fubar
if test $? -ne 0 ; then no_result; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
