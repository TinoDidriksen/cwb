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

#include <ctype.h>
#include <sys/types.h>

#include "globals.h"

#include "endian.h"
#include "corpus.h"
#include "macros.h"
#include "fileutils.h"
#include "cdaccess.h"
#include "makecomps.h"
#include "list.h"

#include "attributes.h"


/*******************************************************************
 * FLAGS
 *******************************************************************/

/* when ENSURE_COMPONENT_EXITS is defined, ensure_component will exit
 * when the component can't be created or loaded.
 */

/* when ALLOW_COMPONENT_CREATION is defined, components may be created
 * on the fly by ensure_component.
 */

/*
 * if KEEP_SILENT is defined, ensure_component won't complain about
 * non-accessible data.
 */

#define KEEP_SILENT

/*******************************************************************/

typedef struct component_field_spec {
  ComponentID id;
  char *name;
  int using_atts;
  char *default_path;
} component_field_spec;

static struct component_field_spec Component_Field_Specs[] =
{ 
  { CompDirectory,    "DIR",     ATT_ALL,    "$APATH"},

  { CompCorpus,       "CORPUS",  ATT_POS,    "$DIR/$ANAME.corpus"},
  { CompRevCorpus,    "REVCORP", ATT_POS,    "$CORPUS.rev"},
  { CompRevCorpusIdx, "REVCIDX", ATT_POS,    "$CORPUS.rdx"},
  { CompCorpusFreqs,  "FREQS",   ATT_POS,    "$CORPUS.cnt"},
  { CompLexicon,      "LEXICON", ATT_POS,    "$DIR/$ANAME.lexicon"},
  { CompLexiconIdx,   "LEXIDX",  ATT_POS,    "$LEXICON.idx"},
  { CompLexiconSrt,   "LEXSRT",  ATT_POS,    "$LEXICON.srt"},


  { CompAlignData,    "ALIGN",   ATT_ALIGN,  "$DIR/$ANAME.alg"},
  { CompXAlignData,   "XALIGN",  ATT_ALIGN,  "$DIR/$ANAME.alx"},

  { CompStrucData,    "STRUC",   ATT_STRUC,  "$DIR/$ANAME.rng"},
  { CompStrucAVS,     "STRAVS",  ATT_STRUC,  "$DIR/$ANAME.avs"},
  { CompStrucAVX,     "STRAVX",  ATT_STRUC,  "$DIR/$ANAME.avx"},

  { CompHuffSeq,      "CIS",     ATT_POS,    "$DIR/$ANAME.huf"},
  { CompHuffCodes,    "CISCODE", ATT_POS,    "$DIR/$ANAME.hcd"},
  { CompHuffSync,     "CISSYNC", ATT_POS,    "$CIS.syn"},

  { CompCompRF,       "CRC",     ATT_POS,    "$DIR/$ANAME.crc"},
  { CompCompRFX,      "CRCIDX",  ATT_POS,    "$DIR/$ANAME.crx"},

  { CompLast,         "INVALID", 0,          "INVALID"}
};

/* ---------------------------------------------------------------------- */

ComponentState comp_component_state(Component *component);

/* ---------------------------------------------------------------------- */

struct component_field_spec *find_cid_name(char *name)
{
  int i;

  for (i = 0; i < CompLast; i++) {
    if (strcmp(Component_Field_Specs[i].name, name) == 0)
      return &Component_Field_Specs[i];
  }
  return NULL;
}

struct component_field_spec *find_cid_id(ComponentID id)
{
  if (id < CompLast)
    return &Component_Field_Specs[id];
  else
    return NULL;
}

char *cid_name(ComponentID id)
{
  struct component_field_spec *spec = find_cid_id(id);
  return (spec == NULL ? "((NULL))" : spec->name);
}

ComponentID component_id(char *name)
{
  struct component_field_spec *spec = find_cid_name(name);
  return (spec == NULL ? CompLast : spec->id);
}

int MayHaveComponent(int attr_type, ComponentID cid)
{
  struct component_field_spec *spec;

  spec = find_cid_id(cid);

  if (spec && (spec->id != CompLast))
    return (spec->using_atts & attr_type) ? 1 : 0;
  else
    return 0;
}

