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

#include <stdarg.h>
#include <math.h>
#include <ctype.h>
#include <sys/types.h>

#include "globals.h"

#include "endian.h"
#include <regex.h>
#include "macros.h"
#include "attributes.h"
#include "special-chars.h"
#include "bitio.h"
#include "compression.h"
#include "regopt.h"

#include "cdaccess.h"

#define COMPRESS_DEBUG 0

int cderrno;                    /* is set after access */

/* ---------------------------------------------------------------------- */

#define check_arg(arg,atyp,rval) \
if (arg == NULL) { \
  cderrno = CDA_ENULLATT; return rval; \
} \
else if (arg->type != atyp) { \
  cderrno = CDA_EATTTYPE; return rval; \
}

int cl_strcmp(char *s1, char *s2)
{
  /* BIG BIG BIG warning:
     although it says unsigned_char_strcmp it really operates
     on SIGNED char strings (and it didn't even say that explicitly, so the 
     whole thing broke down on 'unsigned char'-machines) !!!  */

  register signed char* c1;
  register signed char* c2;

  c1 = (signed char *)s1; c2 = (signed char *)s2;

  for ( ; *c1 == *c2; c1++,c2++)
    if ( *c1 == '\0')
      return 0;
  
  return *c1 - *c2;
}

char *cdperror_string(int errno)
{
  char *s;

  switch (errno) {
  case CDA_OK:
    s = "CL: No error";
    break;
  case CDA_ENULLATT:
    s = "CL: NULL passed as attribute argument of function";
    break;
  case CDA_EATTTYPE:
    s = "CL: function called with illegal attribute type";
    break;
  case CDA_EIDORNG:
    s = "CL: id is out of range";
    break;
  case CDA_EPOSORNG:
    s = "CL: position is out of range";
    break;
  case CDA_EIDXORNG:
    s = "CL: index is out of range";
    break;
  case CDA_ENOSTRING:
    s = "CL: no such string encoded";
    break;
  case CDA_EPATTERN:
    s = "CL: illegal regular expression/illegal pattern";
    break;
  case CDA_ESTRUC:
    s = "CL: no structure defined for this position";
    break;
  case CDA_EALIGN:
    s = "CL: no alignent defined for this position";
    break;
  case CDA_EREMOTE:
    s = "CL: error during access of remote data";
    break;
  case CDA_ENODATA:
    s = "CL: can't load and/or create necessary data";
    break;
  case CDA_EARGS:
    s = "CL: error in arguments of dynamic call";
    break;
  case CDA_ENOMEM:
    s = "CL: not enough memory";
    break;
  case CDA_EOTHER:
    s = "CL: unspecified error";
    break;
  case CDA_ENYI:
    s = "CL: unimplemented feature/not yet implemented";
    break;
  case CDA_EBADREGEX:
    s = "CL: bad regular expression";
    break;
  case CDA_EFSETINV:
    s = "CL: invalid feature set (syntax error)";
    break;
  case CDA_EBUFFER:
    s = "CL: internal buffer overflow";
    break;
  case CDA_EINTERNAL:
    s = "CL: internal data inconsistency";
    break;
  default:
    s = "CL: ILLEGAL ERROR NUMBER";
    break;
  }
  return s;
}

void cdperror(char *message)
{
  if (message != NULL)
    fprintf(stderr, "%s: %s\n", cdperror_string(cderrno), message);
  else
    fprintf(stderr, "%s\n", cdperror_string(cderrno));
}

/* ================================================== POSITIONAL ATTRIBUTES */

/* ==================== the mapping between strings and their ids */


char *get_string_of_id(Attribute *attribute, int id)
{
  Component *lex;
  Component *lexidx;

  check_arg(attribute, ATT_POS, NULL);

  lex = ensure_component(attribute, CompLexicon, 0);
  lexidx = ensure_component(attribute, CompLexiconIdx, 0);
  
  if ((lex == NULL) || (lexidx == NULL)) {
    cderrno = CDA_ENODATA;
    return NULL;
  }
  else if ((id < 0) || (id >= lexidx->size)) {
    cderrno = CDA_EIDORNG;
    return NULL;
  }
  else {
    cderrno = CDA_OK;
    return ((char *)lex->data.data + ntohl(lexidx->data.data[id]));
  }

  assert("Not reached" && 0);
  return NULL;
}


int get_id_of_string(Attribute *attribute, char *id_string)
{
  int low, high, nr, mid, comp;

  Component *lexidx;
  Component *lexsrt;
  Component *lex;

  char *str2;


  check_arg(attribute, ATT_POS, cderrno);

  lexidx = ensure_component(attribute, CompLexiconIdx, 0);
  lexsrt = ensure_component(attribute, CompLexiconSrt, 0);
  lex =  ensure_component(attribute, CompLexicon, 0);
  
  if ((lexidx == NULL) || (lexsrt == NULL) || (lex == NULL)) {
    cderrno = CDA_ENODATA;
    return CDA_ENODATA;
  }
  else {
    low = 0;
    high = lexidx->size;
    
    /*  simple binary search */
    for(nr = 0; ; nr++) {
      
      if (nr >= 1000000) {
        fprintf(stderr, "get_id_of_string: too many comparisons with %s\n", 
                id_string);
        cderrno = CDA_EOTHER;
        return CDA_EOTHER;
      }
      
      mid = low + (high - low)/2;
      
      str2 = (char *)(lex->data.data) + 
        ntohl(lexidx->data.data[ntohl(lexsrt->data.data[mid])]);
      
      comp = cl_strcmp(id_string, str2);
      
      if(comp == 0) { 
        cderrno = CDA_OK; 
        return ntohl(lexsrt->data.data[mid]); 
      }
      if(mid == low) { 
        cderrno = CDA_ENOSTRING; 
        return cderrno; 
      }
      if(comp > 0) low = mid;
      else high = mid;
    }
  }

  assert("Not reached" && 0);
  return 0;
}


int get_id_string_len(Attribute *attribute, int id)
{
  Component *lexidx;
  char *s;

  check_arg(attribute, ATT_POS, cderrno);

  lexidx = ensure_component(attribute, CompLexiconIdx, 0);
  
  if (lexidx == NULL) {
    cderrno = CDA_ENODATA;
    return CDA_ENODATA;
  }
  else if ((id < 0) || (id >= lexidx->size)) {
    cderrno = CDA_EIDORNG;
    return CDA_EIDORNG;
  }
  else {
    if ((id + 1) == lexidx->size) {
      
      /* last word */
      s = get_string_of_id(attribute, id);
      
      if (s != NULL) {
        cderrno = CDA_OK;
        return strlen(s);
      }
      else if (cderrno != CDA_OK)
        return cderrno;
      else
        return CDA_EOTHER;
    }
    else {
      cderrno = CDA_OK;
      return (ntohl(lexidx->data.data[id+1]) - 
              ntohl(lexidx->data.data[id])) - 1;
    }
  }

  assert("Not reached" && 0);
  return 0;
}


