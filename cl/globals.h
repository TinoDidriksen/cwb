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

extern int cl_debug;
extern int cl_optimize;
extern size_t cl_memory_limit;


/* macros for path-handling: different between Unix and Windows */
/*
 * NOTE:
 * When we move to Glib, it will be better to use G_DIR_SEPARATOR and G_SEARCHPATH_SEPARATOR
 * and delete these two macros.
 */
#ifndef __MINGW__
/** character used to separate different paths in a string variable */
#define PATH_SEPARATOR ':'
/** character used to delimit subdirectories in a path */
#define SUBDIR_SEPARATOR '/'
/** name of directory for temporary files (as string, absolute path) */
#define TEMPDIR_PATH "/tmp"
#else
#define PATH_SEPARATOR ';'
#define SUBDIR_SEPARATOR '\\'
#define TEMPDIR_PATH "." /* A CQP user may not have access to C:\Temp, which is where they SHOULD fo */
#endif
/**
 * size in bytes of string buffers capable of holding absolute paths
 * of temporary filenames; needs to be big enough for TEMPDIR_PATH plus
 * the result of a call to tempnam() plus the length of a process ID, at least.
 */
#define TEMP_FILENAME_BUFSIZE 128


/* default registry settings */
#if (!defined(REGISTRY_DEFAULT_PATH))
#ifndef __MINGW__
/** The default path assumed for the location of the corpus registry. */
#define REGISTRY_DEFAULT_PATH  "/corpora/c1/registry"
#else
/* note that the notion of a default path under Windows is fundamentally dodgy ... */
#define REGISTRY_DEFAULT_PATH  "C:\CWB\Registry"
#endif
#endif

#if (!defined(REGISTRY_ENVVAR))
/** The Unix environment variable from which the value of the registry will be taken. */
#define REGISTRY_ENVVAR        "CORPUS_REGISTRY"
#endif

/**
 *  this is the length of temporary strings which are allocated with a fixed size ... better make it large
 */
#define MAX_LINE_LENGTH        4096

/**
 *  this is the length of fixed-size buffers for names and identifiers
 */
#define MAX_IDENTIFIER_LENGTH  1024

/**
 * Macro which exits the program when a "to do" point is hit.
 */
#define TODO {(void)fprintf(stderr,"TODO point reached: file \"%s\", line %d\n", \
			    __FILE__, \
			    __LINE__); \
			    exit(1);}


#endif