char *aid_name(int i)
{
  switch (i) {
  case ATT_NONE:  return "NONE (ILLEGAL)"; break;
  case ATT_POS:   return "Positional Attribute"; break;
  case ATT_STRUC: return "Structural Attribute"; break;
  case ATT_ALIGN: return "Alignment Attribute"; break;
  case ATT_DYN:   return "Dynamic Attribute"; break;
  default:        return "ILLEGAL ATTRIBUTE TYPE"; break;
  }
}

char *argid_name(int i)
{
  switch (i) {
  case ATTAT_NONE:   return "NONE(ILLEGAL)"; break;
  case ATTAT_POS:    return "CorpusPosition"; break;
  case ATTAT_STRING: return "String"; break;
  case ATTAT_VAR:    return "Variable[StringList]"; break;
  case ATTAT_INT:    return "Integer"; break;
  case ATTAT_FLOAT:  return "Float"; break;
  case ATTAT_PAREF:  return "PARef"; break;
  default:           return "ILLEGAL*ARGUMENT*TYPE"; break;
  }
}

/* ---------------------------------------------------------------------- */

DynArg *makearg(char *type_id)
{
  DynArg *arg;

  arg = NULL;

  if (strcmp(type_id, "STRING") == 0) {
    arg = new(DynArg);
    arg->type = ATTAT_STRING;
    arg->next = NULL;
  }
  else if (strcmp(type_id, "POS") == 0) {
    arg = new(DynArg);
    arg->type = ATTAT_POS;
    arg->next = NULL;
  }
  else if (strcmp(type_id, "INT") == 0) {
    arg = new(DynArg);
    arg->type = ATTAT_INT;
    arg->next = NULL;
  }
  else if (strcmp(type_id, "VARARG") == 0) {
    arg = new(DynArg);
    arg->type = ATTAT_VAR;
    arg->next = NULL;
  }
  else if (strcmp(type_id, "FLOAT") == 0) {
    arg = new(DynArg);
    arg->type = ATTAT_FLOAT;
    arg->next = NULL;
  }
  else
    arg = NULL;

  return arg;
}

/* ---------------------------------------------------------------------- */

Attribute *setup_attribute(Corpus *corpus, 
			   char *attribute_name, 
			   int type,
			   char *data)
{
  Attribute *attr;
  Attribute *prev;

  int a_num;

  attr = NULL;

  if (find_attribute(corpus, attribute_name, type, data) != NULL)
    fprintf(stderr, "attributes:setup_attribute(): Warning: \n"
	    "  Attribute %s of type %s already defined in corpus %s\n",
	    attribute_name, aid_name(type), corpus->id);
  else {

    ComponentID cid;

    attr = new(Attribute);
    attr->type = type;
    attr->any.mother = corpus;
    attr->any.name = attribute_name;

    for (cid = CompDirectory; cid < CompLast; cid++)
      attr->any.components[cid] = NULL;

    if (strcmp(attribute_name, "word") == 0 && type == ATTAT_POS)
      a_num = 0;
    else
      a_num = 1;

    /* insert at end of attribute list */
    
    attr->any.next = NULL;
    if (corpus->attributes == NULL)
      corpus->attributes = attr;
    else {
      for (prev = corpus->attributes; prev->any.next; prev = prev->any.next)
	a_num++;
      assert(prev);
      assert(prev->any.next == NULL);
      prev->any.next = attr;
    }
    attr->any.attr_number = a_num;

    attr->any.path = NULL;

    /* ======================================== type specific initializations */

    switch (attr->type) {

    case ATT_POS:
      attr->pos.hc = NULL;
      attr->pos.this_block_nr = -1;
      break;

    case ATT_STRUC:
      attr->struc.has_attribute_values = -1; /* not yet known */
      break;

    default:
      break;
    }
  }
  
  return attr;
}

Attribute *find_attribute(Corpus *corpus, 
			  char *attribute_name, 
			  int type, 
			  char *data)
{
  Attribute *attr;

  attr = NULL;

  if (corpus == NULL)
    fprintf(stderr, "attributes:find_attribute(): called with NULL corpus\n");
  else {
    
    for (attr = corpus->attributes; attr != NULL; attr = attr->any.next)
      if ((type == attr->type) &&
	  STREQ(attr->any.name, attribute_name))
	break;
  }
  return attr;
}

