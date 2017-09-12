#!/bin/sh
#
#	cook - file construction tool
#	Copyright (C) 1999, 2001, 2003 Peter Miller;
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
# MANIFEST: Test the fingerprint per-directory functionality
#
work=${COOK_TMP:-/tmp}/$$
PAGER=cat
export PAGER
umask 022
unset COOK
here=`pwd`
if test $? -ne 0 ; then exit 1; fi

bin="$here/${1-.}/bin"

no_result()
{
	set +x
   echo 'NO RESULT for test of the fingerprint per-directory functionality' 1>&2
	cd $here
	rm -rf $work
	exit 2
}
fail()
{
	set +x
	echo 'FAILED test of the fingerprint per-directory functionality' 1>&2
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
trap "no_result" 1 2 3 15

mkdir $work $work/lib
if test $? -ne 0 ; then exit 2; fi
cd $work
if test $? -ne 0 ; then no_result; fi

#
# Use the default error messages.  There is no other way to get
# predictable test behaviour on the unknown systems we will be tested on.
#
COOK_MESSAGE_LIBRARY=$work/no-such-dir
export COOK_MESSAGE_LIBRARY
unset LANG

#
# this test is to see of the file "bar" is correctly fingerprinted.
# The second time the recipe is applied, it will fail.
#
cat > Howto.cook << 'fubar'
set time-adjust;

test: foo;

foo: bar/bar
{
	if [exists foo] then
	{
		fail "This recipe should only be run when foo does not exist, \
but if foo exists it has the same fingerprint so the test fails.";
	}
	cp [need] [target];
}
fubar
if test $? -ne 0 ; then no_result; fi

mkdir bar
if test $? -ne 0 ; then no_result; fi
echo "hello" > bar/bar
if test $? -ne 0 ; then no_result; fi

# let the clock tick over
sleep 2

$bin/cook -nl -fp > LOG 2>&1
if test $? -ne 0 ; then cat LOG; fail; fi

# the fingerprint file must exist at this point
if test ! -r .cook.fp; then fail; fi
if test ! -r bar/.cook.fp; then fail; fi

# wait for clock to tick over
sleep 2

echo "hello" > bar/bar
if test $? -ne 0 ; then no_result; fi

$bin/cook -nl -fp > LOG 2>&1
if test $? -ne 0 ; then cat LOG; fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
