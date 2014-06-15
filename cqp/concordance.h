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

#ifndef _concordance_h_
#define _concordance_h_

#include "../cl/corpus.h"
#include "../cl/class-mapping.h"

#include "context_descriptor.h"
#include "print-modes.h"

/* ============================== Affix lists */

typedef enum _conclinelayout {
  ConcLineHorizontal,
  ConcLineVertical
} ConcLineLayout;

typedef struct _ConcLineField {
  int start_position;
  int end_position;
  int type;
} ConcLineField;

/* ========================================== */

/* No longer used. TODO - delete in future. -- AH 2014-06-15
typedef union _concordanceLineElement {

  int type;			/* simple string or subtree * /

  struct {
    int type;			/* leaf * /
    int ElementType;		/* type of this element * /

    int length;			/* length of s * /
    char *s;			/* s itself * /
  } simpleString;

  struct {
    int type;
    int ElementType;		/* type of the captured elements * /

    int nr_subelements;
    union _concordanceLineElement **subElements;
  } nestedString;

} ConcordanceLineElement;

typedef ConcordanceLineElement *ConcordanceLine;
*/

/* ========================================== */

/* this function not exported :
int append(char *s, char *suffix, int *sp, int max_sp);
*/

/* void add_to_string(char **s, int *spos, int *ssize, char *suffix); */

/* ======================================== */



char *compose_kwic_line(Corpus *corpus,
                        int match_start,
                        int match_end,
                        ContextDescriptor *context,
                        int *length,
                        int *string_match_begin,
                        int *string_match_end,
                        char *left_marker,
                        char *right_marker,
                        int *position_list,
                        int nr_positions,
                        int *returned_positions,
                        ConcLineField *fields,
                        int nr_fields,
                        ConcLineLayout orientation,
                        PrintDescriptionRecord *pdr,
                        int nr_mappings,
                        Mapping *mappings);


void cleanup_kwic_line_memory(void);

#endif
