#!/bin/sh
#
#       cook - file construction tool
#       Copyright (C) 1994, 1997, 1998, 2003, 2007, 2008 Peter Miller
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
        echo 'FAILED test of the newest/oldest mtime functionality' 1>&2
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
# create the input files
#
cat > Howto.cook << 'fubar'
set fingerprint update;
test: first second
{
        sleep 2
                set silent;
        cat first second > test;
}

first: same
{
        if [exists first] then
        {
                echo this should not happen
                        set silent;
                fail;
        }
        sleep 2
                set silent;
        cat same same > first;
}

second: different
{
        sleep 2
                set silent;
        cat different different > second;
}

same different: leaf
{
        sleep 2
                set silent;
        echo same > same;
        date > different;
        cat leaf leaf >> different;
}
fubar
if test $? -ne 0 ; then fail; fi
echo "boring" > leaf
if test $? -ne 0 ; then fail; fi

#
# let the clock tick over
#
sleep 2

#
# cook everything up
#
$bin/cook -nl -silent
if test $? -ne 0 ; then fail; fi

#
# let the clock tick over
#
sleep 2

#
# change the leaf-most file
#
echo "ha ha" > leaf

#
# cook everything up
#       should not hit the "fail" action
#
$bin/cook -nl -silent
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
