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

#ifndef _makecomps_h_
#define _makecomps_h_

#include "globals.h"

#include "attributes.h"

/* note: these functions aren't guaranteed to load the component after creating it! */
/* (in fact, creat_rev_corpus() would run out of address space if it tried that on a large corpus) */

/* create a sort index */
int creat_sort_lexicon(Component *lexsrt);

/* create frequency table */
int creat_freqs(Component *lex);

/* create index for reversed corpus */
int creat_rev_corpus_idx(Component *component);

/* create reversed corpus */
int creat_rev_corpus(Component *component);

#endif
