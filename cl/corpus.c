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
#include <sys/utsname.h>
#include <pwd.h>
#include <grp.h>

#include "globals.h"

#include "macros.h"
#include "attributes.h"
#include "registry.tab.h"	/* the bison parser table */

#include "corpus.h"


void creg_scan_string(const char *str);	/* function only used by demo version */


extern FILE *cregin;
extern Corpus *cregcorpus;
void cregerror(char *message);
void cregrestart(FILE *file);
int cregparse();

char errmsg[MAX_LINE_LENGTH];

Corpus *loaded_corpora = NULL;

char *cregin_path = "";		/* for registry parser error messages */
char *cregin_name = "";

/* ---------------------------------------------------------------------- */

static char *regdir = NULL;

char *central_corpus_directory()
{
  if (regdir == NULL) 
    regdir = getenv(REGISTRY_ENVVAR);
  if (regdir == NULL)
    regdir = REGISTRY_DEFAULT_PATH;
  return regdir;
}

/* ---------------------------------------------------------------------- */

Corpus *
find_corpus(char *registry_dir, char *registry_name) {
  Corpus *c;
  char *mark;

  if (registry_dir == NULL)
    registry_dir = cl_standard_registry();

  for (c = loaded_corpora; c != NULL; c = c->next) {
    int l_dir = strlen(c->registry_dir);
    if ( 
	STREQ(registry_name, c->registry_name) && /* corpus ID must be the same */
	(mark = strstr(registry_dir, c->registry_dir)) && /* find registry dir of <c> as substring of list <registry_dir> */
	/* now we must check that the substring corresponds to a full component of the list */
	(mark == registry_dir || mark[-1] == ':') && /* must start at beginning of string or after ':' separator */
	(mark[l_dir] == '\0' || mark[l_dir] == ':') /* must end at end of string or before ':' separator */
	) {
      break;
    }
  }

  return c;			/* either return matching corpus object or NULL at end of list */
}

FILE *
find_corpus_registry(char *registry_dir,
		     char *registry_name,
		     char **real_registry_dir)
{
  char full_name[MAX_LINE_LENGTH];

  int re_p, ins_p, p, start_of_entry, end_of_entry;

  FILE *fd;

  re_p = 0;
  
  for (;;) {
    if (registry_dir[re_p] == '\0') {
      *real_registry_dir = NULL;
      return NULL;
    }
    else {

      ins_p = 0;
      start_of_entry = re_p;

      do {

	full_name[ins_p++] = registry_dir[re_p++];

      } while ((registry_dir[re_p] != ':') && 
	       (registry_dir[re_p] != '\0'));

      end_of_entry = re_p;

      if (full_name[ins_p - 1] != '/')
	full_name[ins_p++] = '/';

      for (p = 0; registry_name[p]; p++)
	full_name[ins_p++] = registry_name[p];

      full_name[ins_p] = '\0';

      if ((fd = fopen(full_name, "r")) != NULL) {
	(*real_registry_dir) = (char *)cl_malloc(end_of_entry - start_of_entry + 1);
	strncpy(*real_registry_dir, 
		registry_dir+start_of_entry, 
		end_of_entry - start_of_entry);
	(*real_registry_dir)[end_of_entry - start_of_entry] = '\0';
	return fd;
      }
      else if (registry_dir[re_p] == ':')
	re_p++;
    }
  }
  
  /* never reached */
  assert(0 && "Not reached");
  return NULL;
}

int CheckAccessConditions(Corpus *corpus, int verbose)
{
  int access_ok = 1;
  struct passwd *pwd = NULL;

  /* get password data only if we have user / group access restrictions */
  if (corpus->userAccessList || corpus->groupAccessList) {
    /*     pwd = getpwuid(getuid()); */
    fprintf(stderr, "CL Error: Sorry, user/group access restrictions are disabled due to incompatibilities.\n");
    if (pwd == NULL) {
      perror("getpwuid(getuid()): can't get user information");
      access_ok = 0;
    }
  }

  if (access_ok && corpus->userAccessList) {
    if (pwd == NULL) {
      perror("getpwuid(getuid()): can't get user information");
      access_ok = 0;
    }
    else if (!memberIDList(pwd->pw_name, corpus->userAccessList)) {
      /* user is not a member of the user access list */
      access_ok = 0;
    }
  }

  if (access_ok && corpus->groupAccessList) {

    gid_t gidset[16];
    int nr_groups;

    if ((nr_groups = getgroups(16, gidset)) < 0) {
      perror("getgroups(2): cant' get group information");
      access_ok = 0;
    }
    else {

      int i;
      struct group *grpent = NULL;
      
      for (i = 0; i < nr_groups; i++) {
	
	/* 	grpent = getgrgid(gidset[i]); */
	fprintf(stderr, "CL Error: Sorry, user/group access restrictions are disabled due to incompatibilities.\n");

	if (grpent == NULL) {
	  perror("getgrgid(2): ");
	  fprintf(stderr, "Can't get group information for gid %d\n",
		  (int) gidset[i]);
	  access_ok = 0;
	}
	else if (memberIDList(grpent->gr_name, corpus->groupAccessList)) {
	  access_ok = 1;
	  break;
	}
      }
    }
  }

  if (access_ok && corpus->hostAccessList) {

    struct utsname hostinfo;

    if (uname(&hostinfo) < 0) {
      perror("uname(2):");
      access_ok = 0;
    }
    else if (!memberIDList(hostinfo.nodename, corpus->hostAccessList)) {
#if 0
      IDList l;

      fprintf(stderr, 
	      "The corpus ``%s'' may be used on the following systems only:\n",
	      corpus->id ? corpus->id : "(unknown)");
      
      for (l = corpus->hostAccessList; l; l = l->next) {
	fprintf(stderr, "\t%s\n", l->string ? l->string : "(null)");
      }
#endif
    }
    else {
      access_ok = 1;
    }
  }
  
  if (!access_ok) {
    fprintf(stderr, "User ``%s'' is not authorized to access corpus ``%s''\n",
	    (pwd && pwd->pw_name) ? pwd->pw_name : "(unknown)",
	    corpus->name);
  }
  
  return access_ok;
}

