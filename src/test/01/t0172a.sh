#!/bin/sh
#
#       cook - file construction tool
#       Copyright (C) 1999, 2003, 2007, 2008 Peter Miller
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
        chmod -R u+w $work
        rm -rf $work
        exit 0
}
fail()
{
        set +x
        echo 'FAILED test of the fingerprint functionality' 1>&2
        cd $here
        chmod -R u+w $work
        rm -rf $work
        exit 1
}
no_result()
{
        set +x
        echo 'NO RESULT for test of the fingerprint functionality' 1>&2
        cd $here
        chmod -R u+w $work
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
# test the fingerprint functionality
#
mkdir a b
if test $? -ne 0 ; then no_result; fi
touch a/.cook.fp
if test $? -ne 0 ; then no_result; fi
touch b/.cook.fp
if test $? -ne 0 ; then no_result; fi
date > one
if test $? -ne 0 ; then no_result; fi
date > a/two
if test $? -ne 0 ; then no_result; fi
date > b/three
if test $? -ne 0 ; then no_result; fi
chmod ogu-w a b/.cook.fp
if test $? -ne 0 ; then no_result; fi

# Solaris delays inode changes for a long time
# make sure the permissions are set solid before the next action.
sync

$bin/cook -nl -fp-update
if test $? -ne 0 ; then fail; fi

# Because a is read-only doesn't mean that a/.cook.fp is read-only,
# and it gets over-written.
if test '!' -s a/.cook.fp
then
        # a/.cook.fp is empty and it shouldn't be
        fail
fi

# However b/.cook.fp is readonly (even though b is writable), and it
# *doesn't* get over-written (on an open-for-writing failure it doesn't
# backtrack and try to unlink again).
if test -s b/.cook.fp
then
        # b/.cook.fp isn't empty and it should be
        fail
fi

# There should be 4 entries in the file.
test `wc .cook.fp|awk '{print$1}'` -eq 4 || fail

$bin/cook -nl -fp-update
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
