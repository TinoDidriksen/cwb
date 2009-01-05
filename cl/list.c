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

#include "cl.h"
#include "macros.h"

#include "list.h"


/* automatically extends lists in steps of at least LUMPSIZE cells (to avoid too frequent reallocs) */
/* (can be configured with cl_int_list_lumpsize() and cl_string_list_lumpsize()) */
#define LUMPSIZE 64


/* ===================================================================  INT LIST FUNCTIONS */

cl_int_list 
cl_new_int_list(void) {
  cl_int_list l = cl_malloc(sizeof(struct _cl_int_list));
  l->data = cl_calloc(LUMPSIZE, sizeof(int));
  l->allocated = LUMPSIZE;
  l->size = 0;
  l->lumpsize = LUMPSIZE;
  return l;
}

void 
cl_delete_int_list(cl_int_list l) {
  cl_free(l->data);
  cl_free(l);
}

void
cl_int_list_lumpsize(cl_int_list l, int s) {
  if (s >= LUMPSIZE) {          /* lumpsize may not be smaller than default */
    l->lumpsize = s;
  }
}

int
cl_int_list_size(cl_int_list l) {
  return l->size;
}

int 
cl_int_list_get(cl_int_list l, int n) {
  if (n < 0 || n >= l->size) {
    return 0;
  }
  else {
    return l->data[n];
  }
}

void 
cl_int_list_set(cl_int_list l, int n, int val) {
  int newalloc, i;
  
  if (n < 0) {
    return;
  }
  else {
    if (n >= l->size) {
      l->size = n+1;
      /* auto-extend list if necessary */
      if (l->size > l->allocated) {
        newalloc = l->size;
        if ((newalloc - l->allocated) < l->lumpsize) {
          newalloc = l->allocated + l->lumpsize;
        }
        l->data = cl_realloc(l->data, newalloc * sizeof(int));
        for (i = l->allocated; i < newalloc; i++) {
          l->data[i] = 0;
        }
        l->allocated = newalloc;
      }
    }
    /* now we can safely set the desired list element */
    l->data[n] = val;
  }
}

void 
cl_int_list_append(cl_int_list l, int val) {
  cl_int_list_set(l, l->size, val);
}

/* comparison function for int list sort */
int
cl_int_list_intcmp(const void *a, const void *b) {
  return (*(int *)a - *(int *)b);
}

void
cl_int_list_qsort(cl_int_list l) {
  qsort(l->data, l->size, sizeof(int), cl_int_list_intcmp);
}


/* ===================================================================  STRING LIST FUNCTIONS */

cl_string_list 
cl_new_string_list(void) {
  cl_string_list l = cl_malloc(sizeof(struct _cl_string_list));
  l->data = cl_calloc(LUMPSIZE, sizeof(char *));
  l->allocated = LUMPSIZE;
  l->size = 0;
  l->lumpsize = LUMPSIZE;
  return l;
}

void 
cl_delete_string_list(cl_string_list l) {
  cl_free(l->data);
  cl_free(l);
}

void
cl_free_string_list(cl_string_list l) {
  int i;

  for (i = 0; i < l->size; i++) {
    cl_free(l->data[i]);                /* cl_free() checks if pointer is NULL */
  }
}

void
cl_string_list_lumpsize(cl_string_list l, int s) {
  if (s >= LUMPSIZE) {          /* lumpsize may not be smaller than default */
    l->lumpsize = s;
  }
}

int
cl_string_list_size(cl_string_list l) {
  return l->size;
}

char *
cl_string_list_get(cl_string_list l, int n) {
  if (n < 0 || n >= l->size) {
    return NULL;
  }
  else {
    return l->data[n];
  }
}

void 
cl_string_list_set(cl_string_list l, int n, char *val) {
  int newalloc, i;
  
  if (n < 0) {
    return;
  }
  else {
    if (n >= l->size) {
      l->size = n+1;
      /* auto-extend list if necessary */
      if (l->size > l->allocated) {
        newalloc = l->size;
        if ((newalloc - l->allocated) < l->lumpsize) {
          newalloc = l->allocated + l->lumpsize;
      }
        l->data = cl_realloc(l->data, newalloc * sizeof(char *));
        for (i = l->allocated; i < newalloc; i++) {
          l->data[i] = NULL;
        }
        l->allocated = newalloc;
      }
    }
    /* now we can safely set the desired list element */
    l->data[n] = val;
  }
}

void 
cl_string_list_append(cl_string_list l, char *val) {
  cl_string_list_set(l, l->size, val);
}

/* comparison function for string list sort */
int
cl_string_list_strcmp(const void *a, const void *b) {
  return cl_strcmp(*(char **)a, *(char **)b); /*, I think. */
}

void
cl_string_list_qsort(cl_string_list l) {
  qsort(l->data, l->size, sizeof(char *), cl_string_list_strcmp);
}

