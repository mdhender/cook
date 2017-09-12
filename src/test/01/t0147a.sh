#!/bin/sh
#
#	cook - file construction tool
#	Copyright (C) 1998, 2001, 2003 Peter Miller;
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
# MANIFEST: Test the cascade functionality
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
PATH=${bin}:${PATH}
export PATH

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
	echo 'FAILED test of the cascade functionality' 1>&2
	cd $here
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo 'NO RESULT for test of the cascade functionality' 1>&2
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
unset LANGUAGE
CC=${CC-cccc}
export CC

#
# test the cascade functionality
#
cat > howto.cook << 'fubar'
test: fred.o;

cc = [getenv CC]; if [not [cc]] then cc = cc;

%.o: %.c { [cc] -c %.c; }

%.d: % { c_incl -norec --prefix\="'cascade % ='" --suffix\="';'" % -o %.d; }

#include-cooked [addsuffix ".d" [glob "*.[ch]" ]]
fubar
if test $? -ne 0 ; then no_result; fi

cat > fred.c << 'fubar'
#include "fred.h"
int main(){exit(0: return 0;}
fubar
if test $? -ne 0 ; then no_result; fi

cat > fred.h << 'fubar'
#include "test.h"
fubar
if test $? -ne 0 ; then no_result; fi

cat > test.h << 'fubar'
/* ha ha */
fubar
if test $? -ne 0 ; then no_result; fi

cook -nl -script > test.out 2> LOG
if test $? -ne 0 ; then cat LOG; fail; fi

cat > test.ok << fubar
#!/bin/sh

#line 5 howto.cook
if test ! -e fred.o \\
	-o ! fred.o -nt fred.c \\
	-o ! fred.o -nt fred.h \\
	-o ! fred.o -nt test.h
then
echo '${CC} -c fred.c'
( ${CC} -c fred.c )
test \$? -eq 0 || exit 1
fi
if test ! -e test \\
	-o ! test -nt fred.o
then
:
fi
exit 0
fubar
if test $? -ne 0 ; then no_result; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