int get_id_from_sortidx(Attribute *attribute, int sort_index_position)
{
  Component *srtidx;

  check_arg(attribute, ATT_POS, cderrno);

  srtidx = ensure_component(attribute, CompLexiconSrt, 0);
  
  if (srtidx == NULL) {
    cderrno = CDA_ENODATA;
    return CDA_ENODATA;
  }
  
  if ((sort_index_position >=0) && (sort_index_position < srtidx->size)) {
    cderrno = CDA_OK;
    return ntohl(srtidx->data.data[sort_index_position]);
  }
  else {
    cderrno = CDA_EIDXORNG;
    return CDA_EIDXORNG;
  }

  assert("Not reached" && 0);
  return 0;
}

int get_sortidxpos_of_id(Attribute *attribute, int id)
{
  Component *srtidx;

  check_arg(attribute, ATT_POS, cderrno);

  srtidx = ensure_component(attribute, CompLexiconSrt, 0);
  
  if (srtidx == NULL) {
    cderrno = CDA_ENODATA;
    return CDA_ENODATA;
  }
  
  if ((id >=0) && (id < srtidx->size)) {
    cderrno = CDA_OK;
    
    cderrno = CDA_ENYI;
    return CDA_ENYI;
  }
  
  assert("Not reached" && 0);
  return 0;
}


/* ==================== information about the corpus */

int item_sequence_is_compressed(Attribute *attribute)
{
  ComponentState state;

  check_arg(attribute, ATT_POS, cderrno);

  /* The item sequence is compressed iff all three components
   * (CompHuffSeq, CompHuffCodes, CompHuffSync) are loaded or unloaded
   * OR when the code description block is already initialized */

  /* further, the CompCorpus component shouldn't be in memory. This is
   * a trick, so that uncompressed access can be enforced if the
   * CompCorpus is ensure'd. */


  if (attribute->pos.hc != NULL)
    return 1;
  else {

    /* if CompCorpus is in memory, we do not access the compressed 
     * sequence.
     */

    state = component_state(attribute, CompCorpus); 
    if (state == ComponentLoaded)
      return 0;
    
    state = component_state(attribute, CompHuffSeq);
    if (!(state == ComponentLoaded || state == ComponentUnloaded))
      return 0;

    state = component_state(attribute, CompHuffCodes);
    if (!(state == ComponentLoaded || state == ComponentUnloaded))
      return 0;

    state = component_state(attribute, CompHuffSync);
    if (!(state == ComponentLoaded || state == ComponentUnloaded))
      return 0;

    return 1;
  }
}

int inverted_file_is_compressed(Attribute *attribute)
{
  ComponentState state;

  check_arg(attribute, ATT_POS, cderrno);

  /* The inverted file is compressed iff both two components
   * (CompCompRF, CompCompRFX) are loaded or unloaded */

  /* as above: when CompRevCorpus and CompRevCorpusIdx are already
   * in memory, we do not use the compressed inverted file.
   */

  if ((component_state(attribute, CompRevCorpus) == ComponentLoaded) &&
      (component_state(attribute, CompRevCorpusIdx) == ComponentLoaded))
    return 0;
  
  state = component_state(attribute, CompCompRF);
  if (!(state == ComponentLoaded || state == ComponentUnloaded))
    return 0;
  
  state = component_state(attribute, CompCompRFX);
  if (!(state == ComponentLoaded || state == ComponentUnloaded))
    return 0;
  
  return 1;
}

/* ------------------------------------------------------------ */

int get_attribute_size(Attribute *attribute)
{
  Component *corpus;

  check_arg(attribute, ATT_POS, cderrno);

  if (item_sequence_is_compressed(attribute) == 1) {

    ensure_component(attribute, CompHuffCodes, 0);
    if (attribute->pos.hc == NULL) {
      cderrno = CDA_ENODATA;
      return CDA_ENODATA;
    }
    cderrno = CDA_OK;

    return attribute->pos.hc->length;

  }
  else {

    corpus = ensure_component(attribute, CompCorpus, 0);

    if (corpus == NULL) {
      cderrno = CDA_ENODATA;
      return CDA_ENODATA;
    }
    else {
      cderrno = CDA_OK;
      return corpus->size;
    }
  }

  assert("Not reached" && 0);
  return 0;
}


int get_id_range(Attribute *attribute)
{
  Component *comp;

  check_arg(attribute, ATT_POS, cderrno);

  comp = ensure_component(attribute, CompLexiconIdx, 0);
    
  if (comp == NULL) {
    cderrno = CDA_ENODATA;
    return CDA_ENODATA;
  }
  else {
    cderrno = CDA_OK;
    return comp->size;
  }

  assert("Not reached" && 0);
  return 0;
}



/* ==================== the relation between ids and the corpus */

int get_id_frequency(Attribute *attribute, int id)
{
  Component *freqs;

  check_arg(attribute, ATT_POS, cderrno);

  freqs = ensure_component(attribute, CompCorpusFreqs, 0);
  
  if (freqs == NULL) {
    cderrno = CDA_ENODATA;
    return CDA_ENODATA;
  }
  else if ((id >= 0) && (id < freqs->size)) {
    cderrno = CDA_OK;
    return ntohl(freqs->data.data[id]);
  }
  else {
    cderrno = CDA_EIDXORNG;
    return CDA_EIDXORNG;
  }

  assert("Not reached" && 0);
  return 0;
}

/* ============================================================ */

