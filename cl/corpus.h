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

#ifndef _corpus_h
#define _corpus_h

#include "globals.h"

#include "storage.h"                      /* gets sys/types.h for caddr_t */

/* ---------------------------------------------------------------------- */

typedef struct _idbuf *IDList;

/**
 * Entry in linked list of strings containing ID
 */
typedef struct _idbuf {
  char *string;
  IDList next;
} IDBuf;

void FreeIDList(IDList *list);

int memberIDList(char *s, IDList l);

/* ---------------------------------------------------------------------- */

/* typedef struct TCorpus Corpus; now in <cl.h> */
/**
 * Contains information on a loaded corpus.
 *
 */
struct TCorpus {

  char *id;          /**< a unique ID (i.e., the registry name identifying the corpus to the CWB) */
  char *name;        /**< the full name of the corpus (descriptive, for information only) */
  char *path;        /**< the ``home directory'' of the corpus  */
  char *info_file;   /**< the path of the info file of the corpus */

  CorpusCharset charset;           /**< a special corpus property: internal support for 'latin1' to 'latin9' planned */
  CorpusProperty properties;

  char *admin;

  IDList groupAccessList;
  IDList userAccessList;
  IDList hostAccessList;
  
  char *registry_dir;
  char *registry_name;

  int nr_of_loads;                 /**< the number of setup_corpus ops */

  union _Attribute *attributes;    /**< the list of attributes */
  
  struct TCorpus *next;            /**< next entry in a linked-list of loaded corpora */

};

/* ---------------------------------------------------------------------- */

extern char *cregin_path;    /**< full path of registry file currently being parsed */
extern char *cregin_name;    /**< name of registry file currently being parsed */

/* ---------------------------------------------------------------------- */

extern Corpus *loaded_corpora;    /**< list of loaded corpus handles (for memory manager) */

/* ---------------------------------------------------------------------- */

/* (most) function prototypes are now in <cl.h> */

void add_corpus_property(Corpus *corpus, char *property, char *value);

#endif
