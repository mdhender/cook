/*
 *	cook - file construction tool
 *	Copyright (C) 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate loopvars
 */

#include <expr.h>
#include <expr/constant.h>
#include <expr/function.h>
#include <expr/list.h>
#include <stmt.h>
#include <stmt/assign.h>
#include <stmt/compound.h>
#include <stmt/if.h>
#include <stmt/list.h>
#include <stmt/loop.h>
#include <stmt/loopvar.h>
#include <str.h>


/*
 * We are going to build a statement tree of the form
 *
 *	{							// s8
 *		<tmp> = <rhs> ;					// s1
 *		loop						// s7 (loop)
 *			if [count [ <tmp> ]]			// s6 (if)
 *			then
 *			{					// s4
 *				<lhs> = [head [ <var> ]];	// s2
 *				<var> = [tail [ <var> ]];	// s3
 *				<body>
 *			}					// s4
 *			else
 *				loopstop;			// s5
 *	}							// s8
 *
 * which implements the loop semantics exactly.
 */

stmt_ty *
stmt_loopvar(lhs, rhs, body)
	expr_list_ty	*lhs;
	expr_list_ty	*rhs;
	stmt_ty		*body;
{
	expr_position_ty *pp;
	string_ty	*s;
	static int	temp_loop_count;
	expr_list_ty	*tlv;
	expr_ty		*ep;
	expr_list_ty	*elp;
	stmt_ty		*s1;
	stmt_ty		*s2;
	stmt_ty		*s3;
	stmt_ty		*s4;
	stmt_ty		*s5;
	stmt_ty		*s6;
	stmt_ty		*s7;
	stmt_ty		*s8;
	stmt_list_ty	*slp;

	pp = expr_list_position(lhs);

	/* <tmp> */
	s = str_format("  temporary loop variable %d  ", ++temp_loop_count);
	ep = expr_constant_new(s, pp);
	str_free(s);
	tlv = expr_list_new();
	expr_list_append(tlv, ep);
	expr_delete(ep);

	/* s1: <tmp> = <rhs> ; */
	++temp_loop_count;
	s1 = stmt_assign_new(tlv, rhs);

	/* s2: <lhs> = [head [ <var> ]]; */
	s = str_from_c("head");
	ep = expr_constant_new(s, pp);
	str_free(s);
	elp = expr_list_new();
	expr_list_append(elp, ep);
	expr_delete(ep);

	ep = expr_function_new(tlv);
	expr_list_append(elp, ep);
	expr_delete(ep);

	ep = expr_function_new(elp);
	expr_list_delete(elp);

	elp = expr_list_new();
	expr_list_append(elp, ep);
	expr_delete(ep);

	s2 = stmt_assign_new(lhs, elp);
	expr_list_delete(elp);

	/* s3: <var> = [tail [ <var> ]]; */
	s = str_from_c("tail");
	ep = expr_constant_new(s, pp);
	str_free(s);
	elp = expr_list_new();
	expr_list_append(elp, ep);
	expr_delete(ep);

	ep = expr_function_new(tlv);
	expr_list_append(elp, ep);
	expr_delete(ep);

	ep = expr_function_new(elp);
	expr_list_delete(elp);

	elp = expr_list_new();
	expr_list_append(elp, ep);
	expr_delete(ep);

	s3 = stmt_assign_new(tlv, elp);
	expr_list_delete(elp);

	/* s4: { <s2> <s3> <body> } */
	slp = stmt_list_new();
	stmt_list_append(slp, s2);
	stmt_delete(s2);
	stmt_list_append(slp, s3);
	stmt_delete(s3);
	stmt_list_append(slp, body);
	s4 = stmt_compound_new(slp);
	stmt_list_delete(slp);

	/* s5: loopstop; */
	s5 = stmt_loopstop_new(pp);

	/* s6: if [count [ <var> ]] then <s4> else <s5> */
	s = str_from_c("count");
	ep = expr_constant_new(s, pp);
	str_free(s);
	elp = expr_list_new();
	expr_list_append(elp, ep);
	expr_delete(ep);

	ep = expr_function_new(tlv);
	expr_list_append(elp, ep);
	expr_delete(ep);

	ep = expr_function_new(elp);
	expr_list_delete(elp);

	s6 = stmt_if_new(ep, s4, s5);
	expr_delete(ep);
	stmt_delete(s4);
	stmt_delete(s5);

	/* s7: loop <s6> */
	s7 = stmt_loop_new(s6);
	stmt_delete(s6);

	/* s8: { <s1> <s7> } */
	slp = stmt_list_new();
	stmt_list_append(slp, s1);
	stmt_delete(s1);
	stmt_list_append(slp, s7);
	stmt_delete(s7);
	s8 = stmt_compound_new(slp);
	stmt_list_delete(slp);

	/*
	 * clean up and return result
	 */
	expr_list_delete(tlv);
	return s8;
}
