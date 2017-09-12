.\"
.\"     cook - file construction tool
.\"     Copyright (C) 1990-1994, 1997, 1998, 2007-2010 Peter Miller
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
.\" must be formatted using "groff -s -t -mm refman.t | lpr"
.\"
.if n .ftr CB B
.if n .ftr CI I
.if n .ftr CW R
.if n .ftr C  R
.if t .ds HF HB H H HI HI HI HI HI HI HI
.if t .ds HP 16 14 12 12 10 10 10 10 10 10
.PH "''''"
.PF "''''"
.so etc/libdir.so
.so etc/version.so
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
.\" ---------------------------------------------------------------------------
\&.
.sp 2i
.ps 36
.vs 38
.ce 1
Cook
.ps 24
.vs 26
.ce 1
A File Construction Tool
.sp 0.5i
.ce 1
.ps 36
.ps 38
User Guide
.sp 1i
.ps 18
.vs 20
.ce 1
Peter Miller
.ft I
.ce 1
pmiller@opensource.org.au
.ft P
.\" ---------------------------------------------------------------------------
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
This document describing the Cook program,
and the Cook program itself,
are
.br
Copyright \*(C)
.nr d) \n(.d
\*(Y) Peter Miller
.if '\n(d)'\n(.d' .br
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
.\" ---------------------------------------------------------------------------
.PH "'Cook''User Guide'"
.bp 1
.PF "'Peter Miller''Page %'"
.\" .MT 4 1
.SK
.nr Ej 1
.so lib/en/user-guide/intro.so
.so lib/en/user-guide/history.so
.so lib/en/user-guide/intro1.so
.so lib/en/user-guide/intro2.so
.so lib/en/user-guide/parallel.so
.so lib/en/user-guide/include.so
.so lib/en/user-guide/large.so
.so lib/en/user-guide/language.so
.so lib/en/user-guide/builtin.so
.so lib/en/user-guide/variables.so
.so lib/en/user-guide/functions.so
.so lib/en/user-guide/how.so
.so lib/en/user-guide/option.so
.so lib/en/user-guide/match.so
.so lib/en/user-guide/system.so
.so lib/en/user-guide/glossary.so
.if o .bp
.pn 1001
.TC
.if o .bp
