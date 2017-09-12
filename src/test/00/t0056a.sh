#!/bin/sh
#
#       cook - file construction tool
#       Copyright (C) 1994, 1997, 1998, 2003, 2007 Peter Miller;
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
work=${COOK_TMP:-/tmp}/$$
PAGER=cat
export PAGER
umask 022
unset COOK
here=`pwd`
if test $? -ne 0 ; then exit 1; fi

bin="$here/${1-.}/bin"

fail()
{
        set +x
        echo 'FAILED test of the search-list functionality' 1>&2
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

#
# make sure targets are not deeper than ingredients
#
cat > Howto.cook << 'fubar'
cc = [getenv CC]; if [not [cc]] then
{ cc = [find_command gcc]; if [not [cc]] then cc = cc; }

exe = ;
if [in [substr 1 6 [os]] CYGWIN NUTCRA] then
        exe = .exe;

set fingerprint update;
search_list = . bl;
testprog[exe]: test.c
{
        [cc] -o [target] test.c;
}
fubar
if test $? -ne 0 ; then fail; fi

cat > test.c << 'fubar'
main(){exit(42);}
fubar
if test $? -ne 0 ; then fail; fi

$bin/cook -nl -silent
if test $? -ne 0 ; then fail; fi

./testprog
if test $? -ne 42 ; then fail; fi

#
# move into "bl"
#
mkdir bl
if test $? -ne 0 ; then fail; fi

mv test.c bl
if test $? -ne 0 ; then fail; fi

mv testprog.exe bl 2> LOG || mv testprog bl
if test $? -ne 0 ; then fail; fi

#
# new file in front of list
#
sleep 2
cat > test.c << 'fubar'
main(){exit(0);}
fubar
if test $? -ne 0 ; then fail; fi

#
# irrelevant update in back of list
#
sleep 2
CC=${CC-gcc}
$CC -o bl/testprog bl/test.c;
if test $? -ne 0 ; then fail; fi

#
# this should cook in front list
#
$bin/cook -nl -silent
if test $? -ne 0 ; then fail; fi

./testprog
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