int drop_attribute(Corpus *corpus,
		   char *attribute_name,
		   int type,
		   char *data)
{
  if (corpus == NULL) {
    fprintf(stderr, "attributes:drop_attribute(): called with NULL corpus\n");
    return 0;
  }
  else
    return attr_drop_attribute(find_attribute(corpus, attribute_name, type, data));
}

int attr_drop_attribute(Attribute *attribute)
{
  Attribute *prev;
  DynArg *arg;
  Corpus *corpus;
  ComponentID cid;


  if (attribute == NULL)
    return 0;
  else {

    prev = NULL;
    corpus = attribute->any.mother;
    
    assert("NULL corpus in attribute" && (corpus != NULL));

    if (attribute == corpus->attributes)
      corpus->attributes = attribute->any.next;
    else {

      /* remove attribute from corpus attribute list */

      for (prev = corpus->attributes; 
	   (prev != NULL) && (prev->any.next != attribute);
	   prev = prev->any.next)
	;
      
      if (prev == NULL)
	fprintf(stderr, "attributes:attr_drop_attribute():\n"
		"  Warning: Attribute %s not in list of corpus attributes\n",
		attribute->any.name);
      else {
	assert("Error in attribute chain" && (prev->any.next == attribute));
	prev->any.next = attribute->any.next;
      }
    }
      
    /* get rid of components */
    for (cid = CompDirectory; cid < CompLast; cid++)
      if (attribute->any.components[cid]) {
	comp_drop_component(attribute->any.components[cid]);
	attribute->any.components[cid] = NULL;
      }

    cl_free(attribute->any.name);
    cl_free(attribute->any.path);
    
    /* get rid of special fields */

    switch (attribute->type) {

    case ATT_POS:
      cl_free(attribute->pos.hc);
      break;

    case ATT_DYN:
      cl_free(attribute->dyn.call);
      while (attribute->dyn.arglist != NULL) {
	arg = attribute->dyn.arglist;
	attribute->dyn.arglist = arg->next;
	cl_free(arg);
      }
      break;

    default:
      break;
    }

    attribute->any.mother = NULL;
    attribute->any.type = ATT_NONE;
    attribute->any.next = NULL;
    /* attribute->any.components = NULL; */
    
    free(attribute);
    return 1;
  }
  
  /* notreached */
  assert("Notreached point reached ..." && 0);
  return 1;
}

/* ---------------------------------------------------------------------- */

Component *declare_component(Attribute *attribute, ComponentID cid, char *path)
{
  Component *component;

  if (attribute == NULL) {
    fprintf(stderr, "attributes:declare_component(): \n"
	    "  NULL attribute passed in declaration of %s component\n",
	    cid_name(cid));
    return NULL;
  }
  else if ((component = attribute->any.components[cid]) == NULL) {

    component = new(Component);

    component->id = cid;
    component->corpus = attribute->any.mother;
    component->attribute = attribute;
    component->path = NULL;

    init_mblob(&(component->data));

    /* important to do this before the call to c_full_name */
    attribute->any.components[cid] = component;
    
    /* initialize component path */
    (void) component_full_name(attribute, cid, path);

    return component;
  }
  else {

    fprintf(stderr, "attributes:declare_component(): Warning:\n"
	    "  Component %s of %s declared twice\n",
	    cid_name(cid), attribute->any.name);
    return component;
  }

  /* notreached */
  assert("Notreached point reached ..." && 0);
  return NULL;
}

void declare_default_components(Attribute *attribute)
{
  int i;

  if (attribute == NULL)
    fprintf(stderr, "attributes:declare_default_components(): \n"
	    "  NULL attribute passed -- can't create defaults\n");
  else {
    for (i = CompDirectory; i < CompLast; i++)
      if (((Component_Field_Specs[i].using_atts & attribute->type) != 0) &&
	  (attribute->any.components[i] == NULL))
	(void) declare_component(attribute, i, NULL);
  }
}

/* ---------------------------------------------------------------------- */

