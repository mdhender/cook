#!/bin/sh
#
#       cook - file construction tool
#       Copyright (C) 1999, 2007 Peter Miller;
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
        echo 'FAILED test of the local variable functionality' 1>&2
        cd $here
        rm -rf $work
        exit 1
}
no_result()
{
        set +x
        echo 'NO RESULT for test of the local variable functionality' 1>&2
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
# test the local variable functionality
#
cat > howto.cook << 'fubar'

function capitalize =
{
        local result = ;
        local tmp = ;
        loop tmp = [arg]
        {
                result += [upcase [substr 1 1 [tmp]]][downcase [substr
                        2 999 [tmp]]];
        }
        return [result];
}


fred = right;

test.out: xxx
{
        if [defined tmp] then fail;
        if [defined result] then fail;
        function write [target] [fred];
}

xxx:
{
        local fred = [capitalize wrong];
        function write [target] [fred];
}
fubar
if test $? -ne 0 ; then no_result; fi

cat > test.ok << 'fubar'
right
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
