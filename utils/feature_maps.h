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

typedef struct vstack_t {
  int *fcount;
  struct vstack_t *next;
} vstack_t;


typedef struct feature_maps_t {
  Attribute *att1;		/* word attribute of source corpus */
  Attribute *att2;		/* word attribute of target corpus */
  Attribute *s1;		/* sentence regions of source corpus */
  Attribute *s2;		/* sentence regions of target corpus */
  int n_features;		/* number of allocated features */
  int **w2f1;			/* feature maps */
  int **w2f2;
  int *fweight;			/* feature weights */

  vstack_t *vstack;

} feature_maps_t;

typedef feature_maps_t *FMS;	/* FMS : feature map handle */


/* FMS = create_feature_maps(config_data, nr_of_config_lines, source_word, target_word, source_s, target_s); 

Create feature maps for a source/target corpus pair. <config_data> points to a list of strings 
representing the feature map configuration. <nr_of_config_lines> is the number of configuration
items stored in <config_data>.
*/
FMS
create_feature_maps(char **config, int config_lines,		    
		    Attribute *w_attr1, Attribute *w_attr2,
		    Attribute *s_attr1, Attribute *s_attr2
		    );

/* feature count vector handling
   (used internally by feature_match) 
   */ 
int *
get_fvector(FMS fms);

void
release_fvector(int *fvector, FMS fms);

void
check_fvectors(FMS fms);


/* Sim = feature_match(FMS, source_first, source_last, target_first, target_last); 

compute similarity measure for source and target regions, where *_first and *_last
specify the index of the first and last sentence in a region 
*/
int
feature_match(FMS fms, 
	      int f1, int l1, int f2, int l2);

/* show_feature(FMS, 1/2, "word");

print all features listed in FMS for the token "word"; "word" is looked up in the
source corpus if the 2nd argument == 1, and in the target corpus otherwise
*/
void
show_features(FMS fms, int which, char *word);

/* best_path(FMS, f1, l1, f2, l2, beam_width, 0/1, 
             &steps, &out1, &out2, &out_quality);

Find the best alignment path for the given regions of sentences in source and
target corpus. Allocates return vectors <out1> and <out2> which contain <steps>
alignment points each. Alignment points are given as sentence numbers and 
correspond to the start points of the sentences. at the end-of-region alignment
point, sentence numbers will be <l1> + 1 and <l2> + 1, which must be considered by
the caller if <l1> (or <l2>) is the last sentence in the corpus!
The similarity measures of aligned regions are returned in the vector <out_quality>.

NB memory allocation of the output vectors is handled internally. <out1>, <out2> and 
<out_quality> must not be dealloctaed by the caller. calling best_path() will overwrite
the results of the previous search.
*/

void
best_path(FMS fms,
	  int f1, int l1,
	  int f2, int l2,
	  int beam_width,	/* beam search */
	  int verbose,		/* echo progress info on stdout ? */
	  /* output */
	  int *steps,
	  int **out1,
	  int **out2,
	  int **out_quality);



