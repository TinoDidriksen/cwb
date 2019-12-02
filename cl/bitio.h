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


#ifndef _BITIO_H_
#define _BITIO_H_

#include <sys/types.h>

#include "globals.h"

/**
 * File buffer for bit input / output.
 */
typedef struct _bfilebuf {
  FILE *fd;
  char mode;
  uint8_t buf;
  int64_t bits_in_buf;
  off_t position;
} BFile;

/**
 * Stream buffer for bit input / output.
 */
typedef struct _bstreambuf {
  uint8_t *base;
  char mode;
  uint8_t buf;
  int64_t bits_in_buf;
  off_t position;
} BStream;



bool BFopen(char *filename, char *type, BFile *bf);
bool BFclose(BFile *stream);

bool BSopen(uint8_t *base, char *type, BStream *bf);
void BSclose(BStream *stream);

bool BFflush(BFile *stream);
bool BSflush(BStream *stream);

bool BFwrite(uint8_t data, int64_t nbits, BFile *stream);
bool BSwrite(uint8_t data, int64_t nbits, BStream *stream);

bool BFread(uint8_t *data, int64_t nbits, BFile *stream);
bool BSread(uint8_t *data, int64_t nbits, BStream *stream);

bool BFwriteWord(uint64_t data, int64_t nbits, BFile *stream);

bool BFreadWord(uint64_t *data, int64_t nbits, BFile *stream);

int64_t BFposition(BFile *stream);
int64_t BSposition(BStream *stream);

void BSseek(BStream *stream, off_t offset);

#endif
