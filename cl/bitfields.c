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

#include <limits.h>

#include "globals.h"

#include "macros.h"

#include "bitfields.h"

static int BaseTypeSize = sizeof(BFBaseType);
static int BaseTypeBits = sizeof(BFBaseType) * CHAR_BIT;

Bitfield create_bitfield(int nr_of_elements)
{
  int full_cells;

  Bitfield bf = (Bitfield)cl_malloc(sizeof(BFBuf));

  full_cells = nr_of_elements / BaseTypeBits;
  if (nr_of_elements % BaseTypeBits != 0)
    full_cells++;
  
  bf->bytes = full_cells * BaseTypeSize;
  bf->elements = nr_of_elements;
  bf->nr_bits_set = 0;

  bf->field = (BFBaseType *)cl_malloc(bf->bytes);
  if (bf->field)
    memset((char *)bf->field, '\0', bf->bytes);

  return bf;
}

Bitfield copy_bitfield(Bitfield source)
{
  if (source) {
    Bitfield target = create_bitfield(source->elements);

    if (target) {
      target->nr_bits_set = source->nr_bits_set;
      memcpy(target->field, source->field, source->bytes);
    }

    return target;
  }
  else 
    return NULL;
}

int destroy_bitfield(Bitfield *bptr)
{
  assert(bptr);

  if (*bptr != NULL) {
    if ((*bptr)->field != NULL) {
      free((*bptr)->field);
      (*bptr)->field = NULL;
    }
    free(*bptr);
    *bptr = NULL;
  }
  return 1;
}

int set_bit(Bitfield bitfield, int element)
{
  if ((bitfield != NULL) && (element < bitfield->elements)) {

    BFBaseType v1 = bitfield->field[element/BaseTypeBits];

    bitfield->field[element/BaseTypeBits] |= (1<<(element % BaseTypeBits));

    if (bitfield->field[element/BaseTypeBits] != v1)
      bitfield->nr_bits_set++;
    return 1;
  }
  else {
    fprintf(stderr, "Illegal offset %d in set_bit\n", element);
    return 0;
  }
}

int clear_bit(Bitfield bitfield, int element)
{
  if ((bitfield != NULL) && (element < bitfield->elements)) {

    BFBaseType v1 = bitfield->field[element/BaseTypeBits];

    bitfield->field[element/BaseTypeBits] &= ~(1<<(element % BaseTypeBits));

    if (bitfield->field[element/BaseTypeBits] != v1)
      bitfield->nr_bits_set--;

    return 1;
  }
  else {
    fprintf(stderr, "Illegal offset %d in clear_bit\n", element);
    return 0;
  }
}

int clear_all_bits(Bitfield bitfield)
{
  if ((bitfield != NULL)) {
    memset((char *)bitfield->field, '\0', bitfield->bytes);
    bitfield->nr_bits_set = 0;
    return 1;
  }
  else
    return 0;
}

int set_all_bits(Bitfield bitfield)
{
  if ((bitfield != NULL)) {
    memset((char *)bitfield->field, 0xff, bitfield->bytes);
    bitfield->nr_bits_set = bitfield->elements;
    return 1;
  }
  else
    return 0;
}

int get_bit(Bitfield bitfield, int element)
{
  if ((bitfield != NULL) && (element < bitfield->elements))
    return 
      (
       ((bitfield->field[element/BaseTypeBits] & 
	 (1<<(element % BaseTypeBits))) == 0)
       ? 0 : 1);
  else {
    fprintf(stderr, "Illegal offset %d in get_bit\n", element);
    return -1;
  }
}

int toggle_bit(Bitfield bitfield, int element)
{
  if ((bitfield != NULL) && (element < bitfield->elements)) {

    if ((bitfield->field[element/BaseTypeBits] 
	 & (1<<(element % BaseTypeBits))) == 0)
      bitfield->nr_bits_set++;
    else
      bitfield->nr_bits_set--;

    bitfield->field[element/BaseTypeBits] ^= (1<<(element % BaseTypeBits));
    return 1;
  }
  else {
    fprintf(stderr, "Illegal offset %d in toggle_bit\n", element);
    return 0;
  }
}

/* ---------------------------------------------------------------------- */

int bf_equal(Bitfield bf1, Bitfield bf2) 
{
  int i, max;

  assert(bf1->bytes == bf2->bytes);

  max = bf1->bytes / sizeof(BFBaseType);

  for (i = 0; i <max; i++)
    if (bf1->field[i] != bf2->field[i])
      return 0;
  return 1;
}

int bf_compare(Bitfield bf1, Bitfield bf2) 
{
  int i, max;

  assert(bf1->bytes == bf2->bytes);

  max = bf1->bytes / sizeof(BFBaseType);

  for (i = 0; i <max; i++) {
    if (bf1->field[i] - bf2->field[i] < 0)
      return -1;
    else if (bf1->field[i] - bf2->field[i] > 0)
      return 1;
  }
  return 0;
}

int nr_bits_set(Bitfield bitfield)
{
  return bitfield->nr_bits_set;
}

