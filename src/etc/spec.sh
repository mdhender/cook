#!/bin/sh
#
#	cook - file construction tool
#	Copyright (C) 1993-1997, 1999, 2001, 2003, 2004 Peter Miller;
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
# MANIFEST: shell script to generate RedHat spec file
#
version=${version-0.0.0}
echo 'Summary: a file construction tool'
echo 'Name: cook'
echo "Version: ${version}"
echo 'Release: 1'
echo 'Copyright: GPL'
echo 'Group: Development/Building'
echo "Source: http://www.canb.auug.org.au/~millerp/cook/cook-${version}.tar.gz"
echo 'URL: http://www.canb.auug.org.au/~millerp/cook/'
echo 'BuildRoot: /tmp/cook-build-root'
echo 'Icon: cook-${version}/cook.gif'
echo 'Provides: perl(host_lists.pl)'

echo ''

cat << 'fubar'
%description
Cook is a tool for constructing files. It is given a set of files to
create, and recipes of how to create them. In any non-trivial program
there will be prerequisites to performing the actions necessary to
creating any file, such as include files.  The cook program provides a
mechanism to define these.

When a program is being developed or maintained, the programmer will
typically change one file of several which comprise the program.  Cook
examines the last-modified times of the files to see when the
prerequisites of a file have changed, implying that the file needs to be
recreated as it is logically out of date.

Cook also provides a facility for implicit recipes, allowing users to
specify how to form a file with a given suffix from a file with a
different suffix.  For example, to create filename.o from filename.c

* Cook is a replacement for the traditional make(1) tool.  However, it
  is necessary to convert makefiles into cookbooks using the make2cook
  utility included in the distribution.

* Cook has a simple but powerful string-based description language with
  many built-in functions.  This allows sophisticated filename
  specification and manipulation without loss of readability or
  performance.

* Cook is able to use fingerprints to supplement file modification
  times.  This allows build optimization without contorted rules.

* Cook is able to build your project with multiple parallel threads,
  with support for rules which must be single threaded.  It is possible
  to distribute parallel builds over your LAN, allowing you to turn your
  network into a virtual parallel build engine.

If you are putting together a source-code distribution and planning to
write a makefile, consider writing a cookbook instead.  Although Cook
takes a day or two to learn, it is much more powerful and a bit more
intuitave than the traditional make(1) tool.  And Cook doesn't interpret
tab differently to 8 space characters!

%package psdocs
Summary: Cook documentation, PostScript format
Group: Development/Building

%description psdocs
Cook documentation in PostScript format.

%package dvidocs
Summary: Cook documentation, DVI format
Group: Development/Building

%description dvidocs
Cook documentation in DVI format.

%prep
%setup -q
%configure
grep datadir config.status

%build
make

%install
test -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / && rm -rf "$RPM_BUILD_ROOT"
make RPM_BUILD_ROOT=$RPM_BUILD_ROOT install

%clean
test -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / && rm -rf "$RPM_BUILD_ROOT"

%files
%defattr (-,root,root)
fubar

#
# remember things for the %files section
#
files=
binfiles=
psdocs=
dvidocs=

remember_prog()
{
	if eval "test \"\${prog_${1}-no}\" != yes"
	then
		eval "prog_${1}=yes"
		binfiles="$binfiles %_prefix/bin/${1}"
	fi
}

for file in $*
do
	case $file in

	common/*)
		;;
	fstrcmp/* | file_check/*)
		;;
	etc/*)
		;;

	*/main.c)
		dir=`echo $file | sed 's|/.*||'`
		remember_prog $dir
		;;

	test/*/*)
		;;
	lib/*/building/*)
		;;
	lib/*/lsm/*)
		;;
	lib/*/readme/*)
		;;
	lib/*/release/*)
		;;

	lib/*/LC_MESSAGES/common.po)
		;;
	lib/*/LC_MESSAGES/fstrcmp.po)
		;;

	lib/*.po)
		stem=`echo $file | sed 's|^lib/\(.*\)\.po$|\1|'`
		dst="%_prefix/lib/cook/$stem.mo"
		files="$files $dst"
		;;

	lib/*/*/*.so)
		;;
	lib/*/*/*.pic)
		;;

	lib/*/man?/*)
		# some versions of RPM gzip man pages for free, so use a
		# wild card to find them, rather than an exact name.  Sigh.
		stem=`echo $file | sed 's|^lib/||'`
		files="$files %_prefix/share/cook/${stem}*"

		case $file in
		lib/en/*)
			stem2=`echo $file | sed 's|^lib/en/||'`
			files="$files %_prefix/share/man/${stem2}*"
			;;
		esac
		;;

	lib/*/*/main.*)
		stem=`echo $file | sed 's|^lib/\(.*\)/main.*$|\1|'`
		psdocs="$psdocs %_prefix/share/cook/$stem.ps"
		dvidocs="$dvidocs %_prefix/share/cook/$stem.dvi"
		txtdocs="$txtdocs %_prefix/share/cook/$stem.txt"
		;;

	lib/*)
		rest=`echo $file | sed 's|^lib/||'`
		dst="%_prefix/share/cook/$rest"
		files="$files $dst"
		;;

	script/*.in)
		rest=`echo $file | sed 's|^script/\(.*\)\.in|\1|'`
		binfiles="$binfiles %_prefix/bin/$rest"
		;;
	*)
		;;
	esac
done
(
for file in $binfiles
do
	echo "%attr(0755,root,bin) $file"
done
for file in $files
do
	echo "%attr(0644,root,bin) $file"
done
for file in $txtdocs
do
	echo "%attr(0644,root,bin) $file"
done
) | sort +1

echo ''
echo '%files psdocs'
for file in $psdocs
do
	echo "%attr(0644,root,bin) $file"
done

echo ''
echo '%files dvidocs'
for file in $dvidocs
do
	echo "%attr(0644,root,bin) $file"
done
