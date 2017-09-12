/*
 *	cook - file construction tool
 *	Copyright (C) 1994, 1997, 2001 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate assign statements
 */

#include <emit.h>
#include <stmt/assign.h>
#include <symtab.h>
#include <trace.h>
#include <variable.h>

typedef struct stmt_assign_ty stmt_assign_ty;
struct stmt_assign_ty
{
	STMT
	int		override;
	blob_list_ty	*lhs;
	int		op;
	blob_list_ty	*rhs;
	int		check_env;
};

int stmt_assign_environment_variables;


static void destructor _((stmt_ty *));

static void
destructor(that)
	stmt_ty		*that;
{
	stmt_assign_ty	*this;

	trace(("assign::destructor()\n{\n"/*}*/));
	this = (stmt_assign_ty *)that;
	blob_list_free(this->lhs);
	blob_list_free(this->rhs);
	trace((/*{*/"}\n"));
}


static void emit _((stmt_ty *));

static void
emit(that)
	stmt_ty		*that;
{
	stmt_assign_ty	*this;
	long		j;

	trace(("assign::emit()\n{\n"/*}*/));
	this = (stmt_assign_ty *)that;
	if (this->op != stmt_assign_op_plus && !this->override)
	{
		emit_set_file(this->lhs->list[0]->file_name);
		emit_str("if [not [defined ");
		emit_string(this->lhs->list[0]->text);
		emit_str("]] then\n");
		if (this->check_env && !this->rhs->length)
		{
			this->check_env = 0;
			blob_list_append
			(
				this->rhs,
				blob_alloc
				(
					str_format
					(
						"[getenv %S]",
						this->lhs->list[0]->text
					),
					this->lhs->list[0]->file_name,
					this->lhs->list[0]->line_number
				)
			);
		}
		if (this->check_env)
		{
			emit_str("{\n"/*}*/);
			emit_indent_more();
			emit_string(this->lhs->list[0]->text);
			emit_str(" = [getenv ");
			emit_string(this->lhs->list[0]->text);
			emit_str("];\n");
			emit_str("if [not [");
			emit_string(this->lhs->list[0]->text);
			emit_str("]] then\n");
		}
		emit_indent_more();
	}

	blob_emit(this->lhs->list[0]);
	emit_str(" =");
	if (this->op == stmt_assign_op_plus)
	{
		emit_char(' ');
		emit_char('[');
		blob_emit(this->lhs->list[0]);
		emit_char(']');
	}
	for (j = 0; j < this->rhs->length; ++j)
	{
		emit_char(' ');
		blob_emit(this->rhs->list[j]);
	}
	if (!this->rhs->length)
		emit_char(' ');
	emit_str(";\n");

	if (this->op != stmt_assign_op_plus && !this->override)
	{
		emit_indent_less();
		if (this->check_env)
		{
			emit_indent_less();
			emit_str(/*{*/"}\n");
		}
	}
	trace((/*{*/"}\n"));
}


static stmt_method_ty method =
{
	sizeof(stmt_assign_ty),
	"assign",
	0, /* constructor */
	destructor,
	emit,
};


stmt_ty *
stmt_assign_alloc(override, lhs, op, rhs)
	int		override;
	blob_ty		*lhs;
	int		op;
	blob_list_ty	*rhs;
{
	stmt_assign_ty	*this;
	blob_list_ty	*lhs2;
	blob_list_ty	*rhs2;
	size_t		j;

	trace(("stmt_assign_alloc()\n{\n"/*}*/));
	trace(("lhs = \"%s\"\n", lhs->text->str_text));
	this = (stmt_assign_ty *)stmt_alloc(&method);

	/*
	 * turn the make names into cook names
	 *	(if they aren't already)
	 */
	lhs2 = blob_list_alloc();
	if (op == stmt_assign_op_default)
	{
		this->check_env = (stmt_assign_environment_variables != 0);
		op = stmt_assign_op_normal;
		blob_list_append(lhs2, lhs);
	}
	else
	{
		variable_rename(lhs, lhs2, &this->ref, VAREN_QUOTE_SPACES);
		blob_free(lhs);
		this->check_env = 0;
	}

	/*
	 * translate the names and functions of the value
	 */
	for (j = 0; j < lhs2->length; ++j)
	{
		if (op == stmt_assign_op_normal)
		{
			trace(("mdef %s\n", lhs2->list[j]->text->str_text));
			string_list_append_unique
			(
				&this->mdef,
				lhs2->list[j]->text
			);
		}
		string_list_append_unique(&this->cdef, lhs2->list[j]->text);
		if (op == stmt_assign_op_plus)
		{
			string_list_append_unique
			(
				&this->ref,
				lhs2->list[j]->text
			);
		}
	}
	rhs2 = blob_list_alloc();
	variable_rename_list(rhs, rhs2, &this->ref, VAREN_QUOTE_SPACES);
	blob_list_free(rhs);

	/*
	 * set the instance variables
	 */
	this->override = override;
	this->lhs = lhs2;
	this->op = op;
	this->rhs = rhs2;

	trace((/*{*/"}\n"));
	return (stmt_ty *)this;
}