ComponentState 
comp_component_state(Component *comp) {
  assert(comp);

  if (comp->data.data != NULL)
    return ComponentLoaded;
  else if (comp->id == CompDirectory)
    return ComponentDefined;
  else if (comp->path == NULL)
    return ComponentUndefined;
  else if (file_length(comp->path) < 0) /* access error == EOF -> assume file doesn't exist */
    return ComponentDefined;
  else
    return ComponentUnloaded;
}

ComponentState 
component_state(Attribute *attribute, ComponentID cid) {
  if (cid < CompLast) {
    
    Component *comp = attribute->any.components[cid];
    
    if (comp == NULL)
      return ComponentUndefined;
    else
      return comp_component_state(comp);
  }
  else
    return ComponentUndefined;
}

/* ---------------------------------------------------------------------- */

char *component_full_name(Attribute *attribute, ComponentID cid, char *path)
{
  component_field_spec *compspec;
  Component *component;
  
  static char buf[MAX_LINE_LENGTH];
  char rname[MAX_LINE_LENGTH];
  char *reference;
  char c;

  int ppos, bpos, dollar, rpos;


  /*  did we do the job before? */
  
  if ((component = attribute->any.components[cid]) != NULL &&
      (component->path != NULL))
    return component->path;

  /*  yet undeclared. So try to guess the name: */
  
  compspec = NULL;

  if (path == NULL) {
    if ((compspec = find_cid_id(cid)) == NULL) {
      fprintf(stderr, "attributes:component_full_name(): Warning:\n"
	      "  can't find component table entry for Component #%d\n", cid);
      return NULL;
    }
    path = compspec->default_path;
  }

  ppos = 0;
  bpos = 0;
  dollar = 0;
  rpos = 0;
  buf[bpos] = '\0';

  while ((c = path[ppos]) != '\0') {
  
    if (c == '$') {

      /*  reference to the name of another component. */

      dollar = ppos;		/* memorize the position of the $ */

      rpos = 0;
      c = path[++ppos];		/* first skip the '$' */
      while (isupper(c)) {
	rname[rpos++] = c;
	c = path[++ppos];
      }
      rname[rpos] = '\0';

      /* ppos now points to the first character after the reference 
       * rname holds the name of the referenced component 
       */

      reference = NULL;

      if (STREQ(rname, "HOME"))
	reference = getenv(rname);
      else if (STREQ(rname, "APATH"))
	reference = (attribute->any.path ? attribute->any.path 
		     : attribute->any.mother->path);
      else if (STREQ(rname, "ANAME"))
	reference = attribute->any.name;
      else if ((compspec = find_cid_name(rname)) != NULL)
	reference = component_full_name(attribute, compspec->id, NULL);
      
      if (reference == NULL) {
	fprintf(stderr, "attributes:component_full_name(): Warning:\n"
		"  Can't reference to the value of %s -- copying\n",
		rname);
	reference = rname;
      }

      for (rpos = 0; reference[rpos] != '\0'; rpos++) {
	buf[bpos] = reference[rpos];
	bpos++;
      }
    }
    else {
      buf[bpos] = c;
      bpos++;
      ppos++;
    }
  }
  buf[bpos] = '\0';

  if (component != NULL)
    component->path = (char *)cl_strdup(buf);
  else
    (void) declare_component(attribute, cid, buf);

  /*  and return it */
  return &buf[0];
}

/* ---------------------------------------------------------------------- */

