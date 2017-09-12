/*
 *	cook - file construction tool
 *	Copyright (C) 1994, 1997, 1998, 2001 Peter Miller;
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
 * MANIFEST: functions to manipulate rule statements
 */

#include <ac/ctype.h>
#include <ac/string.h>

#include <emit.h>
#include <mem.h>
#include <stmt/command.h>
#include <stmt/compound.h>
#include <stmt/rule.h>
#include <trace.h>
#include <variable.h>
#include <str_list.h>

enum
{
	fake_export_all_variables = 1,
	fake_ignore,
	fake_phony,
	fake_precious,
	fake_silent,
	fake_suffixes
};

typedef struct stmt_rule_ty stmt_rule_ty;
struct stmt_rule_ty
{
	STMT
	blob_list_ty	*target;
	blob_list_ty	*ingredient;
	blob_list_ty	*set;
	blob_list_ty	*pred;
	blob_list_ty	*single_thread;
	int		op;
	stmt_ty		*body;
	int		fake;
	string_ty	*archive_target;
	string_ty	*archive_member;
};

static string_list_ty phony;
static string_list_ty precious;
static string_list_ty suffix;
static int	suffix_initted;
static string_list_ty implict_rules_done;
int		stmt_rule_default_history;


static void suffix_init _((void));

static void
suffix_init()
{
	static char *table[] =
	{
		".a", ".C", ".c", ".cc", ".ch", ".def", ".dvi", ".el",
		".elc", ".F", ".f", ".h", ".info", ".l", ".ln", ".mod",
		".o", ".out", ".p", ".r", ".S", ".s", ".sh", ".sym",
		".tex", ".texi", ".texinfo", ".txinfo", ".w", ".web",
		".y",
	};

	size_t		j;
	string_ty	*s;

	if (suffix_initted)
		return;
	trace(("suffix_init()\n{\n"/*}*/));
	suffix_initted = 1;

	for (j = 0; j < SIZEOF(table); ++j)
	{
		s = str_from_c(table[j]);
		string_list_append(&suffix, s);
		str_free(s);
	}
	trace((/*{*/"}\n"));
}


static void check_for_default _((stmt_rule_ty *));

static void
check_for_default(this)
	stmt_rule_ty	*this;
{
	static string_ty *dot_default;

	trace(("check_for_default()\n{\n"/*}*/));
	if (!dot_default)
		dot_default = str_from_c(".DEFAULT");
	if
	(
		this->target->length == 1
	&&
		str_equal(this->target->list[0]->text, dot_default)
	)
	{
		str_free(this->target->list[0]->text);
		this->target->list[0]->text = str_from_c("%0%");
	}
	trace((/*{*/"}\n"));
}


static void check_for_export_all_variables _((stmt_rule_ty *));

static void
check_for_export_all_variables(this)
	stmt_rule_ty	*this;
{
	static string_ty *dot_export;

	trace(("check_for_export_all_variables()\n{\n"/*}*/));
	if (!dot_export)
		dot_export = str_from_c(".EXPORT_ALL_VARIABLES");
	if
	(
		this->target->length == 1
	&&
		str_equal(this->target->list[0]->text, dot_export)
	)
	{
		this->fake = fake_export_all_variables;
	}
	trace((/*{*/"}\n"));
}


static void check_for_ignore _((stmt_rule_ty *));

static void
check_for_ignore(this)
	stmt_rule_ty	*this;
{
	static string_ty *dot_ignore;

	trace(("check_for_ignore()\n{\n"/*}*/));
	if (!dot_ignore)
		dot_ignore = str_from_c(".IGNORE");
	if
	(
		this->target->length == 1
	&&
		str_equal(this->target->list[0]->text, dot_ignore)
	)
	{
		this->fake = fake_ignore;
	}
	trace((/*{*/"}\n"));
}


static void check_for_phony _((stmt_rule_ty *));

static void
check_for_phony(this)
	stmt_rule_ty	*this;
{
	static string_ty *dot_phony;
	long		j;

	trace(("check_for_phony()\n{\n"/*}*/));
	if (!dot_phony)
		dot_phony = str_from_c(".PHONY");
	if
	(
		this->target->length == 1
	&&
		str_equal(this->target->list[0]->text, dot_phony)
	)
	{
		this->fake = fake_phony;
		for (j = 0; j < this->ingredient->length; ++j)
		{
			string_list_append_unique
			(
				&phony,
				this->ingredient->list[j]->text
			);
		}
	}
	trace((/*{*/"}\n"));
}


