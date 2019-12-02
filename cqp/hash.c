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

#include <xxhash.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

int64_t 
is_prime(int64_t n) {
  int64_t i;
  for(i = 2; i*i <= n; i++)
    if ((n % i) == 0) 
      return 0;
  return 1;
}

int64_t 
find_prime(int64_t n) {
  for( ; n > 0 ; n++)
    if (is_prime(n)) 
      return n;
  return 0;
}

int64_t
hash_string(char *string) {
  size_t len = strlen(string);
  int64_t result = llabs(XXH64(string, len, 0));
  return result;
}

int64_t
hash_macro(char *macro_name, int64_t args) {
  size_t len = strlen(macro_name);
  int64_t result = llabs(XXH64(macro_name, len, args));
  return result;
}