int *get_positions(Attribute *attribute, int id, int *freq, int *restrictor_list, int restrictor_list_size)
{
  Component *revcorp, *revcidx;
  int *buffer;
  int size, range;

  check_arg(attribute, ATT_POS, NULL);

  size  = get_attribute_size(attribute);
  if ((size <= 0) || (cderrno != CDA_OK)) {
    /*       fprintf(stderr, "Cannot determine size of PA %s\n", */
    /*        attribute->any.name); */
      return NULL;
  }
  
  range  = get_id_range(attribute);
  if ((range <= 0) || (cderrno != CDA_OK)) {
    /*       fprintf(stderr, "Cannot determine ID range of PA %s\n", */
    /*        attribute->any.name); */
    return NULL;
  }

  if ((id <0) || (id >= range)) {
    cderrno = CDA_EIDORNG;
    /*       fprintf(stderr, "ID %d out of range of PA %s\n", */
    /*        id, attribute->any.name); */
    *freq = 0;
    return NULL;
  }

  *freq = get_id_frequency(attribute, id);
  if ((*freq < 0) || (cderrno != CDA_OK)) {
    /*       fprintf(stderr, "Frequency %d of ID %d illegal (PA %s)\n", */
    /*        *freq, id, attribute->any.name); */
    return NULL;
  }
  
  
  /* there are no items in a PA with freq 0 - we don't have to
     catch that special case. */


  buffer = (int *)cl_malloc(*freq * sizeof(int));
  /* error handling removed because cl_malloc() is now used */
  
  if (inverted_file_is_compressed(attribute) == 1) {
    
    BStream bs;
    unsigned int i, b, last_pos, gap, offset, ins_ptr, res_ptr;
    
    revcorp = ensure_component(attribute, CompCompRF, 0);
    revcidx = ensure_component(attribute, CompCompRFX, 0);
    
    if (revcorp == NULL || revcidx == NULL) {
      cderrno = CDA_ENODATA;
      *freq = 0;
      return NULL;
    }
    
    b = compute_ba(*freq, size);
    
    offset = ntohl(revcidx->data.data[id]); /* byte offset in RFC */
    
    BSopen((unsigned char *)revcorp->data.data, "r", &bs);
    BSseek(&bs, offset);
    
    last_pos = 0;
    ins_ptr = 0;
    res_ptr = 0;
    
    for (i = 0; i < *freq; i++) {
      
      gap = read_golomb_code_bs(b, &bs);
      last_pos += gap;
      
      /* when the end of the restrictor list is reached, we can
           also leave the for loop above -- TODO
           */
      
      if (restrictor_list && restrictor_list_size > 0) {
        while (res_ptr < restrictor_list_size &&
               last_pos > restrictor_list[res_ptr * 2 + 1]) {
          /* beyond last restricting range */
          res_ptr++;
        }
        if (res_ptr < restrictor_list_size && 
            last_pos >= restrictor_list[res_ptr * 2] &&
            last_pos <= restrictor_list[res_ptr * 2 + 1]) {
          buffer[ins_ptr++] = last_pos;
          }
      }
      else {
          /* no restrictor list: copy */
        buffer[ins_ptr++] = last_pos;
      }
    }
    
    BSclose(&bs);
    
      /* reduce, if possible */
    
    if (ins_ptr < *freq && ins_ptr != *freq) {
      if (ins_ptr == 0) {
        assert(buffer != NULL);
        cl_free(buffer);
      }
      else {
        buffer = cl_realloc(buffer, ins_ptr * sizeof(int));
      }
      *freq = ins_ptr;
    }
    
  }
  else {
      
    revcorp = ensure_component(attribute, CompRevCorpus, 0);
    revcidx = ensure_component(attribute, CompRevCorpusIdx, 0);
    
    if (revcorp == NULL || revcidx == NULL) {
      cderrno = CDA_ENODATA;
      /*        fprintf(stderr, "Cannot load REVCORP or REVCIDX component of %s\n",  */
      /*                attribute->any.name); */
      *freq = 0;
      return NULL;
    }
    
    memcpy(buffer, 
           revcorp->data.data + ntohl(revcidx->data.data[id]),
           *freq * sizeof(int));

    { /* convert network byte order to native integers */
      int i;
      for (i = 0; i < *freq; i++)
        buffer[i] = ntohl(buffer[i]);
    }
    
    if (restrictor_list != NULL && restrictor_list_size > 0) {
      int res_ptr, buf_ptr, ins_ptr;
      
      /* force all items to be within the restrictor's ranges */
      
      ins_ptr = 0;
      res_ptr = 0;
      buf_ptr = 0;
      
      while (buf_ptr < *freq && res_ptr < restrictor_list_size) {
        
        if (buffer[buf_ptr] < restrictor_list[res_ptr*2]) {
          /* before start */
          buf_ptr++;
          }
        else if (buffer[buf_ptr] > restrictor_list[res_ptr*2+1]) {
          /* beyond end */
          res_ptr++;
        }
        else {
          /* within range */
          buffer[ins_ptr++] = buffer[buf_ptr++];
        }
      }
      
      if (ins_ptr < *freq && ins_ptr != *freq) {
        if (ins_ptr == 0) {
          cl_free(buffer);
        }
        else {
          buffer = cl_realloc(buffer, ins_ptr * sizeof(int));
        }
        *freq = ins_ptr;
      }
      
    }
    
  }
      
    cderrno = CDA_OK;
    return buffer;
    
  assert("Not reached" && 0);
  return NULL;
}


/* ---------------------------------------- stream-like reading */

typedef struct _position_stream_rec_ {
  Attribute *attribute;
  int id;
  int id_freq;                  /* id frequency */
  int nr_items;                 /* how many items delivered so far */
  
  int is_compressed;            /* attribute REVCORP is compressed? */

  /* for compressed streams */
  BStream bs;
  int b;
  int last_pos;

  /* for uncompressed streams */
  int *base;

} PositionStreamRecord;

/* -------------------- */


PositionStream
OpenPositionStream(Attribute *attribute, 
                   int id)
{
  Component *revcorp, *revcidx;
  int size, freq, range;

  PositionStream ps = NULL;
  
  check_arg(attribute, ATT_POS, NULL); 
  
  size  = get_attribute_size(attribute);
  if ((size <= 0) || (cderrno != CDA_OK))
    return NULL;
  
  range  = get_id_range(attribute);
  if ((range <= 0) || (cderrno != CDA_OK))
    return NULL;
  
  if ((id <0) || (id >= range)) {
    cderrno = CDA_EIDORNG;
    return NULL;
  }
    
  freq = get_id_frequency(attribute, id);
  if ((freq < 0) || (cderrno != CDA_OK))
    return NULL;
  
  ps = new(PositionStreamRecord);
  assert(ps);
    
  ps->attribute = attribute;
  ps->id = id;
  ps->id_freq = freq;
  ps->nr_items = 0;
  ps->is_compressed = 0;
  ps->b = 0; ps->last_pos = 0;
  ps->base = NULL;

  if (inverted_file_is_compressed(attribute) == 1) {

    int offset;

    ps->is_compressed = 1;

    revcorp = ensure_component(attribute, CompCompRF, 0);
    revcidx = ensure_component(attribute, CompCompRFX, 0);

    if (revcorp == NULL || revcidx == NULL) {
      cderrno = CDA_ENODATA;
      free(ps);
      return NULL;
    }

    ps->b = compute_ba(ps->id_freq, size);
    
    offset = ntohl(revcidx->data.data[id]); /* byte offset in RFC */
      
    BSopen((unsigned char *)revcorp->data.data, "r", &(ps->bs));
    BSseek(&(ps->bs), offset);
    
    ps->last_pos = 0;
      
  }
  else {
    
    ps->is_compressed = 0;
    
    revcorp = ensure_component(attribute, CompRevCorpus, 0);
    revcidx = ensure_component(attribute, CompRevCorpusIdx, 0);

    if (revcorp == NULL || revcidx == NULL) {
      cderrno = CDA_ENODATA;
      free(ps);
      return NULL;
    }
      
    ps->base = revcorp->data.data + ntohl(revcidx->data.data[ps->id]);
    
  }
  
  return ps;
}

int
ClosePositionStream(PositionStream *ps)
{
  assert(ps && *ps);

  (*ps)->attribute = NULL;
  (*ps)->id = -1;
  (*ps)->id_freq = -1;
  (*ps)->nr_items = -1;
  (*ps)->is_compressed = 0;

  if ((*ps)->is_compressed) {
    BSclose(&((*ps)->bs));
    (*ps)->b = 0;
    (*ps)->last_pos = 0;
  }
  else {
    (*ps)->base = NULL;
  }
  free(*ps);
  *ps = NULL;

  return 1;
}

