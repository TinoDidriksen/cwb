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
   ntohl() and htonl() functions, either included from <netinet/in.h>
   (or another system library), or with the CWB's own implementation
*/
   

#ifndef _ENDIAN_H_
#define _ENDIAN_H_


/* The CWB currently supports two types of byte ordering, indicated by the preprocessor symbols

   CWB_LITTLE_ENDIAN	LSB first (Intel, Vax, ...)
   CWB_BIG_ENDIAN	MSB first (Network, SUN, 68k, ...)

   The CWB uses _network_ byte order for 32-bit quantities stored in disk files.
   On little-endian machines, data must be converted from big-endian - or "network" - to 
   machine format and back.  This is done through the BSD functions/macros htonl() and ntohl()
   when they are available.  Otherwise, the CWB defines its own macros (which are either
   no-ops or fairly expensive subroutine calls).

   Note that the LITTLE or BIG endian option has to be set even when htonl() and ntohl()
   are available from the system libraries.  This is because conversion loops for integer
   vectors loaded into memory are skipped completely on big-endian systems, rather than
   being loops of no-ops (which the compiler may or may not optimize away).
*/

/*
 * Macros for network/internal number representation conversion.
 */

/* macros ntohl() and htonl() should usually be defined here (or in <arpa/inet.h>?) */
#include <netinet/in.h>

/* use system library macros if available, which should be no-ops on big-endian machines
   and compiled to efficient machine code instructions on little-endian machines */

#if !( defined(ntohl) && defined(htonl) ) 

/* In the CWB_BIG_ENDIAN case, network byte order and machine byte order
   are the same, so we simply define dummy macros. */

#ifdef CWB_BIG_ENDIAN

#define	ntohl(x)	(x)
#define	htonl(x)	(x)

#else

#define ntohl(x) cwb_bswap32(x)
#define htonl(x) cwb_bswap32(x)

#endif

#endif

/* This function is a portable bswap implementation of the macros ntohl() and htonl(),
   which is used on little-endian systems where the macros are not available in the 
   system libraries.  This function is visible on all platforms, so it is possible
   to convert integers explicitly to a little-endian format (such functionality is
   provided by cwb-itoa and cwb-atoi, for instance). */
int cwb_bswap32(int x);		/* internal function defined in endian.c */

#endif /* _ENDIAN_H_ */