static void check_for_precious _((stmt_rule_ty *));

static void
check_for_precious(this)
	stmt_rule_ty	*this;
{
	static string_ty *dot_precious;
	long		j;

	trace(("check_for_precious()\n{\n"/*}*/));
	if (!dot_precious)
		dot_precious = str_from_c(".PRECIOUS");
	if
	(
		this->target->length == 1
	&&
		str_equal(this->target->list[0]->text, dot_precious)
	)
	{
		this->fake = fake_precious;
		for (j = 0; j < this->ingredient->length; ++j)
		{
			string_list_append_unique
			(
				&precious,
				this->ingredient->list[j]->text
			);
		}
	}
	trace((/*{*/"}\n"));
}


static void check_for_silent _((stmt_rule_ty *));

static void
check_for_silent(this)
	stmt_rule_ty	*this;
{
	static string_ty *dot_silent;

	trace(("check_for_silent()\n{\n"/*}*/));
	if (!dot_silent)
		dot_silent = str_from_c(".SILENT");
	if
	(
		this->target->length == 1
	&&
		str_equal(this->target->list[0]->text, dot_silent)
	)
	{
		this->fake = fake_silent;
	}
	trace((/*{*/"}\n"));
}


static void check_for_suffixes _((stmt_rule_ty *));

static void
check_for_suffixes(this)
	stmt_rule_ty	*this;
{
	static string_ty *dot_suffixes;
	long		j;

	if (!dot_suffixes)
		dot_suffixes = str_from_c(".SUFFIXES");
	if (this->target->length != 1)
		return;
	if (!str_equal(this->target->list[0]->text, dot_suffixes))
		return;

	trace(("check_for_suffixes()\n{\n"/*}*/));
	this->fake = fake_suffixes;
	if (this->ingredient->length == 0)
		string_list_destructor(&suffix);
	else
	{
		for (j = 0; j < this->ingredient->length; ++j)
		{
			string_list_append_unique
			(
				&suffix,
				this->ingredient->list[j]->text
			);
		}
	}
	trace((/*{*/"}\n"));
}


static int single_suffix _((string_ty *));

static int
single_suffix(in)
	string_ty	*in;
{
	return string_list_member(&suffix, in);
}


static int double_suffix _((string_ty *, string_ty **, string_ty **));

static int
double_suffix(in, out1, out2)
	string_ty	*in;
	string_ty	**out1;
	string_ty	**out2;
{
	long		j, k;
	string_ty	*s;

	for (j = 0; j < suffix.nstrings; ++j)
	{
		for (k = 0; k < suffix.nstrings; ++k)
		{
			s = str_catenate(suffix.string[j], suffix.string[k]);
			if (str_equal(in, s))
			{
				str_free(s);
				*out1 = suffix.string[j];
				*out2 = suffix.string[k];
				return 1;
			}
			str_free(s);
		}
	}
	return 0;
}


static void constructor _((stmt_ty *));

static void
constructor(that)
	stmt_ty		*that;
{
	stmt_rule_ty	*this;

	trace(("rule::constructor()\n{\n"/*}*/));
	this = (stmt_rule_ty *)that;
	this->target = 0;
	this->ingredient = 0;
	this->set = 0;
	this->pred = 0;
	this->single_thread = 0;
	this->op = 0;
	this->body = 0;
	this->fake = 0;
	this->archive_target = 0;
	this->archive_member = 0;
	trace((/*{*/"}\n"));
}


static void destructor _((stmt_ty *));

static void
destructor(that)
	stmt_ty		*that;
{
	stmt_rule_ty	*this;

	trace(("rule::destructor()\n{\n"/*}*/));
	this = (stmt_rule_ty *)that;
	if (this->target)
		blob_list_free(this->target);
	if (this->ingredient)
		blob_list_free(this->ingredient);
	if (this->set)
		blob_list_free(this->set);
	if (this->pred)
		blob_list_free(this->pred);
	if (this->single_thread)
		blob_list_free(this->single_thread);
	if (this->body)
		stmt_free(this->body);
	if (this->archive_target)
		str_free(this->archive_target);
	if (this->archive_member)
		str_free(this->archive_member);
	trace((/*{*/"}\n"));
}


static void emit _((stmt_ty *));

