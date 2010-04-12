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

#ifndef _CONTEXT_DESCRIPTOR_H_
#define _CONTEXT_DESCRIPTOR_H_


#include "attlist.h"


#define CHAR_CONTEXT -1
#define WORD_CONTEXT -2
#define STRUC_CONTEXT -3
#define ALIGN_CONTEXT -4	/* allow alignment blocks as context */

/**
 * ContextDescriptor object: a bundle of CQP options
 * describing how a list of corpus positions is to be
 * displayed: with left context, with right context,
 * with what attributes, etc.
 *
 * It is passed around between different print functions
 * so that they know what to do!
 */
typedef struct _context_description_block {

  /* ==================== left context */

  int left_width;
  int left_type;
  char *left_structure_name;
  Attribute *left_structure;

  /* ==================== right context */

  int right_width;
  int right_type;
  char *right_structure_name;
  Attribute *right_structure;

  /* ==================== flag whether to print corpus position */
  int print_cpos;

  /* ==================== positional attributes to print */
  AttributeList *attributes;

  /* ==================== structural attributes to print */
  AttributeList *strucAttributes;

  /* ==================== structure tag (values) to print */
  AttributeList *printStructureTags;

  /* ==================== aligned lines to print */
  AttributeList *alignedCorpora;

} ContextDescriptor;


/* Contextdescriptor methods */

int verify_context_descriptor(Corpus *corpus, 
                              ContextDescriptor *cd,
                              int remove_illegal_entries);

int initialize_context_descriptor(ContextDescriptor *cd);

int update_context_descriptor(Corpus *corpus, ContextDescriptor *cd);

ContextDescriptor *NewContextDescriptor();

void FreeContextDescriptor(ContextDescriptor **cdp);

void PrintContextDescriptor(ContextDescriptor *cdp);

#endif
