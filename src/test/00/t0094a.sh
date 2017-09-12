#!/bin/sh
#
#       cook - file construction tool
#       Copyright (C) 1996-1998, 2000, 2006, 2007 Peter Miller
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

bin=$here/${1-.}/bin

fail()
{
        set +x
        echo 'FAILED test of the time-adjust-back functionality' 1>&2
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
# test the time-adjust-back functionality
#
cat > Howto.cook << 'fubar'
set time-adjust-back;
test: a b;

a:
{
    cp /dev/null a;
}

b: leaf
{
    cp leaf b;
}
fubar
if test $? -ne 0 ; then fail; fi

cp /dev/null leaf;
if test $? -ne 0 ; then fail; fi
$bin/cooktime -m '16-Feb-1996 12:34:56 GMT' leaf;
if test $? -ne 0 ; then fail; fi

# need to calibrate the timestamp granularity

$bin/cook -nl > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi

cat > test.ok << 'fubar'
leaf
    modify Fri, 16 Feb 1996 12:34:56 GMT
b
    modify Fri, 16 Feb 1996 12:34:57 GMT
fubar
if test $? -ne 0 ; then fail; fi
$bin/cooktime -report leaf b > test.out
if test $? -ne 0 ; then fail; fi
grep -v access test.out > test.out2
if test $? -ne 0 ; then fail; fi
diff test.ok test.out2
if test $? -ne 0 ; then fail; fi

#
# Now repeat the above with a more coarse timestamp granularity.
# This test uses a granularity of 2 seconds. An example of this would
# be a FAT32 filesystem with Cygwin.
#
cat > Howto.cook << 'fubar'
set time-adjust-back;

timestamp_granularity = 2;

test: a b c d;

a:
{
    cp /dev/null a;
}

b: a
{
    cp a b;
}

c: b
{
    cp b c;
}

d: c
{
    cp c d;
}
fubar
if test $? -ne 0 ; then fail; fi

rm -f a b leaf
if test $? -ne 0 ; then fail; fi

touch a;
if test $? -ne 0 ; then fail; fi

# Set the time to an odd number of seconds and check that we correctly
# round to an even number.
$bin/cooktime -tsg 2 -m '16-Feb-1996 12:34:51 GMT' a;
if test $? -ne 0 ; then fail; fi

# need to calibrate the timestamp granularity
$bin/cook -nl > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi

cat > test.ok << 'fubar'
a
    modify Fri, 16 Feb 1996 12:34:52 GMT
b
    modify Fri, 16 Feb 1996 12:34:54 GMT
c
    modify Fri, 16 Feb 1996 12:34:56 GMT
d
    modify Fri, 16 Feb 1996 12:34:58 GMT
fubar
if test $? -ne 0 ; then fail; fi

$bin/cooktime -report a b c d > test.out
if test $? -ne 0 ; then fail; fi

grep -v access test.out > test.out2
if test $? -ne 0 ; then fail; fi

diff test.ok test.out2
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
