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

#ifndef _fileutils_h_
#define _fileutils_h_

#include <sys/types.h>

#include "globals.h"

off_t file_length(char *filename);
off_t fd_file_length(FILE *fd);
off_t fi_file_length(int64_t fileno);

int64_t fprobe(char *fname);

int64_t is_directory(char *path);
int64_t is_file(char *path);
int64_t is_link(char *path);

/* data structure for managing I/O streams */
typedef struct _CLStream *CLStream;
struct _CLStream {
  FILE *handle;
  int64_t mode; /* not really needed */
  int64_t type; /* the specified or guessed stream type */
  CLStream next;
};

/* flags and function prototypes are declared in <cl.h> */

#endif
