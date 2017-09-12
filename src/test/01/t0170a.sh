#!/bin/sh
#
#       cook - file construction tool
#       Copyright (C) 1999, 2001, 2007 Peter Miller;
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
if test $? -ne 0 ; then exit 2; fi
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
        echo 'FAILED test of the cook -web functionality' 1>&2
        cd $here
        rm -rf $work
        exit 1
}
no_result()
{
        set +x
        echo 'NO RESULT for test of the cook -web functionality' 1>&2
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

#
# test the cook -web functionality
#
cat > test.ok << 'fubar'
<html><head><title>
Dependency Graph
</title></head><body><h1>
Dependency Graph
</h1>
<h2>Files</h2>

<h3><a name="a.c"><tt>
<a href="file:a.c">
a.c</a>
</tt></h3><dl>
<dt>Consumed By:<dd>
<a href="#Recipe_Instance_1">
Recipe Instance 1</a> (<a href="#a.o">a.o</a>),
</dl>

<h3><a name="a.o"><tt>
<a href="file:a.o">
a.o</a>
</tt></h3><dl>
<dt>Created By:<dd>
<a href="#Recipe_Instance_1">
Recipe Instance 1</a> (<a href="#a.c">a.c</a>),
<dt>Consumed By:<dd>
<a href="#Recipe_Instance_4">
Recipe Instance 4</a> (<a href="#test">test</a>),
</dl>

<h3><a name="b.c"><tt>
<a href="file:b.c">
b.c</a>
</tt></h3><dl>
<dt>Consumed By:<dd>
<a href="#Recipe_Instance_2">
Recipe Instance 2</a> (<a href="#b.o">b.o</a>),
</dl>

<h3><a name="b.o"><tt>
<a href="file:b.o">
b.o</a>
</tt></h3><dl>
<dt>Created By:<dd>
<a href="#Recipe_Instance_2">
Recipe Instance 2</a> (<a href="#b.c">b.c</a>),
<dt>Consumed By:<dd>
<a href="#Recipe_Instance_4">
Recipe Instance 4</a> (<a href="#test">test</a>),
</dl>

<h3><a name="c.c"><tt>
<a href="file:c.c">
c.c</a>
</tt></h3><dl>
<dt>Consumed By:<dd>
<a href="#Recipe_Instance_3">
Recipe Instance 3</a> (<a href="#c.o">c.o</a>),
</dl>

<h3><a name="c.o"><tt>
<a href="file:c.o">
c.o</a>
</tt></h3><dl>
<dt>Created By:<dd>
<a href="#Recipe_Instance_3">
Recipe Instance 3</a> (<a href="#c.c">c.c</a>),
<dt>Consumed By:<dd>
<a href="#Recipe_Instance_4">
Recipe Instance 4</a> (<a href="#test">test</a>),
</dl>

<h3><a name="test"><tt>
<a href="file:test">
test</a>
</tt></h3><dl>
<dt>Created By:<dd>
<a href="#Recipe_Instance_4">
Recipe Instance 4</a> (<a href="#a.o">a.o</a>, ...),
</dl>
<h2>Recipe Instances</h2>

<h3><a name="Recipe_Instance_1">
Recipe Instance 1</h3>
<dl>
<dt>Location:
<dd>
<a href="file:howto.cook#6">
file <tt>howto.cook</tt>, line 6</a>
<dt>Target:<dd>
<tt><a href="#a.o">a.o</a></tt>,
<dt>Ingredient:<dd>
<tt><a href="#a.c">a.c</a></tt>,
<dt>Body:<dd>
<pre>echo 'cc -c a.c'
( cc -c a.c )
test $? -eq 0 || exit 1
</pre>
</dl>

<h3><a name="Recipe_Instance_2">
Recipe Instance 2</h3>
<dl>
<dt>Location:
<dd>
<a href="file:howto.cook#6">
file <tt>howto.cook</tt>, line 6</a>
<dt>Target:<dd>
<tt><a href="#b.o">b.o</a></tt>,
<dt>Ingredient:<dd>
<tt><a href="#b.c">b.c</a></tt>,
<dt>Body:<dd>
<pre>echo 'cc -c b.c'
( cc -c b.c )
test $? -eq 0 || exit 1
</pre>
</dl>

<h3><a name="Recipe_Instance_3">
Recipe Instance 3</h3>
<dl>
<dt>Location:
<dd>
<a href="file:howto.cook#6">
file <tt>howto.cook</tt>, line 6</a>
<dt>Target:<dd>
<tt><a href="#c.o">c.o</a></tt>,
<dt>Ingredient:<dd>
<tt><a href="#c.c">c.c</a></tt>,
<dt>Body:<dd>
<pre>echo 'cc -c c.c'
( cc -c c.c )
test $? -eq 0 || exit 1
</pre>
</dl>

<h3><a name="Recipe_Instance_4">
Recipe Instance 4</h3>
<dl>
<dt>Location:
<dd>
<a href="file:howto.cook#1">
file <tt>howto.cook</tt>, line 1</a>
<dt>Target:<dd>
<tt><a href="#test">test</a></tt>,
<dt>Ingredients:<dd>
<tt><a href="#a.o">a.o</a></tt>,
<tt><a href="#b.o">b.o</a></tt>,
<tt><a href="#c.o">c.o</a></tt>,
<dt>Body:<dd>
<pre>echo 'cc -o test a.o b.o c.o'
( cc -o test a.o b.o c.o )
test $? -eq 0 || exit 1
</pre>
</dl>
</body></html>
fubar
if test $? -ne 0 ; then no_result; fi

cat > howto.cook << 'fubar'
test: a.o b.o c.o
{
        cc -o [target] [resolve [need]];
}

%.o: %.c
{
        cc -c %.c;
};
fubar
if test $? -ne 0 ; then no_result; fi

date > a.c
if test $? -ne 0 ; then no_result; fi

date > b.c
if test $? -ne 0 ; then no_result; fi

date > c.c
if test $? -ne 0 ; then no_result; fi

$bin/cook -nl -web > test.out
if test $? -ne 0 ; then fail; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