Component *load_component(Attribute *attribute, ComponentID cid)
{

  Component *comp;

  assert((attribute != NULL) && "Null attribute passed to load_component");

  comp = attribute->any.components[cid];

  if (comp == NULL) {
    fprintf(stderr, "attributes:load_component(): Warning:\n"
	    "  Component %s is not declared for %s attribute\n",
	    cid_name(cid), aid_name(attribute->type));
  }
  else if (comp_component_state(comp) == ComponentUnloaded) {

    assert(comp->path != NULL);

    if (cid == CompHuffCodes) {

      if (item_sequence_is_compressed(attribute)) {

	if (read_file_into_blob(comp->path, MMAPPED, sizeof(int), &(comp->data)) == 0)
	  fprintf(stderr, "attributes:load_component(): Warning:\n"
		  "  Data of %s component of attribute %s can't be loaded\n",
		  cid_name(cid), attribute->any.name);
	else {
	  
	  if (attribute->pos.hc != NULL)
	    fprintf(stderr, "attributes:load_component: WARNING:\n\t"
		    "HCD block already loaded, overwritten.\n");
	  
	  attribute->pos.hc = new(HCD);
	  /* bcopy(comp->data.data, attribute->pos.hc, sizeof(HCD)); */
	  memcpy(attribute->pos.hc, comp->data.data, sizeof(HCD));

#if defined(CWB_LITTLE_ENDIAN)
	  { 
	    int i;
	    /* shit, but we have to */
	    attribute->pos.hc->size = ntohl(attribute->pos.hc->size);
	    attribute->pos.hc->length = ntohl(attribute->pos.hc->length);
	    attribute->pos.hc->min_codelen = ntohl(attribute->pos.hc->min_codelen);
	    attribute->pos.hc->max_codelen = ntohl(attribute->pos.hc->max_codelen);
	    for (i = 0; i < MAXCODELEN; i++) {
	      attribute->pos.hc->lcount[i] = ntohl(attribute->pos.hc->lcount[i]);
	      attribute->pos.hc->symindex[i] = ntohl(attribute->pos.hc->symindex[i]);
	      attribute->pos.hc->min_code[i] = ntohl(attribute->pos.hc->min_code[i]);
	    }
	  }
#endif
	  attribute->pos.hc->symbols = comp->data.data + (4+3*MAXCODELEN);
	
	  comp->size = attribute->pos.hc->length;
	  assert(comp_component_state(comp) == ComponentLoaded);
	}
      }
      else {
	fprintf(stderr, "attributes/load_component: missing files of compressed PA,\n"
		"\tcomponent CompHuffCodes not loaded\n");
      }

    }
    else if ((cid > CompDirectory) && (cid < CompLast)) {

      if (read_file_into_blob(comp->path, MMAPPED, sizeof(int), &(comp->data)) == 0)
	fprintf(stderr, "attributes:load_component(): Warning:\n"
		"  Data of %s component of attribute %s can't be loaded\n",
		cid_name(cid), attribute->any.name);
      else {
	comp->size = comp->data.nr_items;
	assert(comp_component_state(comp) == ComponentLoaded);
      }
    }
  }
  else if (comp_component_state(comp) == ComponentDefined)
    comp->size = 0;

  return comp;
}

/* ---------------------------------------------------------------------- */

Component *create_component(Attribute *attribute, ComponentID cid)
{
  
  Component *comp = attribute->any.components[cid];
  
  if (cl_debug) {
    fprintf(stderr, "Creating %s\n", cid_name(cid));
  }

  if (component_state(attribute, cid) == ComponentDefined) {

    assert(comp != NULL);
    assert(comp->data.data == NULL);
    assert(comp->path != NULL);
    
    switch (cid) {
      
    case CompLast:
    case CompDirectory:
      /*  cannot create these */
      break;
      
    case CompCorpus:
    case CompLexicon:
    case CompLexiconIdx:
      fprintf(stderr, "attributes:create_component(): Warning:\n"
	      "  Can't create the '%s' component. Use 'encode' to create it"
	      " out of a text file\n",
	      cid_name(cid));
      return NULL;
      break;

    case CompHuffSeq:
    case CompHuffCodes:
    case CompHuffSync:
      fprintf(stderr, "attributes:create_component(): Warning:\n"
	      "  Can't create the '%s' component. Use 'huffcode' to create it"
	      " out of an item sequence file\n",
	      cid_name(cid));
      return NULL;
      break;
      
    case CompCompRF:
    case CompCompRFX:
      fprintf(stderr, "attributes:create_component(): Warning:\n"
	      "  Can't create the '%s' component. Use 'compress-rdx' to create it"
	      " out of the reversed file index\n",
	      cid_name(cid));
      return NULL;
      break;
      
    case CompRevCorpus:
      creat_rev_corpus(comp);
      break;
      
    case CompRevCorpusIdx:
      creat_rev_corpus_idx(comp);
      break;

    case CompLexiconSrt:
      creat_sort_lexicon(comp);
      break;
      
    case CompCorpusFreqs:
      creat_freqs(comp);
      break;

    case CompAlignData:
    case CompXAlignData:
    case CompStrucData:
    case CompStrucAVS:
    case CompStrucAVX:
      fprintf(stderr, "attributes:create_component(): Warning:\n"
	      "  Can't create the '%s' component of %s attribute %s.\n"
	      "  Use the appropriate external tool to create it.\n",
	       cid_name(cid), aid_name(attribute->type), attribute->any.name);
      return NULL;
      break;
      

    default:
      comp = NULL;
      fprintf(stderr, "attributes:create_component(): Unknown cid: %d\n", cid);
      assert(0);
      break;
    }
    return comp;
  }
  return NULL;
}

