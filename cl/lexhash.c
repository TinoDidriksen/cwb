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
#include "lexhash.h"


/* by default, use 250,000 buckets */
#define DEFAULT_NR_OF_BUCKETS 250000

/* update interval for hash performance estimation */
#define PERFORMANCE_COUNT 1000

/* default value for performance limit (avg no of comparisons) before hash is expanded */
#define DEFAULT_PERFORMANCE_LIMIT 10



/*
 * basic utility functions
 */

int 
is_prime(int n) {
  int i;
  for(i = 2; i*i <= n; i++)
    if ((n % i) == 0) 
      return 0;
  return 1;
}

int 
find_prime(int n) {
  for( ; n > 0 ; n++)           /* will exit on int overflow */
    if (is_prime(n)) 
      return n;
  return 0;
}

unsigned int 
hash_string(char *string) {
  unsigned char *s = (unsigned char *)string;
  unsigned int result = 0;
  for( ; *s; s++)
    result = (result * 33 ) ^ (result >> 27) ^ *s;
  return result;
}


/*
 * cl_lexhash / cl_lexhash_entry  object definition
 */


/* cl_lexhash_entry now in <cl.h> */

typedef void (*cl_lexhash_cleanup_func)(cl_lexhash_entry);

/* typedef struct _cl_lexhash *cl_lexhash; in <cl.h> */
struct _cl_lexhash {
  cl_lexhash_entry *table;      /* list of buckets, each one being a pointer to a list of entries */
  unsigned int buckets;         /* number of buckets in list */
  int next_id;                  /* ID that will be assigned to next new entry */
  int entries;                  /* current number of entries in hash */
  cl_lexhash_cleanup_func cleanup_func; /* callback function used when deleting entries (see <cl.h>) */
  int performance_counter;      /* variables used for estimating hash performance (avg no of comparisons) */
  int comparisons;
  double last_performance;
  int auto_grow;                /* whether to expand hash automatically */
};


/*
 * cl_lexhash methods
 */

cl_lexhash 
cl_new_lexhash(int buckets) {
  cl_lexhash hash;
  
  if (buckets <= 0) buckets = DEFAULT_NR_OF_BUCKETS;
  hash = (cl_lexhash) cl_malloc(sizeof(struct _cl_lexhash));
  hash->buckets = find_prime(buckets);
  hash->table = cl_calloc(hash->buckets, sizeof(cl_lexhash_entry));
  hash->next_id = 0;
  hash->entries = 0;
  hash->cleanup_func = NULL;
  hash->performance_counter = PERFORMANCE_COUNT;
  hash->comparisons = 0;
  hash->last_performance = 0.0;
  hash->auto_grow = 1;
  return hash;
}             

/* internal function:
   cl_delete_lexhash_entry(lexhash, entry);
   <deallocate cl_lexhash_entry object and its key string>
   */
void
cl_delete_lexhash_entry(cl_lexhash hash, cl_lexhash_entry entry) {
  if (hash != NULL) {
    /* if necessary, let cleanup callback delete objects associated with the data field */
    if (hash->cleanup_func != NULL) {
      (*(hash->cleanup_func))(entry);
    }
    cl_free(entry->key);
    cl_free(entry);
  }
}

void 
cl_delete_lexhash(cl_lexhash hash) {
  int i;
  cl_lexhash_entry entry, temp;
  
  if (hash != NULL && hash->table != NULL) {
    for (i = 0; i < hash->buckets; i++) {
      entry = hash->table[i];
      while (entry != NULL) {
        temp = entry;
        entry = entry->next;
        cl_delete_lexhash_entry(hash, temp);
      }
    }
  }
  cl_free(hash->table);
  cl_free(hash);
}                        

void
cl_lexhash_set_cleanup_function(cl_lexhash hash, cl_lexhash_cleanup_func func) {
  if (hash != NULL)
    hash->cleanup_func = func;
}

void
cl_lexhash_auto_grow(cl_lexhash hash, int flag) {
  if (hash != NULL)
    hash->auto_grow = flag;
}


/* internal function:
   expanded = cl_lexhash_check_grow(cl_lexhash hash);
   updates performance estimate; if above threshold and auto_grow is enabled, expands hash to avg fill rate of 1 
   [note: this function also implements the hashing algorithm and must be consistent with cl_lexhash_find_i()]
*/
int
cl_lexhash_check_grow(cl_lexhash hash) {
  double fill_rate = ((double) hash->entries) / hash->buckets;
  cl_lexhash temp;
  cl_lexhash_entry entry, next;
  int idx, offset, old_buckets, new_buckets;

  hash->last_performance = ((double) hash->comparisons) / PERFORMANCE_COUNT;
  if (hash->auto_grow && (hash->last_performance > DEFAULT_PERFORMANCE_LIMIT)) {
    if (cl_debug) {
      fprintf(stderr, "[lexhash autogrow: (perf = %3.1f  @ fill rate = %3.1f (%d/%d)]\n",
              hash->last_performance, fill_rate, hash->entries, hash->buckets);
    }
    if (fill_rate < 2.0) {
      if (cl_debug)
        fprintf(stderr, "[autogrow aborted because of low fill rate]\n");
      return 0;
    }
    temp = cl_new_lexhash(hash->entries); /* create new hash with fill rate == 1.0 */
    old_buckets = hash->buckets;
    new_buckets = temp->buckets; /* will be a prime number >= hash->entries */
    /* move all entries from hash to the appropriate bucket in temp */
    for (idx = 0; idx < old_buckets; idx++) {
      entry = hash->table[idx];
      while (entry != NULL) {
        next = entry->next;     /* remember pointer to next entry */
        offset = hash_string(entry->key) % new_buckets;
        entry->next = temp->table[offset]; /* insert entry into its bucket in temp (most buckets should contain only 1 entry) */
        temp->table[offset] = entry;
        temp->entries++;
        entry = next;           /* continue while loop */
      }
    }
    assert((temp->entries == hash->entries) && "lexhash.c: inconsistency during hash expansion");
    cl_free(hash->table);               /* old hash table should be empty and can be deallocated */
    hash->table = temp->table;  /* update hash from temp (copy hash table and its size) */
    hash->buckets = temp->buckets;
    hash->last_performance = 0.0; /* reset performance estimate */
    cl_free(temp);              /* we can simply deallocate temp now, having stolen its hash table */
    if (cl_debug) {
      fill_rate = ((double) hash->entries) / hash->buckets;
      fprintf(stderr, "[grown to %d buckets  @ fill rate = %3.1f (%d/%d)]\n",
              hash->buckets, fill_rate, hash->entries, hash->buckets);
    }
  }
  return 0;
}

