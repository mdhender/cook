#!/bin/sh
#
#       cook - file construction tool
#       Copyright (C) 1997, 1998, 2007 Peter Miller;
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
        echo 'FAILED test of the c_incl functionality' 1>&2
        cd $here
        rm -rf $work
        exit 1
}
no_result()
{
        set +x
        echo 'NO RESULT for test of the c_incl functionality' 1>&2
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
# test the c_incl -o functionality
#
cat > test.in << 'fubar'
one
#include "two.h"
three
fubar
if test $? -ne 0 ; then no_result; fi

cat > two.h << 'fubar'
two
fubar
if test $? -ne 0 ; then no_result; fi

cat > test.ok << 'fubar'
two.h
fubar
if test $? -ne 0 ; then no_result; fi

$bin/c_incl -nc -ns test.in -o test.out
if test $? -ne 0 ; then fail; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# test the c_incl -nsri functionality
#
$bin/c_incl -nc -nsri -ns test.in > test.out 2>&1
if test $? -ne 1 ; then cat test.out; fail; fi

#
# test the c_incl -rlp functionality
#
cat > test.in << 'fubar'
one
#include <two.h>
three
fubar
if test $? -ne 0 ; then no_result; fi

cat > test.ok << 'fubar'
two.h
fubar
if test $? -ne 0 ; then no_result; fi

mkdir bl
if test $? -ne 0 ; then no_result; fi
mv two.h bl
if test $? -ne 0 ; then no_result; fi

$bin/c_incl -nc -I. -Ibl -rlp=. -rlp=bl -ns test.in -o test.out
if test $? -ne 0 ; then fail; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# test the c_incl -slp functionality
#
cat > test.ok << 'fubar'
[bl]/two.h
fubar
if test $? -ne 0 ; then no_result; fi

$bin/c_incl -nc -I. -Ibl -rlp=. -slp bl '[bl]' -ns test.in -o test.out
if test $? -ne 0 ; then fail; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
