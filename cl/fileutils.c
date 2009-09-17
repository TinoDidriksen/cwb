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


#include <sys/stat.h>
#include <fcntl.h>

#include "globals.h"

#include "fileutils.h"


off_t
file_length(char *filename)
{
  struct stat stat_buf;
  if(stat(filename, &stat_buf) == EOF) return(EOF);
  else return(stat_buf.st_size);
}

off_t
fd_file_length(FILE *fd)
{
  struct stat stat_buf;
  if (fstat(fileno(fd), &stat_buf) == EOF) return(EOF);
  else return(stat_buf.st_size);
}

off_t
fi_file_length(int fileno)
{
  struct stat stat_buf;
  if(fstat(fileno, &stat_buf) == EOF) return(EOF);
  else return(stat_buf.st_size);
}

long
fprobe(char *fname)
{
  struct stat stat_buf;
  
  if(stat(fname, &stat_buf) == EOF) {
    return (long) EOF;
  }
  else
    /* stat_buf->st_mode holds the permission */
    /* we return the file size */
    return stat_buf.st_size;
}

int
is_directory(char *path)
{
  struct stat sBuf;

  if (stat(path, &sBuf) < 0) {
    return 0;
  }
  else {
    return S_ISDIR(sBuf.st_mode) ? 1 : 0;
  }
}

int
is_file(char *path)
{
  struct stat sBuf;

  if (stat(path, &sBuf) < 0) {
    return 0;
  }
  else {
    return S_ISREG(sBuf.st_mode) ? 1 : 0;
  }
}

int
is_link(char *path)
{
  struct stat sBuf;
  
  if (stat(path, &sBuf) < 0) {
    return 0;
  }
  else {
    return S_ISLNK(sBuf.st_mode) ? 1 : 0;
  }
}