static void
emit(that)
	stmt_ty		*that;
{
	stmt_rule_ty	*this;
	size_t		j;
	string_ty	*s;
	blob_ty		*bp;

	trace(("rule::emit()\n{\n"/*}*/));
	this = (stmt_rule_ty *)that;
	switch (this->fake)
	{
	case fake_export_all_variables:
		trace((/*{*/"}\n"));
		return;

	case fake_ignore:
		bp = this->target->list[0];
		emit_line_number(bp->line_number, bp->file_name);
		emit_str("set errok;\n");
		trace((/*{*/"}\n"));
		return;

	case fake_phony:
	case fake_precious:
		trace((/*{*/"}\n"));
		return;

	case fake_silent:
		bp = this->target->list[0];
		emit_line_number(bp->line_number, bp->file_name);
		emit_str("set silent;\n");
		trace((/*{*/"}\n"));
		return;

	case fake_suffixes:
		trace((/*{*/"}\n"));
		return;
	}

	assert(this->target);
	assert(this->target->length);
	assert(this->target->list[0]);
	assert(this->target->list[0]->file_name);
	emit_set_file(this->target->list[0]->file_name);
	for (j = 0; j < this->target->length; ++j)
	{
		if (j)
			emit_char(' ');
		blob_emit(this->target->list[j]);
	}
	emit_char(':');

	for (j = 0; j < this->ingredient->length; ++j)
	{
		emit_char(' ');
		blob_emit(this->ingredient->list[j]);
	}

	/*
	 * see if the recipe should have any flags
	 */
	for (j = 0; j < this->target->length; ++j)
	{
		blob_ty	*lhs;

		lhs = this->target->list[j];
		if (string_list_member(&phony, lhs->text))
		{
			if (!this->set)
				this->set = blob_list_alloc();
			s = str_from_c("force");
			blob_list_append
			(
				this->set,
				blob_alloc(s, lhs->file_name, lhs->line_number)
			);
		}
		if (string_list_member(&precious, lhs->text))
		{
			if (!this->set)
				this->set = blob_list_alloc();
			s = str_from_c("precious");
			blob_list_append
			(
				this->set,
				blob_alloc(s, lhs->file_name, lhs->line_number)
			);
		}
	}
	if (this->set && this->set->length)
	{
		emit_bol();
		emit_indent_more();
		emit_str("set");
		for (j = 0; j < this->set->length; ++j)
		{
			emit_char(' ');
			blob_emit(this->set->list[j]);
		}
		emit_indent_less();
	}

	/*
	 * emit predicate
	 */
	if (this->pred && this->pred->length)
	{
		emit_bol();
		emit_indent_more();
		emit_str("if [in [target]");
		for (j = 0; j < this->pred->length; ++j)
		{
			emit_char(' ');
			blob_emit(this->pred->list[j]);
		}
		emit_str("]");
		emit_indent_less();
	}

	/*
	 * emit single thread
	 */
	if (this->single_thread && this->single_thread->length)
	{
		emit_bol();
		emit_indent_more();
		emit_str("single-thread");
		for (j = 0; j < this->single_thread->length; ++j)
		{
			emit_char(' ');
			blob_emit(this->single_thread->list[j]);
		}
		emit_indent_less();
	}

	/*
	 * emit the body of the recipe
	 */
	if (!this->body)
		emit_str(";\n");
	else
	{
		emit_str("\n{\n"/*}*/);
		emit_indent_more();
		stmt_emit(this->body);
		emit_indent_less();
		emit_str(/*{*/"}\n");
	}
	trace((/*{*/"}\n"));
}


static stmt_method_ty method =
{
	sizeof(stmt_rule_ty),
	"rule",
	constructor,
	destructor,
	emit,
};


static string_ty *work_over_percent _((string_ty *));

static string_ty *
work_over_percent(s)
	string_ty	*s;
{
	static char	*buffer;
	static size_t	max;
	size_t		pos;
	char		*cp;

	pos = 0;
	for (cp = s->str_text; *cp; ++cp)
	{
		if (*cp == '%' && (cp == s->str_text || cp[-1] == '/'))
		{
			if (pos + 3 > max)
			{
				max = max * 2 + 4;
				buffer = mem_change_size(buffer, max);
			}
			buffer[pos++] = '%';
			buffer[pos++] = '0';
			buffer[pos++] = '%';
		}
		else
		{
			if (pos >= max)
			{
				max = max * 2 + 4;
				buffer = mem_change_size(buffer, max);
			}
			buffer[pos++] = *cp;
		}
	}
	return str_n_from_c(buffer, pos);
}


