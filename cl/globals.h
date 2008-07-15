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


#ifndef _globals_h_
#define _globals_h_

/* ensure that cl.h is included by all source files */
#include "cl.h"

/* standard libraries used by most source files */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <assert.h>
#include <unistd.h>


/* global configuration variables */

extern int cl_debug;		/* 0 = none, 1 = some, 2 = heavy */

extern int cl_optimize;		/* 0 = off, 1 = on  (untested / expensive optimisations) */

extern size_t cl_memory_limit;	/* in megabytes (0 = off); some functions will try to keep to this limit */

/* default registry settings */
#if (!defined(REGISTRY_DEFAULT_PATH))
#define REGISTRY_DEFAULT_PATH  "/corpora/c1/registry"
#endif

#if (!defined(REGISTRY_ENVVAR))
#define REGISTRY_ENVVAR        "CORPUS_REGISTRY"
#endif

/* this is the length of temporary strings which are allocated with a fixed size ... better make it large */
#define MAX_LINE_LENGTH        4096

/* this is the length of fixed-size buffers for names and identifiers */
#define MAX_IDENTIFIER_LENGTH  1024


#define TODO {(void)fprintf(stderr,"TODO point reached: file \"%s\", line %d\n", \
			    __FILE__, \
			    __LINE__); \
			    exit(1);}


/* Make sure that byte order has been appropriately defined. */
#if (defined(CWB_LITTLE_ENDIAN)) && (defined(CWB_BIG_ENDIAN))

#error Only one of the symbols CWB_LITTLE_ENDIAN and CWB_BIG_ENDIAN may be defined!

#endif

#if (!defined(CWB_LITTLE_ENDIAN) && !defined(CWB_BIG_ENDIAN))

#error Byte order not specified. Define either CWB_LITTLE_ENDIAN or CWB_BIG_ENDIAN!

#endif

#endif
