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

#ifndef _list_h_
#define _list_h_

#include "cl.h"

/* just defined the structs here; <cl.h> contains object definitions:
   typedef struct _cl_int_list    *cl_int_list;
   typedef struct _cl_string_list *cl_string_list; */
struct _cl_int_list {
  int size;			/* number of elements */
  int allocated;		/* number of elements, for which space has been allocated */
  int lumpsize;			/* lump size by which list is reallocated */
  int *data;
};

struct _cl_string_list {
  int size;
  int allocated;
  int lumpsize;
  char **data;
};

/* function prototypes now in <cl.h> */

#endif
