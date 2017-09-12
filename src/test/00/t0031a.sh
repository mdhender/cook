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
        echo 'FAILED test of the os_stat_chache() function of cook' 1>&2
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
cd $tmp

#
# Use the default error messages.  There is no other way to get
# predictable test behaviour on the unknown systems we will be tested on.
#
COOK_MESSAGE_LIBRARY=$work/no-such-dir
export COOK_MESSAGE_LIBRARY
unset LANG

mkdir a a/b a/b/c
cp /dev/null a/fubar

cat > Howto.cook <<foobar
test: test1 test2 test3 test4 test5 test6 test7;
test1: { if [not [exists Howto.cook]] then fail; }
test2: { if [exists snot] then fail; }
test3: { if [not [exists a/fubar]] then fail; }
test4: { if [exists a/fubar/b/c] then fail; }
test5: { if [not [exists a]] then fail; }
test6: { if [not [exists a/b]] then fail; }
test7: { if [not [exists a/b/c]] then fail; }
foobar
if test $? -ne 0 ; then fail; fi

$bin/cook -nl -tr > /dev/null 2>&1
if test $? -ne 0 ; then fail; fi

# probably OK
pass