stmt_ty *
stmt_rule_alloc(lhs, op, rhs, set, pred, single_thread)
	blob_list_ty	*lhs;
	int		op;
	blob_list_ty	*rhs;
	blob_list_ty	*set;
	blob_list_ty	*pred;
	blob_list_ty	*single_thread;
{
	stmt_rule_ty	*result;
	string_ty	*s1;
	string_ty	*s2;
	blob_list_ty	*lhs2;
	blob_list_ty	*rhs2;
	blob_list_ty	*pred2;
	static string_ty *dot_a;
	string_ty	*archive_target;
	string_ty	*archive_member;

	/*
	 * rewite horrible target rules
	 */
	trace(("stmt_rule_alloc()\n{\n"/*}*/));
	s1 = 0;
	s2 = 0;
	archive_target = 0;
	archive_member = 0;
	if (!dot_a)
		dot_a = str_from_c(".a");
	suffix_init();
	trace(("lhs->length = %ld;\n", (long)lhs->length));
	if (lhs->length == 1 && single_suffix(lhs->list[0]->text))
	{
		trace(("mark\n"));
		string_list_append(&implict_rules_done, lhs->list[0]->text);
		blob_list_prepend
		(
			rhs,
			blob_alloc
			(
				str_format("%%0%%%S", lhs->list[0]->text),
				lhs->list[0]->file_name,
				lhs->list[0]->line_number
			)
		);
		str_free(lhs->list[0]->text);
		lhs->list[0]->text = str_from_c("%0%");
	}
	else if
	(
		lhs->length == 1
	&&
		double_suffix(lhs->list[0]->text, &s1, &s2)
	)
	{
		trace(("mark\n"));
		if (str_equal(s2, dot_a))
		{
			archive_target = str_format("%%0%%1%S", s2);
			archive_member = str_format("%%%S", s1);
		}
		string_list_append(&implict_rules_done, lhs->list[0]->text);
		blob_list_append
		(
			lhs,
			blob_alloc
			(
				(
					archive_target
				?
					str_format("%%0%%1%S(%%%S)", s2, s1)
				:
					str_format("%%0%%%S", s2)
				),
				lhs->list[0]->file_name,
				lhs->list[0]->line_number
			)
		);

		blob_list_prepend
		(
			rhs,
			blob_alloc
			(
				str_format("%%0%%%S", s1),
				lhs->list[0]->file_name,
				lhs->list[0]->line_number
			)
		);

		/* drop old target */
		blob_list_delete(lhs, lhs->list[0]);
	}
	else if
	(
		lhs->length == 1
	&&
		lhs->list[0]->text->str_text[0] == '('/*)*/
	&&
		(
			lhs->list[0]->text->str_text
			[
				lhs->list[0]->text->str_length - 1
			]
		==
			/*(*/')'
		)
	)
	{
		size_t		j;
		blob_ty		*bp;

		trace(("mark\n"));
		archive_target = str_from_c("%0%1");
		archive_member =
			str_n_from_c
			(
				lhs->list[0]->text->str_text + 1,
				lhs->list[0]->text->str_length - 2
			);
		str_free(lhs->list[0]->text);
		lhs->list[0]->text =
			str_format("%S(%S)", archive_target, archive_member);

		/* fix rhs */
		for (j = 0; j < rhs->length; ++j)
		{
			bp = rhs->list[j];
			rhs->list[j] =
				blob_alloc
				(
					work_over_percent(bp->text),
					bp->file_name,
					bp->line_number
				);
			blob_free(bp);
		}
	}
	else
	{
		size_t		j;
		blob_ty		*bp;

		/* fix lhs */
		trace(("mark\n"));
		for (j = 0; j < lhs->length; ++j)
		{
			bp = lhs->list[j];
			lhs->list[j] =
				blob_alloc
				(
					work_over_percent(bp->text),
					bp->file_name,
					bp->line_number
				);
			blob_free(bp);
		}

		/* fix rhs */
		for (j = 0; j < rhs->length; ++j)
		{
			bp = rhs->list[j];
			rhs->list[j] =
				blob_alloc
				(
					work_over_percent(bp->text),
					bp->file_name,
					bp->line_number
				);
			blob_free(bp);
		}
	}

	/*
	 * rewrite the variable names
	 */
	trace(("mark\n"));
	result = (stmt_rule_ty *)stmt_alloc(&method);
	lhs2 = blob_list_alloc();
	variable_rename_list
	(
		lhs,
		lhs2,
		&result->ref,
		VAREN_QUOTE_SPACES | VAREN_KNOW_ARCHIVE
	);
	blob_list_free(lhs);
	trace(("lhs2->length = %ld;\n", (long)lhs2->length));
	assert(lhs2->length);

	rhs2 = blob_list_alloc();
	variable_rename_list
	(
		rhs,
		rhs2,
		&result->ref,
		VAREN_QUOTE_SPACES | VAREN_KNOW_ARCHIVE
	);
	blob_list_free(rhs);

	if (pred)
	{
		pred2 = blob_list_alloc();
		variable_rename_list
		(
			pred,
			pred2,
			&result->ref,
			VAREN_QUOTE_SPACES | VAREN_KNOW_ARCHIVE
		);
		blob_list_free(pred);
		assert(pred2->length);
	}
	else
		pred2 = 0;

	result->archive_target = archive_target;
	result->archive_member = archive_member;

	if (single_thread)
	{
		blob_list_ty	*blp;

		blp = blob_list_alloc();
		stmt_rule_context((stmt_ty *)result);
		variable_rename_list
		(
			single_thread,
			blp,
			&result->rref,
			VAREN_NO_FLAGS
		);
		blob_list_free(single_thread);
		single_thread = blp;
	}

	result->target = lhs2;
	result->op = op;
	result->ingredient = rhs2;
	result->set = set;
	result->pred = pred2;
	result->single_thread = single_thread;
	check_for_default(result);
	check_for_export_all_variables(result);
	check_for_ignore(result);
	check_for_phony(result);
	check_for_precious(result);
	check_for_silent(result);
	check_for_suffixes(result);
	trace((/*{*/"}\n"));
	return (stmt_ty *)result;
}


