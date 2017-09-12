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
# MANIFEST: Test the make2cook archive functionality
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
	echo 'FAILED test of the make2cook archive functionality' 1>&2
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
# test the make2cook archive functionality
#
cat > test.in << 'fubar'
.SUFFIXES:
.SUFFIXES: .a .o .c
foo.a: foo.a(a.o) foo.a(b.o)
	ranlib $@
fubar
if test $? -ne 0 ; then fail; fi

cat > test.ok << 'fubar'
if [not [defined ARFLAGS]] then
{
	ARFLAGS = [getenv ARFLAGS];
	if [not [ARFLAGS]] then
		ARFLAGS = rv;
}
if [not [defined AR]] then
{
	AR = [getenv AR];
	if [not [AR]] then
		AR = ar;
}
if [not [defined CPPFLAGS]] then
	CPPFLAGS = [getenv CPPFLAGS];
if [not [defined CFLAGS]] then
	CFLAGS = [getenv CFLAGS];
if [not [defined TARGET_ARCH]] then
	TARGET_ARCH = [getenv TARGET_ARCH];
if [not [defined CC]] then
{
	CC = [getenv CC];
	if [not [CC]] then
		CC = cc;
}
if [not [defined COMPILE.c]] then
{
	COMPILE.c = [getenv COMPILE.c];
	if [not [COMPILE.c]] then
		COMPILE.c = [CC] [CFLAGS] [CPPFLAGS] [TARGET_ARCH] -c;
}
if [not [defined LDFLAGS]] then
	LDFLAGS = [getenv LDFLAGS];
if [not [defined LINK.c]] then
{
	LINK.c = [getenv LINK.c];
	if [not [LINK.c]] then
		LINK.c = [CC] [CFLAGS] [CPPFLAGS] [LDFLAGS] [TARGET_ARCH];
}
if [not [defined LDLIBS]] then
	LDLIBS = [getenv LDLIBS];
if [not [defined LOADLIBES]] then
	LOADLIBES = [getenv LOADLIBES];
if [not [defined LINK.o]] then
{
	LINK.o = [getenv LINK.o];
	if [not [LINK.o]] then
		LINK.o = [CC] [LDFLAGS] [TARGET_ARCH];
}

foo.a: foo.a(a.o) foo.a(b.o)
{
	ranlib [target];
}

%0%: %0%.o
{
	[LINK.o] [resolve [need]] [LOADLIBES] [LDLIBS] -o [target];
}
%0%: %0%.c
{
	[LINK.c] [resolve [need]] [LOADLIBES] [LDLIBS] -o [target];
}
%0%.o: %0%.c
{
	[COMPILE.c] [resolve [head [need]]];
}
%0%1.a(%.o): %0%.o
	single-thread %0%1.a
{
	[AR] [ARFLAGS] %0%1.a [resolve [head [need]]];
}
%0%1(%): %0%
	single-thread %0%1
{
	[AR] [ARFLAGS] %0%1 [resolve [head [need]]];
}
fubar
if test $? -ne 0 ; then fail; fi

$bin/make2cook -e test.in test.out
if test $? -ne 0 ; then fail; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