/* ---------------------------------------------------------------------- */

Component *ensure_component(Attribute *attribute, ComponentID cid, int try_creation)
{
  Component *comp = NULL;
  
  if ((comp = attribute->any.components[cid]) == NULL) {

    /*  component is undeclared */
    fprintf(stderr, "attributes:ensure_component(): Warning:\n"
	    "  Undeclared component: %s\n", cid_name(cid));
#ifdef ENSURE_COMPONENT_EXITS    
    exit(1);
#endif
    return NULL;
  }
  else {
    switch (comp_component_state(comp)) {

    case ComponentLoaded:
      /*  already here, so do nothing */
      break;

    case ComponentUnloaded:
      (void) load_component(attribute, cid); /* try to load the component */
      if (comp_component_state(comp) != ComponentLoaded) {
#ifndef KEEP_SILENT
	fprintf(stderr, "attributes:ensure_component(): Warning:\n"
		"  Can't load %s component of %s\n", 
		cid_name(cid), attribute->any.name);
#endif
#ifdef ENSURE_COMPONENT_EXITS    
	exit(1);
#endif
	return NULL;
      }
      break;

    case ComponentDefined:	  /*  try to create the component */

      if (try_creation != 0) {

#ifdef ALLOW_COMPONENT_CREATION

	(void) create_component(attribute, cid);
	if (comp_component_state(comp) != ComponentLoaded) {
#ifndef KEEP_SILENT
	  fprintf(stderr, "attributes:ensure_component(): Warning:\n"
		  "  Can't load or create %s component of %s\n", 
		  cid_name(cid), attribute->any.name);
#endif
#ifdef ENSURE_COMPONENT_EXITS
	  exit(1);
#endif
	  return NULL;
	}
#else
	fprintf(stderr, "Sorry, but this program is not set up to allow the\n"
		"creation of corpus components. Please refer to the manuals\n"
		"or use the ''makeall'' tool.\n");
#ifdef ENSURE_COMPONENT_EXITS    
	exit(1);
#endif
	return NULL;
#endif

      }
      else {
#ifndef KEEP_SILENT
	fprintf(stderr, "attributes:ensure_component(): Warning:\n"
		"  I'm not allowed to create %s component of %s\n", 
		  cid_name(cid), attribute->any.name);
#endif
#ifdef ENSURE_COMPONENT_EXITS    
	exit(1);
#endif
	return NULL;
      }
      break;

    case ComponentUndefined:      /*  don't have this, -> error */
      fprintf(stderr, "attributes:ensure_component(): Warning:\n"
	      "  Can't ensure undefined/illegal %s component of %s\n", 
	      cid_name(cid), attribute->any.name);
#ifdef ENSURE_COMPONENT_EXITS    
      exit(1);
#endif
      break;

    default:
      fprintf(stderr, "attributes:ensure_component(): Warning:\n"
	      "  Illegal state of  %s component of %s\n", 
	      cid_name(cid), attribute->any.name);
#ifdef ENSURE_COMPONENT_EXITS    
      exit(1);
#endif
      break;
    }
  }
  return comp;
}

/* ---------------------------------------------------------------------- */

Component *find_component(Attribute *attribute, ComponentID cid)
{
  return attribute->any.components[cid];
}

/* ---------------------------------------------------------------------- */