void
stmt_rule_body(that, body)
	stmt_ty		*that;
	stmt_ty		*body;
{
	stmt_rule_ty	*this;

	trace(("stmt_rule_body()\n{\n"/*}*/));
	this = (stmt_rule_ty *)that;
	this->body = body;
	stmt_variable_merge(that, body);
	trace((/*{*/"}\n"));
}


void
stmt_rule_context(that)
	stmt_ty		*that;
{
	stmt_rule_ty	*this;

	trace(("stmt_rule_context()\n{\n"/*}*/));
	this = (stmt_rule_ty *)that;
	if (this->archive_target)
		variable_archive(this->archive_target, this->archive_member);
	trace((/*{*/"}\n"));
}


typedef struct table_ty table_ty;
struct table_ty
{
	char	*ingredients;
	char	*target;
	char	*body;
	char	*set;
	char	*single_thread;
	int	history;
};

static table_ty table[] =
{
	{
		".o",				/* ingredients */
		"",				/* target */
		"$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@",
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		".s",				/* ingredients */
		"",				/* target */
		"$(LINK.s) $^ $(LOADLIBES) $(LDLIBS) -o $@",
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		".S",				/* ingredients */
		"",				/* target */
		"$(LINK.S) $^ $(LOADLIBES) $(LDLIBS) -o $@",
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		".c",				/* ingredients */
		"",				/* target */
		"$(LINK.c) $^ $(LOADLIBES) $(LDLIBS) -o $@",
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		".cc",				/* ingredients */
		"",				/* target */
		"$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@",
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		".C",				/* ingredients */
		"",				/* target */
		"$(LINK.C) $^ $(LOADLIBES) $(LDLIBS) -o $@",
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		".f",				/* ingredients */
		"",				/* target */
		"$(LINK.f) $^ $(LOADLIBES) $(LDLIBS) -o $@",
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		".p",				/* ingredients */
		"",				/* target */
		"$(LINK.p) $^ $(LOADLIBES) $(LDLIBS) -o $@",
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		".F",				/* ingredients */
		"",				/* target */
		"$(LINK.F) $^ $(LOADLIBES) $(LDLIBS) -o $@",
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		".r",				/* ingredients */
		"",				/* target */
		"$(LINK.r) $^ $(LOADLIBES) $(LDLIBS) -o $@",
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		".mod",				/* ingredients */
		"",				/* target */
		"$(COMPILE.mod) -o $@ -e $@ $^",
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		".def",				/* ingredients */
		".sym",				/* target */
		"$(COMPILE.def) -o $@ $<",	/* body */
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		".sh",				/* ingredients */
		"",				/* target */
		"cat $< > $@\n\
		chmod a+x $@",			/* body */
		"unlink",			/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		".s",				/* ingredients */
		".o",				/* target */
#if !defined(M_XENIX) || defined(__GNUC__)
		"$(COMPILE.s) -o $@ $<",
#else	/* Xenix.  */
		"$(COMPILE.s) -o$@ $<",
#endif	/* Not Xenix.  */
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		".S",				/* ingredients */
		".o",				/* target */
#if !defined(M_XENIX) || defined(__GNUC__)
		"$(COMPILE.S) -o $@ $<",
#else	/* Xenix.  */
		"$(COMPILE.S) -o$@ $<",
#endif	/* Not Xenix.  */
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		".c",				/* ingredients */
		".o",				/* target */
		"$(COMPILE.c) $<",		/* body */
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		".cc",				/* ingredients */
		".o",				/* target */
		"$(COMPILE.cc) $<",		/* body */
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		".C",				/* ingredients */
		".o",				/* target */
		"$(COMPILE.C) $<",		/* body */
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		".f",				/* ingredients */
		".o",				/* target */
		"$(COMPILE.f) $<",		/* body */
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		".p",				/* ingredients */
		".o",				/* target */
		"$(COMPILE.p) $<",		/* body */
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		".F",				/* ingredients */
		".o",				/* target */
		"$(COMPILE.F) $<",		/* body */
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		".r",				/* ingredients */
		".o",				/* target */
		"$(COMPILE.r) $<",		/* body */
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		".mod",				/* ingredients */
		".o",				/* target */
		"$(COMPILE.mod) -o $@ $<",	/* body */
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		".c",				/* ingredients */
		".ln",				/* target */
		"$(LINT.c) -C$* $<",		/* body */
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		".y",				/* ingredients */
		".ln",				/* target */
		"$(YACC.y) $<\n\
		$(LINT.c) -C$* y.tab.c\n\
		$(RM) y.tab.c",			/* body */
		0,				/* set */
		"y.tab.c",			/* single thread */
		0,				/* history */
	},
	{
		".l",				/* ingredients */
		".ln",				/* target */
		"@$(RM) $*.c\n\
		$(LEX.l) $< > $*.c\n\
		$(LINT.c) -i $*.c -o $@\n\
		$(RM) $*.c",			/* body */
		0,				/* set */
		"$*.c",				/* single thread */
		0,				/* history */
	},
	{
		".y",				/* ingredients */
		".c",				/* target */
		"$(YACC.y) $<\n\
		mv y.tab.c $@",			/* body */
		0,				/* set */
		"y.tab.c",			/* single thread */
		0,				/* history */
	},
	{
		".l",				/* ingredients */
		".c",				/* target */
		"$(LEX.l) $< > $@",		/* body */
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		".F",				/* ingredients */
		".f",				/* target */
		"$(PREPROCESS.F) $<",		/* body */
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		".r",				/* ingredients */
		".f",				/* target */
		"$(PREPROCESS.r) $<",		/* body */
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		/*
		 * This might actually make lex.yy.c if there's
		 * no %R% directive in $*.l, but in that case
		 * why were you trying to make $*.r anyway?
		 */
		".l",				/* ingredients */
		".r",				/* target */
		"$(LEX.l) $< > $@\n\
		mv lex.yy.r $@",		/* body */
		0,				/* set */
		"lex.yy.r lex.yy.c",		/* single thread */
		0,				/* history */
	},
	{
		".S",				/* ingredients */
		".s",				/* target */
		"$(PREPROCESS.S) $< > $@",	/* body */
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		".texinfo",			/* ingredients */
		".info",			/* target */
		"$(MAKEINFO) $< -o $@",		/* body */
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		".texi",			/* ingredients */
		".info",			/* target */
		"$(MAKEINFO) $< -o $@",		/* body */
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		".txinfo",			/* ingredients */
		".info",			/* target */
		"$(MAKEINFO) $< -o $@",		/* body */
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		".tex",				/* ingredients */
		".dvi",				/* target */
		"$(TEX) $<",			/* body */
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		".texinfo",			/* ingredients */
		".dvi",				/* target */
		"$(TEXI2DVI) $<",		/* body */
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		".texi",			/* ingredients */
		".dvi",				/* target */
		"$(TEXI2DVI) $<",		/* body */
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		".txinfo",			/* ingredients */
		".dvi",				/* target */
		"$(TEXI2DVI) $<",		/* body */
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		/* The `-' says there is no `.ch' file.  */
		".w",				/* ingredients */
		".c",				/* target */
		"$(CTANGLE) $< - $@",		/* body */
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		".web",				/* ingredients */
		".p",				/* target */
		"$(TANGLE) $<",			/* body */
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		/* The `-' says there is no `.ch' file.  */
		".w",				/* ingredients */
		".tex",				/* target */
		"$(CWEAVE) $< - $@",		/* body */
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		".web",				/* ingredients */
		".tex",				/* target */
		"$(WEAVE) $<",			/* body */
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},

	/*
	 * the default archive rule
	 */
	{
		".o",				/* ingredients */
		".a",				/* target */
		"$(AR) $(ARFLAGS) $@ $<",	/* body */
		0,				/* set */
		"$@",				/* single thread */
		0,				/* history */
	},
};

