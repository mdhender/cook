#!/bin/sh
#
#       cook - file construction tool
#       Copyright (C) 1990-1994, 1997, 1998, 2007 Peter Miller;
#       All rights reserved.
#
#       This program is free software; you can redistribute it and/or modify
#       it under the terms of the GNU General Public License as published by
#       the Free Software Foundation; either version 3 of the License, or
#       (at your option) any later version.
#
#       This program is distributed in the hope that it will be useful,
#       but WITHOUT ANY WARRANTY; without even the implied warranty of
#       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#       GNU General Public License for more details.
#
#       You should have received a copy of the GNU General Public License
#       along with this program. If not, see
#       <http://www.gnu.org/licenses/>.
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
        echo 'FAILED test of roffpp command' 1>&2
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

mkdir $tmp $tmp/lib $tmp/bloggs
cd $tmp

#
# Use the default error messages.  There is no other way to get
# predictable test behaviour on the unknown systems we will be tested on.
#
COOK_MESSAGE_LIBRARY=$work/no-such-dir
export COOK_MESSAGE_LIBRARY
unset LANG

#
# test the plain-vanilla form
#
cat > test.in << 'foobar'
testing
.so test2.in
line three
foobar
if test $? -ne 0 ; then fail; fi

cat > test2.in << 'foobar'
one
two
three
foobar
if test $? -ne 0 ; then fail; fi

cat > test.ok << 'foobar'
.lf 1 test.in
testing
.lf 1 test2.in
one
two
three
.lf 3 test.in
line three
foobar
if test $? -ne 0 ; then fail; fi

$bin/roffpp test.in test.out
if test $? -ne 0 ; then fail; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# test the .lf directive
#
cat > test.in << 'foobar'
testing
.lf 47 granny-smith
Granny Smith
.so test2.in
line three
foobar
if test $? -ne 0 ; then fail; fi

cat > test2.in << 'foobar'
one
two
three
foobar
if test $? -ne 0 ; then fail; fi

cat > test.ok << 'foobar'
.lf 1 test.in
testing
.lf 47 granny-smith
Granny Smith
.lf 1 test2.in
one
two
three
.lf 49 granny-smith
line three
foobar
if test $? -ne 0 ; then fail; fi

$bin/roffpp test.in test.out
if test $? -ne 0 ; then fail; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# test the explicit include path form
#
cat > test.in << 'foobar'
testing
.so test2.in
.so ../test2.in
line four
foobar
if test $? -ne 0 ; then fail; fi

cat > bloggs/test2.in << 'foobar'
Bodgie Rent-a-Wreck
-- free --
with every 200 lb rotweiler sold
foobar
if test $? -ne 0 ; then fail; fi

cat > test.ok << 'foobar'
.lf 1 test.in
testing
.lf 1 bloggs/test2.in
Bodgie Rent-a-Wreck
-- free --
with every 200 lb rotweiler sold
.lf 3 test.in
.lf 1 bloggs/../test2.in
one
two
three
.lf 4 test.in
line four
foobar
if test $? -ne 0 ; then fail; fi

$bin/roffpp -Ibloggs -I. test.in test.out
if test $? -ne 0 ; then fail; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

# probably OK
pass