Corpus *
setup_corpus(char *registry_dir,
	     char *registry_name)
{ 
  char *real_registry_name;
  static char *canonical_name = NULL;
  Corpus *corpus;
  
  /* corpus name must be all lowercase at this level -> canonicalise (standard) uppercase and (deprecated) mixed-case forms */
  cl_free(canonical_name);	/* if necessary, free buffer allocated in previous call to setup_corpus() */

  canonical_name = cl_strdup(registry_name);
  cl_string_canonical(canonical_name, IGNORE_CASE);

  /* ------------------------------------------------------------------ */

  if ((corpus = find_corpus(registry_dir, canonical_name)) != NULL) {
    
    /* we already have the beast loaded, so just increment the references */

    corpus->nr_of_loads++;

  }
  else {

    /* it's not yet in memory, so create and load it */

    if (registry_dir == NULL) 
      registry_dir = central_corpus_directory();

    cregin = find_corpus_registry(registry_dir, 
				  canonical_name, 
				  &real_registry_name);

    if (cregin == NULL) {
      fprintf(stderr, "setup_corpus: can't locate <%s> in %s\n", 
	      registry_name, 
	      registry_dir);
    }
    else {
      cregrestart(cregin);
      cregin_path = real_registry_name;
      cregin_name = canonical_name;
      if (cregparse() == 0) {	/* OK */
	if (CheckAccessConditions(cregcorpus, 0)) {
	  corpus = cregcorpus;
	  corpus->registry_dir = real_registry_name;
	  corpus->registry_name = cl_strdup(canonical_name);
	  corpus->next = loaded_corpora;
	  loaded_corpora = corpus;
	  /* check whether ID field corresponds to name of registry file */
	  if (corpus->id && (strcmp(corpus->id, canonical_name) != 0)) {
	    fprintf(stderr, "CL warning: ID field '%s' does not match name of registry file %s/%s\n", corpus->id, real_registry_name, canonical_name);
	  }
	}
	else {
	  drop_corpus(cregcorpus);
	}
      }
      cregin_path = "";
      cregin_name = "";

      cregcorpus = NULL;

      fclose(cregin);
    }

  }

  /* Check access permissions */

  return corpus;
}

int drop_corpus(Corpus *corpus)	
{
  Corpus *prev;

  assert(corpus != NULL);
  assert(corpus->nr_of_loads > 0);

  /* decrement the number of references to corpus */
  corpus->nr_of_loads--;

  if (corpus->nr_of_loads == 0) {

    if (corpus == loaded_corpora)
      loaded_corpora = corpus->next;
    else {

      prev = loaded_corpora;

      while (prev && (prev->next != corpus))
	prev = prev->next;
      
      if (prev == NULL) {
	if (corpus != cregcorpus)
	  assert("Error in list of loaded corpora" && 0);
      }
      else {
	assert(prev->next == corpus);
	prev->next = corpus->next;
      }
    }

    /* delete it physically iff nobody wants to have it any more */
    
    while (corpus->attributes != NULL)
      attr_drop_attribute(corpus->attributes);
    
    corpus->attributes = NULL;
    corpus->next = NULL;

    cl_free(corpus->id);
    cl_free(corpus->name);
    cl_free(corpus->path);
    cl_free(corpus->info_file);
    cl_free(corpus->registry_dir);
    cl_free(corpus->registry_name);

    cl_free(corpus->admin);

    if (corpus->groupAccessList)
      FreeIDList(&(corpus->groupAccessList));

    if (corpus->userAccessList)
      FreeIDList(&(corpus->userAccessList));

    if (corpus->hostAccessList)
      FreeIDList(&(corpus->userAccessList));

    corpus->next = NULL;

    corpus->nr_of_loads = 0;

    cl_free(corpus);

  }

  return 1;
}


/* ---------------------------------------------------------------------- */