static stmt_ty *default_setting _((string_ty *));

static stmt_ty *
default_setting(name)
	string_ty	*name;
{
	typedef struct table_ty table_ty;
	struct table_ty
	{
		char	*name;
		char	*value;
	};

	static table_ty table[] =
	{
		{ ".CURDIR", "$(pathname .)", },
		{ "AR", "ar", },
		{ "ARFLAGS", "rv", },
		{ "AS", "as", },
		{ "CC", "cc", },
		{ "CXX", "g++", },
		{ "CHECKOUT,v", "$(CO) $(COFLAGS)" },
		{ "CO", "co", },
		{ "CPP", "$(CC) -E", },
#ifdef	CRAY
		{ "CF77PPFLAGS", "-P", },
		{ "CF77PP", "/lib/cpp", },
		{ "CFT", "cft77", },
		{ "CF", "cf77", },
		{ "FC", "$(CF)", },
#else	/* Not CRAY.  */
#ifdef	_IBMR2
		{ "FC", "xlf", },
#else
#ifdef	__convex__
		{ "FC", "fc", },
#else
		{ "FC", "f77", },
#endif /* __convex__ */
#endif /* _IBMR2 */
		/*
		 * System V uses these, so explicit rules using them
		 * should work.  However, there is no way to make
		 * implicit rules use them and FC.
		 */
		{ "F77", "$(FC)", },
		{ "F77FLAGS", "$(FFLAGS)", },
#endif	/* Cray.  */
		{ "GET", SCCS_GET, },
		{ "LD", "ld", },
		{ "LEX", "lex", },
		{ "LINT", "lint", },
		{ "M2C", "m2c", },
#ifdef	pyr
		{ "PC", "pascal", },
#else
#ifdef	CRAY
		{ "PC", "PASCAL", },
		{ "SEGLDR", "segldr", },
#else
		{ "PC", "pc", },
#endif	/* CRAY.  */
#endif	/* pyr.  */
		{ "YACC", "yacc", },
		{ "MAKEINFO", "makeinfo", },
		{ "TEX", "tex", },
		{ "TEXI2DVI", "texi2dvi", },
		{ "WEAVE", "weave", },
		{ "CWEAVE", "cweave", },
		{ "TANGLE", "tangle", },
		{ "CTANGLE", "ctangle", },
		{ "RM", "rm -f", },
		{ "LINK.o", "$(CC) $(LDFLAGS) $(TARGET_ARCH)", },
		{
			"COMPILE.c",
			"$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c",
		},
		{
			"LINK.c",
			"$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH)",
		},
		{
			"COMPILE.cc",
			"$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c",
		},
		{ "COMPILE.C", "$(COMPILE.cc)", },
		{
			"LINK.cc",
		     "$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH)",
		},
		{ "LINK.C", "$(LINK.cc)", },
		{ "YACC.y", "$(YACC) $(YFLAGS)", },
		{ "LEX.l", "$(LEX) $(LFLAGS) -t", },
		{ "COMPILE.f", "$(FC) $(FFLAGS) $(TARGET_ARCH) -c", },
		{ "LINK.f", "$(FC) $(FFLAGS) $(LDFLAGS) $(TARGET_ARCH)", },
		{
			"COMPILE.F",
			"$(FC) $(FFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c",
		},
		{
			"LINK.F",
			"$(FC) $(FFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH)",
		},
		{ "COMPILE.r", "$(FC) $(FFLAGS) $(RFLAGS) $(TARGET_ARCH) -c", },
		{
			"LINK.r",
			"$(FC) $(FFLAGS) $(RFLAGS) $(LDFLAGS) $(TARGET_ARCH)",
		},
		{
			"COMPILE.def",
			"$(M2C) $(M2FLAGS) $(DEFFLAGS) $(TARGET_ARCH)",
		},
		{
			"COMPILE.mod",
			"$(M2C) $(M2FLAGS) $(MODFLAGS) $(TARGET_ARCH)",
		},
		{
			"COMPILE.p",
			"$(PC) $(PFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c",
		},
		{
			"LINK.p",
			"$(PC) $(PFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH)",
		},
		{ "LINK.s", "$(CC) $(ASFLAGS) $(LDFLAGS) $(TARGET_MACH)", },
		{ "COMPILE.s", "$(AS) $(ASFLAGS) $(TARGET_MACH)", },
		{
			"LINK.S",
		       "$(CC) $(ASFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_MACH)",
		},
		{
			"COMPILE.S",
			"$(CC) $(ASFLAGS) $(CPPFLAGS) $(TARGET_MACH) -c",
		},
#if !defined(M_XENIX) || defined(__GNUC__)
		{ "PREPROCESS.S", "$(CC) -E $(CPPFLAGS)", },
#else	/* Xenix.  */
		{ "PREPROCESS.S", "$(CC) -EP $(CPPFLAGS)", },
#endif	/* Not Xenix.  */
		{
			"PREPROCESS.F",
			"$(FC) $(FFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -F",
		},
		{
			"PREPROCESS.r",
			"$(FC) $(FFLAGS) $(RFLAGS) $(TARGET_ARCH) -F",
		},
		{
			"LINT.c",
			"$(LINT) $(LINTFLAGS) $(CPPFLAGS) $(TARGET_ARCH)",
		},
	};

	static symtab_ty *stp;
	static string_ty *builtin;
	string_ty	*data;
	blob_ty		*lhs;
	blob_list_ty	*rhs;
	static long	linum;
	stmt_ty		*result;

	trace(("default_setting()\n{\n"/*}*/));
	if (!stp)
	{
		table_ty	*tp;

		stp = symtab_alloc(SIZEOF(table));
		for (tp = table; tp < ENDOF(table); ++tp)
		{
			string_ty	*s;

			s = str_from_c(tp->name);
			symtab_assign(stp, s, str_from_c(tp->value));
			str_free(s);
		}
	}
	++linum;
	if (!builtin)
		builtin = str_from_c("builtin");
	lhs = blob_alloc(str_copy(name), builtin, linum);
	rhs = blob_list_alloc();
	data = symtab_query(stp, name);
	if (data)
	{
		string_list_ty	wl;
		size_t		j;

		str2wl(&wl, data, (char *)0, 0);
		for (j = 0; j < wl.nstrings; ++j)
		{
			string_ty	*s;

			s = str_copy(wl.string[j]);
			blob_list_append(rhs, blob_alloc(s, builtin, linum));
		}
		string_list_destructor(&wl);
	}
	result = stmt_assign_alloc(0, lhs, stmt_assign_op_default, rhs);
	trace(("return %8.8lX;\n", (long)result));
	trace((/*{*/"}\n"));
	return result;
}


