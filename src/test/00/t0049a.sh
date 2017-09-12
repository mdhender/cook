#!/bin/sh
#
#       cook - file construction tool
#       Copyright (C) 1994, 1997, 1998, 2001, 2002, 2007 Peter Miller;
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
        echo 'FAILED test of the cookfp repeat functionality' 1>&2
        cd $here
        rm -rf $work
        exit 1
}
no_result()
{
        set +x
        echo 'NO RESULT test of the cookfp repeat functionality' 1>&2
        cd $here
        rm -rf $work
        exit 2
}
pass()
{
        set +x
        cd $here
        rm -rf $work
        exit 0
}
trap \"no_result\" 1 2 3 15

mkdir $work $work/lib
if test $? -ne 0 ; then exit 2; fi
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
# create simple file to verify
#
cat > test.in.uue << 'fubar'
begin 644 test.in
#:&D*
`
end
fubar
if test $? -ne 0 ; then no_result; fi
uudecode test.in.uue
if test $? -ne 0 ; then no_result; fi

#
# make sure it works every which way
#
$bin/cookfp test.in - test.in < test.in > test.out
if test $? -ne 0 ; then fail; fi

cat > test.ok.uue << 'fubar'
begin 644 test.ok
M2F]816AD9FQ'>%ID4S@Y=$1D-7IP1G@P2E)*>5@P:S8W14@P2F]D-V%E=#1H
M26I"+FA':&]N,'9E3C!"6E)F:3`U.$A:43`V53!*1"!T97-T+FEN"DIO6$5H
M9&9L1WA:9%,X.71$9#5Z<$9X,$I22GE8,&LV-T5(,$IO9#=A970T:$EJ0BYH
M1VAO;C!V94XP0EI29FDP-3A(6E$P-E4P2D0@<W1D:6X*2F]816AD9FQ'>%ID
M4S@Y=$1D-7IP1G@P2E)*>5@P:S8W14@P2F]D-V%E=#1H26I"+FA':&]N,'9E
<3C!"6E)F:3`U.$A:43`V53!*1"!T97-T+FEN"@``
`
end
fubar
if test $? -ne 0 ; then no_result; fi

uudecode test.ok.uue
if test $? -ne 0 ; then no_result; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
