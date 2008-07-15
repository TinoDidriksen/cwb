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


#include "../cl/globals.h"
#include "../cl/cl.h"


char *progname;

void usage() {
  fprintf(stderr, 
	  "\n"
	  "Usage: %s [options] corpus-id -S attribute-name\n"
	  "Options:\n"
	  "  -r <reg> use registry directory <reg>\n"
	  "  -h       print this usage text.\n"
	  "Output line format:\n"
	  "   <region_start> TAB <region_end> [ TAB <annotation> ]\n"
	  "Part of the IMS Open Corpus Workbench v" VERSION "\n\n"
	  , progname);
  exit(1);
}

int
main(int argc, char **argv) {
  char *registry_directory = NULL;
  char *corpus_id = NULL;
  char *attr_name = NULL;
  Corpus *corpus = NULL;
  Attribute *att = NULL;
  int has_values, att_size, n, start, end;
  char *annot;

  extern int optind;
  extern char *optarg;
  int c;

  /* ------------------------------------------------- PARSE ARGUMENTS */

  progname = argv[0];

  /* parse arguments */
  while ((c = getopt(argc, argv, "+r:h")) != EOF) 
    switch (c) {

    /* r: registry directory */
    case 'r': 
      if (registry_directory == NULL) registry_directory = optarg;
      else {
	fprintf(stderr, "%s: -r option used twice\n", progname);
	exit(2);
      }
      break;
      
    default: 
    case 'h':
      usage();
      break;

    }
  
  /* expect three arguments: <corpus> -S <attribute> */
  if (argc <= (optind + 2))
    usage();

  /* first argument: corpus id */
  corpus_id = argv[optind++];
  if ((corpus = cl_new_corpus(registry_directory, corpus_id)) == NULL) {
    fprintf(stderr, "%s: Corpus <%s> not registered in %s\n", 
	      progname,
	      corpus_id,
	      (registry_directory ? registry_directory 
	       : central_corpus_directory()));
    exit(1);
  }

  /* second argument: -S */
  if (strcmp(argv[optind++], "-S")) 
    usage();

  /* third argument: attribute name */
  attr_name = argv[optind];
  if ((att = cl_new_attribute(corpus, attr_name, ATT_STRUC)) == NULL) {
    fprintf(stderr, "%s: Can't access s-attribute <%s.%s>\n", 
	      progname,
	      corpus_id, attr_name);
    exit(1);
  }

  /* check if attribute has annotations */
  has_values = cl_struc_values(att);

  /* attribute size, i.e. number of regions */
  att_size = cl_max_struc(att);

  /* print all regions on STDOUT */
  for (n = 0; n < att_size; n++) {
    if (!cl_struc2cpos(att, n, &start, &end)) {
      cl_error("Can't find region boundaries");
      exit(1);
    }
    printf("%d\t%d", start, end);
    if (has_values) {
      annot = cl_struc2str(att, n);
      if (annot == NULL) {
	printf("\t<no annotation>");
      }
      else {
	printf("\t%s", annot);
      }
    }
    printf("\n");
  }

  /* that was all ...  */
  return 0;
}

