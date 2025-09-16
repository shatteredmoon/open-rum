/***************************************************************************\
|* platform.h -- Platform specific items of general interest (such as      *|
|* #includes and whatnot).  This should be included by any file that needs *|
|* platform specific items.  Anything platform specific should be moved in *|
|* here.                                                                   *|
|***************************************************************************|
|* Special defines: WANTS_NETINCLUDES   If defined, will do all the system *|
|*                                      dependant network includes.        *|
\***************************************************************************/

#ifndef _RUM_PLATFORM_H_
#define _RUM_PLATFORM_H_

#include <stdint.h>

using rumByte  = uint8_t;
using rumWord  = uint16_t;
using rumDWord = uint32_t;
using rumQWord = uint64_t;

#define PLATFORM_STRING_WINDOWS "windows"
#define PLATFORM_STRING_LINUX   "linux"
#define PLATFORM_STRING_MAC     "mac"
#define PLATFORM_STRING_UNKNOWN "unknown"

// If adding a platform, be sure to give it an enumerated value here
enum PlatformType { PLATFORM_UNKNOWN = 0, PLATFORM_WINDOWS, PLATFORM_LINUX, PLATFORM_MAC };

const char* GetPlatformString( PlatformType ePlatform );
PlatformType GetPlatformType( const char* strPlatform );

/* System specific includes */

#ifdef _MSC_VER
  // not #if defined(_WIN32) || defined(_WIN64) because we have strncasecmp in mingw
  #define strncasecmp _strnicmp
  #define strcasecmp  _stricmp
  #define ltoa        _ltoa
  #define lltoa       _i64toa
  #define atoi64      _atoi64
#endif // _MSC_VER

#ifdef WIN32

  #define PLATFORM_TYPE       PLATFORM_WINDOWS
  #define DEFAULT_PATH_BIN    ""
  #define DEFAULT_PATH_ETC    ""
  #define DEFAULT_PATH_SHARE  ""
  #define DEFAULT_PATH_LOG    ""

  #define DEFAULT_CONFIG_EXT  ".ini"
  #define DEFAULT_BIN_EXT     ".exe"

  // Prevent inclusion of winsock.h in windows.h
  #define _WINSOCKAPI_
  #define WIN32_LEAN_AND_MEAN

  #ifndef RUM_EDITOR
    #include <winsock2.h>
    typedef int socklen_t;
  //#else
  //  #ifndef SOCKET
  //  #define SOCKET unsigned int
  //  #endif
  #endif

  #ifdef RUM_PORTAL
  #include <process.h>
  #endif

#else /* UNIX needs these things */

  #define PLATFORM_TYPE   PLATFORM_LINUX

  #ifndef RUM_EDITOR
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <inttypes.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <netdb.h>
    #include <errno.h>
  #endif

  #include "config.h"

  #define DEFAULT_CONFIG_EXT  ""
  #define DEFAULT_BIN_EXT     ""

  #ifndef WORD
  #define WORD unsigned short
  #endif

  #ifndef SOCKET
  #define SOCKET unsigned int
  #endif

  #ifndef INVALID_SOCKET
  #define INVALID_SOCKET -1
  #endif

  #ifndef SOCKET_ERROR
  #define SOCKET_ERROR -1
  #endif

  #ifndef SD_SEND
  #define SD_SEND 1
  #endif

  #ifndef FALSE
  #define FALSE 0
  #endif

  #ifndef TRUE
  #define TRUE 1
  #endif

  #ifndef LARGE_INTEGER
  #define LARGE_INTEGER long long
  #endif

  #define closesocket(s) close(s)

  // At least my flavor of unix (solaris) needs an itoa and ltoa implementation
  char* itoa( int value, char* string, int radix );
  char* ltoa( long value, char* string, int radix );

  // Network defines, to make the patcher work nicer
  #ifndef SOCKADDR_IN
    #define SOCKADDR_IN struct sockaddr_in
  #endif

  #ifndef LPSOCKADDR
  #define LPSOCKADDR struct sockaddr*
  #endif

  #ifndef IN_ADDR
  #define IN_ADDR struct in_addr
  #endif

  #ifndef LPIN_ADDR
  #define LPIN_ADDR struct in_addr*
  #endif

  #ifndef LPHOSTENT
  #define LPHOSTENT struct hostent*
  #endif

  #ifndef IPPROTO_TCP
  #define IPPROTO_TCP PF_INET
  #endif

  #ifndef INADDR_NONE
  #define INADDR_NONE -1
  #endif

#endif // UNIX

#endif // _RUM_PLATFORM_H_
