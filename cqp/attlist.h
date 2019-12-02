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

/** The linked list in an AttributeList consists of these. */
typedef struct _attrbuf {
  char *name;                           /**< name of the attribute */
  Attribute *attribute;                 /**< the relevant Attribute object, only valid if list_valid==1 */
  int64_t status;                           /**< This is user-settable; 0 on initialisation. */

  struct _attrbuf * next;               /**< Next in chain. */
  struct _attrbuf * prev;               /**< Previous in chain */
} AttributeInfo;

/** The AttributeList object: holds a list of attributes */
typedef struct _attlist {

  int64_t list_valid;                       /**< Are all the Attributes in this list valid? 0: check list */
  int64_t element_type;                     /**< One of the constants defined in attributes.h, format:ATT_x */

  AttributeInfo *list;                  /**< Head of the linked list of attribute-info structures. */

} AttributeList;

/* ======================================== Allocation & Deallocation */

AttributeList *NewAttributeList(int64_t element_type);

int64_t DestroyAttributeList(AttributeList **list);

/* ======================================== Adding and Removing */

AttributeInfo *AddNameToAL(AttributeList *list,
                           char *name,
                           int64_t initial_status,
                           int64_t position);

int64_t RemoveNameFromAL(AttributeList *list, char *name);

int64_t NrOfElementsAL(AttributeList *list);

int64_t MemberAL(AttributeList *list, char *name);

AttributeInfo *FindInAL(AttributeList *list, char *name);

int64_t RecomputeAL(AttributeList *list, Corpus *corpus, int64_t initial_status);

int64_t VerifyList(AttributeList *list, 
               Corpus *corpus,
               int64_t remove_illegal_entries);



#endif
