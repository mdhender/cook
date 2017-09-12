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
# MANIFEST: Test the leaf-explicit functionality
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
	echo 'FAILED test of the leaf-explicit functionality' 1>&2
	cd $here
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo 'NO RESULT for test of the leaf-explicit functionality' 1>&2
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
# test the leaf-explicit functionality
#
# This initial test should pass.
# It establishes what the default functionality does.
#
cat > Howto.cook << 'fubar'
test: a.o;
%.o: %.c { date > [target]; }
fubar
if test $? -ne 0 ; then no_result; fi

date > a.o
if test $? -ne 0 ; then no_result; fi

$bin/cook -nl > LOG 2>&1
if test $? -ne 0 ; then fail; fi

#
# This test verifies the graph_leaf_file variable.
# It should fail.
#
cat > Howto.cook << 'fubar'
graph_leaf_file = a.c;
this-should-fail: a.o;
%.o: %.c { date > [target]; }
fubar
if test $? -ne 0 ; then no_result; fi

$bin/cook -nl > LOG 2>&1
if test $? -ne 1 ; then cat LOG; fail; fi

#
# This test verifies the graph_interior_file variable.
# It should fail.
#
cat > Howto.cook << 'fubar'
graph_interior_file = a.o;
this-should-fail: a.o;
%.o: %.c { date > [target]; }
fubar
if test $? -ne 0 ; then no_result; fi

$bin/cook -nl > LOG 2>&1
if test $? -ne 1 ; then cat LOG; fail; fi

#
# This test verifies the graph_leaf_pattern variable.
# It should fail.
#
cat > Howto.cook << 'fubar'
graph_leaf_pattern = %0%.c;
this-should-fail: a.o;
%.o: %.c { date > [target]; }
fubar
if test $? -ne 0 ; then no_result; fi

$bin/cook -nl > LOG 2>&1
if test $? -ne 1 ; then cat LOG; fail; fi

#
# This test verifies the graph_interior_pattern variable.
# It should fail.
#
cat > Howto.cook << 'fubar'
graph_interior_pattern = %.o;
this-should-fail: a.o;
%.o: %.c { date > [target]; }
fubar
if test $? -ne 0 ; then no_result; fi

$bin/cook -nl > LOG 2>&1
if test $? -ne 1 ; then cat LOG; fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