int
ReadPositionStream(PositionStream ps,
                   int *buffer,
                   int buffer_size)
{
  int items_to_read;
  
  assert(ps);
  assert(buffer);
  
  /* gib 0 zurück, wenn wir schon >= freq items gelesen haben */

  if (ps->nr_items >= ps->id_freq)
    return 0;
  
  if (ps->nr_items + buffer_size > ps->id_freq) {
    items_to_read = ps->id_freq - ps->nr_items;
  }
  else {
    items_to_read = buffer_size;
  }

  assert(items_to_read >= 0);
  
  if (items_to_read == 0)
    return 0;
  
  if (ps->is_compressed) {
    
    int gap, i;

    for (i = 0; i < items_to_read; i++,ps->nr_items++) {
      
      gap = read_golomb_code_bs(ps->b, &(ps->bs));
      ps->last_pos += gap;
      
      *buffer = ps->last_pos;
      buffer++;
    }
  }
  else {

    memcpy(buffer, 
           ps->base + ps->nr_items,
           items_to_read * sizeof(int));

    ps->nr_items += items_to_read;

    { /* convert network byte order to native integers */
      int i;
      for (i = 0; i < items_to_read; i++)
        buffer[i] = ntohl(buffer[i]);
    }
    
  }

  return items_to_read;
}

/* ---------------------------------------------------------------------- */

int get_id_at_position(Attribute *attribute, int position)
{
  Component *corpus;

  check_arg(attribute, ATT_POS, cderrno);

  if (item_sequence_is_compressed(attribute) == 1) {

    Component *cis;
    Component *cis_sync;
    Component *cis_map;
    BStream bs;

    unsigned char bit;
    int item;
    unsigned int block, rest, offset, max, v, l, i;

    if (COMPRESS_DEBUG > 1)
      fprintf(stderr, "Accessing position %d of %s via compressed item sequence\n",
              position, attribute->any.name);

    cis      = ensure_component(attribute, CompHuffSeq, 0);
    cis_map  = ensure_component(attribute, CompHuffCodes, 0);
    cis_sync = ensure_component(attribute, CompHuffSync, 0);

    if ((cis == NULL) || (cis_map == NULL) || (cis_sync == NULL)) {
      cderrno = CDA_ENODATA;
      return CDA_ENODATA;
    }

    if ((position >= 0) && (position < attribute->pos.hc->length)) {

      block = position / SYNCHRONIZATION;
      rest  = position % SYNCHRONIZATION;
      
      if (attribute->pos.this_block_nr != block) {

        /* the current block in the decompression buffer is not the
         * block we need. So we read the proper block into the buffer
         * and hope that we'll get a cache hit next time. */

        if (COMPRESS_DEBUG > 0)
          fprintf(stderr, "Block miss: have %d, want %d\n", 
                  attribute->pos.this_block_nr, block);

        /* is the block we read the last block of the corpus? Then, we
         * cannot read SYNC items, but only as much as there are left.
         * */

        max = attribute->pos.hc->length - block * SYNCHRONIZATION;
        if (max > SYNCHRONIZATION)
          max = SYNCHRONIZATION;


        attribute->pos.this_block_nr = block;


        offset = ntohl(cis_sync->data.data[block]);

        if (COMPRESS_DEBUG > 1)
          fprintf(stderr, "-> Block %d, rest %d, offset %d\n",
                  block, rest, offset);
      
        BSopen((unsigned char *)cis->data.data, "r", &bs);
        BSseek(&bs, offset);
        
        for (i = 0; i < max; i++) {
          
          if (!BSread(&bit, 1, &bs)) {
            fprintf(stderr, "cdaccess:decompressed read: Read error/1\n");
            cderrno = CDA_ENODATA;
            return cderrno;
          }
        
          v = (bit ? 1 : 0);
          l = 1;
          
          while (v < attribute->pos.hc->min_code[l]) {
            
            if (!BSread(&bit, 1, &bs)) {
              fprintf(stderr, "cdaccess:decompressed read: Read error/2\n");
              cderrno = CDA_ENODATA;
              return cderrno;
            }
            
            v <<= 1;
            if (bit)
              v++;
            l++;
          }

          /* we now have the item - store it in the decompression block */

          item = ntohl(attribute->pos.hc->symbols[attribute->pos.hc->symindex[l] + v - 
                                                 attribute->pos.hc->min_code[l]]);

          attribute->pos.this_block[i] = item;
        }

        BSclose(&bs);

      }
      else if (COMPRESS_DEBUG > 0) 
        fprintf(stderr, "Block hit: block[%d,%d]\n", block, rest);
     
      assert(rest < SYNCHRONIZATION);
      
      cderrno = CDA_OK;         /* hi 'Oli' ! */
      return attribute->pos.this_block[rest];
    }
    else {
      cderrno = CDA_EPOSORNG;
      return CDA_EPOSORNG;
    }
  }
  else {

    corpus = ensure_component(attribute, CompCorpus, 0);

    if (corpus == NULL) {
      cderrno = CDA_ENODATA;
      return CDA_ENODATA;
    }

    if ((position >= 0) && (position < corpus->size)) {
      cderrno = CDA_OK;
      return ntohl(corpus->data.data[position]);
    }
    else {
      cderrno = CDA_EPOSORNG;
      return CDA_EPOSORNG;
    }
  }

  assert("Not reached" && 0);
  return 0;
}


char *get_string_at_position(Attribute *attribute, int position)
{
  int id;

  check_arg(attribute, ATT_POS, NULL);

  id = get_id_at_position(attribute, position);
  if ((id < 0) || (cderrno != CDA_OK))
    return NULL;

  return get_string_of_id(attribute, id);
}


/* ========== some high-level constructs */

char *get_id_info(Attribute *attribute, int index, int *freq, int *slen)
{
  check_arg(attribute, ATT_POS, NULL);

  *freq = get_id_frequency(attribute, index);
  if ((*freq < 0) || (cderrno != CDA_OK))
    return NULL;

  *slen = get_id_string_len(attribute, index);
  if ((*slen < 0) || (cderrno != CDA_OK))
    return NULL;
  
  return get_string_of_id(attribute, index);
}



/* collect_matching_word_ids:
 *
 * Returns a pointer to a sequence of ints of size number_of_matches. The list
 * is alloced with malloc, so do a free() when you don't need it any more.  
 */

