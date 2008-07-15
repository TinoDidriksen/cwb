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

#ifndef __storage_h
#define __storage_h

#include <sys/types.h>

#include "globals.h"


/* data allocation methods */
#define UNALLOCATED 0
#define MMAPPED  1
#define MALLOCED 2
#define PAGED    3


#define SIZE_BIT   0
#define SIZE_BYTE  sizeof(char)
#define SIZE_SHINT sizeof(short)
#define SIZE_INT   sizeof(int)
#define SIZE_LONG  sizeof(long)

typedef struct TMblob {
  size_t size;			/* the number of allocated bytes */

  int item_size;		/* the size of one item */
  unsigned int nr_items;	/* the number of items represented */

  int *data;			/* pointer to the data */
  int allocation_method;	/* the allocation method */

  int writeable;		/* can we write to the data? */
  int changed;			/* needs update? (not yet in use) */

  /* fields for paged memory -- not yet used */
  char *fname;
  off_t fsize, offset;
} MemBlob;

/* ---------------------------------------------------------------------- */

/* same as fwrite(&val, sizeof(int), 1, fd), but converts to network
 * byte order. */

void NwriteInt(int val, FILE *fd);
void NreadInt(int *val, FILE *fd);

/* same as fwrite(vals, sizeof(int), nr_vals, fd), but converts to
 * network byte order. */

void NwriteInts(int *vals, int nr_vals, FILE *fd);
void NreadInts(int *vals, int nr_vals, FILE *fd);


/* ---------------------------------------------------------------------- */

/* mfree:
 * free the memory used by the blob, regardless of its allocation
 * method
 */

void mfree(MemBlob *blob);

/* init_mblob:
 * clears all fields, regardless of their usage 
 */

void init_mblob(MemBlob *blob);

/* alloc_mblob:
 * allocate blob holding nr_items of size item_size in memory;
 * if clear_blob is set, data space will be initialised to zero bytes;
 * returns 1 if OK, 0 on error
 */
int
alloc_mblob(MemBlob *blob, int nr_items, int item_size, int clear_blob);


/* ================================================================ FILE IO */

/* read_file_into_blob:
 * read contents of file "filename" into memory represented by blob.
 * you can choose the allocation method - MMAPPED is faster, but 
 * writeable areas of memory should be taken with care. MALLOCED is
 * slower (and far more space consuming), but writing data into malloced
 * memory is no problem. mode is either "r" or "w", depending on whether
 * the memory should be written to.
 * item_size is used for the access methods below, it is simply copied into
 * the MemBlob data structure.
 * blob must not be in use -- the fields are overwritten.
 * returns 0 on failure, 1 if everything went fine.
 */

int read_file_into_blob(char *filename, 
			int allocation_method, 
			int item_size,
			MemBlob *blob);


/* write_file_from_blob:
 * guess what it does -- it writes the data stored in blob into the file
 * "filename". 1 if ok, 0 if fail.
 * if convert_to_nbo is 1, the data is converted to network byte order
 * before it's written.
 */

int write_file_from_blob(char *filename, 
			 MemBlob *blob,
			 int convert_to_nbo);

/* ======================================================= ACCESS FUNCTIONS */

/* not yet implemented */

/* ==================================================== LOW LEVEL FUNCTIONS */

/* return the contents of file in filename as a pointer to a memory area.
 * len_ptr (return value) is the number of bytes the pointer points to.
 * "mode" is either "r" or "w", nothing else. If mode is "r", len_ptr is
 * taken as an input parameter (*len_ptr bytes are allocated)
 */

caddr_t mmapfile(char *filename, size_t *len_ptr, char *mode);

/* mallocfile:
 * does virtually the same as mmapfile (same parameters, same return
 * value), but the memory is taken with malloc(3), not with mmap(2).
 */

caddr_t mallocfile(char *filename, size_t *len_ptr, char *mode);

#endif
