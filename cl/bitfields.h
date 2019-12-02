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


#ifndef _bitfields_h
#define _bitfields_h

#include "globals.h"

#include <limits.h>

typedef uint8_t BFBaseType;

/**
 * The Bitfield object.
 */
typedef struct {
  int64_t elements;         /**< The number of bits in the bitfield */
  int64_t bytes;            /**< The number of bytes the bitfield occupies */
  int64_t nr_bits_set;      /**< The number of bits whose value has been assigned. Initialised to 0. */
  BFBaseType *field;    /**< the bitfield data itself. All elements initialised to 0. */
} BFBuf, *Bitfield;



Bitfield create_bitfield(int64_t nr_of_elements);

Bitfield copy_bitfield(Bitfield source);

int64_t destroy_bitfield(Bitfield *bptr);

int64_t set_bit(Bitfield bitfield, int64_t element);

int64_t clear_bit(Bitfield bitfield, int64_t element);

int64_t clear_all_bits(Bitfield bitfield);

int64_t set_all_bits(Bitfield bitfield);

int64_t get_bit(Bitfield bitfield, int64_t element);

int64_t toggle_bit(Bitfield bitfield, int64_t element);

int64_t nr_bits_set(Bitfield bitfield);

int64_t bf_equal(Bitfield bf1, Bitfield bf2);

int64_t bf_compare(Bitfield bf1, Bitfield bf2);

#endif