int *
collect_matching_ids(Attribute *attribute, char *pattern, int flags, int *number_of_matches)
{
  Component *lexidx;
  Component *lex;
  int *lexidx_data;
  char *lex_data;

  int *table;                   /* list of matching IDs */
  int match_count;              /* count matches in local variable while scanning */
  /* note that it would also be possible to use a Bitfield <bitfield.h>, but this custom implementation is somewhat more efficient */
  unsigned char *bitmap = NULL; /* use bitmap while scanning lexicon to reduce memory footprint and avoid large realloc() */
  int bitmap_size;              /* size of allocated bitmap in bytes */
  int bitmap_offset;            /* current bitmap offset (in bytes) */
  unsigned char bitmap_mask;    /* current bitmap offset (within-byte part, as bit mask) */
  /* might move bitmap to static variable and re-allocate only when necessary */
  
  int regex_result, idx, i, len, lexsize;
  int optimised, grain_match, grain_hits;

  CL_Regex rx;
  char *word, *iso_string;

  check_arg(attribute, ATT_POS, NULL);

  lexidx = ensure_component(attribute, CompLexiconIdx, 0);
  lex = ensure_component(attribute, CompLexicon, 0);
  
  if ((lexidx == NULL) || (lex == NULL)) {
    cderrno = CDA_ENODATA;
    return NULL;
  }
  
  lexsize = lexidx->size;
  lexidx_data = (int *) lexidx->data.data;
  lex_data = (char *) lex->data.data;
  match_count = 0;

  rx = cl_new_regex(pattern, flags, latin1);
  if (rx == NULL) {
    fprintf(stderr, "Regex Compile Error: %s\n", cl_regex_error);
    cderrno = CDA_EBADREGEX;
    return NULL;
  }
  optimised = cl_regex_optimised(rx);

  /* allocate bitmap for matching IDs */
  bitmap_size = (lexsize + 7) / 8;  /* this is the exact number of bytes needed, I hope */
  bitmap = (unsigned char *) cl_calloc(bitmap_size, sizeof(unsigned char)); /* initialise: no bits set */
  bitmap_offset = 0;
  bitmap_mask = 0x80;           /* start with MSB of first byte */

  grain_hits = 0;               /* report how often we have a grain match when using optimised search */
  for (idx = 0; idx < lexsize; idx++) {
    int off_start, off_end;     /* start and end offset of current lexicon entry */
    char *p;
    int i;

    /* compute start offset and length of current lexicon entry from lexidx, if possible) */
    off_start = ntohl(lexidx_data[idx]);
    word = lex_data + off_start;
    if (idx < lexsize-1) {
      off_end = ntohl(lexidx_data[idx + 1]) - 1;
      len = off_end - off_start;
    }
    else {
      len = strlen(word);
    }

    /* make working copy of current lexicon entry if necessary */
    if (rx->flags) {
      iso_string = rx->iso_string; /* use pre-allocated buffer */
      for (i = len, p = iso_string; i >= 0; i--) {
        *(p++) = *(word++);     /* string copy includes NUL character */
      }
      cl_string_canonical(iso_string, rx->flags);
    }
    else {
      iso_string = word;
    }

    /* the code below duplicates cl_regex_match() in order to reduce overhead and allow profiling */ 
    grain_match = 0;
    if (optimised) {
      int i, di, k, max_i;
      /* string offset where first character of each grain would be */
      max_i = len - rx->grain_len; /* stop trying to match when i > max_i */
      if (rx->anchor_end) 
        i = (max_i >= 0) ? max_i : 0; /* if anchored at end, align grains with end of string */
      else
        i = 0;

      while (i <= max_i) {
        int jump = rx->jumptable[(unsigned char) iso_string[i + rx->grain_len - 1]];
        if (jump > 0) {
          i += jump;            /* Boyer-Moore search */
        }
        else {
          for (k = 0; k < rx->grains; k++) {
            char *grain = rx->grain[k];
            di = 0;
            while ((di < rx->grain_len) && (grain[di] == iso_string[i+di]))
              di++;
            if (di >= rx->grain_len) {
              grain_match = 1;
              break;            /* we have found a grain match and can quit the loop */
            }
          }
          i++;
        }
        if (rx->anchor_start) break; /* if anchored at start, only the first iteration can match */
      }
      if (grain_match) grain_hits++;
    }
        
    if (optimised && !grain_match) /* enabled since version 2.2.b94 (14 Feb 2006) -- before: && cl_optimize */
      regex_result = REG_NOMATCH; /* according to POSIX.2 */
    else
      regex_result = regexec(&(rx->buffer), iso_string, 0, NULL, 0);
    /* regex was compiled with start and end anchors, so we needn't check that the full string was matched */

    if (regex_result == 0) {    /* regex match */
      bitmap[bitmap_offset] |= bitmap_mask; /* set bit */
      match_count++;

#if 0  /* debugging code used before version 2.2.b94 */
      if (optimised && !grain_match) /* critical: optimiser didn't accept candidate, but regex matched */
        fprintf(stderr, "CL ERROR: regex optimiser did not accept '%s' for regexp /%s/%s%s\n", 
                iso_string, pattern, (rx->flags & IGNORE_CASE) ? "c" : "", (rx->flags & IGNORE_DIAC) ? "d" : "");
#endif
    }

    bitmap_mask >>= 1;
    if (bitmap_mask == 0) {
      bitmap_offset++;
      bitmap_mask = 0x80;
    }

  }
  if (cl_debug && optimised) 
    fprintf(stderr, "CL: regexp optimiser found %d candidates out of %d strings\n", grain_hits, lexsize);

  if (match_count == 0) {       /* no matches */
    table = NULL;
  }
  else {                        /* generate list of matching IDs from bitmap */
    table = (int *) cl_malloc(match_count * sizeof(int));
    bitmap_offset = 0;
    bitmap_mask = 0x80;
    idx = 0;
    for (i = 0; i < lexsize; i++) {
      if (bitmap[bitmap_offset] & bitmap_mask) {
        table[idx] = i;
        idx++;
      }
      bitmap_mask >>= 1;
      if (bitmap_mask == 0) {
        bitmap_offset++;
        bitmap_mask = 0x80;
      }
    }
    assert((idx == match_count) && "cl_regex2id(): bitmap inconsistency");
  }
  *number_of_matches = match_count;

  cl_free(bitmap);
  cl_delete_regex(rx);
  cderrno = CDA_OK;
  return table;
}



int cumulative_id_frequency(Attribute *attribute,
                            int *word_ids,
                            int number_of_words)
{
  int k, sum;

  check_arg(attribute, ATT_POS, cderrno);

  sum = 0;

  if (word_ids == NULL) {
    cderrno = CDA_ENODATA;
    return CDA_ENODATA;
  }
  else {
    for (k = 0; k < number_of_words; k++) {
      sum += get_id_frequency(attribute, word_ids[k]);
      if (cderrno != CDA_OK)
        return cderrno;
    }
    cderrno = CDA_OK;
    return sum;
  }

  assert("Not reached" && 0);
  return 0;
}

/* this is the way qsort(..) is meant to be used: give it void* args
   and cast them to the actual type in the compare function;
   this definition conforms to ANSI and POSIX standards according to the LDP */
static int intcompare(const void *i, const void *j)
{ return(*(int *)i - *(int *)j); }

int *collect_matches(Attribute *attribute,
                     
                     int *word_ids,       /* a list of word_ids */
                     int number_of_words, /* the length of this list */
                     
                     int sort,            /* return sorted list? */
                     
                     int *size_of_table,  /* the size of the allocated table */
                     
                     int *restrictor_list,
                     int restrictor_list_size)
{
  int size, k, p, word_id, freq;

  int *table, *start;

  Component *lexidx;

  check_arg(attribute, ATT_POS, NULL);

  *size_of_table = 0;

  lexidx = ensure_component(attribute, CompLexiconIdx, 0);

  if ((lexidx == NULL) || (word_ids == NULL)) {
    cderrno = CDA_ENODATA;
    return NULL;
  }

  size = cumulative_id_frequency(attribute, word_ids, number_of_words);
  if ((size < 0) || (cderrno != CDA_OK)) {
    return NULL;
  }

  if (size > 0) {
      
    table = (int *)cl_malloc(size * sizeof(int));
    /* error handling removed because of cl_malloc() */
      
    p = 0;

    for (k = 0; k < number_of_words; k++) {

      word_id = word_ids[k];
      
      if ((word_id < 0) || (word_id >= lexidx->size)) {
        cderrno = CDA_EIDORNG;
        free(table);
        return NULL;
      }

      start = get_positions(attribute, word_id, &freq, NULL, 0);
      if ((freq < 0) || (cderrno != CDA_OK)) {
        free(table);
        return NULL;
      }

      /* lets hack: */
      memcpy(&table[p], start, freq * sizeof(int));
      p += freq;
        
      /* f_o now always mallocs. */
      free(start);
    }
      
    assert(p == size);
      
    if (sort)
      qsort(table, size, sizeof(int), intcompare);
      
    *size_of_table = size;
    cderrno = CDA_OK;
    return table;
  }
  else {
    *size_of_table = 0;
    cderrno = CDA_OK;
    return NULL;
  }
  
  assert("Not reached" && 0);
  return NULL;
}

