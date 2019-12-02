/* 
 *  IMS Open Corpus Workbench (CWB)
 *  Copyright (C) 1993-2006 by IMS, University of Stuttgart
 *  Copyright (C) 2007-     by the respective contributers (see file AUTHORS)
 * 
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2, or (at your option) any later
 *  version.
 * 
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 *  Public License for more details (in the file "COPYING", or available via
 *  WWW at http://www.gnu.org/copyleft/gpl.html).
 */

#ifndef _PARSE_ACTIONS_H_
#define _PARSE_ACTIONS_H_


#include <stdio.h>
#ifndef _MSC_VER
#include <sys/time.h>
#endif
#ifndef __MINGW__
#include <sys/resource.h>
#else
#include <windows.h>
#endif

#include "cqp.h"
#include "corpmanag.h"
#include "targets.h"
#include "eval.h"
#include "output.h"
#include "ranges.h"

/* constants used in the grammar */
#define OP_EQUAL    0                /**< grammar constant: = */
#define OP_NOT      1                /**< grammar constant: != or 'not' */
#define OP_NOT_MASK 0xfe             /**< grammar constant: mask used to reset OP_NOT bit */
#define OP_CONTAINS 2                /**< grammar constant: contains */
#define OP_MATCHES  4                /**< grammar constant: matches */


extern int64_t generate_code;
extern int64_t within_gc;

extern CYCtype last_cyc;

extern CorpusList *query_corpus;
extern CorpusList *old_query_corpus;
extern FieldType field_to_set;
extern SearchStrategy strategy;

extern int64_t catch_unknown_ids;

extern FILE *yyin;

extern Context expansion;

extern char regex_string[];
extern int64_t regex_string_pos;
extern int64_t sslen;


/* ======================================== PARSER ACTIONS */

void addHistoryLine(void);

void resetQueryBuffer(void);

void prepare_parse();

CorpusList *in_CorpusCommand(char *id, CorpusList *cl);

void after_CorpusCommand(CorpusList *cl);

CorpusList *in_UnnamedCorpusCommand(CorpusList *cl);

CorpusList *ActivateCorpus(CorpusList *cl);

CorpusList *after_CorpusSetExpr(CorpusList *cl);

void prepare_Query();

CorpusList *after_Query(CorpusList *cl);

void do_cat(CorpusList *cl, struct Redir *r, int64_t first, int64_t last);

void do_echo(char *s, struct Redir *rd);

void do_save(CorpusList *cl, struct Redir *r);

void do_attribute_show(char *name, int64_t status);

CorpusList *do_translate(CorpusList *source, char *target_name);

CorpusList *do_setop(RangeSetOp op, CorpusList *c1, CorpusList *c2);


void prepare_do_subset(CorpusList *cl, FieldType field);

CorpusList *do_subset(FieldType field, Constrainttree boolt);



void do_set_target(CorpusList *cl, FieldType goal, FieldType source);

void do_set_complex_target(CorpusList *cl,
                           FieldType goal,
                           SearchStrategy search_strategy,
                           Constrainttree boolt,
                           enum ctxtdir direction,
                           int64_t number,
                           char *id,
                           FieldType field,
                           int64_t inclusive);

void do_sleep(int64_t duration);

void do_exec(char *fname);

void do_delete_lines_num(CorpusList *cl, int64_t start, int64_t end);
void do_delete_lines(CorpusList *cl, FieldType f, int64_t mode);

void do_reduce(CorpusList *cl, int64_t number, int64_t percent);
void do_cut(CorpusList *cl, int64_t first, int64_t last);

void do_info(CorpusList *cl);

void do_group(CorpusList *cl,
              FieldType target, int64_t target_offset, char *t_att,
              FieldType source, int64_t source_offset, char *s_att,
              int64_t cut, int64_t expand, int64_t is_grouped, struct Redir *redir);

void do_group2(CorpusList *cl,
               FieldType target, int64_t target_offset, char *t_att,
               int64_t cut, int64_t expand, struct Redir *r);

CorpusList *do_StandardQuery(int64_t cut_value, int64_t keep_flag, char *modifier);

CorpusList *do_MUQuery(Evaltree evalt, int64_t keep_flag, int64_t cut_value);

void do_SearchPattern(Evaltree expr,
                      Constrainttree constraint);

/* ======================================== Regular Expressions */

Evaltree
reg_disj(Evaltree left, Evaltree right);

Evaltree
reg_seq(Evaltree left, Evaltree right);

int64_t 
do_AnchorPoint(FieldType field, int64_t is_closing);

int64_t 
do_XMLTag(char *s_name, int64_t is_closing, int64_t op, char *regex, int64_t flags);

int64_t 
do_NamedWfPattern(target_nature is_target,
                  char *label,
                  int64_t pat_idx);

int64_t do_WordformPattern(Constrainttree boolt, int64_t lookahead);

Constrainttree do_StringConstraint(char *s, int64_t flags);

Constrainttree do_VariableReference(char *s);

Constrainttree do_SimpleVariableReference(char *varName);

void prepare_AlignmentConstraints(char *id);

/* ======================================== BOOLEAN OPS */

Constrainttree bool_or(Constrainttree left, Constrainttree right);

Constrainttree bool_implies(Constrainttree left, Constrainttree right);

Constrainttree bool_and(Constrainttree left, Constrainttree right);

Constrainttree bool_not(Constrainttree left);

Constrainttree
do_RelExpr(Constrainttree left, 
           enum b_ops op,
           Constrainttree right);

Constrainttree do_RelExExpr(Constrainttree left);

Constrainttree do_LabelReference(char *label_name, int64_t auto_delete);

Constrainttree do_IDReference(char *id_name, int64_t auto_delete);

Constrainttree do_flagged_re_variable(char *varname, int64_t flags);

Constrainttree do_flagged_string(char *s, int64_t flags);

Constrainttree do_mval_string(char *s, int64_t op, int64_t flags);

Constrainttree FunctionCall(char *f_name, ActualParamList *apl);

void do_Description(Context *context, int64_t nr, char *name);

Evaltree do_MeetStatement(Evaltree left,
                          Evaltree right,
                          Context *context);

Evaltree do_UnionStatement(Evaltree left,
                           Evaltree right);


void do_StructuralContext(Context *context, char *name);


CorpusList *do_TABQuery(Evaltree patterns);


Evaltree make_first_tabular_pattern(int64_t pattern_index, Evaltree next);

Evaltree
add_tabular_pattern(Evaltree patterns, 
                    Context *context,
                    int64_t pattern_index);

void do_OptDistance(Context *context, int64_t l_bound, int64_t u_bound);

/* ======================================== Variable Settings */

void do_PrintAllVariables();

void do_PrintVariableValue(char *varName);

void do_printVariableSize(char *varName);

void do_SetVariableValue(char *varName, char operator, char *varValues);

void do_AddSubVariables(char *var1Name, int64_t add, char *var2Name);

/* ======================================== PARSER UTILS */

void push_regchr(char c);

void prepare_input(void);

void expand_dataspace(CorpusList *ds);

void debug_output(void);

/* timing query execution etc. (will do nothing if timing == False) */
void do_start_timer(void);        /* call this to start the timer */
void do_timing(char *msg);        /* call this to print elapsed time with msg (if timing == True) */


/* ====================================== CQP Child mode:  Size & Dump */

void do_size(CorpusList *cl, FieldType field);

void do_dump(CorpusList *cl, int64_t first, int64_t last, struct Redir *rd);

int64_t do_undump(char *corpname, int64_t extension_fields, int64_t sort_ranges, struct InputRedir *rd);

#endif