stmt_ty *
stmt_assign_default(sp)
	stmt_ty		*sp;
{
	size_t		j;
	string_ty	*name;
	stmt_ty		*result;

	/*
	 * find a symbol in ref not in cdef
	 *	i.e. a symbol referenced but never defined
	 */
	trace(("stmt_assign_default()\n{\n"/*}*/));
	for (j = 0; j < sp->ref.nstrings; ++j)
	{
		name = sp->ref.string[j];
		if (!string_list_member(&sp->cdef, name))
		{
			result = default_setting(name);
			assert(string_list_member(&result->cdef, name));
			assert(string_list_member(&result->mdef, name));
			trace(("return %d;\n", result));
			trace((/*{*/"}\n"));
			return result;
		}
	}

	/*
	 * find a symbol in rref not in cdef
	 *	i.e. a symbol referenced by a rule body but never defined
	 */
	for (j = 0; j < sp->rref.nstrings; ++j)
	{
		name = sp->rref.string[j];
		if (!string_list_member(&sp->cdef, name))
		{
			result = default_setting(name);
			assert(string_list_member(&result->cdef, name));
			assert(string_list_member(&result->mdef, name));
			trace(("return %d;\n", result));
			trace((/*{*/"}\n"));
			return result;
		}
	}

	/*
	 * no further definitions required
	 */
	trace(("return 0;\n"));
	trace((/*{*/"}\n"));
	return 0;
}