void describe_corpus(Corpus *corpus)
{
  Attribute *attr;
  
  assert(corpus != NULL);

  printf("\n\n-------------------- CORPUS SETUP ---------------------\n\n");

  printf("ID:\t%s\n",   corpus->id   ? corpus->id   : "(null)");
  printf("Name:\t%s\n", corpus->name ? corpus->name : "(null)");
  printf("Path:\t%s\n", corpus->path ? corpus->path : "(null)");
  printf("Info:\t%s\n", corpus->info_file ? corpus->info_file : "(null)");

  printf("\nRegistry Directory:\t%s\n", 
	 corpus->registry_dir ? corpus->registry_dir : "(null)");
  printf("Registry Name:     \t%s\n\n", 
	 corpus->registry_name ? corpus->registry_name : "(null)");

  printf("Attributes:\n");
  for (attr = (Attribute *)(corpus->attributes); 
       attr != NULL; 
       attr = (Attribute *)(attr->any.next))
    describe_attribute(attr);

  printf("\n\n------------------------- END -------------------------\n\n");
}

/* ---------------------------------------------------------------------- */

void FreeIDList(IDList *list)
{
  IDList l;
  
  while (*list) {
    l = *list;
    *list = (*list)->next;
    cl_free(l->string);
    free(l);
  }
  *list = NULL;
}

int memberIDList(char *s, IDList l)
{
  while (l) {
    if (strcmp(s, l->string) == 0)
      return 1;
    l = l->next;
  }
  return 0;
}

/* ---------------------------------------------------------------------- */

/*
 * corpus properties
 */

/* corpus properties iterator / property datatype is public */
CorpusProperty 
cl_first_corpus_property(Corpus *corpus) {
  return corpus->properties;
}
CorpusProperty
cl_next_corpus_property(CorpusProperty prop) {
  if (prop == NULL)
    return NULL;
  else
    return prop->next;
}

/* returns property value or NULL if undefined */
char *
cl_corpus_property(Corpus *corpus, char *property) {
  CorpusProperty p = cl_first_corpus_property(corpus);

  while ((p != NULL) && strcmp(property, p->property)) 
    p = cl_next_corpus_property(p);
  if (p != NULL) 
    return p->value;
  else
    return NULL;
}

/* ---------------------------------------------------------------------- */

/* the special 'charset' property */
CorpusCharset
cl_corpus_charset(Corpus *corpus) {
  return corpus->charset;
}

/* list of charset names */
typedef struct _charset_spec {
  CorpusCharset id;
  char *name;
} charset_spec;

charset_spec charset_names[] = {
  { ascii,   "ascii"},
  { latin1,  "latin1"},
  { latin1,  "iso-8859-1"},
  { latin2,  "latin2"},
  { latin2,  "iso-8859-2"},
  { latin3,  "latin3"},
  { latin3,  "iso-8859-3"},
  { latin4,  "latin4"},
  { latin4,  "iso-8859-4"},
  { cyrillic,"cyrillic"},
  { cyrillic,"iso-8859-5"},
  { arabic,  "arabic"},
  { arabic,  "iso-8859-6"},
  { greek,   "greek"},
  { greek,   "iso-8859-7"},
  { hebrew,  "hebrew"},
  { hebrew,  "iso-8859-8"},
  { latin5,  "latin5"},
  { latin5,  "iso-8859-9"},
  { latin6,  "latin6"},
  { latin6,  "iso-8859-10"},
  { latin7,  "latin7"},
  { latin7,  "iso-8859-13"},
  { latin8,  "latin8"},
  { latin8,  "iso-8859-14"},
  { latin9,  "latin9"},
  { latin9,  "iso-8859-15"},
  { utf8,    "utf8"},
  { unknown_charset, NULL} };

/* return name of charset */
char *
cl_charset_name(CorpusCharset id) {
  int i;

  for (i = 0; charset_names[i].name; i++) {
    if (id == charset_names[i].id) 
      return charset_names[i].name;
  }
  return "<unsupported>";
}

/* add property to list of corpus properties
   :: ignore and warn if already defined
   :: if property=='charset', set corpus charset as well */
void 
add_corpus_property(Corpus *corpus, char *property, char *value) {
  CorpusProperty new_prop;
  CorpusCharset charset;
  int i;

  if (cl_corpus_property(corpus, property) != NULL) {
    fprintf(stderr, "REGISTRY WARNING (%s/%s): re-defintion of property '%s' (ignored)\n", cregin_path, cregin_name, property);
  }
  else {
    new_prop = (CorpusProperty) cl_malloc(sizeof(struct TCorpusProperty));
    new_prop->property = property; /* use this function from registry.y only! */
    new_prop->value = value;	   /* property & value are strdup()ed in registry.l */
    new_prop->next = corpus->properties;
    corpus->properties = new_prop;
    
    /* if property=='charset', set corpus->charset accordingly */
    if (0 == strcmp(property, "charset")) {
      charset = unknown_charset;

      for (i = 0; charset_names[i].name; i++) {
	if (strcasecmp(value, charset_names[i].name) == 0) {
	  charset = charset_names[i].id;
	  break;
	}
      }
      corpus->charset = charset;
    }
  }
}