static table_ty table2[] =
{
	/*
	 * The X.out rules are only in BSD's default set
	 * because BSD Make has no null-suffix rules, so
	 * `foo.out' and `foo' are the same thing.
	 */
	{
		"%",				/* ingredients */
		"%.out",			/* taregt */
		"cp $< $@",			/* body */
		"unlink",			/* set */
		0,				/* single thread */
		0,				/* history */
	},

	/*
	 * tangle and weave, the C web rules
	 */
	{
		"%.w %.ch",			/* ingredients */
		"%.c",				/* target */
		"$(CTANGLE) $^ $@",		/* body */
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},
	{
		"%.w %.ch",			/* ingredients */
		"%.tex",			/* target */
		"$(CWEAVE) $^ $@",		/* body */
		0,				/* set */
		0,				/* single thread */
		0,				/* history */
	},

	/*
	 * the GNU default archive rule
	 */
	{
		"%",				/* ingredients */
		"(%)",				/* target */
		"$(AR) $(ARFLAGS) $@ $<",	/* body */
		0,				/* set */
		"$@",				/* single thread */
		0,				/* history */
	},

	/*
	 * history retrieval rules
	 */
	{
		"%,v",				/* ingredients */
		"%",				/* target */
		"+$(CHECKOUT,v) $^ $@",		/* body */
		"no-implicit-ingredients",	/* set */
		0,				/* single thread */
		1,				/* hsitory */
	},
	{
		"RCS/%,v",			/* ingredients */
		"%",				/* target */
		"+$(CHECKOUT,v) $^ $@",		/* body */
		"no-implicit-ingredients",	/* set */
		0,				/* single thread */
		1,				/* history */
	},
	{
		"s.%",				/* ingredients */
		"%",				/* target */
		"$(GET) $(GFLAGS) $^",		/* body */
		"no-implicit-ingredients",	/* set */
		0,				/* single thread */
		1,				/* history */
	},
	{
		"SCCS/s.%",			/* ingredients */
		"%",				/* target */
		"$(GET) $(GFLAGS) $^",		/* body */
		"no-implicit-ingredients",	/* set */
		0,				/* single thread */
		1,				/* history */
	},
};


