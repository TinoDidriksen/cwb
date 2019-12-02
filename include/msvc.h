#ifndef __ms_vc_h
#define __ms_vc_h

// warning C4005: macro definition
#pragma warning (disable: 4005)
// warning C4100: unreferenced formal parameter
#pragma warning (disable: 4100)
// warning C4996: POSIX function names
#pragma warning (disable: 4996)

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