/* ================================================== UTILITY FUNCTIONS */

int *get_previous_mark(int *data, int size, int position)
{
  int nr;

  int mid, high, low, comp;

  int max = size/2;

  low = 0;
  nr = 0;
  high = max - 1;

  while (low <= high) {

    nr++;

    if (nr > 100000) {
      fprintf(stderr, "Binary search in get_surrounding_positions failed\n");
      return NULL;
    }

    mid = (low + high)/2;

    comp = position - ntohl(data[mid*2]);

    if (comp == 0)
      return &data[mid*2];
    else if (comp > 0) {
      if (position <= ntohl(data[mid*2+1]))
        return &data[mid*2];
      else
        low = mid + 1;
    }
    else if (mid == low) {
      /* fail; */
      return NULL;
    }
    else /* comp < 0 */
      high = mid - 1;
  }
  return NULL;
}

/* ================================================== STRUCTURAL ATTRIBUTES */

/* new style functions with normalised behaviour */

int cl_cpos2struc(Attribute *a, int cpos) { /* normalised to standard return value behaviour */
  int struc = -1;
  if (get_num_of_struc(a, cpos, &struc))
    return struc;
  else
    return cderrno;
}

int cl_cpos2boundary(Attribute *a, int cpos) {  /* convenience function: within region or at boundary? */
  int start = -1, end = -1;
  if (cl_cpos2struc2cpos(a, cpos, &start, &end)) {
    int flags = STRUC_INSIDE;
    if (cpos == start)
      flags |= STRUC_LBOUND;
    if (cpos == end)
      flags |= STRUC_RBOUND;
    return flags;
  }
  else if (cderrno == CDA_ESTRUC) {
    return 0; /* outside region */
  }
  else
    return cderrno; /* some error occurred */
}

int cl_max_struc(Attribute *a) { /* normalised to standard return value behaviour */
  int nr = -1;
  if (get_nr_of_strucs(a, &nr)) 
    return nr;
  else
    return cderrno;
}



int get_struc_attribute(Attribute *attribute, 
                        int position,
                        int *struc_start,
                        int *struc_end)
{
  Component *struc_data;
  
  int *val;

  check_arg(attribute, ATT_STRUC, cderrno);

  *struc_start = 0;
  *struc_end = 0;

  struc_data = ensure_component(attribute, CompStrucData, 0);
    
  if (struc_data == NULL) {
    cderrno = CDA_ENODATA;
    return 0;
  }

  val = get_previous_mark(struc_data->data.data,
                            struc_data->size,
                            position);
    
  if (val != NULL) {
    *struc_start = ntohl(*val);
    *struc_end   = ntohl(*(val + 1));
    cderrno = CDA_OK;
    return 1;
  }
  else {
    cderrno = CDA_ESTRUC;
    return 0;
  }

  assert("Not reached" && 0);
  return 0;
}


int get_num_of_struc(Attribute *attribute,
                     int position,
                     int *struc_num)
{

  Component *struc_data;
  int *val;

  check_arg(attribute, ATT_STRUC, cderrno);

  struc_data = ensure_component(attribute, CompStrucData, 0);
    
  if (struc_data == NULL) {
    cderrno = CDA_ENODATA;
    return 0;
  }
  
  val = get_previous_mark(struc_data->data.data,
                          struc_data->size,
                            position);
    
  if (val != NULL) {
    *struc_num = (val - struc_data->data.data)/2;
    cderrno = CDA_OK;
    return 1;
  }
  else {
    cderrno = CDA_ESTRUC;
    return 0;
  }

  assert("Not reached" && 0);
  return 0;
}


int get_bounds_of_nth_struc(Attribute *attribute,
                            int struc_num,
                            int *struc_start,
                            int *struc_end)
{

  Component *struc_data;

  check_arg(attribute, ATT_STRUC, cderrno);

  struc_data = ensure_component(attribute, CompStrucData, 0);
    
  if (struc_data == NULL) {
    cderrno = CDA_ENODATA;
    return 0;
  }
  
  if ((struc_num < 0) || (struc_num >= (struc_data->size / 2))) {
    cderrno = CDA_EIDXORNG;
    return 0;
  }
  else {
    *struc_start = ntohl(struc_data->data.data[struc_num * 2]);
    *struc_end   = ntohl(struc_data->data.data[(struc_num * 2)+1]);
    cderrno = CDA_OK;
    return 1;
  }

  assert("Not reached" && 0);
  return 0;
}


int get_nr_of_strucs(Attribute *attribute,
                     int *nr_strucs)
{
  Component *struc_data;

  check_arg(attribute, ATT_STRUC, cderrno);

  struc_data = ensure_component(attribute, CompStrucData, 0);
    
  if (struc_data == NULL) {
    cderrno = CDA_ENODATA;
    return 0;
  }

  *nr_strucs = struc_data->size / 2;
  cderrno = CDA_OK;
  return 1;

  assert("Not reached" && 0);
  return 0;
}

int 
structure_has_values(Attribute *attribute) {

  check_arg(attribute, ATT_STRUC, cderrno);

  if (attribute->struc.has_attribute_values < 0) {

    /* if h_a_v < 0 then we didn't yet test whether it has values or not */

    ComponentState avs_state, avx_state;
      
    avs_state = component_state(attribute, CompStrucAVS);
    avx_state = component_state(attribute, CompStrucAVX);
      
    if ((avs_state == ComponentLoaded || avs_state == ComponentUnloaded) &&
        (avx_state == ComponentLoaded || avx_state == ComponentUnloaded))
      attribute->struc.has_attribute_values = 1;
    else
      attribute->struc.has_attribute_values = 0;
  }

  cderrno = CDA_OK;
  return attribute->struc.has_attribute_values;
}

/* does not strdup(), so don't free returned char * */

int s_v_comp(const void *v1, const void *v2)
{
  return ntohl(*((int *)v1)) - ntohl(*((int *)v2));
}