static void split_by_lines _((char *, stmt_ty *, string_ty *, long));

static void
split_by_lines(s, sp, fn, ln)
	char		*s;
	stmt_ty		*sp;
	string_ty	*fn;
	long		ln;
{
	stmt_ty		*body;
	char		*ep;

	/*
	 * build a compound statement for the body
	 */
	body = stmt_compound_alloc();
	for (;;)
	{
		while (isspace(*s))
			++s;
		if (!*s)
			break;
		ep = strchr(s, '\n');
		if (!ep)
			ep = s + strlen(s);
		while (ep > s && isspace(ep[-1]))
			--ep;

		/*
		 * construct the command and add it to the body
		 */
		stmt_rule_context(sp);
		stmt_compound_append
		(
			body,
			stmt_command_alloc
			(
				blob_alloc(str_n_from_c(s, ep - s), fn, ln)
			)
		);

		s = ep;
	}

	/*
	 * attach the body to the rule
	 */
	stmt_rule_body(sp, body);
}


static blob_list_ty *string_to_blob_list _((char *, string_ty *, long));

static blob_list_ty *
string_to_blob_list(s, fn, ln)
	char		*s;
	string_ty	*fn;
	long		ln;
{
	blob_list_ty	*blp;
	char		*ep;

	/*
	 * build a compound statement for the body
	 */
	if (!s)
		return 0;
	blp = blob_list_alloc();
	for (;;)
	{
		while (*s && isspace(*s))
			++s;
		if (!*s)
			break;
		for (ep = s + 1; *ep && !isspace(*ep); ++ep)
			;

		/*
		 * construct the blob and add it to the list
		 */
		blob_list_append
		(
			blp,
			blob_alloc(str_n_from_c(s, ep - s), fn, ln)
		);
		s = ep;
	}

	return blp;
}


