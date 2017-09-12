Summary: a file construction tool
Name: cook
Version: 2.31
Release: 1
Copyright: GPL
Group: Development/Building
Source: http://www.canb.auug.org.au/~millerp/cook/cook-2.31.tar.gz
URL: http://www.canb.auug.org.au/~millerp/cook/
BuildRoot: /tmp/cook-build-root
Icon: cook-${version}/cook.png
Provides: perl(host_lists.pl)

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

%files psdocs
%attr(0644,root,bin) %_prefix/share/cook/en/refman.ps
%attr(0644,root,bin) %_prefix/share/cook/en/tutorial.ps
%attr(0644,root,bin) %_prefix/share/cook/en/user-guide.ps

%files dvidocs
%attr(0644,root,bin) %_prefix/share/cook/en/refman.dvi
%attr(0644,root,bin) %_prefix/share/cook/en/tutorial.dvi
%attr(0644,root,bin) %_prefix/share/cook/en/user-guide.dvi
