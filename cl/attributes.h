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

#ifndef __attributes_h
#define __attributes_h

#include "globals.h"

#include "storage.h"		/* gets sys/types.h, so we don't need it here */
#include "corpus.h"

#define DEFAULT_ATT_NAME "word"  /* don't change this or we'll all end up in hell !!! I MEAN IT !!!! */

/* attribute allocation classes */
#define ATTS_NONE 0
#define ATTS_LOCAL 1

/* attribute and argument/result types now defined in <cl.h> */


/* ================================================== Arguments for dynamic attrs */

typedef struct _DynArg {
  int type;
  struct _DynArg *next;
} DynArg;

DynArg *makearg(char *type_id);

/* ================================================== Huffman compressed item seq */

#define SYNCHRONIZATION 128
#define MAXCODELEN 32

typedef struct _huffman_code_descriptor {

  int size;			/* the id range of the item sequence */
  int length;			/* the number of items in the sequence */

  int min_codelen;		/* minimal code length */
  int max_codelen;		/* maximal code length */

  int lcount[MAXCODELEN];	/* number of codes of length i */
  int symindex[MAXCODELEN];	/* starting point of codes of length i in symbols */
  int min_code[MAXCODELEN];	/* minimal code of length i */

  int *symbols;			/* the code->id mapping table */
} HCD;				/* Huffman Code Descriptor block */

/* ================================================== ATTRIBUTE COMPONENTS */

typedef enum wattr_components {
  CompDirectory,		/* the directory where an attribute is stored  */

  CompCorpus,			/* the sequence of word IDs */
  CompRevCorpus,		/* reversed file of corpus */
  CompRevCorpusIdx,		/* index to reversed file  */
  CompCorpusFreqs,		/* absolute frequencies of corpus  */
  CompLexicon,			/* wordlist */
  CompLexiconIdx,		/* index to wordlist */
  CompLexiconSrt,		/* sorted index to wordlist */

  CompAlignData,		/* alignment data */
  CompXAlignData,		/* extended alignment attribute */

  CompStrucData,		/* structure data */
  CompStrucAVS,			/* structure attribute values */
  CompStrucAVX,			/* structure attribute value index */

  CompHuffSeq,			/* Huffman compressed item sequence */
  CompHuffCodes,		/* Code descriptor data for CompHuffSeq */
  CompHuffSync,			/* Synchronisation of Compressed Item Seq */

  CompCompRF,			/* compressed reversed file (CompRevCorpus) */
  CompCompRFX,			/* index for CompCompRFX (subst CompRCIdx) */

  CompLast			/* MUST BE THE LAST ELEMENT OF THIS ENUM */
} ComponentID;

typedef enum component_states {
  ComponentLoaded,            /* valid and loaded  */
  ComponentUnloaded,          /* valid and on disk */
  ComponentDefined,           /* valid but not yet created */
  ComponentUndefined          /* invalid */
} ComponentState;

typedef struct TComponent {
  char *path;			/* the full filename of this component */
  Corpus *corpus;		/* the corpus this component belongs to */
  union _Attribute *attribute;  /* the attribute this component belongs to */
  ComponentID id;		/* the type of this component */
  int size;			/* a copy of the number of items in the structure */

  MemBlob data;

  /* struct TComponent *next; */
} Component;


char *cid_name(ComponentID cid);

ComponentID component_id(char *name);

int MayHaveComponent(int attr_type, ComponentID cid);

/* ============================================================ ATTRIBUTES */

#define COMMON_ATTR_FIELDS  \
int type;			/* the attribute type */                \
char *name;		/* the attribute name or multi-purpose field*/ \
union _Attribute *next;		/* the next member of the attr chain */ \
int attr_number;                /* a number, unique in this corpus, 0 for word */ \
char *path;		/* path to attribute data files */     \
 \
struct TCorpus *mother;		/* corpus this att is assigned to */    \
Component *components[CompLast]	/* the component vector of the attribute */

/* NO SEMICOLON AFTER LAST FIELD ABOVE!!! DUE TO EMACS INDENTATION */

typedef struct {
  COMMON_ATTR_FIELDS;
} Any_Attribute;

typedef struct {
  COMMON_ATTR_FIELDS;
  HCD *hc;			/* New (Tue Jan 10 16:20:34 1995):
				 * positional attribute may have a
				 * huffman code descriptor block */
  int this_block_nr;		/* number of the current decompression block */
  int this_block[SYNCHRONIZATION]; /* the decompression block proper */
} POS_Attribute;

typedef struct {
  COMMON_ATTR_FIELDS;
  int has_attribute_values;
} Struc_Attribute;

typedef struct {
  COMMON_ATTR_FIELDS;
} Alg_Attribute;

typedef struct {
  COMMON_ATTR_FIELDS;
  char *call;
  int res_type;
  DynArg *arglist;
} Dynamic_Attribute;

/* typedef union _Attribute Attribute; now in <cl.h> */

union _Attribute {
  int type;
  Any_Attribute any;
  POS_Attribute pos;
  Struc_Attribute struc;
  Alg_Attribute align;
  Dynamic_Attribute dyn;
};

/* ============================================================ FUNCTIONS */


/* NEVER CALL THIS!! ONLY USED WHILE PARSING A REGISTRY ENTRY!!!! */

Attribute *setup_attribute(Corpus *corpus, 
			   char *attribute_name, 
			   int type,
			   char *data);

/* find_attribute() and attr_drop_attribute() function prototypes now in <cl.h> */

/* after calling this, the corpus does not have the attribute any
 * longer -- it is the same as it was never defined */

int drop_attribute(Corpus *corpus,
		   char *attribute_name,
		   int type,
		   char *data);	/* depends on type, either char* or int*, but ***UNUSED*** */

/* ======================================== COMPONENTS FOR ALL ATTRS */


Component *load_component(Attribute *attribute, ComponentID component);

int drop_component(Attribute *attribute, ComponentID component);

int comp_drop_component(Component *component);

Component *create_component(Attribute *attribute, ComponentID component);

Component *find_component(Attribute *attribute, ComponentID component);

Component *ensure_component(Attribute *attribute, ComponentID component, 
			    int try_creation);

Component *declare_component(Attribute *attribute, ComponentID cid, 
			     char *path);

void declare_default_components(Attribute *attribute);


ComponentState component_state(Attribute *attribute, ComponentID component);

ComponentState comp_component_state(Component *comp);

char *component_full_name(Attribute *attribute,
				   ComponentID component, 
				   char *path);

/* =============================================== LOOP THROUGH ATTRIUBTES */

Attribute *first_corpus_attribute(Corpus *corpus);

Attribute *next_corpus_attribute();

/* =============================================== INTERACTIVE FUNCTIONS */

void describe_attribute(Attribute *attribute);

void describe_component(Component *component);

/* ================================================================= EOF */

#endif