/* internal function:
   entry = cl_lexhash_find_i(cl_lexhash hash, char *token, unsigned int *offset);
   same as cl_lexhash_find(), but *offset is set to the hashtable offset computed for token, unless *offset == NULL
   [this function hides the hashing algorithm details from the rest of the lexhash implementation] 
*/
cl_lexhash_entry
cl_lexhash_find_i(cl_lexhash hash, char *token, unsigned int *ret_offset) {
  unsigned int offset;
  cl_lexhash_entry entry;

  assert((hash != NULL && hash->table != NULL && hash->buckets > 0) && "cl_lexhash object was not properly initialised");
  offset = hash_string(token) % hash->buckets;
  if (ret_offset != NULL) *ret_offset = offset;
  entry = hash->table[offset];
  if (entry != NULL) 
    hash->comparisons++;        /* will need one comparison at least */
  while (entry != NULL && strcmp(entry->key, token) != 0) {
    entry = entry->next;
    hash->comparisons++;        /* this counts additional comparisons */
  }
  hash->performance_counter--;
  if (hash->performance_counter <= 0) {
    if (cl_lexhash_check_grow(hash)) 
      entry = cl_lexhash_find_i(hash, token, ret_offset); /* if hash was expanded, need to recompute offset */
    hash->performance_counter = PERFORMANCE_COUNT;
    hash->comparisons = 0;
  }
  return entry;
}

cl_lexhash_entry
cl_lexhash_find(cl_lexhash hash, char *token) {
  return cl_lexhash_find_i(hash, token, NULL);
}

cl_lexhash_entry
cl_lexhash_add(cl_lexhash hash, char *token) {
  cl_lexhash_entry entry, insert_point;
  unsigned int offset;
  
  entry = cl_lexhash_find_i(hash, token, &offset);
  if (entry != NULL) {
    /* token already in hash -> increment frequency count */
    entry->freq++;
    return entry;
  }
  else {
    /* add new entry for this token */
    entry = (cl_lexhash_entry) cl_malloc(sizeof(struct _cl_lexhash_entry));
    entry->key = cl_strdup(token);
    entry->freq = 1;
    entry->id = (hash->next_id)++;
    entry->data.integer = 0;    /* initialise data fields to zero values */
    entry->data.numeric = 0.0;
    entry->data.pointer = NULL;
    entry->next = NULL;
    insert_point = hash->table[offset]; /* insert entry into its bucket in the hash table */
    if (insert_point == NULL) {
      hash->table[offset] = entry; /* only entry in this bucket so far */
    }
    else { /* always insert as last entry in its bucket (because of Zipf's Law:
              frequent lexemes tend to occur early in the corpus and should be first in their buckets for faster access) */
      while (insert_point->next != NULL)
        insert_point = insert_point->next;
      insert_point->next = entry;
    }
    hash->entries++;
    return entry;
  }
}

int 
cl_lexhash_id(cl_lexhash hash, char *token) {
  cl_lexhash_entry entry;

  entry = cl_lexhash_find_i(hash, token, NULL);
  return (entry != NULL) ? entry->id : -1;
} 

int 
cl_lexhash_freq(cl_lexhash hash, char *token) {
  cl_lexhash_entry entry;

  entry = cl_lexhash_find_i(hash, token, NULL);
  return (entry != NULL) ? entry->freq : 0;
} 

int 
cl_lexhash_del(cl_lexhash hash, char *token) {
  cl_lexhash_entry entry, previous;
  unsigned int offset, f;

  entry = cl_lexhash_find_i(hash, token, &offset);
  if (entry == NULL) {
    return 0;                   /* not in lexhash */
  }
  else {
    f = entry->freq;
    if (hash->table[offset] == entry) {
      hash->table[offset] = entry->next;
    }
    else {
      previous = hash->table[offset];
      while (previous->next != entry)
        previous = previous->next;
      previous->next = entry->next;
    }
    cl_delete_lexhash_entry(hash, entry);
    hash->entries--;
    return f;
  }
}

int 
cl_lexhash_size(cl_lexhash hash) {
  cl_lexhash_entry entry;
  int i, size = 0;

  assert((hash != NULL && hash->table != NULL && hash->buckets > 0) && "cl_lexhash object was not properly initialised");
  for (i = 0; i < hash->buckets; i++) {
    entry = hash->table[i];
    while (entry != NULL) {
      size++;
      entry = entry->next;
    }
  }

  return size;
}
