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

#ifndef _VARIABLES_H_
#define _VARIABLES_H_

/* ---------------------------------------------------------------------- */

#include "../cl/corpus.h"
#include "../cl/attributes.h"

/* ---------------------------------------------------------------------- */

/** VariableItem object: an item within a variable */
typedef struct _variable_item {
  int64_t free;               /**< Boolean flag: is this item empty? */
  char *sval;             /**< The actual string value of the item. */
  int64_t ival;               /**< Lexicon number associated with the item.
                               Set to -1 on creation, but when the variable is verified
                               against a corpus attribute, it is set to the lexicon number
                               from that attribute. */
} VariableItem;

/**
 * The Variable object: a list of strings that can be used as a variable within
 * a query (to match all tokens whose type is identical to one of the strings
 * on the list).
 *
 * (Plus also VariableBuffer: the former is a pointer to the latter.)
 */
typedef struct _variable_buf {

  int64_t valid;              /**< flag: whether I'm valid or not (valid = associated with a corpus/attribute,
                               and known to match at least one entry in that attribute's lexicon) */
  char *my_name;          /**< my name */

  char *my_corpus;        /**< name of corpus I'm valid for */
  char *my_attribute;     /**< name of attribute I'm valid for */

  int64_t nr_valid_items;     /**< only valid after validation */
  int64_t nr_invalid_items;
  
  int64_t nr_items;           /**< number of items (size of the "items" array) */
  VariableItem *items;    /**< array of items - the set of strings within the variable. */
  
} VariableBuffer, *Variable;

extern int64_t nr_variables;
extern Variable *VariableSpace;

/* ---------------------------------------------------------------------- */

/* Variable methods */

Variable FindVariable(char *varname);

bool VariableItemMember(Variable v, char *item);

void VariableAddItem(Variable v, char *item);

void VariableSubtractItem(Variable v, const char *item);

void VariableDeleteItems(Variable v);

void DropVariable(Variable *vp);

Variable NewVariable(char *varname);

bool
SetVariableValue(char *varName, 
                 char operator,
                 char *varValues);

/* variable iterator functions */
void variables_iterator_new(void);
Variable variables_iterator_next(void);


bool
VerifyVariable(Variable v, Corpus *corpus, Attribute *attribute);

int64_t *
GetVariableItems(Variable v, 
                 Corpus *corpus,
                 Attribute *attribute,
                 /* returned: */
                 int64_t *nr_items);

char **GetVariableStrings(Variable v, int64_t *nr_items);

#endif
