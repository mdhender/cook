#!/bin/sh
#
#       cook - file construction tool
#       Copyright (C) 1997, 1998, 2007 Peter Miller
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
        echo 'FAILED test of the make2cook functionality' 1>&2
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

TAB=`awk 'BEGIN{printf("%c",9)}' /dev/null`
test $? -eq 0 || fail

#
# test the make2cook functionality
#
cat > test.in << 'fubar'
.SUFFIXES:

libs_for_gcc = -lgnu
normal_libs =

ifeq ($(CC),gcc)
foo: $(objects);$(CC) -o foo $(objects) $(libs_for_gcc)
else
foo: $(objects);$(CC) -o foo $(objects) $(normals_libs)
endif
fubar
if test $? -ne 0 ; then fail; fi

cat > test.ok << 'fubar'
if [not [defined normals_libs]] then
        normals_libs = [getenv normals_libs];
if [not [defined objects]] then
        objects = [getenv objects];
if [not [defined CC]] then
{
        CC = [getenv CC];
        if [not [CC]] then
                CC = cc;
}

if [not [defined libs_for_gcc]] then
#line 3 "test.in"
        libs_for_gcc = -lgnu;
if [not [defined normal_libs]] then
#line 4 "test.in"
        normal_libs = ;

#if [in [CC] gcc ]
foo: [objects]
{
#line 7 "test.in"
        [CC] -o foo [objects] [libs_for_gcc];
}
#else
#line 9 "test.in"
foo: [objects]
{
#line 9 "test.in"
        [CC] -o foo [objects] [normals_libs];
}
#endif
fubar
if test $? -ne 0 ; then fail; fi

$bin/make2cook -e -ln test.in test.out
if test $? -ne 0 ; then fail; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# test the make2cook functionality
#
cat > test.in << 'fubar'
.SUFFIXES:

libs_for_gcc = -lgnu
normal_libs =

ifneq "$(CC)" "gcc"
libs = $(normal_libs)
else
libs = $(libs_for_gcc)
endif

foo: $(objects);$(CC) -o foo $(objects) $(libs)
fubar
if test $? -ne 0 ; then fail; fi

cat > test.ok << 'fubar'
if [not [defined objects]] then
        objects = [getenv objects];
if [not [defined CC]] then
{
        CC = [getenv CC];
        if [not [CC]] then
                CC = cc;
}

if [not [defined libs_for_gcc]] then
#line 3 "test.in"
        libs_for_gcc = -lgnu;
if [not [defined normal_libs]] then
#line 4 "test.in"
        normal_libs = ;

#if [not [in [CC] gcc ] ]
if [not [defined libs]] then
#line 7 "test.in"
        libs = [normal_libs];
#else
if [not [defined libs]] then
#line 9 "test.in"
        libs = [libs_for_gcc];
#endif

foo: [objects]
{
#line 12 "test.in"
        [CC] -o foo [objects] [libs];
}
fubar
if test $? -ne 0 ; then fail; fi

$bin/make2cook -e -ln test.in test.out
if test $? -ne 0 ; then fail; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# test the make2cook functionality
#
cat > test.in << 'fubar'
.SUFFIXES:

ifndef slartibartfast

ifneq "$(CC)" "gcc"
libs = $(normal_libs)
else
libs = $(libs_for_gcc)
endif

foo: $(objects);$(CC) -o foo $(objects) $(libs)
endif
fubar
if test $? -ne 0 ; then fail; fi

cat > test.ok << 'fubar'
if [not [defined objects]] then
        objects = [getenv objects];
if [not [defined libs_for_gcc]] then
        libs_for_gcc = [getenv libs_for_gcc];
if [not [defined normal_libs]] then
        normal_libs = [getenv normal_libs];
if [not [defined CC]] then
{
        CC = [getenv CC];
        if [not [CC]] then
                CC = cc;
}
#line 3 "test.in"
#if [not [defined slartibartfast ] ]

#if [not [in [CC] gcc ] ]
if [not [defined libs]] then
#line 6 "test.in"
        libs = [normal_libs];
#else
if [not [defined libs]] then
#line 8 "test.in"
        libs = [libs_for_gcc];
#endif

foo: [objects]
{
#line 11 "test.in"
        [CC] -o foo [objects] [libs];
}
#endif
fubar
if test $? -ne 0 ; then fail; fi

$bin/make2cook -e -ln test.in test.out
if test $? -ne 0 ; then fail; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# test the make2cook functionality
#
sed "s|<TAB>|$TAB|g" > test.in << 'fubar'
.SUFFIXES:

libs_for_gcc = -lgnu
normal_libs =

foo: $(objects)
ifeq ($(CC),gcc)
<TAB>$(CC) -o foo $(objects) $(libs_for_gcc)
else
<TAB>$(CC) -o foo $(objects) $(normals_libs)
endif
fubar
if test $? -ne 0 ; then fail; fi

cat > test.ok << 'fubar'
if [not [defined normals_libs]] then
        normals_libs = [getenv normals_libs];
if [not [defined CC]] then
{
        CC = [getenv CC];
        if [not [CC]] then
                CC = cc;
}
if [not [defined objects]] then
        objects = [getenv objects];

if [not [defined libs_for_gcc]] then
#line 3 "test.in"
        libs_for_gcc = -lgnu;
if [not [defined normal_libs]] then
#line 4 "test.in"
        normal_libs = ;

foo: [objects]
{
#line 7 "test.in"
        #if [in [CC] gcc ]
        [CC] -o foo [objects] [libs_for_gcc];
        #else
        [CC] -o foo [objects] [normals_libs];
        #endif
}
fubar
if test $? -ne 0 ; then fail; fi

$bin/make2cook -e -ln test.in test.out
if test $? -ne 0 ; then fail; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
