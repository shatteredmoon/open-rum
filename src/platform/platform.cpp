/***************************************************************************\
|* platform.cpp -- Platform specific items of general interest (such as    *|
|* #includes and whatnot).  This should be included by any file that needs *|
|* platform specific items.  Anything platform specific should be moved in *|
|* here.                                                                   *|
\***************************************************************************/

#include <platform.h>

const char* GetPlatformString( PlatformType ePlatform )
{
  switch( ePlatform )
  {
    case PLATFORM_WINDOWS: return PLATFORM_STRING_WINDOWS;
    case PLATFORM_LINUX:   return PLATFORM_STRING_LINUX;
    case PLATFORM_MAC:     return PLATFORM_STRING_MAC;
  }

  return PLATFORM_STRING_UNKNOWN;
}


PlatformType GetPlatformType( const char* strPlatform )
{
  if( strPlatform != NULL )
  {
    if( strncmp( strPlatform, PLATFORM_STRING_WINDOWS, strlen( PLATFORM_STRING_WINDOWS ) ) == 0 )
    {
      return PLATFORM_WINDOWS;
    }
    else if( strncmp( strPlatform, PLATFORM_STRING_LINUX, strlen( PLATFORM_STRING_LINUX ) ) == 0 )
    {
      return PLATFORM_LINUX;
    }
    else if( strncmp( strPlatform, PLATFORM_STRING_MAC, strlen( PLATFORM_STRING_MAC ) ) == 0 )
    {
      return PLATFORM_MAC;
    }
  }

  return PLATFORM_UNKNOWN;
}

#ifndef WIN32

#include <sys/time.h>
#include <time.h>
#include <stdlib.h>

extern time_t server_start_time;

/*
 * An itoa implementation based off one taken from the web
 * copyright DJ Delorie.
 */
char* itoa( int value, char* string, int radix )
{
  char tmp[33];
  char* tp = tmp;
  int i;
  unsigned v;
  int sign;
  char* sp;

  if( radix > 36 || radix <= 1 )
  {
    return (char*)NULL;
  }

  sign = ( radix == 10 && value < 0 );
  if( sign )
  {
    v = -value;
  }
  else
  {
    v = (unsigned)value;
  }

  while( v || tp == tmp )
  {
    i = v % radix;
    v = v / radix;
    if( i < 10 )
    {
      *tp++ = i + '0';
    }
    else
    {
      *tp++ = i + 'a' - 10;
    }
  }

  if( string == 0 )
  {
    string = (char*)malloc( ( tp - tmp ) + sign + 1 );
  }

  sp = string;

  if( sign )
  {
    *sp++ = '-';
  }

  while( tp > tmp )
  {
    *sp++ = *--tp;
  }

  *sp = 0;

  return string;
}

/*
 * A ltoa implemetnation based off one taken off the web;
 * copyright DJ Delorie.
 */
char*
ltoa( long value, char* string, int radix )
{
  char tmp[33];
  char* tp = tmp;
  long i;
  unsigned long v;
  int sign;
  char* sp;

  if( radix > 36 || radix <= 1 )
  {
    return 0;
  }

  sign = ( radix == 10 && value < 0 );
  if( sign )
  {
    v = -value;
  }
  else
  {
    v = (unsigned long)value;
  }

  while( v || tp == tmp )
  {
    i = v % radix;
    v = v / radix;
    if( i < 10 )
    {
      *tp++ = i + '0';
    }
    else
    {
      *tp++ = i + 'a' - 10;
    }
  }

  if( string == 0 )
    string = (char*)malloc( ( tp - tmp ) + sign + 1 );
  sp = string;

  if( sign )
  {
    *sp++ = '-';
  }

  while( tp > tmp )
  {
    *sp++ = *--tp;
  }

  *sp = 0;
  return string;
}

/* These two functions are normally windows only, but I took them from
 * WINE so we can emulate them here.
 */
int QueryPerformanceCounter( long long* counter )
{
  struct timeval now;

  gettimeofday( &now, 0 );

  *counter = ( ( ( now.tv_sec - server_start_time ) * ( unsigned long long )1000000 + now.tv_usec ) * 105 ) / 88;

  return 1;
}

int QueryPerformanceFrequency( long long* rate )
{
  *rate = 1193182;
  return 1;
}

#endif // !WIN32
