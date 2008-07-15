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

#ifndef _PRINT_MODES_H_
#define _PRINT_MODES_H_

#include "corpmanag.h"
#include "attlist.h"

typedef enum outputmode { 
  PrintASCII, 
  PrintSGML, 
  PrintHTML, 
  PrintLATEX, 
  PrintBINARY, 
  PrintUNKNOWN
} PrintMode;

typedef struct _print_option_rec_ {
  int print_header;		/* header/hdr,noheader */
  int print_tabular;		/* table/tbl,notable */
  int print_wrap;		/* wrap, nowrap */
  int print_border;		/* border/bdr,noborder */
  int number_lines;
} PrintOptions;

typedef struct _print_descr_rec_ {

  char *CPOSPrintFormat;	/* printf()-Formatting String */

  char *BeforePrintStructures;	/* to print before PS */
  char *PrintStructureSeparator; /* to print as separator */
  char *AfterPrintStructures;

  char *StructureBeginPrefix;	/* prefix of structure start tag */
  char *StructureBeginSuffix;	/* suffix of structure start tag*/

  char *StructureSeparator;	/* separator of structures */

  char *StructureEndPrefix;	/* prefix of structure end tag */
  char *StructureEndSuffix;	/* suffix of structure end tag */

  char *BeforeToken;		/* what to print before a token */
  char *TokenSeparator;		/* what to print between tokens */
  char *AttributeSeparator;	/* what to print as PA separator */
  char *AfterToken;		/* what to print after a token */

  char *BeforeField;		/* what to print before a field */
  char *FieldSeparator;		/* what to print between fields */
  char *AfterField;		/* what to print after fields */

  char *BeforeLine;		/* what to print before a line */
  char *AfterLine;		/* what to print after a line */

  char *BeforeConcordance;	/* what to print before the concordance */
  char *AfterConcordance;	/* what to print after the concordance */

  char *(*printToken)(char *);
  char *(*printField)(FieldType, int);
  
} PrintDescriptionRecord;

typedef char * (*TokenEscapeFunction)(char *);

extern PrintMode GlobalPrintMode;

extern PrintOptions GlobalPrintOptions;

AttributeList *ComputePrintStructures(CorpusList *cl);

void ParsePrintOptions();

void CopyPrintOptions(PrintOptions *target, PrintOptions *source);

#endif