stmt_ty *
stmt_rule_default(n)
	int		n;
{
	static long	linum	= 1000;
	static string_ty *builtin;
	table_ty	*tp;

	trace(("stmt_rule_default()\n{\n"/*}*/));
	if (!builtin)
		builtin = str_from_c("builtin");
	for (tp = table; tp < ENDOF(table); ++tp)
	{
		string_ty	*target;
		string_ty	*ingredient;
		string_ty	*s;
		blob_list_ty	*targets;
		blob_list_ty	*ingredients;
		blob_list_ty	*set;
		blob_list_ty	*pred;
		blob_list_ty	*single_thread;
		stmt_ty		*sp;

		if (!stmt_rule_default_history && tp->history)
			continue;
		ingredient = str_from_c(tp->ingredients);
		if (!string_list_member(&suffix, ingredient))
		{
			str_free(ingredient);
			continue;
		}
		target = str_from_c(tp->target);
		if (target->str_length && !string_list_member(&suffix, target))
		{
			str_free(target);
			str_free(ingredient);
			continue;
		}
		s = str_catenate(ingredient, target);
		str_free(ingredient);
		str_free(target);
		if (string_list_member(&implict_rules_done, s))
		{
			str_free(s);
			continue;
		}
		trace(("\"%s\"\n", s->str_text));

		++linum;

		targets = string_to_blob_list(s->str_text, builtin, linum);
		ingredients = blob_list_alloc();
		set = string_to_blob_list(tp->set, builtin, linum);
		pred = 0;
		single_thread =
			string_to_blob_list(tp->single_thread, builtin, linum);

		sp =
			stmt_rule_alloc
			(
				targets,
				1,
				ingredients,
				set,
				pred,
				single_thread
			);

		split_by_lines(tp->body, sp, builtin, linum);
		trace((/*{*/"}\n"));
		return sp;
	}

	for (tp = table2; tp < ENDOF(table2); ++tp)
	{
		string_ty	*target;
		string_ty	*ingredient;
		string_ty	*s;
		blob_list_ty	*targets;
		blob_list_ty	*ingredients;
		blob_list_ty	*set;
		blob_list_ty	*pred;
		blob_list_ty	*single_thread;
		stmt_ty		*sp;
		string_list_ty	wl2;
		size_t		j;

		if (!stmt_rule_default_history && tp->history)
			continue;
		ingredient = str_from_c(tp->ingredients);
		target = str_from_c(tp->target);

		/*
		 * make sure we understand the suffixes
		 */
		if (!tp->history)
		{
			string_list_ty	wl;
			int		ok;

			string_list_constructor(&wl);
			str2wl(&wl2, ingredient, (char *)0, 0);
			string_list_append_list_unique(&wl, &wl2);
			string_list_destructor(&wl2);
			str2wl(&wl2, target, (char *)0, 0);
			string_list_append_list_unique(&wl, &wl2);
			string_list_destructor(&wl2);

			ok = 1;
			for (j = 0; j < wl.nstrings; ++j)
			{
				s = wl.string[j];
				if (s->str_text[0] == '%')
					s =
						str_n_from_c
						(
							s->str_text + 1,
							s->str_length - 1
						);
				else
					s = str_from_c(".a");
				if
				(
					s->str_length
				&&
					!string_list_member(&suffix, s)
				)
				{
					ok = 0;
				}
				str_free(s);
			}
			string_list_destructor(&wl);

			if (!ok)
			{
				str_free(ingredient);
				str_free(target);
				continue;
			}
		 }

		/*
		 * remember we have done it
		 */
		s = str_format("%S:%S", ingredient, target);
		str_free(ingredient);
		str_free(target);
		if (string_list_member(&implict_rules_done, s))
		{
			str_free(s);
			continue;
		}
		trace(("\"%s\"\n", s->str_text));
		string_list_append(&implict_rules_done, s);
		str_free(s);

		/*
		 * construct and instanciate the rule
		 */
		++linum;
		targets = string_to_blob_list(tp->target, builtin, linum);
		ingredients =
			string_to_blob_list(tp->ingredients, builtin, linum);
		set = string_to_blob_list(tp->set, builtin, linum);
		pred = 0;
		single_thread =
			string_to_blob_list(tp->single_thread, builtin, linum);
		sp =
			stmt_rule_alloc
			(
				targets,
				1,
				ingredients,
				set,
				pred,
				single_thread
			);
		split_by_lines(tp->body, sp, builtin, linum);
		trace((/*{*/"}\n"));
		return sp;
	}
	trace((/*{*/"}\n"));
	return 0;
}
