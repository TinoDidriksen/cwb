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

#include <time.h>

/*
 * memory allocation functions with integrate success test 
 * (this functions will be used as hooks for the CL MMU)
 */

void *
cl_malloc(size_t bytes) {
  void *block;

  block = malloc(bytes);
  if (block == NULL) {
    fprintf(stderr, "CL: Out of memory. (killed)\n");
    fprintf(stderr, "CL: [cl_malloc(%d)]\n", bytes);
    printf("\n");		/* for CQP's child mode */
    exit(1);
  }
  return block;
}

void *
cl_calloc(size_t nr_of_elements, size_t element_size) {
  void *block;

  block = calloc(nr_of_elements, element_size);
  if (block == NULL) {
    fprintf(stderr, "CL: Out of memory. (killed)\n");
    fprintf(stderr, "CL: [cl_calloc(%d*%d bytes)]\n", nr_of_elements, element_size);
    printf("\n");		/* for CQP's child mode */
    exit(1);
  }
  return block;
}

void *
cl_realloc(void *block, size_t bytes) {
  void *new_block;

  if (block == NULL) 
    new_block = malloc(bytes);	/* some OSs don't fall back to malloc() if block == NULL */
  else
    new_block = realloc(block, bytes);

  if (new_block == NULL) {
    if (bytes == 0) {
      /* don't warn any more, reallocating to 0 bytes should create no problems, at least on Linux and Solaris */
      /* (the message was probably shown on Linux only, because Solaris doesn't return NULL in this case) */
      /* fprintf(stderr, "CL: WARNING realloc() to 0 bytes!\n"); */      
    }
    else {
      fprintf(stderr, "CL: Out of memory. (killed)\n");
      fprintf(stderr, "CL: [cl_realloc(block at %p to %d bytes)]\n", block, bytes);
      printf("\n");		/* for CQP's child mode */
      exit(1);
    }
  }
  return new_block;
}

char *
cl_strdup(char *string) {
  char *new_string;

  new_string = strdup(string);
  if (new_string == NULL) {
    fprintf(stderr, "CL: Out of memory. (killed)\n");
    fprintf(stderr, "CL: [cl_strdup(addr=%p, len=%d)]\n", string, strlen(string));
    printf("\n");		/* for CQP's child mode */
    exit(1);
  }
  return new_string;
}


/*
 * built-in random number generator (avoid dependence on quality of system's rand() function)
 */

/* this random number generator is a version of Marsaglia-multicarry which is one of the RNGs used by R */

static unsigned int RNG_I1=1234, RNG_I2=5678;

void
cl_set_rng_state(unsigned int i1, unsigned int i2) {
  RNG_I1 = (i1) ? i1 : 1; 	/* avoid zero values as seeds */
  RNG_I2 = (i2) ? i2 : 1;
}

void 
cl_get_rng_state(unsigned int *i1, unsigned int *i2) {
  *i1 = RNG_I1; 
  *i2 = RNG_I2;
}

/* initialise RNG from single 32bit number as seed */
void
cl_set_seed(unsigned int seed) {
  cl_set_rng_state(seed, 69069 * seed + 1); /* this is the way that R does it */
}

/* initialise RNG from current system time */
void
cl_randomize(void) {
  cl_set_seed(time(NULL));
}

/* returns unsigned 32-bit integer with uniform distribution */
unsigned int
cl_random(void) {
  RNG_I1 = 36969*(RNG_I1 & 0177777) + (RNG_I1 >> 16);
  RNG_I2 = 18000*(RNG_I2 & 0177777) + (RNG_I2 >> 16);
  return((RNG_I1 << 16) ^ (RNG_I2 & 0177777));
}

/* returns random number in the range [0,1] with uniform distribution */
double 
cl_runif(void) {
  return cl_random() * 2.328306437080797e-10; /* = cl_random / (2^32 - 1) */
}


/*
 *  display progress bar in terminal window (STDOUT) 
 */

int progress_bar_pass = 1;
int progress_bar_total = 1;
int progress_bar_simple = 0;

void
progress_bar_child_mode(int on_off) {
  progress_bar_simple = progress_bar_child_mode;
}

void
progress_bar_clear_line(void) {	/* assumes line width of 60 characters */
  if (progress_bar_simple) {
    /* messages are on separated lines, so do nothing here */
  }
  else {
    fprintf(stderr, "                                                            \r");
    fflush(stderr);
  }
}

void
progress_bar_message(int pass, int total, char *message) {
  /* [pass <pass> of <total>: <message>]   (uses pass and total values from last call if total == 0)*/
  if (total <= 0) {
    pass = progress_bar_pass;
    total = progress_bar_total;
  }
  else {
    progress_bar_pass = pass;
    progress_bar_total = total;
  }
  if (progress_bar_simple) {
    fprintf(stdout, "-::-PROGRESS-::-\t%d\t%d\t%s\n", pass, total, message);
    fflush(stdout);
  }
  else {
    fprintf(stderr, "[");
    fprintf(stderr, "pass %d of %d: ", pass, total);
    fprintf(stderr, "%s]     \r", message);
    fflush(stderr);
  }
}

void
progress_bar_percentage(int pass, int total, int percentage) {
  /* [pass <pass> of <total>: <percentage>% complete]  (uses progress_bar_message) */
  char message[20];
  sprintf(message, "%3d%c complete", percentage, '%');
  progress_bar_message(pass, total, message);
}


/*
 *  print indented 'tabularised' lists
 */

/* status variables */
int ilist_cursor;		/* the 'cursor' (column where next item will be printed) */
int ilist_linewidth;		/* so start_indented_list() can override default config */
int ilist_tab;			/* ... */
int ilist_indent;		/* ... */

/* internal function: print <n> blanks */
void
ilist_print_blanks(int n) {
  while (n > 0) {
    printf(" ");
    n--;
  }
}

void
start_indented_list(int linewidth, int tabsize, int indent) {
  ilist_linewidth = (linewidth > 0) ? linewidth : ILIST_LINEWIDTH;
  ilist_tab = (tabsize > 0) ? tabsize : ILIST_TAB;
  ilist_indent = (indent > 0) ? indent : ILIST_INDENT;
  ilist_cursor = 0;
  ilist_print_blanks(ilist_indent);
} 

void
print_indented_list_br(char *label) {
  int llen = (label != NULL) ? strlen(label) : 0;
  
  if (ilist_cursor != 0) {
    printf("\n");
  }
  else {
    printf("\r");
  }
  if (llen <= 0) {
    ilist_print_blanks(ilist_indent);
  }
  else {
    printf("%s", label);
    ilist_print_blanks(ilist_indent - llen);
  }
  ilist_cursor = 0;
}

void
print_indented_list_item(char *string) {
  int len;

  if (string != NULL) {
    len = strlen(string);
    if ((ilist_cursor + len) > ilist_linewidth) {
      print_indented_list_br("");
    }
    printf("%s", string);
    ilist_cursor += len;
    /* advance cursor to next tabstop */
    if (ilist_cursor < ilist_linewidth) {
      printf(" ");
      ilist_cursor++;
    }
    while ((ilist_cursor < ilist_linewidth) && ((ilist_cursor % ilist_tab) != 0)) {
      printf(" ");
      ilist_cursor++;
    }
  }
}

void
end_indented_list(void) {
  if (ilist_cursor == 0) {
    printf("\r");		/* no output on last line (just indention) -> erase indention */
  }
  else {
    printf("\n");
  }
  ilist_cursor = 0;
  fflush(stdout);
}

