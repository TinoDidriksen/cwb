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

#ifndef __attlist_h_
#define __attlist_h_

#include "../cl/attributes.h"
#include "../cl/corpus.h"

/* ======================================== Data types */

typedef struct _attrbuf {
  char *name;			/* name of attribute */
  Attribute *attribute;		/* structure, only valid if list_valid==1 */
  int status;			/* user-settable, 0 on init */

  struct _attrbuf * next;	/* chain */
  struct _attrbuf * prev;	/* chain */
} AttributeInfo;

typedef struct _attlist {

  /* holds a list of attributes */

  int list_valid;		/* 0: check list */
  int element_type;		/* attributes.h:ATT_x */

  AttributeInfo *list;		/* the list proper */

} AttributeList;

/* ======================================== Allocation & Deallocation */

AttributeList *NewAttributeList(int element_type);

int DestroyAttributeList(AttributeList **list);

/* ======================================== Adding and Removing */

AttributeInfo *AddNameToAL(AttributeList *list,
			   char *name,
			   int initial_status,
			   int position);

int RemoveNameFromAL(AttributeList *list, char *name);

int NrOfElementsAL(AttributeList *list);

int MemberAL(AttributeList *list, char *name);

AttributeInfo *FindInAL(AttributeList *list, char *name);

int RecomputeAL(AttributeList *list, Corpus *corpus, int initial_status);

int VerifyList(AttributeList *list, 
	       Corpus *corpus,
	       int remove_illegal_entries);

/* ======================================== EOF */

#endif
