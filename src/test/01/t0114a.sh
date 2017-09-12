#!/bin/sh
#
#       cook - file construction tool
#       Copyright (C) 1997, 1998, 2003, 2007 Peter Miller;
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
        echo 'FAILED test of the graph functionality' 1>&2
        cd $here
        rm -rf $work
        exit 1
}
no_result()
{
        set +x
        echo 'NO RESULT for test of the graph functionality' 1>&2
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

#
# test the graph functionality
#
cat > Howto.cook << 'fubar'
default.target: all;
all: prog thingumy;

%.c %.h: %.y
{
        /*
         * Should not be run in this test, but it exersizes a variety of
         * backtracking in the build phase.
         */
        yacc -d %.y;
        mv y.tab.c %.c;
        mv y.tab.h %.h;
}

%.o: %.c
{
        echo [need] > [target];
}

prog: a.o b.o c.o
{
        echo [need] > [target];
}

thingumy: whatsit;

whatsit: a.c
{
        echo a.c > [target];
}

a.o: a.h b.h c.h;
b.o: b.h c.h;
c.o: c.h;
fubar
if test $? -ne 0 ; then no_result; fi

# create all of the leaf files
for f in a.c b.c c.c a.h b.h c.h; do date > $f; sleep 2; done
if test $? -ne 0 ; then no_result; fi

#
# Now actually run the build and make sure it does the right things.
#
$bin/cook -nl > test.log 2>&1
if test $? -ne 0 ; then cat test.log; fail; fi

#
# it should create all of the derived files,
# except b.o which already existed.
#
test -f a.o || fail
test -f b.o || fail
test -f c.o || fail
test -f prog || fail
test -f whatsit || fail

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
