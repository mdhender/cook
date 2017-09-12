#!/bin/sh
#
#	cook - file construction tool
#	Copyright (C) 1997, 1998, 2001, 2003 Peter Miller;
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
# MANIFEST: Test the no-implicit-ingredients functionality
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
	echo 'FAILED test of the no-implicit-ingredients functionality' 1>&2
	cd $here
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
     echo 'NO RESULT for test of the no-implicit-ingredients functionality' 1>&2
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

#
# test the no-implicit-ingredients functionality
#
cat > Howto.cook << 'fubar'
%.ww: %.xx
	set no-implicit-ingredients
{
	echo "'('`cat" [need]"`')->'"[target] > [target];
}
%.xx: %.yy
{
	echo "'('`cat" [need]"`')->'"[target] > [target];
}
test.out: a.ww b.ww c.ww
{
	echo "'('`cat" [need]"`')->'"[target] > [target];
}
b.xx:;
fubar
if test $? -ne 0 ; then no_result; fi

echo a.xx > a.xx
if test $? -ne 0 ; then no_result; fi
echo b.xx > b.xx
if test $? -ne 0 ; then no_result; fi
echo c.xx > c.xx
if test $? -ne 0 ; then no_result; fi
sleep 2
echo a.yy > a.yy
if test $? -ne 0 ; then no_result; fi
echo b.yy > b.yy
if test $? -ne 0 ; then no_result; fi
echo c.yy > c.yy
if test $? -ne 0 ; then no_result; fi

cat > test.ok << 'fubar'
((a.xx)->a.ww ((b.yy)->b.xx)->b.ww (c.xx)->c.ww)->test.out
fubar
if test $? -ne 0 ; then no_result; fi

$bin/cook -nl > LOG 2>&1
if test $? -ne 0 ; then cat LOG; fail; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
