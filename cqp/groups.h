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

#ifndef _GROUPS_H_
#define _GROUPS_H_

#include "../cl/attributes.h"
#include <stdio.h>
#include "corpmanag.h"
#include "output.h"

#define SEPARATOR \
"#---------------------------------------------------------------------\n"
#define SEPARATOR2 \
"#=====================================================================\n"

#define ANY_ID -2


typedef struct _id_cnt_mapping {
  int64_t s, t, freq, s_freq;
} ID_Count_Mapping;

typedef struct _grouptable {

  CorpusList *my_corpus;

  Attribute *source_attribute;
  int64_t source_is_struc;
  char *source_base;
  FieldType source_field;
  int64_t source_offset;

  Attribute *target_attribute;
  int64_t target_is_struc;
  char *target_base;
  FieldType target_field;
  int64_t target_offset;

  int64_t cutoff_frequency;
  int64_t is_grouped;

  int64_t nr_cells;
  ID_Count_Mapping *count_cells;

} Group;

Group *compute_grouping(CorpusList *cl,
			FieldType source_field,
			int64_t source_offset,
			char *source_attr_name,
			FieldType target_field,
			int64_t target_offset,
			char *target_attr_name,
			int64_t cutoff_freq,
      int64_t is_grouped);

void free_group(Group **group);

void print_group(Group *group, int64_t expand, struct Redir *rd);

char *Group_id2str(Group *group, int64_t i, int64_t target);


#endif
