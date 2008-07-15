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

#include <sys/types.h>

#include "globals.h"

#include "endian.h"

#include "bitio.h"

/* ================================================== IMPLEMENTATION */

int BFopen(char *filename, char *type, BFile *bf)
{
  bf->fd = fopen(filename, type);
  bf->buf = '\0';
  bf->bits_in_buf = 0;
  bf->mode = type[0];
  bf->position = 0;

  assert((bf->mode == 'r') || (bf->mode == 'w'));

  return (bf->fd ? 1 : 0);
}

int BSopen(unsigned char *base, char *type, BStream *bf)
{
  bf->base = base;
  bf->buf = '\0';
  bf->bits_in_buf = 0;
  bf->mode = type[0];
  bf->position = 0;

  assert((bf->mode == 'r') || (bf->mode == 'w'));

  return (bf->base ? 1 : 0);
}

int BFclose(BFile *stream)
{
  if (stream->mode == 'w') 
    BFflush(stream);
  
  return (fclose(stream->fd) == 0 ? 1 : 0);
}

int BSclose(BStream *stream)
{
  if (stream->mode == 'w') 
    BSflush(stream);
  stream->base = NULL;

  return 1;
}

int BFflush(BFile *stream)
{
  int retval;

  retval = 0;

  if (stream->mode == 'w') {
    if ((stream->bits_in_buf > 0) && 
	(stream->bits_in_buf < 8)) {
      
      stream->buf <<= (8 - stream->bits_in_buf);
      fwrite(&stream->buf, sizeof(unsigned char), 1, stream->fd);
      stream->position++;
      
      if (fflush(stream->fd) == 0)
	retval = 1;

      stream->buf = '\0';
      stream->bits_in_buf = 0;

    }
    else
      assert(stream->bits_in_buf == 0);
  }
  else if (stream->mode == 'r') {
    if (fread(&stream->buf, sizeof(unsigned char), 1, stream->fd) == 1)
      retval = 1;
    stream->bits_in_buf = 8;
    stream->position++;
  }
  else
    assert(0 && "Illegal BitFile mode");

  return retval;
}

int BSflush(BStream *stream)
{
  int retval;

  retval = 0;

  if (stream->mode == 'w') {
    if ((stream->bits_in_buf > 0) && 
	(stream->bits_in_buf < 8)) {
      
      stream->buf <<= (8 - stream->bits_in_buf);
      stream->base[stream->position] = stream->buf;
      stream->position++;
      
      retval = 1;

      stream->buf = '\0';
      stream->bits_in_buf = 0;

    }
    else
      assert(stream->bits_in_buf == 0);
  }
  else if (stream->mode == 'r') {
    stream->buf = stream->base[stream->position];
    retval = 1;
    stream->bits_in_buf = 8;
    stream->position++;
  }
  else
    assert(0 && "Illegal BitFile mode");

  return retval;
}

int BFwrite(unsigned char data, int nbits, BFile *stream)
{

  unsigned char mask;

  mask = 1 << (nbits - 1);

  while (nbits > 0) {
  
    assert(mask);
    assert(stream->bits_in_buf < 8);

    stream->bits_in_buf++;
    stream->buf <<= 1;

    if (data & mask) 
      stream->buf |= 1;
    
    if (stream->bits_in_buf == 8) {
      if (fwrite(&stream->buf, sizeof(unsigned char), 1, stream->fd) != 1)
	return 0;
      stream->position++;
      stream->buf = 0;
      stream->bits_in_buf = 0;
    }

    nbits--;
    mask >>= 1;
  }
  return 1;
}

int BSwrite(unsigned char data, int nbits, BStream *stream)
{
  unsigned char mask;

  mask = 1 << (nbits - 1);

  while (nbits > 0) {
  
    assert(mask);
    assert(stream->bits_in_buf < 8);

    stream->bits_in_buf++;
    stream->buf <<= 1;

    if (data & mask) 
      stream->buf |= 1;
    
    if (stream->bits_in_buf == 8) {
      stream->base[stream->position] = stream->buf;
      stream->position++;
      stream->buf = 0;
      stream->bits_in_buf = 0;
    }

    nbits--;
    mask >>= 1;
  }
  return 1;
}

int BFread(unsigned char *data, int nbits, BFile *stream)
{
  *data = '\0';

  while (nbits > 0) {
  
    if (stream->bits_in_buf == 0) {
      if (fread(&stream->buf, sizeof(unsigned char), 1, stream->fd) != 1)
	return 0;
      stream->position++;
      stream->bits_in_buf = 8;
    }

    *data <<= 1;
    if (stream->buf & (1<<7))
      *data |= 1;

    stream->buf <<= 1;
    stream->bits_in_buf--;
    
    nbits--;
  }
  return 1;
}

int BSread(unsigned char *data, int nbits, BStream *stream)
{
  *data = '\0';

  while (nbits > 0) {
  
    if (stream->bits_in_buf == 0) {
      stream->buf = stream->base[stream->position];
      stream->position++;
      stream->bits_in_buf = 8;
    }

    *data <<= 1;
    if (stream->buf & (1<<7))
      *data |= 1;

    stream->buf <<= 1;
    stream->bits_in_buf--;
    
    nbits--;
  }
  return 1;
}



int BFwriteWord(unsigned int data, int nbits, BFile *stream)
{
  int bytes, rest, i;
  unsigned char *cdata;

  if ((nbits > 32) || (nbits < 0)) {
    fprintf(stderr, "bitio.o/BFwriteWord: nbits (%d) not in legal bounds\n", nbits);
    return 0;
  }

  cdata = (unsigned char *)&data;

  /* We need to normalise this to Network format in order to create
     the same bitstream on all machines!!
     This code is extremely ugly. */
  data = htonl(data);

  bytes = nbits / 8;
  rest  = nbits % 8;

  if (rest)
    if (!BFwrite(cdata[3-bytes], rest, stream))
      return 0;

  for (i = 4 - bytes; i < 4; i++)
    if (!BFwrite(cdata[i], 8, stream))
      return 0;

  return 1;
}

int BFreadWord(unsigned int *data, int nbits, BFile *stream)
{
  int bytes, rest, i;
  unsigned char *cdata;

  if ((nbits > 32) || (nbits < 0)) {
    fprintf(stderr, "bitio.o/BFreadWord: nbits (%d) not in legal bounds\n", nbits);
    return 0;
  }

  cdata = (unsigned char *)data;

  bytes = nbits / 8;
  rest  = nbits % 8;

  if (rest)
    if (!BFread(cdata + 3 - bytes, rest, stream))
      return 0;

  for (i = 4 - bytes; i < 4; i++)
    if (!BFread(cdata + i, 8, stream))
      return 0;

  /* As in BFwriteWord, the above code assumes that integers are 4 bytes long
     and stored in LSB first fashion. To avoid rewriting the whole code, we just
     convert from this Network byte-order to the platform's native byte-order
     in the end (which assumes that ints are 4 bytes ... but hey, we've got to 
     live with that in the CWB! */
  *data = ntohl(*data);

  return 1;
}

/* I'm just glad Oli didn't implement BSwriteWord() ... */


int BFposition(BFile *stream)
{
  assert(stream);
  return stream->position;
}

int BSposition(BStream *stream)
{
  assert(stream);
  return stream->position;
}

int BSseek(BStream *stream, off_t offset)
{
  stream->buf = '\0';
  stream->bits_in_buf = 0;
  stream->position = offset;
  return 1;
}
