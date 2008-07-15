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

#include "macros.h"

#include "binsert.h"

#define REALLOC_THRESHOLD 16


#include <string.h>

/* DEFINITIONS BELOW SHOULD NOT BE NECESSARY ON MODERN UNIX SYSTEMS */
/* #else /\* not __svr4__ *\/ */
/* #define memmove(dest,src,bytes) bcopy((char *)src, (char *)dest, (size_t) bytes) */
/* #extern void bcopy(char *b1, char *b2, int length); */
/* #endif /\* ifdef __svr4__ *\/ */


void *binsert_g(const void *key,
		void **base,
		size_t *nel,
		size_t size,
		int (*compar)(const  void  *,  const  void *))
{
  int low, high, found, mid, comp;
  

  if (*base == NULL) {

    *base = (void *)cl_malloc(size * REALLOC_THRESHOLD);

    memmove(*base, key, size);
    *nel = 1;

    return *base;		/* address of element */

  }
  else {

    low = 0;
    high = *nel - 1;
    found = 0;
    mid = 0;
    comp = 0;
    
    while (low <= high && !found) {
      
      mid = (low + high)/2;
      
      comp = (*compar)(*base + (mid * size), key);
      
      if (comp < 0)
	low = mid + 1;
      else if (comp > 0)
	high = mid - 1;
      else
	found = 1;
    }

    if (found)
      return *base + (mid * size); /* address of element */
    else {
      
      int ins_pos;

      if (comp < 0)
	ins_pos = mid + 1;
      else
	ins_pos = mid;

      if (*nel % REALLOC_THRESHOLD == 0) {

	(*base) = (void *)cl_realloc(*base,
				  size * (*nel +
					  REALLOC_THRESHOLD));
      }

      /* shift the elements from the insertion position to the right */

      if (ins_pos < *nel)

	memmove(*base + ((ins_pos+1) * size),
		*base + ((ins_pos)   * size),
		(*nel - ins_pos) * size);

      memmove(*base + (ins_pos * size), key, size);
      *nel = *nel + 1;

      return *base + (ins_pos * size); /* address of new element */
    }
  }
}

/* call:

    (void) binsert_g(&nr,
		     (void **)&Table,
		     &Nr_Elements,
		     sizeof(int),
		     intcompare);

*/
