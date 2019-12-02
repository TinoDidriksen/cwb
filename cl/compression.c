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

#include <math.h>

#include "globals.h"

#include "bitio.h"

#include "compression.h"


#if __STDC_VERSION__ >= 199901L
/* C99 has log2() already, but many pre-C99 Cs don't */
#else
#define log2(x) (log(x)/log(2.0))
#endif

/**
 * {I have no idea what this does -- AH}
 *
 * @param ft          ??
 * @param corpus_size ??
 * @return            ??
 */
int64_t
compute_ba(int64_t ft, int64_t corpus_size)
{
  double p;
  int64_t pa;
  
  p = (ft * (double)1.0)/(corpus_size * (double)1.0);
  pa = ceil(-log(2.0-p)/log(1.0-p));

  /* safety check for high-frequency IDs, such as the NIL Id
     in sparse attributes (evert, 01 Mar 99) */
  if (pa > 1) 
    return pa;
  else
    return 2;
}

/*
 * Version with Alistair Moffat's comments worked in 
 * Tue Jan 24 10:07:12 1995 (oli)
 */
/**
 * Writes an integer to a Golomb-coded bit-file-buffer.
 *
 * @param x   Integer to write
 * @param b   ???
 * @param bf  The bit-file to read from.
 * @return    Always 1.
 */
void
write_golomb_code(int64_t x, int64_t b, BFile *bf)
{
  int64_t q, res, lb, ub, nr_sc, i;

  uint8_t bit1 = '\1';
  uint8_t bit0 = '\0';

  q = x / b;
  res = x - q * b;


  ub = ceil(log2(b * 1.0));
  lb = ub - 1;

  /* write the unary part q */

  for (i = 0; i < q; i++)
    BFwrite(bit1, 1, bf);
  BFwrite(bit0, 1, bf);


  /* write the binary part */

  /* nr_sc = 2^ub - b */

  nr_sc = (1ll << ub) - b;
  
  if (res < nr_sc)
    BFwriteWord((uint64_t)res, lb, bf);
  else
    BFwriteWord((uint64_t)(res + nr_sc), ub, bf);
}

/**
 * Reads an integer from a Golomb-coded bit-file-buffer.
 *
 * @param b   ???
 * @param bf  The bit-file to read from.
 * @return    The integer that is read.
 */
int64_t read_golomb_code_bf(int64_t b, BFile *bf)
{
  int64_t q, i, nr_sc, lb, ub;

  uint64_t r;
  uint8_t bit;

  ub = ceil(log2(b * 1.0));
  lb = ub - 1;

  /* read unary part */

  q = 0;
  do {
    BFread(&bit, 1, bf);
    if (bit)
      q++;
  } while (bit);

  nr_sc = (1ll << ub) - b;
  
  /* read binary part, bitwise */

  r = 0;
  for (i = 0; i < lb; i++) {
    r <<= 1;
    BFread(&bit, 1, bf);
    r |= bit;
  }

  if (r >= nr_sc) {
    r <<= 1;
    BFread(&bit, 1, bf);
    r |= bit;
    r -= nr_sc;
  }

  return r + q * b;
}


/**
 * Reads an integer from a Golomb-coded bitstream.
 *
 * @param b   ???
 * @param bs  The bitstream to read from.
 * @return    The integer that is read.
 */
int64_t
read_golomb_code_bs(int64_t b, BStream *bs)
{
  int64_t q, i, nr_sc, lb, ub;

  uint64_t r;
  uint8_t bit;

  ub = ceil(log2(b * 1.0));
  lb = ub - 1;

  /* read unary part */

  q = 0;
  do {
    BSread(&bit, 1, bs);
    if (bit)
      q++;
  } while (bit);

  nr_sc = (1ll << ub) - b;
  
  /* read binary part, bitwise */

  r = 0;
  for (i = 0; i < lb; i++) {
    r <<= 1;
    BSread(&bit, 1, bs);
    r |= bit;
  }

  if (r >= nr_sc) {
    r <<= 1;
    BSread(&bit, 1, bs);
    r |= bit;
    r -= nr_sc;
  }

  return r + q * b;
}