char *structure_value(Attribute *attribute, int struc_num)
{
  check_arg(attribute, ATT_STRUC, NULL);

  if (structure_has_values(attribute) && (cderrno == CDA_OK)) {

    typedef struct _idx_el { 
      int id;
      int offset;
    } IndexElement;
    
    Component *avs;
    Component *avx;
    
    IndexElement key, *idx;

    avs = ensure_component(attribute, CompStrucAVS, 0);
    avx = ensure_component(attribute, CompStrucAVX, 0);

    if (avs == NULL || avx == NULL) {
      cderrno = CDA_ENODATA;
      return 0;
    }

    key.id = htonl(struc_num);

    /* current redundant file format allows regions without annotations, so the index file (avx)
     * consists of (region index, ptr) pairs, where ptr is an offset into the lexicon file (avs)
     */
    idx = (IndexElement *)bsearch(&key, avx->data.data, avx->size / 2,
                                  2 * sizeof(int), s_v_comp);
    if (idx) {
      int offset;
      offset = ntohl(idx->offset);

      if (offset >= 0 && offset < avs->data.size) {
        cderrno = CDA_OK;
        return (char *)(avs->data.data) + offset;
      }
      else {
        cderrno = CDA_EINTERNAL; /* this is a bad data inconsistency! */
        return NULL;
      }
    }
    else {
      /* we don't allow regions with missing annotations, so this must be an index error */
      cderrno = CDA_EIDXORNG;
      return NULL;
    }
  }
  else
    return NULL;
}

char *structure_value_at_position(Attribute *struc, int position)
{
  int snum = -1;
  
  if ((struc == NULL) || 
      (!get_num_of_struc(struc, position, &snum)))
    return NULL;
  else 
    return structure_value(struc, snum);
}

/* ================================================== ALIGNMENT ATTRIBUTES */

int get_alignment(int *data, int size, int position)   /* ALIGN component */
{
  int nr;
  int mid, high, low, comp;
  int max = size/2;

  /* organisation of ALIGN component is
       source boundary #1
       target boundary #1
       source boundary #2
       target boundary #2
       ...
  */

  low = 0;
  nr = 0;
  high = max - 1;

  while (low <= high) {
    nr++;
    if (nr > 100000) {
      fprintf(stderr, "Binary search in get_alignment_item failed\n");
      return -1;
    }

    mid = (low + high)/2;

    comp = position - ntohl(data[mid*2]);
    if (comp == 0)
      return mid;
    else if (comp > 0) {
      if ((mid*2 < size) && (position < ntohl(data[(mid+1)*2])))
        return mid;
      else
        low = mid + 1;
    }
    else if (mid == low) {
      /* fail; */
      return -1;
    }
    else /* comp < 0 */
      high = mid - 1;
  }
  return -1;
}

int get_extended_alignment(int *data, int size, int position)   /* XALIGN component */
{
  int nr;
  int mid, high, low, start, end;
  int max = size/4;

  /* organisation of XALIGN component is
       source region #1 start
       source region #1 end
       target region #1 start
       target region #1 end
       source region #2 start
       ...
  */

  low = 0;
  nr = 0;
  high = max - 1;

  while (low <= high) {
    nr++;
    if (nr > 100000) {
      fprintf(stderr, "Binary search in get_extended_alignment_item failed\n");
      return -1;
    }

    mid = (low + high)/2;

    start = ntohl(data[mid*4]);
    end = ntohl(data[mid*4 + 1]);
    if (start <= position) {
      if (position <= end) {
        return mid;             /* return nr of alignment region */
      }
      else {
        low = mid + 1;
      }
    }
    else {
      high = mid - 1;
    }

  }
  return CDA_EALIGN;    /* high < low --> search failed  */
}


int get_alg_attribute(Attribute *attribute, /* accesses alignment attribute */
                      int position, 
                      int *source_corpus_start,
                      int *source_corpus_end,
                      int *aligned_corpus_start,
                      int *aligned_corpus_end)
{
  int *val;
  int alg;                      /* nr of alignment region */

  Component *align_data;

  check_arg(attribute, ATT_ALIGN, cderrno);

  *source_corpus_start = -1;
  *aligned_corpus_start = -1;
  *source_corpus_end = -1;
  *aligned_corpus_end = -1;

  align_data = ensure_component(attribute, CompAlignData, 0);

  if (align_data == NULL) {
    cderrno = CDA_ENODATA;
    return 0;
  }
  
  alg = get_alignment(align_data->data.data, 
                      align_data->size,
                      position);
  if (alg >= 0) {
    val = align_data->data.data + (alg * 2);
    *source_corpus_start  = ntohl(val[0]);
    *aligned_corpus_start = ntohl(val[1]);
    
    if (val + 3 - align_data->data.data >= align_data->size) {
      *source_corpus_end = -1;
      *aligned_corpus_end = -1;
    }
    else {
      *source_corpus_end  = ntohl(val[2])-1;
      *aligned_corpus_end = ntohl(val[3])-1;
    }
      
    cderrno = CDA_OK;
    return 1;
  }
  else {
    cderrno = CDA_EPOSORNG;
    return 0;
  }

  assert("Not reached" && 0);
  return 0;
}

int cl_has_extended_alignment(Attribute *attribute) {
  ComponentState xalign;

  check_arg(attribute, ATT_ALIGN, cderrno);
  xalign = component_state(attribute, CompXAlignData);
  if ((xalign == ComponentLoaded) || (xalign == ComponentUnloaded)) {
    return 1;                   /* XALIGN component exists */
  }
  else {
    return 0;
  }
}

/* extended alignment functions use new-style prototypes */
int cl_max_alg(Attribute *attribute) 
{
  Component *align_data;

  if (! cl_has_extended_alignment(attribute)) /* subsumes check_arg() */
    {
      align_data = ensure_component(attribute, CompAlignData, 0);
      if (align_data == NULL) {
        cderrno = CDA_ENODATA;
        return cderrno;
      }
      cderrno = CDA_OK;
      return (align_data->size / 2) - 1; /* last alignment boundary doesn't correspond to region */
    }
  else
    {
      align_data = ensure_component(attribute, CompXAlignData, 0);
      if (align_data == NULL) {
        cderrno = CDA_ENODATA;
        return cderrno;
      }
      cderrno = CDA_OK;
      return (align_data->size / 4);
    }
}

int cl_cpos2alg(Attribute *attribute, int cpos)
{
  int alg;
  Component *align_data;

  if (! cl_has_extended_alignment(attribute)) /* subsumes check_arg() */
    {
      align_data = ensure_component(attribute, CompAlignData, 0);
      if (align_data == NULL) {
        cderrno = CDA_ENODATA;
        return cderrno;
      }
      alg = get_alignment(align_data->data.data, 
                          align_data->size,
                          cpos);
      if (alg >= 0) {
        cderrno = CDA_OK;
        return alg;
      }
      else {
        cderrno = CDA_EPOSORNG; /* old alignment files don't allow gaps -> index error */
        return cderrno;
      }
    }
  else
    {
      align_data = ensure_component(attribute, CompXAlignData, 0);
      if (align_data == NULL) {
        cderrno = CDA_ENODATA;
        return cderrno;
      }
      alg = get_extended_alignment(align_data->data.data, 
                                   align_data->size,
                                   cpos);
      if (alg >= 0) {
        cderrno = CDA_OK;
        return alg;
      }
      else {
        cderrno = CDA_EALIGN;
        return alg;               /* not a real error (just an "exception" condition) */
      }
    }
}

  
int cl_alg2cpos(Attribute *attribute, int alg,
                int *source_region_start, int *source_region_end,
                int *target_region_start, int *target_region_end)
{
  int *val, size;
  Component *align_data;

  *source_region_start = -1;
  *target_region_start = -1;
  *source_region_end = -1;
  *target_region_end = -1;
  if (! cl_has_extended_alignment(attribute)) /* subsumes check_arg() */
    {
      align_data = ensure_component(attribute, CompAlignData, 0);
      if (align_data == NULL) {
        cderrno = CDA_ENODATA;
        return 0;
      }
      size = (align_data->size / 2) - 1; /* last alignment boundary doesn't correspond to region */
      if ((alg < 0) || (alg >= size)) {
        cderrno = CDA_EIDXORNG;
        return 0;
      }
      val = align_data->data.data + (alg * 2);
      *source_region_start  = ntohl(val[0]);
      *target_region_start = ntohl(val[1]);
      *source_region_end  = ntohl(val[2]) - 1;
      *target_region_end = ntohl(val[3]) - 1;
      cderrno = CDA_OK;
      return 1;
    }
  else 
    {
      align_data = ensure_component(attribute, CompXAlignData, 0);
      if (align_data == NULL) {
        cderrno = CDA_ENODATA;
        return 0;
      }
      size = align_data->size / 4;
      if ((alg < 0) || (alg >= size)) {
        cderrno = CDA_EIDXORNG;
        return 0;
      }
      val = align_data->data.data + (alg * 4);
      *source_region_start  = ntohl(val[0]);
      *source_region_end    = ntohl(val[1]);
      *target_region_start = ntohl(val[2]);
      *target_region_end   = ntohl(val[3]);
      cderrno = CDA_OK;
      return 1;
    }
}



