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
# MANIFEST: Test the strip-dot functionality
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
	echo 'FAILED test of the strip-dot functionality' 1>&2
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
# test the no-strip-dot functionality
#
cat > Howto.cook << 'fubar'
set nostripdot;
a_obj = ;
all : ./a;
./% : [%_obj] { echo ok > [target]; }
fubar
if test $? -ne 0 ; then fail; fi
$bin/cook -nl -silent
if test $? -ne 0 ; then fail; fi

#
# test the strip-dot functionality
#
cat > b.c << 'fubar'
int main() { exit(0); return 0; }
fubar
if test $? -ne 0 ; then fail; fi
cat > Howto.cook << 'fubar'
set stripdot;
all: b;
./%: %.c { echo ok > %; }
fubar
if test $? -ne 0 ; then fail; fi
$bin/cook -nl -silent
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
