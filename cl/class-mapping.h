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

#ifndef _MAPPING_H_
#define _MAPPING_H_

#include "globals.h"
#include "corpus.h"
#include "attributes.h"

/* simple many-to-one mapping */


/* -------------------- read mapping file */



typedef struct _single_mapping {
  char *class_name;		/* my class name */
  int nr_tokens;		/* the number of tokens which I own */
  int *tokens;			/* the tokens themselves */
} SingleMappingRecord, 
    *SingleMapping;


typedef struct _mapping {
  Corpus *corpus;		/* the corpus I'm valid in */
  Attribute *attribute;		/* the attribute I'm valid for */
  char *mapping_name;		/* my name */
  int nr_classes;		/* the number of mappings */
  SingleMappingRecord *classes;	/* the mappings themselves */
} MappingRecord, *Mapping;


/* -------------------- create/destruct mappings */

Mapping
read_mapping(Corpus *corpus,	/* the corpus for which I'm valid */
	     char *attr_name,	/* the attribute for which I'm valid */
	     char *file_name,	/* the filename of the map spec */
	     char **error_string); /* a char * (not char[]), 
				    * set to an error string or NULL if ok */

int
drop_mapping(Mapping *map);

void
print_mapping(Mapping map);

/* -------------------- token -> class */

SingleMapping
map_token_to_class(Mapping map, 
		   char *token);

int
map_token_to_class_number(Mapping map, 
			  char *token);

int
map_id_to_class_number(Mapping map, 
		       int id);

/* -------------------- class -> {tokens} */

/* returns pointer to token IDs (don't free!), and number of tokens in
 * nr_tokens */

int *
map_class_to_tokens(SingleMapping map,
		    int *nr_tokens);


/* -------------------- utils */

int
number_of_classes(Mapping map);

SingleMapping
find_mapping(Mapping map, char *name);

int
number_of_tokens(SingleMapping map);

/* -------------------- predicates */

int
member_of_class_s(Mapping map, 
		  SingleMapping class, 
		  char *token);

int
member_of_class_i(Mapping map, 
		  SingleMapping class, 
		  int id);

#endif