int comp_drop_component(Component *comp)
{
  assert((comp != NULL) && "NULL component passed to attributes:comp_drop_component");

  assert(comp->attribute);

  if (comp->attribute->any.components[comp->id] != comp)
    assert(0 && "comp is not member of that attr");

  comp->attribute->any.components[comp->id] = NULL;

  if (comp->id == CompHuffCodes) {

    /* it may be empty, since declare_component doesn't yet load the
     * data */

    cl_free(comp->attribute->pos.hc);
  }

  mfree(&(comp->data));
  cl_free(comp->path);
  comp->corpus = NULL;
  comp->attribute = NULL;
  comp->id = CompLast;

  free(comp);

  return 1;
}

int drop_component(Attribute *attribute, ComponentID cid)
{
  Component *comp;

  if ((comp = attribute->any.components[cid]) != NULL)
    return comp_drop_component(comp);
  else
    return 1;
}

/* =============================================== LOOP THROUGH ATTRIUBTES */

Attribute *loop_ptr;

Attribute *first_corpus_attribute(Corpus *corpus)
{
  if (corpus)
    loop_ptr = corpus->attributes;
  else
    loop_ptr = NULL;

  return loop_ptr;
}

Attribute *next_corpus_attribute()
{
  if (loop_ptr)
    loop_ptr = loop_ptr->any.next;
  return loop_ptr;
}

/* =============================================== INTERACTIVE FUNCTIONS */


void describe_attribute(Attribute *attribute)
{
  DynArg *arg;
  ComponentID cid;
  
  printf("Attribute %s:\n", attribute->any.name);
  printf("  Type:        %s\n", aid_name(attribute->any.type));

  /* print type dependent additional data */

  if (attribute->type == ATT_DYN) {
    printf("  Arguments:   (");
    for (arg = attribute->dyn.arglist; arg; arg = arg->next) {
      printf("%s", argid_name(arg->type));
      if (arg->next != NULL)
	printf(", ");
    }
    printf("):%s\n"
           "               by \"%s\"\n",
	   argid_name(attribute->dyn.res_type),
	   attribute->dyn.call);
  }
  printf("\n");
  for (cid = CompDirectory; cid < CompLast; cid++)
    if (attribute->any.components[cid])
      describe_component(attribute->any.components[cid]);

  printf("\n\n");
}

void describe_component(Component *component)
{
  printf("  Component %s:\n", cid_name(component->id));
  printf("    Attribute:   %s\n", component->attribute->any.name);
  printf("    Path/Value:  %s\n", component->path);
  printf("    State:       ");

  switch (comp_component_state(component)) {
  case ComponentLoaded: 
    printf("loaded");
    break;
  case ComponentUnloaded:
    printf("unloaded (valid & on disk)");
    break;
  case ComponentDefined:
    printf("defined  (valid, but not on disk)");
    break;
  case ComponentUndefined:
    printf("undefined (not valid)");
    break;
  default:
    printf("ILLEGAL! (Illegal component state %d)", comp_component_state(component));
    break;
  }
  printf("\n\n");
}

/* =============================================== SET ATTRIBUTES */

/* generate set attribute value in standard syntax ('|' delimited, sorted with cl_strcmp)
   from input string <s>; expects input in '|'-delimited format if <split> is False, 
   otherwise <s> is split on whitespace;
   if there is any syntax error, cl_make_set() returns NULL
 */ 
