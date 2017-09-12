#!/bin/sh
#
#       cook - file construction tool
#       Copyright (C) 1997-1999, 2007, 2008 Peter Miller
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

rsh=rsh
IFS="${IFS=     }"; ifs="$IFS"; IFS=":"
dummy="$PATH"
for dir in $dummy
do
        test -z "$dir" && dir=.
        if test -f $dir/remsh; then
                rsh="remsh"
                break
        fi
done
IFS="$ifs"

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
        echo 'FAILED test of the parallel_hosts functionality' 1>&2
        cd $here
        rm -rf $work
        exit 1
}
no_result()
{
        set +x
        echo 'NO RESULT for test of the parallel_hosts functionality' 1>&2
        cd $here
        rm -rf $work
        exit 2
}
no_point_testing()
{
        set +x
        echo ''
        echo "  The $rsh command on your system does not appear to work."
        echo '  This means that the parallel virtual machine'
        echo '  functionality of Cook will probably not work, either.'
        echo '  This test is declared to pass by default.'
        echo ''
        cd $here
        rm -rf $work
        exit 0
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

#
# Make sure rsh is working.  If it can't be found, assume the test passes.
#
echo "$$ chuckle $$" > test.in
if test $? -ne 0 ; then no_result; fi
$rsh localhost "cat $work/test.in" < /dev/null > test.out
if test $? -ne 0; then no_point_testing; fi
diff test.in test.out
if test $? -ne 0; then no_point_testing; fi

#
# test the parallel_hosts functionality
#
magic="MAGIC_$$_MAGIC_$$_MAGIC"
eval "$magic=\"command-did-not-go-remote\""
eval "export $magic"

cat > Howto.cook << fubar
parallel_hosts = localhost localhost;

test.out2: test.in
{
        cat '\${${magic}:-test.in}' > [target];
}
fubar
if test $? -ne 0 ; then no_result; fi

$bin/cook -nl > LOG 2>&1
if test $? -ne 0 ; then cat LOG; fail; fi

diff test.in test.out2
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
