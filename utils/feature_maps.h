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


#include "../cl/cl.h"


#define MAXBLOCKS 10 

/** Data structure for the vstack member of the FMS object. @see FMS */
typedef struct vstack_t {
  int64_t *fcount;
  struct vstack_t *next;
} vstack_t;


/** Underlying structure for the FMS object. */
typedef struct feature_maps_t {
  Attribute *att1;              /**< word attribute of source corpus */
  Attribute *att2;              /**< word attribute of target corpus */
  Attribute *s1;                /**< sentence regions of source corpus */
  Attribute *s2;                /**< sentence regions of target corpus */
  int64_t n_features;           /**< number of allocated features */
  int64_t **w2f1;               /**< feature map 1 */
  int64_t **w2f2;               /**< feature map 2 */
  int64_t *fweight;             /**< array of feature weights */

  vstack_t *vstack;             /**< a stack (implemented as linked list) of integer vectors,
                                     each containing <n_features> integers. */

} feature_maps_t;

/**
 * The FMS object: contains memory space for a feature map between two attributes,
 * used in aligning corpora.
 *
 * The "feature map" is a very large and complex data structure of all the different features
 * we can look at, together with weights.
 *
 * Basically, it is a "compiled" version of the features defined by the cwb-align configuration
 * flags *AS APPLIED TO THIS SPECIFIC CORPUS* - a massive list of "things to look for"
 * when comparing any two potentially-corresponding regions from a source/target corpus pair.
 */
typedef feature_maps_t *FMS;


FMS create_feature_maps(char **config, int64_t config_lines,
                        Attribute *w_attr1, Attribute *w_attr2,
                        Attribute *s_attr1, Attribute *s_attr2
                        );


int64_t *get_fvector(FMS fms);

void release_fvector(int64_t *fvector, FMS fms);

void check_fvectors(FMS fms);



int64_t feature_match(FMS fms, int64_t f1, int64_t l1, int64_t f2, int64_t l2);


void show_features(FMS fms, int64_t which, char *word);



void best_path(FMS fms,
               int64_t f1, int64_t l1,
               int64_t f2, int64_t l2,
               int64_t beam_width,       /* beam search */
               int64_t verbose,          /* echo progress info on stdout ? */
               /* output */
               int64_t *steps,
               int64_t **out1,
               int64_t **out2,
               int64_t **out_quality);
