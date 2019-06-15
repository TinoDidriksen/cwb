#ifndef __ms_vc_h
#define __ms_vc_h

#include <winsock2.h>

#define popen _popen
#define pclose _pclose
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#define access _access

#define S_ISDIR(x) ((x) & S_IFDIR)
#define S_ISREG(x) ((x) & S_IFREG)

#define R_OK 4

#endif
