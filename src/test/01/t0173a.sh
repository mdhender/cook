#!/bin/sh
#
#	cook - file construction tool
#	Copyright (C) 1999 Peter Miller;
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
# MANIFEST: Test the regex-recipe-pattern functionality
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
	echo 'FAILED test of the regex-recipe-pattern functionality' 1>&2
	cd $here
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo 'NO RESULT for test of the regex-recipe-pattern functionality' 1>&2
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
# test the regex-recipe-pattern functionality
#
cat > howto.cook << 'fubar'
set match-mode-regex;
\\(.*\\)\\.o: \\1.c
{
	echo "targets =" [targets] > \\1.o;
	echo "need =" [need] >> \\1.o;
}

\\(.*\\)\\.c \\(.*\\)\\.h: \\1.y
{
	echo "targets =" [targets] > \\1.c;
	echo "need =" [need] >> \\1.c;
	date > \\1.h;
}

test: gram.o
{
	date > [target];
}
fubar
if test $? -ne 0 ; then no_result; fi

date > gram.y
if test $? -ne 0 ; then no_result; fi

$bin/cook -nl > LOG 2>&1
if test $? -ne 0 ; then cat LOG; fail; fi

test -f gram.c || fail
test -f gram.h || fail
test -f gram.o || fail
test -f test || fail

cat > ok << 'fubar'
targets = gram.c gram.h
need = gram.y
fubar
if test $? -ne 0 ; then no_result; fi

diff ok gram.c
if test $? -ne 0 ; then fail; fi

cat > ok << 'fubar'
targets = gram.o
need = gram.c
fubar
if test $? -ne 0 ; then no_result; fi

diff ok gram.o
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