char *
cl_make_set(char *s, int split) {
  char *copy = cl_strdup(s);	           /* work on copy of <s> */
  cl_string_list l = cl_new_string_list(); /* list of set elements */
  int ok = 0;			/* for split and element check */
  char *p, *mark, *set;
  int i, sl, length;

  /* (1) split input string into set elements */
  if (split) {
    /* split on whitespace */
    p = copy;
    while (*p != 0) {
      while (*p == ' ' || *p == '\t' || *p == '\n') {
	p++;
      }
      mark = p;
      while (*p != 0 && *p != ' ' && *p != '\t' && *p != '\n') {
	p++;
      }
      if (*p != 0) {		/* mark end of substring */
	*p = 0;
	p++;
      }
      else {
	/* p points to end of string; since it hasn't been advanced, the while loop will terminate */
      }
      if (p != mark) {
	cl_string_list_append(l, mark);
      }
    }
    ok = 1;			/* split on whitespace can't really fail */
  }
  else {
    /* check and split '|'-delimited syntax */
    if (copy[0] == '|') {
      mark = p = copy+1;
      while (*p != 0) {
	if (*p == '|') {
	  *p = 0;
	  cl_string_list_append(l, mark);
	  mark = p = p+1;
	}
	else {
	  p++;
	}
      }
      if (p == mark) {		/* otherwise, there was no trailing '|' */
	ok = 1;
      }
    }
  }

  /* (2) check set elements: must not contain '|' character */
  length = cl_string_list_size(l);
  for (i = 0; i < length; i++) {
    if (strchr(cl_string_list_get(l, i), '|') != NULL) {
      ok = 0;
    }
  }

  /* (3) abort if there was any error */
  if (!ok) {
    cl_delete_string_list(l);
    cl_free(copy);
    return NULL;
  }

  /* (4) sort set elements (for unify() function) */
  cl_string_list_qsort(l);

  /* (5) combine elements into set attribute string */
  sl = 2;			/* compute length of string */
  for (i = 0; i < length; i++) {
    sl += strlen(cl_string_list_get(l, i)) + 1;
  }
  set = cl_malloc(sl);		/* allocate string of exact size */
  p = set;
  *p++ = '|';
  for (i = 0; i < length; i++) {
    strcpy(p, cl_string_list_get(l, i));
    p += strlen(cl_string_list_get(l, i));
    *p++ = '|';			/* overwrites EOS mark inserted by strcpy() */
  }
  *p = 0;			/* EOS */
 
  /* (6) free intermediate data and return the set string */
  cl_delete_string_list(l);
  cl_free(copy);
  return set;
}



/* number of elements in set attribute value (using '|'-delimited standard syntax);
   returns -1 on error (in particular, if set is malformed) 
 */ 
int
cl_set_size(char *s) {
  int count = 0;

  if (*s++ != '|') {
    return -1;
  }
  while (*s) {
    if (*s == '|') count++;
    s++;
  }
  if (s[-1] != '|') {
    return -1;
  }
  return count;
}

/* compute intersection of two set attribute values (in standard syntax, i.e. sorted and '|'-delimited);
   memory for the result string must be allocated by the caller;
   returns 0 on error, 1 otherwise
*/

int 
cl_set_intersection(char *result, const char *s1, const char *s2) {
  static char f1[CL_DYN_STRING_SIZE], f2[CL_DYN_STRING_SIZE];	/* static feature buffers (hold current feature) */
  char *p;
  int comparison;

  if ((*s1++ != '|') || (*s2++ != '|'))
    return 0;

  *result++ = '|';		/* Initialise result */

  while (*s1 && *s2) {
    /* while a feature is active, *s_i points to the '|' separator at its end;
       when the feature is used up, *s_i is advanced and we read the next feature */
    if (*s1 != '|') { 
      for (p = f1; *s1 != '|'; s1++) {
	if (!*s1) return 0;	/* unexpected end of string */
	*p++ = *s1;
	/* should check for buffer overflow here! */
      }
      *p = 0;			/* terminate feature string */
    }
    if (*s2 != '|') { 
      for (p = f2; *s2 != '|'; s2++) {
	if (!*s2) return 0;	/* unexpected end of string */
	*p++ = *s2;
	/* should check for buffer overflow here! */
      }
      *p = 0;			/* terminate feature string */
    }
    /* now compare the two active features (uses cl_strcmp to ensure standard behaviour) */
    comparison = cl_strcmp(f1,f2);
    if (comparison == 0) {
      /* common feature -> copy to result vector */
      for (p = f1; *p; p++)
	*result++ = *p;
      *result++ = '|';
      /* both features are used up now */
      s1++; s2++;
    }
    else if (comparison < 0) {
      /* advance s1 */
      s1++;
    }
    else {
      /* advance s2 */
      s2++;
    }
  } /* ends: while (*s1 && *s2) */

  /* computation complete: terminate result string */
  *result = 0;
  return 1;
}


/* EOF */
