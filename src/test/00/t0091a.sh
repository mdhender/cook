#!/bin/sh
#
#       cook - file construction tool
#       Copyright (C) 1996-1998, 2007, 2008 Peter Miller
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
PATH=${bin}:${PATH}
export PATH
SHELL=/bin/sh
export SHELL

fail()
{
        set +x
        echo 'FAILED test of the options builtin functionality' 1>&2
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
# test the options builtin function
#
cat > Howto.cook << 'fubar'
%:
{
        dirlist = [dirname [glob '*/Howto.cook' ]];
        loop
        {
                dir = [head [dirlist]];
                if [not [dir]] then
                        loopstop;
                dirlist = [tail [dirlist]];

                cd [dir]\; cook [options] %;
        }
}

/*
 * This recipe sets the default.
 * It doesn't actually do anything.
 */
all:;
fubar
if test $? -ne 0 ; then fail; fi

mkdir a b c
if test $? -ne 0 ; then fail; fi

cat > a/Howto.cook << 'fubar'
all: { echo hello > [target]; }
fubar
if test $? -ne 0 ; then fail; fi
cp a/Howto.cook b
if test $? -ne 0 ; then fail; fi

cook -nl > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