/* ================================================== DYNAMIC ATTRIBUTES */

/* ...: parameters (of *int or *char) and structure
 * which gets the result (*int or *char)
 */

int call_dynamic_attribute(Attribute *attribute,
                           DynCallResult *dcr,
                           DynCallResult *args,
                           int nr_args)
{
  char call[2048];
  char istr[32];

  int i, k, ap, ins;

  FILE *pipe;
  DynCallResult arg;
  int argnum, val;
  DynArg *p;
  char c;


  check_arg(attribute, ATT_DYN, cderrno);

  if ((args == NULL) || (nr_args <= 0))
      goto error;
  
  p = attribute->dyn.arglist;
  argnum = 0;
  
  while ((p != NULL) && (argnum < nr_args)) {
    
    arg = args[argnum];
    
    if ((p->type == args->type) ||
          ((p->type == ATTAT_POS) && (args->type == ATTAT_INT))) {
      p = p->next;
      argnum++;
    }
    else if (p->type == ATTAT_VAR) {
      argnum++;
    }
    else
      goto error;
  }
  
  if (((p == NULL) && (argnum == nr_args)) ||
      ((p != NULL) && (p->type == ATTAT_VAR))) {
    
    /* everything should be OK then. */
      
    
    /* build the call string */
    
    i = 0; ins = 0;
    
    while ((c = attribute->dyn.call[i]) != '\0') {
      
      if ((c == '$') && isdigit(attribute->dyn.call[i+1])) {
        
        /* reference */
        
        i++;
        val = 0;
        while (isdigit(attribute->dyn.call[i])) {
          val = val * 10 + attribute->dyn.call[i] - '0';
          i++;
        }
        
        /* find the corresponding argument in the definition of args */
        
        if ((val > 0) && (val <= nr_args)) {
          p = attribute->dyn.arglist;
          k = val - 1;  /* 0 .. max. nr_args-1 */
          ap = 0;
          while ((p != NULL) && (p->type != ATTAT_VAR) && (k > 0)) {
            p = p->next;
            ap++;
            k--;
          }
          
          if (p != NULL) {
            
            assert(ap < nr_args);
            
            if (p->type == ATTAT_VAR) {
              
              /* put all args >= ap into the call string */
              for (; ap < nr_args; ap++)
                
                switch (args[ap].type) {
                case ATTAT_STRING:
                  for (k = 0; args[ap].value.charres[k]; k++)
                    call[ins++] = args[ap].value.charres[k];
                  break;
                  
                case ATTAT_INT:
                case ATTAT_POS:
                  sprintf(istr, "%d", args[ap].value.intres);
                  for (k = 0; istr[k]; k++)
                    call[ins++] = istr[k];

                  break;

                case ATTAT_FLOAT:
                  sprintf(istr, "%f", args[ap].value.floatres);
                  for (k = 0; istr[k]; k++)
                    call[ins++] = istr[k];
                  break;
                  
                case ATTAT_NONE:
                case ATTAT_VAR:
                case ATTAT_PAREF:
                default:
                  goto error;
                  break;
                }
            }
            else {
              /* just put arg ap into the call string */
              switch (args[ap].type) {
              case ATTAT_STRING:
                for (k = 0; args[ap].value.charres[k]; k++)
                  call[ins++] = args[ap].value.charres[k];
                break;
                
              case ATTAT_INT:
              case ATTAT_POS:
                sprintf(istr, "%d", args[ap].value.intres);
                for (k = 0; istr[k]; k++)
                  call[ins++] = istr[k];
                  
                break;
                  
              case ATTAT_FLOAT:
                sprintf(istr, "%f", args[ap].value.floatres);
                for (k = 0; istr[k]; k++)
                  call[ins++] = istr[k];
                break;

              case ATTAT_NONE:
              case ATTAT_VAR:
              case ATTAT_PAREF:
              default:
                goto error;
                break;
              }
            }
          }
          else
            goto error;
        }
        else
          goto error;
      }
      else {
        call[ins++] = c;
        call[ins] = '\0';       /* for debugging */
        i++;                    /* get next char */
      }
    }
    call[ins++] = '\0';
    
    /*       fprintf(stderr, "Composed dynamic call: \"%s\"\n", call); */

    pipe = popen(call, "r");
      
    if (pipe == NULL) 
      goto error;
    
    dcr->type = attribute->dyn.res_type;
    
    switch (attribute->dyn.res_type) {

    case ATTAT_POS:             /* convert output to int */
    case ATTAT_INT:
      if (fscanf(pipe, "%d", &(dcr->value.intres)) == 0)
        dcr->value.intres = -1;
      break;
      
    case ATTAT_STRING:  /* copy output */
      fgets(call, 1024, pipe);
      dcr->value.charres = (char *)cl_strdup(call);
        
      break;
        
    case ATTAT_FLOAT:
      if (fscanf(pipe, "%lf", &(dcr->value.floatres)) == 0)
        dcr->value.floatres = 0.0;
      break;

    case ATTAT_NONE:
    case ATTAT_VAR:             /* not possible */
    case ATTAT_PAREF:
    default:
      goto error;
      break;
    }
    
    pclose(pipe);
    
    cderrno = CDA_OK;
    return 1;
  }
  else 
    goto error;

 error:
  cderrno = CDA_EARGS;
  dcr->type = ATTAT_NONE;
  return 0;
}

int nr_of_arguments(Attribute *attribute)
{
  int nr;
  DynArg *arg;

  check_arg(attribute, ATT_DYN, cderrno);

  nr = 0;
  for (arg = attribute->dyn.arglist; arg != NULL; arg = arg->next)
    if (arg->type == ATTAT_VAR) {
      nr = -nr;
      break;
    }
    else
      nr++;
  
  cderrno = CDA_OK;
  return nr;

  assert("Not reached" && 0);
  return 0;
}

/* ================================================== EOF */

