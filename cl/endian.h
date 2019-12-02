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

/* endian.h
   is now part of the CWB. It attempts to hide the details of integer
   storage from the Corpus Library code. In particular, it provides the
   ntohll() and htonll() functions, either included from <netinet/in.h>
   (or another system library), or with the CWB's own implementation
*/


#ifndef _ENDIAN_H_
#define _ENDIAN_H_

/**
 * @file
 *
 * globals.h checks whether endianness is well-defined.
 *
 * The CWB uses _network_ byte order (i.e. Most Significant Byte First)
 * for 64-bit integers stored in its platform-independent disk files.
 *
 * Conversion to and from processor-native byte order is handled by the
 * BSD functions/macros htonll() and ntohll(), which must be available in
 * the system library.
 */

#include <stdint.h>

/*
 * Macros for network/internal number representation conversion.
 */

#ifndef __MINGW__
/* macros ntohll() and htonll() should usually be defined here (or in <arpa/inet.h>?) */
#include <netinet/in.h>
#else
/* but in windows we need to use winsock2.h (poss also windows.h?) from the Windows API instead */
#include <windows.h>
#include <winsock2.h>
/* note, in order for all this to work, it is necessary to link against ws2_32 in MinGW. */
#endif

/* rely on system library macros ntohll() and htonll(), so we don't have to work out endianness of the current architecture;
   this is essential for Universal builds on Mac OS X, and should also be the most efficient solution because these
   macros are no-ops on big-endian machines and compiled to efficient machine code instructions on little-endian machines

   (whether any of the above is true for Windows is an open question...)
*/

/* -- skip check for availability of macros, since it breaks during "make depend" on some Linux platforms

#if !( defined(ntohll) && defined(htonll) )
#error Sorry, ntohll() and htonll() macros are required by the CWB code, but do not seem to be defined on your machine.
#endif

*/


int64_t cl_bswap64(int64_t x);

#endif /* _ENDIAN_H_ */
