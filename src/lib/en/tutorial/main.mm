.\"
.\"     cook - file construction tool
.\"     Copyright (C) 2002 Aryeh M. Friedman
.\"     Groff formatting general editorial work
.\"     Copyright (C) 2002, 2003, 2007 Peter Miller
.\"
.\"     This program is free software; you can redistribute it and/or modify
.\"     it under the terms of the GNU General Public License as published by
.\"     the Free Software Foundation; either version 3 of the License, or
.\"     (at your option) any later version.
.\"
.\"     This program is distributed in the hope that it will be useful,
.\"     but WITHOUT ANY WARRANTY; without even the implied warranty of
.\"     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
.\"     GNU General Public License for more details.
.\"
.\"     You should have received a copy of the GNU General Public License
.\"     along with this program. If not, see
.\"     <http://www.gnu.org/licenses/>.
.\"
.if n .ftr CB B
.if n .ftr CI I
.if n .ftr CW R
.if n .ftr C  R
.if t .ds HF HB H H HI HI HI HI HI HI HI
.if t .ds HP 16 14 12 12 10 10 10 10 10 10
.PH "''''"
.PF "''''"
.so libdir.so \"from the etc directory, by configure
.so version.so \"from the etc directory
.\" stuff for figuring dates
.\"
.ds MONTH1 January
.ds MONTH2 February
.ds MONTH3 March
.ds MONTH4 April
.ds MONTH5 May
.ds MONTH6 June
.ds MONTH7 July
.ds MONTH8 August
.ds MONTH9 September
.ds MONTH10 October
.ds MONTH11 November
.ds MONTH12 December
.ds MO \\*[MONTH\n[mo]]
.nr *year \n[yr]+1900
.ds DY \n[dy] \*[MO] \n[*year]
.nr Hb 9
.de eB
.br
.ft CW
.in +0.5i
.ta 8n 16n 24n 32n 40n 48n
.nf
..
.de eE
.br
.ft R
.fi
.in -0.5i
..
.\" ------------------------------------------------------------------------
\&.
.sp 2i
.ps 36
.vs 38
.ce 1
Cook
.sp 0.5i
.ce 1
Tutorial
.sp 1i
.ps 18
.vs 20
.ce 1
Aryeh M. Friedman
.ft I
.ce 1
aryeh@m-net.arbornet.org
.ft P
.\" ------------------------------------------------------------------------
.bp
.ps 12
.vs 14
\&.
.sp 2i
This document describes Cook version \*(v)
.br
and was prepared \*(DY.
.br
.sp 1i
.if n .ds C) (C)
.if t .ds C) \(co
This document describing the Cook program is
.br
Copyright \*(C) 2002 Aryeh M. Friedman
.sp
Cook itself is
.br
Copyright \*(C) \*(Y) Peter Miller
.sp
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.
.sp
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
.sp
You should have received a copy of the GNU General Public License along
with this program. If not, see <http://www.gnu.org/licenses/>.
.\" ------------------------------------------------------------------------
.PH "'Cook''Tutorial'"
.bp 1
.PF "'Aryeh M. Friedman''Page %'"
.\" .MT 4 1
.SK
.\" .nr Ej 1
.if t .2C
.\" ------------------------------------------------------------------------
.H 1 "Building Programs"
If you write simple programs (a few hundred lines of code at most)
compiling the program is often no more then something like this:
.eB
gcc foo.c -o foo
.eE
If you have a few files in your program you just do:
.eB
gcc foo.c ack.c -o foo
.eE
But what happens if some file that is being compiled is the output of an
other program (like using yacc/lex to construct a command line parser)?
Obviously \f[CW]foo.c\fP does not exist before \f[CW]foo.y\fP is processed
by yacc.  Thus you have to do:
.eB
yacc foo.y
cc foo.c ack.c -o foo
.eE
What happens if say you modify \f[CW]ack.c\fP but do not modify
\f[CW]foo.y\fP?  You can skip the yacc step.  For a small program
like the one above it is possible to remember what order you need to do
stuff in and what needs to be done depending on what file you modify.
.P
Let's add one more complication let's say you have a library that
also needs to be "built" before the executable(s) is built.  You need to
not only remember what steps are needed to construct the library object
file but you also need to remember that it needs to be done you make your
executables.  Now add to this you also need to keep track of different
versions as well figuring out how to build different versions for different
platforms and/or customers (say you support Windows, Unix and have a Client,
Server and trial, desktop and enterprise versions of each and you need to
produce any and all combination of things... that's 24 different versions of
the same set of executables).  It now becomes almost impossible to to
remember how each on is built.  On top all this if you build it differently
every time you need to recompile the program there is no guarantee you
will not introduce bugs due to only the order stuff was built in.
.P
And the above example is for a "small" applications (maybe 10 to 20
files) what happens if you have a medium or large project (100s or
1000s of files) and 10+ or 100+ executables with each one having 10+
different configurations.  It is clearly the number of possible ways to
make this approaches infinity very rapidly (in algorithm designer terms
\fIO(n!)\fP).  There has to be a easier way!  Traditionally people have
used a tool called \fImake\fP to handle this complexity, but make has
some major flaws such that it is very hard if not impossible to make
know how to build the entire project without some super nasty and flawed
"hacks".  In the last few years a program called Cook has gained a small
but growing popularity as a extremely "intelligent" replacement for make.
.H 1 "Dependency Graphs"
Clearly, for any build process the build management utility
(e.g. \fIcook\fP or \fImake\fP) needs to know that for event Y to occur
event X has to happen first.  This knowledge is called a dependency.
In simple programs it is possible to just tell the build manager that
X depends on Y.  This has a few problems:
.BL
.LI
You can not define generic dependencies for example you can not say that
all \f[CW].o\fP files depend on \f[CW].c\fP files of the same name.
.LI
Often there are intermediate files created during the build process for
example \f[CW]foo.y\fP \(-> \f[CW]foo.c\fP \(-> \f[CW]foo.o\fP \(->
\f[CW]foo\fP.  This means that each intermediate file needs to be made
before the final program is built.
.LI
In almost all projects there is no single way of producing any given
file type.  For example \f[CW]ack.c\fP does not need to be created from
the \f[CW]ack.y\fP file but \f[CW]foo.c\fP does need to be created from
the \f[CW]foo.y\fP file.
.LI
Many times many things depend on event X but X can not happen until Y
happens.  For example if you need to compile all the \f[CW].c\fP files
into \f[CW].o\fP files before you can combine them into a library then
once the library is made then and \fIonly\fP then can you build all the
executables that need that library.
.LI
Depending on what variant of an executable you are building you
may have a total different set of dependencies for that executable.
For example the Microsoft version of your program may be totally
different than the Unix one.
.LE
.P
Thus one of the most fundamental things any build manager needs to
know is create a "graph" of all the dependencies (i.e. what depends on what
and what order stuff needs to be built in).
.P
Obviously if you modify only a file or two and rebuild the project you
only need to recreate those files that depend on the ones you changed.
For example if I modify \f[CW]foo.y\fP but not \f[CW]ack.c\fP then
\f[CW]ack.c\fP does not need to be recompiled but \f[CW]foo.c\fP after
it is recreated does.  All build managers know how to do this.
.H 1 "Cook \fIvs.\fP Make"
Many times the contents of entire directories depend on the
building of everything in other directories.  Make has traditionally done
this with "recursive make".  There is a basic flaw with this method though:
if you "blindly" make each directory in some preset order you are
doing stuff that is either unneeded and/or may cause problems in the build
process down the road.
For a more complete explanation, see
Recursive Make Considered Harmful\*F.
.FS
Miller, P.A. (1998).
\fIRecursive Make Considered Harmful\fP,
AUUGN Journal of AUUG Inc., 19(1), pp. 14-25.
.br
http://aegis.sourceforge.net/auug97.pdf
.FE
.P
Cook takes the opposite approach.  It makes a \fIcomplete\fP
dependency graph of your entire project then does the entire "cook" at the
root directory of your project.
.H 1 "Teaching Cook about Dependencies"
Each \fInode\fP in a dependency graph has two basic attributes.  The first
is what other nodes (if any) it depends on, and the second is a list
of actions needed to be performed to bring the node \fIup to date\fP
(bring it to a state in which any nodes that depend on it can use it's
products safely).
.P
One issue we have right off the bat is which node do we start at.
While by convention this node is usually called '\f[CW]all\fP' it does
not have to be, as we will see later it might not even have a hard coded
name at all.  Once we know where to start we need someway of linking
nodes together in the dependency graph.
.P
In cook all this functionality is handled by \fIrecipes\fP.
In basic terms a recipe is:
.BL
.LI
The name of the node so other nodes know how to link to it
(this name can be dynamic).  This name is usually the name of a file,
but not always.
.LI
A list of other recipes that need to be "cooked" before this recipe can
be processed.  The best way to think of this is to use the metaphor that
cook is based on.  That being in order to make meal at a fine restaurant
you need to make each dish.  For each dish you need to combine the
ingredients in the right order at the right time.  You keep dividing up
the task until you get to a task that does not depend on something else
like seeing if you have enough eggs to make the bread.  A dependency
graph for building a software project is almost identical except the
\fIingredients\fP are source code not food.
.LI
A list of actions to perform once all the ingredient are ready.
Again using the cooking example, in order to make a French cream sauce you
gather all the ingredients (in cook's cases the output from other recipes)
and then and \fIonly\fP then put the butter in the pan with the the flour
and brown it, then slowly mix the milk in, and finally add in the cheese.
.LE
.P
So in summary we have the following parts of a recipe:
.BL
.LI
The name of the recipe's node in the graph
.LI
A list of ingredients needed to cook the recipe
.LI
A list of steps performed to cook the recipe
.LE
.P
From the top level view in order to make a hypothetical project we
do the following recipes:
.BL
.LI
We repeatedly process dependency graph nodes until we get a \fIleaf\fP node
(one that does not have any ingredients).  Namely we go from the
general to the specific not the other way.
.LI
Visit the \f[CW]all\fP recipe which has \f[CW]program1\fP and
\f[CW]program2\fP as its ingredients
.LI
Visit the \f[CW]program1\fP node which has \f[CW]program1.o\fP and
\f[CW]libutils.a\fP as its ingredients
.LI
Visit \f[CW]program1.o\fP which has \f[CW]program1.c\fP and
\f[CW]program1.h\fP as its ingredients
.LI
Visit \f[CW]program1.c\fP to discover that it is a leaf node,
because the file already exists we need to do nothing to create it.
.LI
Visit \f[CW]program1.h\fP to discover that it is a leaf node,
because the file already exists we need to do nothing to create it.
.LI
Now that we have all the ingredients for \f[CW]program1.o\fP we can
cook it with a command something like
.eB
gcc -c program1.c \e
    -o program1.o
.eE
.LI
Visit the \f[CW]libutils.a\fP node which has \f[CW]lib1.o\fP as its
only ingredient.
.LI
Visit \f[CW]lib1.c\fP to discover that it is a leaf node,
because the file already exists we need to do nothing to create it.
.LI
Now that we have all the ingredients for \f[CW]lib1.o\fP we can
cook it with a command something like
.eB
gcc -c lib1.c -o lib1.o
.eE
.LI
Now that we have all the ingredients for \f[CW]libutils.a\fP we can
cook it with a command something like
.eB
rm libutils.a
ar cq libutils.a lib1.o
.eE
.LI
Now that we have all the ingredients for \f[CW]program1\fP we can
cook it with a command something like
.eB
gcc program1.o libutils.a \e
    -o program1
.eE
.LI
Visit the \f[CW]program2\fP node which has \f[CW]program2.o\fP and
\f[CW]libutils.a\fP as its ingredients
.LI
Visit \f[CW]program2.o\fP which has \f[CW]program2.c\fP and
\f[CW]program1.h\fP as its ingredients
.LI
Visit \f[CW]program2.c\fP to discover that it is a leaf node,
because the file already exists we need to do nothing to create it.
.LI
Visit \f[CW]program2.h\fP to discover that it is a leaf node,
because the file already exists we need to do nothing to create it.
.LI
Now that we have all the ingredients for \f[CW]program2.o\fP we can
cook it with a command something like
.eB
gcc -c program2.c \e
    -o program2.o
.eE
.LI
There is no need to visit the \f[CW]libutils.a\fP node,
or any of its ingredient nodes,
because Cook remembers that they have been brought up to date already.
.LI
Now that we have all the ingredients for \f[CW]program2\fP we can
cook it with a command something like
.eB
gcc program2.o libutils.a \e
    -o program2
.eE
.LI
Return to the \f[CW]all\fP recipe and find that we have cooked all the
ingredients and there are no other actions for it.  We are done and our
entire project is built!
.LE
.P
Now what happens if I say modify \f[CW]program2.c\fP all we have to
do is walk to the entire graph from \f[CW]all\fP and we find that
\f[CW]program2.c\fP has changed, and do any node which depends on
\f[CW]program2.c\fP needs to be brought up to date, and any nodes which
depend on \fIthem\fP, and so on.
In this example, this would be
\f[CW]program2.c\fP \(->
\f[CW]program2.o\fP \(->
\f[CW]program2\fP \(->
\f[CW]all\fP.
.H 1 "Recipe Syntax"
All statements, recipes and otherwise, are in the form of
.eB
\fIstatement\fP;
.eE
Note the terminating simicolon (\f[CW];\fP).
An example statement is
.eB
echo aryeh;
.eE
The only time the the simicolon (\f[CW];\fP) is not needed is in compound
statements surrounded by \f[CW]{\fP curly braces \f[CW]}\fP.  In general
the convention is to follow the same general form that C uses, as it is
with most modern programming languages.  This means that for the main
part almost everything you have learned about writing legal statements
works just fine in cook.  The only exception are the \f[CW][\fP square
brackets \f[CW]]\fP used instead of \f[CW](\fP parentheses \f[CW])\fP
in most cases.
.P
The general form of a recipe, there are some advanced options
that do not fit well into this format, is:
.eB
\fIname\fP: \fIingredients\fP
{
    \fIactions\fP
}
.eE
.P
Note: the actions and ingredients are optional.
.\" Thus the simplest possible recipe is \f[CW]foo:;\fP which does absolutely
.\" nothing.
.P
Here is a recipe from the above example:
.eB
program1.o: program1.c program1.h
{
    gcc -c program1.c
        -o program1.o;
}
.eE
.P
The only thing to remember here is that \f[CW]program1.c\fP either has
to exist or Cook needs to know how to cook it.  If you reference an
ingredient that Cook does not know how to cook you get the following
error:
.eB
.\" -----------------------|
cook: program1: don't know how
cook: cookfile: 1: "program1"
    not derived due to errors
    deriving "program1.o"
.eE
.P
All this says is there is no algorithmic way to build \f[CW]example1.o\fP
that Cook can find.
.P
A \fIcookbook\fP file can contain zero or more recipes.  If there is
no \fIdefault\fP recipe (the first recipe whose name is hard coded)
you get the following error:
.eB
cook: no default target
.eE
.P
Most of the time this just means that Cook cannot figure out
what the "concrete" name of a recipe is based solely by reading the cookbook.
By default cook looks for the cookbook in "Howto.cook" [note 1].
.H 1 "A Sample Project"
For the remainder of the tutorial we will be using the following
sample project source tree:
.so fig1.so
.P
The final output of the build process will be completely working
and installed executables of prog1 and prog2 installed in /usr/local/bin and
the documentation being placed in /usr/local/share/doc/myproj.
.H 1 "Our First Cookbook"
The first step in making a cookbook is to sketch out the decencies
in our sample project the graph would be:
.\" .P
.\" Key:
.\" .TS
.\" tab(;);
.\" r l.
.\" ULB;/usr/local/bin
.\" ULSDM;/usr/local/share/doc/myprog
.\" P1;prog1
.\" P2;prog2
.\" .TE
.P
.so fig2.so
.P
.ft R
Now we know enough to write the first version of our cookbook.
The cookbook which follows doesn't actually cook anything, because it
contains ingredients and no actions.  We will add the actions needed
in a later section.  Here it is:
.eB
.\" -----------------------|
/* top level target */
all: /usr/local/bin/prog1
    /usr/local/bin/prog2
    /usr/local/share/doc/prog1/manual
    /usr/local/share/doc/prog2/manual
    ;
.sp 0.5
.\" -----------------------|
/* where to install stuff */
/usr/local/bin/prog1:
    bin/prog1 ;
/usr/local/bin/prog2:
    bin/prog2 ;
/usr/local/share/doc/prog1/manual:
    doc/prog1/manual ;
/usr/local/share/doc/prog2/manual:
    doc/prog2/manual ;
.sp 0.5
.\" -----------------------|
/* how to link each program */
bin/prog1:
    prog1/main.o
    prog1/src1.o
    prog1/src2.o
    lib/liblib.a ;
bin/prog2:
    prog2/main.o
    prog2/src1.o
    prog2/src2.o
    lib/liblib.a ;
.sp 0.5
.\" -----------------------|
/* how to use yacc */
prog2/src2.c: prog2/src2.y ;
.sp 0.5
.\" -----------------------|
/* how to compile sources */
prog1/main.o: prog1/main.c ;
prog1/src1.o: prog1/src1.c ;
prog1/src2.o: prog1/src2.c ;
prog2/main.o: prog2/main.c ;
prog2/src1.o: prog2/src1.c ;
prog2/src2.o: prog2/src2.c ;
lib/src1.o: lib/src1.c ;
lib/src2.o: lib/src2.c ;
.sp 0.5
.\" -----------------------|
/* include file dependencies */
prog1/main.o: lib/lib.h ;
prog1/src1.o: lib/lib.h ;
prog1/src2.o: lib/lib.h ;
prog2/main.o: lib/lib.h ;
prog2/src1.o: lib/lib.h ;
prog2/src2.o: lib/lib.h ;
lib/src1.o: lib/lib.h ;
lib/src2.o: lib/lib.h ;
.sp 0.5
.\" -----------------------|
/* how to build the library */
lib/liblib.a:
    lib/src1.o
    lib/src2.o ;
.eE
.P
In order to cook this cookbook just type the
.eB
cook
.eE
command in the same
directory as the cookbook is in.
.H 1 "Soft coding Recipes"
One of the most glaring problems with this first version of our
cookbook is it hard codes everything.  This has two problems:
.BL
.LI
We have to be super verbose in how we describe stuff since
we have to specify every single recipe by hand.
.LI
If we add new files (maybe we add a third executable to the project) we
have to rewrite the cookbook for \fIevery\fP file we add.
.LE
.P
Fortunately, Cook has a way of automating the build with implicit recipes.
It has a way of saying how to move from any arbitrary \f[CW].c\fP file to
its \f[CW].o\fP file.
.P
Cook provides several methods for being able to soft code these
relationships.  This section discusses file "patterns" that can be used
to do pattern matching on what recipe to cook for a given file.
.P
Note on pattern matching notation used in this section:
.P
\fI[string]\fP means the matched pattern.
.P
The first thing to keep in mind about cook's pattern matching
is once a pattern is matched it will have the same value for the remainder
of the recipe.  So for example if we matched prog/[src1].c then any other
reference to that pattern will also return src1.  For example:
.eB
prog/\fI[src1]\fP.o: prog/\fI[src1]\fP.o ;
.eE
if we matched \fIsrc1\fP on the first match
(\f[CW]prog1/\fP\fI[src1]\fP\f[CW].o\fP) then we will always match
\fIsrc1\fP in this recipe (\f[CW]prog1/\fP\fI[src1]\fP\f[CW].c\fP).
.P
Cook uses the percent (\f[CW]%\fP) character to denote matches of the
relative file name (no path).  Thus the above recipe would be written:
.eB
prog/%.o: prog/%.c ;
.eE
.P
Cook also lets you match the full path of a file, or parts of the path to
a file.  This done with \f[CW]%\fP\fIn\fP where \fIn\fP is a part number.
For example
.eB
/usr/local/bin/prog1
.eE
could match the pattern
.eB
/%1/%2/%3/%
.eE
with the parts be assigned
.TS
center,tab(;);
r l.
%1;usr
%2;local
%3;bin
%;prog1
.TE
.P
Note that the final component of the path has no \fIn\fP (there is
no \f[CW]%4\fP for \f[CW]prog1\fP).
If we want to reference the whole path,
Cook uses %0 as a special pattern to do this.
.eB
/usr/local/bin/prog1
.eE
could match the pattern
.eB
%0%
.eE
with the parts be assigned
.TS
center,tab(;);
r l.
%0;/usr/local/bin/
%;prog1
.TE
.P
Patterns are connected together thus \f[CW]%0%.c\fP will match any
\f[CW]\&.c\fP file in any pattern.
.P
Let's rewrite the cookbook for our sample project using pattern
matching.  The relevant portions of our cookbook are replaced by
.eB
.\" -----------------------|
/* how to use yacc */
%0%.c: %0%.y;
.sp 0.5
.\" -----------------------|
/* include file dependencies */
%0%.c: lib/lib.h;
.sp 0.5
.\" -----------------------|
/* how to compile sources */
%0%.o: %0%.c;
.eE
.P
When constructing the dependency graph Cook will match the the
first recipe it sees that meets all the requirements to meet a
given pattern.  I.e. if we have a pattern for \f[CW]prog1/%.c\fP
and one for \f[CW]%0%.o\fP and it needs to find the right recipe for
\f[CW]prog1/src.o\fP it will match the one that appears first in the
cookbook.  So if the first one is \f[CW]%0%.c\fP then it does that recipe
even if we meant for it to match \f[CW]prog1/%.c\fP.
.H 1 "Arbitrary Statements and Variables"
Any statement that is not a recipe, and not a statment inseide a
recipe, is executed as soon as it is seen.  For example I can have a
\f[CW]Howto.cook\fP file that only contains the following line:
.eB
echo Aryeh;
.eE
and when ever I ise the \f[CW]cook\fP command it will print my name.
.P
This in and upon it self is quite pointless but it does give a
clue about how we can set some cookbook-wide values.  Now the question is
how do we symbolically represent those variables.
.P
Cook has only one type of variable and that is a list of string literals,
i.e. \f[CW]"ack"\fP, \f[CW]"foo"\fP, \f[CW]"bar"\fP, \fIetc\fP.
There are no restrictions on
how you name variables, except they can not be reserved words, this
is pretty close to the restrictions most programming languages have.
There is one major difference though: variables can start with numbers and
contain punctuation characters.  Additionally you can vary variable names,
i.e. the name of the actual variable can use a variable expression (this
is hard to explain but easy to show which we will do in a few paragraphs).
.P
All variables, when queried for their value, are \f[CW][\fP in square
brackets \f[CW]]\fP for example if the "name" variable contains "Aryeh"
then:
.eB
echo [name];
.eE
Has exactly the same result as the previous example.  Variables
are simply set by using \f[CW]var = value;\fP  For example:
.eB
name = Aryeh;
echo [name];
.eE
Let's say I need to have two variables called 'prog1_obj' and 'prog2_obj'
that contain a list of all the .o ingredients in the prog1 and prog2
directories respectively.  Obviously the same operation that produces
the value of prog1_obj is identical to the one that produces prog2_obj
except it operates on a different directories.  So why then do we
need two different operations to do the same thing, this violates the
principle of any given operation it should only occur in one place.
In reality all we need to do is have some way of changing the just the
variable name and not the values it produces.  In cook we do this with
something like [[dir_name]_obj].  The actual procedure for getting the
list of files will be covered in the "control structures" section.
.P
Let's revise some sections of our sample project's cookbook to take
advantage of
variables:
.eB
.\" -----------------------|
/* where to install stuff */
prefix = /usr/local;
idoc_dir = [prefix]/share/doc;
ibin_dir = [prefix]/bin;
.sp 0.5
.\" -----------------------|
/* top level target */
all:
    [ibin_dir]/prog1
    [ibin_dir]/prog2
    [idoc_dir]/prog1/manual
    [idoc_dir]/prog2/manual;
.sp 0.5
.\" -----------------------|
/* where to install each program */
[ibin_dir]/%: bin/% ;
[idoc_dir]/%/manual: doc/%/manual ;
.eE
.P
As you can see we didn't make the cookbook any simpler because we
do not know how to intelligently set stuff based on what the actual file
structure of our project.  The only thing we gain here is the ability to
change where we install stuff very quickly be just changing install_dir.
We also gain a little flexibility in how we name the directories in our
source tree.
.H 1 "Using Built-in Functions"
If all you could do was set variables to static values and do
pattern matching cook would not be very useful, i.e. every time we add
a new source file to our project we need to rewrite the cookbook.  We
need some way to extract useful data from variables and leave out what
we do not want.  For example if we want to know what all the .c files
in the prog1 directory are we just ask for all files that match prog1/%.c.
We could use the match_mask built-in function to extract the needed sublist
of files.  Built-in functions can do many other manipulations of our source
tree contents and how to process them.  In general I will introduce a given
built-in function as we encounter them.
.P
As far as cook is concerned, for the most part, functions and
variables are treated identically.  This means anywhere where you would
use a variable you can use a function.  In general a function is
called like this:
.eB
[func arg1 arg2 ... argN]
.eE
.P
For example:
.eB
name = [foobar aryeh];
.eE
.H 1 "Source Tree Scanning"
The first thing we need to do to automate the process of handling
new files is to collect the list of source files.  In order to do this
we need to ask the operating system to give us a list of all files in
a directory and all it's subdirectories.  In Unix the best way to do this
is with the find(1) command.  Thus to get a complete list of all files in
say the current directory we do:
.eB
find . -print
.eE
or any variation thereof.
.P
Great, now how do we get the output of find into a variable so cook can
use it.  Well, the \f[CW]collect\fP function does this.  We then just
assign the results of \f[CW]collect\fP to a list of files, build experts
like to call this the manifest.  So here is how we get the manifest:
.eB
.\" -----------------------|
manifest = [stripdot
    [collect find . -print]];
.eE
.P
That is all nice and well but how do we get the list of source files
in \f[CW]prog1\fP only, for example.   There is a function called
\f[CW]match_mask\fP that does this.  The \f[CW]match_mask\fP function
returns all "words" that match some pattern in our list.  For example
to get a list of all \f[CW].c\fP files in our project we do:
.eB
.\" -----------------------|
src = [match_mask %0%.c
    [manifest]];
.eE
It is fine to know what files are already in our source tree but what
we really want to do is find the list of files that need to be cooked.
We use the \f[CW]fromto\fP function to do this.  The \f[CW]fromto\fP
function takes all the words in our list and transforms all the names
which match to some other name.  For example to get a list of all the
\f[CW].o\fP files we need to cook we do:
.eB
.\" -----------------------|
obj = [fromto %0%\&.c %0%\&.o
       [src]];
.eE
It is rare that we need to know about the existence of \f[CW].c\fP files
since in most cases, unless they are derived from cooking something else,
they either exist or they do not exist.  In the case of them not existing
the \f[CW].o\fP target for that source should fail.  For this reason we
really do not need a \f[CW]src\fP variable at all.  Remember I mentioned
that a function call can be used anywhere a variable can.  This means
that we can do the \f[CW]match_mask\fP call in the same line that we do
the fromto.  Thus the new statement is:
.eB
.\" -----------------------|
obj = [fromto %0%.c %0%.o
       [match_mask %0%.c
        [manifest]]];
.eE
Time to update some sections of our sample project's cookbook one more time:
.eB
.\" -----------------------|
/* info about our files */
manifest =
    [collect find . -print];
obj = [fromto %0%.c %0%.o
       [match_mask %0%.c
        [manifest]]];
.sp 0.5
.\" -----------------------|
/* how to build each program */
prog1_obj = [match_mask
    prog1/%.o [obj]];
prog2_obj = [match_mask
    prog2/%.o [obj]];
bin/%: [%_obj] lib/lib.a;
.sp 0.5
.\" -----------------------|
/* how to build the library */
lib_obj = [match_mask lib/%.o
    [obj]];
lib/lib.a: [lib_obj];
.eE
.P
The important thing to observe here is that it is now possible to add
a source file to one of the probram or library directories and Cook
will automagically notice, without any need to modify the cookbook.
It doesn't matter whether there are 3 files or 300 in these directories,
the cookbook is the same.
.H 1 "Flow Control"
If there was no conditional logic in programming would be rather
pointless, who wants to write I program that can only do something once,
the same is true in cook.  Even though the stuff we need to conditional in
a build is often very trivial as far as conditional logic goes, namely there
are if statements and the equivalent of while loops and thats all.
.P
If statements are pretty straight forward.  If you are used to C, C++,
\fIetc\fP, the only surprise is the need for the \f[CW]then\fP keyword.
Here is a example if statement:
.eB
.\" -----------------------|
if [not [count [file]]] then
    echo no file provided;
.eE
The count function returns the number of words in the "file" list
and the not function is true if the argument is 0.  Other then that the
if statement works much the way you would expect it to.
.P
Cook has only one type of loop that being the \f[CW]loop\fP statement and
it takes no conditions.  A loop is terminated by the \f[CW]loopstop\fP
statement (like a C \fIbreak\fP statement).  Other then that loops pretty
much work the way you expect them to.  Here is an example loop:
.eB
.\" -----------------------|
/* set the loop "counter" */
list = [kirk spock 7of9
    janeway worf];
.sp 0.5
/* do the loop */
loop word = [list]
{
    /* print the word */
    echo [word];
}
.eE
.\" Except for the obvious part of how the loop works the only thing to
.\" note here is head returns the first word in the list and tail returns
.\" everything except the first word.
.H 1 "Special Variables"
Like most scripting languages Cook has a set of predefined variables.
While most of them are used internally by Cook and not by the user,
one of them deserves special mention and that is \f[CW]target\fP.
The \f[CW]target\fP variable has no meaning out side of recipes but inside
recipes it refers to the current recipe's target's "real" name, i.e. the
one that Cook "thinks" it is currently building, not the soft coded name
we provided in the cookbook.  For example in our sample project's cook
book if we where compiling \f[CW]lib/src1.c\fP into \f[CW]lib/src.o\fP
the \f[CW]%0%.o: %0%.c;\fP recipe would, as far as Cook is concerned,
actually be \f[CW]lib/src1.o: lib/src1.c;\fP  The recipe name, and thus
the \f[CW][target]\fP, of this is set to the \f[CW]lib/src.o\fP string.
.P
There are other special variables described in the Cook User Guide.
You may want to look them up and use them when you start writing more
advanced cookbooks.
.H 1 "Super Soft coding"
Now we know enough so we can make Cook handle building an arbitrary
number of programs in our sample project.  Note the following example
assumes that all program directories contain a \f[CW]main.c\fP file
and no other directory contains it.  The best way to understand what
is needed it to look at the sample cookbook for this line by line.
So here are the rewritten sections of our sample cookbook:
.eB
.\" -----------------------|
/* names of the programs */
progs = [fromto %/main.c %
         [match_mask %/main.c
          [manifest]]];
.sp 0.5
.\" -----------------------|
/* top level target */
all:
    [addprefix [ibin_dir]/
        [progs]]
    [prepost [idoc_dir]/ /manual
        [progs]];
.sp 0.5
.\" -----------------------|
/* how to build each program */
loop prog = [progs]
{
    [prog]_obj = [match_mask
        [prog]/%.o [obj]];
}
bin/%: [%_obj] lib/lib.a;
.eE
.P
The basic idea is that we use a loop to create the list of \f[CW].o\fP
files for all programs and then we use variable variable names to
reference the right one in the recipe.
.H 1 "Scanning for Hidden Decencies"
In most real programs most \f[CW].c\fP files have a different set of
\f[CW]#include\fP lines in them.  For example \f[CW]prog1/src1.c\fP
might include \f[CW]prog1/hdr1.h\fP but \f[CW]prog1/src2.c\fP does not.
So far we have conveniently avoided this fact on the assumption that once
made \f[CW].h\fP files don't change.  Any experience with a non-trivial
project show this is not true.  So how do we automatically scan for
these dependencies?  It would not only defeat the purpose of soft coding
but would be a pain in the butt to have to encode this in the cookbook.
.P
One way of doing it is to scan each \f[CW].c\fP for \f[CW]#include\fP
lines and say any that are found represent "hidden" dependencies.
It would be fairly trivial to create a shell script or small C program
that does this.  Cook though has been nice enough to include program
that does this for us in most cases that are not insanely non-trivial.
There are several methods of using \f[CW]c_incl\fP we will only cover the
"trivial" method here, if you need higher performance refer to the Cook
User Guide, it has a whole chapter on include dependencies.
.P
The \f[CW]c_incl\fP program essentially just prints a list of
\f[CW]#include\fP files it finds in its argument.  To do this just do:
.eB
c_incl \fIprog\fP.c
.eE
.P
Now all we have to do is have Cook \f[CW]collect\fP this output on the
ingredients list of our recipe and boom we have a list of our hidden
dependencies.  Here is the rewritten portion of our sample cookbook for that:
.eB
.\" -----------------------|
/* how to build each program and
   include file dependencies */
%0%.o: %0%.c
    [collect c_incl -api %0%.c];
.eE
.P
The \f[CW]c_incl -api\fP option means if the file doesn't exist,
just ignore it.
.H 1 "Recipe Actions"
Now that we have all the decencies soft coded all we have to do actually
build our project is to tell each recipe how to actually cook the target
from the ingredients.  This is done by adding actions to a recipe.  The
actions are nothing more "simple" statements that are bound to a recipe.
This is done by leaving off the trailing semicolon (\f[CW];\fP) on the
recipe and putting the actions inside \f[CW]{\fP curly braces \f[CW]}\fP.
This is best shown by example.  So here is our final cookbook for our
sample project:
.eB
.\" -----------------------|
/* where to install stuff */
prefix = /usr/local;
idoc_dir = [prefix]/share/doc;
ibin_dir = [prefix]/bin;
.sp 0.5
.\" -----------------------|
/* info about our files */
manifest =
    [collect find . -print];
obj = [fromto %0%.c %0%.o
       [match_mask %0%.c
        [manifest]]];
.sp 0.5
.\" -----------------------|
/* names of the programs */
progs = [fromto %/main.c %
         [match_mask %/main.c
          [manifest]]];
.sp 0.5
.\" -----------------------|
/* top level target */
all:
    [addprefix [ibin_dir]/
        [progs]]
    [prepost [idoc_dir]/ /manual
        [progs]];
.sp 0.5
.\" -----------------------|
/* how to build each program */
loop prog = [progs]
{
    [prog]_obj = [match_mask
        [prog]/%.o [obj]];
}
bin/%: [%_obj]
{
    gcc [%_obj] -o [target];
}
.sp 0.5
.\" -----------------------|
/* how to build the library */
lib_obj = [match_mask lib/%.o
    [obj]];
lib/lib.a: [lib_obj]
{
    rm [target];
    ar cq [target] [lib_obj];
}
.sp 0.5
.\" -----------------------|
/* how to "install" stuff */
[ibin_dir]/%: bin/%
{
    cp bin/% [target];
}
[idoc_dir]/%/manual: doc/%/manual
{
    cp doc/%/manual [target];
}
.sp 0.5
.\" -----------------------|
/* how to compile sources*/
%0%.o: %0%.c
    [collect c_incl -api %0%.c]
{
    gcc -c %0%.c -o [target];
}
.eE
.H 1 "Advanced Features"
Even though the tutorial part of this document is done, I feel
it is important to just mention some advanced features not covered in
the tutorial.  Except for just stating the basic nature of these features
I will not go into detail on any given one.
.BL
.LI
Platform polymorphism.  This is where Cook can automatically detect
what platform you are on and do some file juggling so that you build
for that platform.
.LI
Support for private work areas.  If you are working within a change
management system, Cook knows how to query it for only the files you
need to work on.  This includes the automatic check-out and in of private
copies of those files.
.LI
Parallel builds.  For large projects it is possible to spread the build
over several processors or machines.
.P
Conditional recipes.  It is possible to execute a recipe one way if
certain conditions are met and an other way if they are not.
.LE
.P
Many more that are not directly supported by Cook but can easily
be integrated using shell scripts.
.H 1 "Contacts"
If you find any bugs in this tutorial please send a bug report
to Aryeh M. Friedman \f[CW]<aryeh@m-net.arbornet.org>\fP.
.P
The Cook web site is \f[CW]http:\%//www.canb.auug.org.au\%/~millerp\%/cook/\fP
.P
If you want to contact Cook's author, send email to
Peter Miller \f[CW]<millerp@canb.auug.org.au>\fP.
.\" .H 1 "Notes"
.\" In the project that forced me to switch to Cook from make
.\" there are multiple libraries each with a "includes.h" file that recursively
.\" includes all the "local" includes and any other library headers needed (i.e.
.\" libutils.h for example).  When the library is built the include.h file
.\" has to be "cooked" in that it has to be scanned for local #include's and
.\" each mentioned include file needs to be "sucked" into the main include,
.\" this is done by a program called pack_h that is unique to that particular
.\" project.  For various reasons the dependency checking in this case has
.\" to be done by a shell script vs. c_incl.
