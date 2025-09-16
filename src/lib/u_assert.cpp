#include <u_assert.h>

#include <platform.h>

#ifdef _DEBUG

#include <md5.h>
#include <unordered_set>

#ifdef WIN32
#include <Windows.h>
#endif // WIN32

#ifdef _MSC_VER
  #define DEBUG_BREAK __debugbreak()
#else
  #include <sigtrap.h>
  raise( SIGTRAP );
#endif // _MSC_VER


namespace
{
  std::unordered_set<std::string> g_setIgnoredAsserts;
}


void AssertDlg( const char* i_strText, const char* i_strCaption )
{
  // Build a hash of the string to determine if it's an assertion that has already been ignored
  MD5 md5Hash;
  md5Hash.update( (const unsigned char*)i_strText, (int32_t)strlen( i_strText ) );
  md5Hash.finalize();

  // The 33-byte MD5 digest
  std::string strDigest( md5Hash.hex_digest() );

  // The assertion has been ignored if the digest already exists in the hash set
  if( g_setIgnoredAsserts.find( strDigest ) != g_setIgnoredAsserts.end() )
  {
    // This assert has been ignored
    return;
  }

#ifdef WIN32
  int32_t iResult{ MessageBox( NULL, i_strText, i_strCaption,
                               MB_ABORTRETRYIGNORE | MB_ICONERROR | MB_DEFBUTTON2 | MB_SYSTEMMODAL |
                               MB_SETFOREGROUND ) };
  switch( iResult )
  {
    case IDABORT:
      exit( 0 );
      break;

    case IDRETRY:
      DEBUG_BREAK;
      break;

    case IDIGNORE:
      // Add the MD5 digest to the ignored hash set
      g_setIgnoredAsserts.insert( strDigest );
      break;
  }
#endif // WIN32
}

#endif // _DEBUG