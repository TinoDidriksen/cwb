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


#ifndef _macros_h_
#define _macros_h_

#include "globals.h"


/* safely allocate/reallocate memory:
 *   cl_malloc()
 *   cl_calloc()
 *   cl_realloc()
 *   cl_strdup()
 *   cl_free(p)
 * on out_of_memory, prints error message & aborts program
 */

/* function prototypes and macros now in <cl.h> */


/* memory allocation macros */
#define new(T) (T *)cl_malloc(sizeof(T))
#define New(P,T)  P = (T *)cl_malloc(sizeof(T))


/* be careful: strings are considered equal if they are both NULL,
 * they are considered non-equal when one of both is NULL
 */
#define STREQ(a,b) (((a) == (b)) || \
		    ((a) && (b) && (strcmp((a), (b)) == 0)))

#define MIN(a,b) ((a)<(b) ? (a) : (b))
#define MAX(a,b) ((a)>(b) ? (a) : (b))


/*
 *  display progress bar in terminal window (STDERR, child mode: STDOUT)
 */

void
progress_bar_child_mode(int on_off); /* 1 = simple messages on STDOUT, 0 = pretty-printed messages with carriage returns ON STDERR */

void
progress_bar_clear_line(void);	/* assumes line width of 60 characters */

void
progress_bar_message(int pass, int total, char *message);
/* [pass <pass> of <total>: <message>]   (if total == 0, uses pass and total values from last call) */

void
progress_bar_percentage(int pass, int total, int percentage);
/* [pass <pass> of <total>: <percentage>% complete]  (if total == 0, uses pass and total values from last call) */


/*
 *  print indented 'tabularised' lists
 */

void
start_indented_list(int linewidth, int tabsize, int indent);
/* <tabsize> = tabulator steps; if <linewidth>, <tabsize>, or <indent> is zero, uses internal default */

void
print_indented_list_br(char *label);	/* starts new line (as <br> in HTML) [showing optional label in indentation] */

void
print_indented_list_item(char *string);

void
end_indented_list(void);

/* default configuration of indented lists */
#define ILIST_INDENT      4
#define ILIST_TAB        12
#define ILIST_LINEWIDTH  72  /* total linewidth needed is ILIST_INDENT + ILIST_LINEWIDTH */


#endif
