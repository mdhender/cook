#!/bin/sh
#
#	cook - file construction tool
#	Copyright (C) 1994, 1997, 1998 Peter Miller;
#	All rights reserved.
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 2 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program; if not, write to the Free Software
#	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
#
# MANIFEST: Test the archive long name functionality
#
ed /dev/null > /dev/null 2>&1 << 'fubar'
q
fubar
if test $? -ne 0; then
	echo ''
	echo '	Your system has no "ed" command.'
	echo '	This test is declared to pass by default.'
	echo ''
	exit 0
fi

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
	echo 'FAILED test of the archive long name functionality' 1>&2
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
# make sure this system understands the standard
# text-based archive format
#
ar qc foo.a /dev/null
if test $? -ne 0 ; then fail; fi
sed '2,$d' < foo.a > foo.aa
if test $? -ne 0 ; then fail; fi
cat > foo.ok << 'fubar'
!<arch>
fubar
if test $? -ne 0 ; then fail; fi
diff foo.ok foo.aa > /dev/null 2>&1
if test $? -ne 0 ; then
	echo ""
	echo "	This system does not understand the standard text-based"
	echo "	archive file format.  This test passes by default."
	echo ""
	pass
fi

#
# test the archive long name functionality
#
cat > Howto.cook << 'fubar'
test: a(b) {}

%1(%2): %2
{
	if [exists %1] then
	{
		echo "This should never happen."
			set silent;
		fail;
	}
	ar qc %1 %2;
}

b:
{
	echo "This is a small file." > b;
}
fubar
if test $? -ne 0 ; then fail; fi

$bin/cook -nl -silent
if test $? -ne 0 ; then fail; fi

#
# now mess with the archive to add a name index
# and cook again - everything should be up-to-date
#
cat > script << 'fubar'
2t2
2a
b
.
2s/^b./\/\//
2s/22        /2         /
4s/^b./\/0/
w
q
fubar
if test $? -ne 0 ; then fail; fi
ed a < script > script.out 2>&1
if test $? -ne 0 ; then cat script.out; fail; fi

$bin/cook -nl -silent
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
