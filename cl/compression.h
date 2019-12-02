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

#ifndef _COMPRESSION_H_
#define _COMPRESSION_H_

#include "globals.h"

#include "bitio.h"

int64_t compute_ba(int64_t ft, int64_t corpus_size);

int64_t read_golomb_code_bs(int64_t b, BStream *bs);
int64_t read_golomb_code_bf(int64_t b, BFile *bf);

void write_golomb_code(int64_t x, int64_t b, BFile *bf);

#endif
