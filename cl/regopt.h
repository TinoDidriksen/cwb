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

#include "globals.h"
#include <regex.h>

#define MAX_GRAINS 12		/* no point in scanning for too many grains, but regexps can be bloody inefficient */

struct _CL_Regex {
  regex_t buffer;
  CorpusCharset charset;
  int flags;			/* IGNORE_CASE and IGNORE_DIAC */
  char *iso_string;		/* buffer of size MAX_LINE_LENGTH used for normalisation by cl_regex_match() */
  /* data from optimiser (see global variables in regopt.c for comments) */
  int grains;			/* number of grains (0 = not optimised) */
  int grain_len;
  char *grain[MAX_GRAINS];
  int anchor_start;
  int anchor_end;
  int jumptable[256];
};


/* interfaces function prototypes are defined in <cl.h> */
