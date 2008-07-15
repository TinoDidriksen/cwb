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

typedef struct _bfilebuf {
  FILE *fd;
  char mode;
  unsigned char buf;
  int bits_in_buf;
  off_t position;
} BFile;

typedef struct _bstreambuf {
  unsigned char *base;
  char mode;
  unsigned char buf;
  int bits_in_buf;
  off_t position;
} BStream;

/* both return 1 on success, 0 on failure (not like fopen/fclose) */

int BFopen(char *filename, char *type, BFile *bf);
int BFclose(BFile *stream);

int BSopen(unsigned char *base, char *type, BStream *bf);
int BSclose(BStream *stream);

/* flushes in case of output stream, even incomplete byte (so the next
 * one begins at a new byte), and skips to the next input byte for
 * input streams. */

int BFflush(BFile *stream);
int BFwrite(unsigned char data, int nbits, BFile *stream);

int BSflush(BStream *stream);
int BSwrite(unsigned char data, int nbits, BStream *stream);

/* ============================================================ 
 * NOTE: be sure that you read the data into an unsigned char !
 * ============================================================ */

int BFread(unsigned char *data, int nbits, BFile *stream);
int BSread(unsigned char *data, int nbits, BStream *stream);


/* the next two read nbits into an unsigned int, padded to the right */

int BFwriteWord(unsigned int data, int nbits, BFile *stream);

int BFreadWord(unsigned int *data, int nbits, BFile *stream);

int BFposition(BFile *stream);
int BSposition(BStream *stream);

int BSseek(BStream *stream, off_t offset);

#endif
