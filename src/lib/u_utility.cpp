#include <u_utility.h>

#include <u_script.h>

#include <charconv>
#include <filesystem>
#include <random>

/* UNIX needs ctype */
#ifndef WIN32
#include <ctype.h>
#else
#include <windows.h>
#endif

#include <md5.h>


namespace rumFileUtils
{
  std::string AppendPathSeparator( const std::string& i_strPath )
  {
    std::filesystem::path fsPath( i_strPath );
    fsPath += std::filesystem::path::preferred_separator;
    return fsPath.lexically_normal().u8string();
  }


  bool FileExists( const std::string& i_strFilePath )
  {
    return FileExists( i_strFilePath, {} );
  }


  bool FileExists( const std::string& i_strFilePath, const std::string& i_strHash )
  {
    bool bExists{ false };

    const std::filesystem::path fsPath( i_strFilePath );
    if( std::filesystem::exists( fsPath ) )
    {
      if( i_strHash.empty() )
      {
        bExists = true;
      }
      else
      {
        std::ifstream cInfile;
        cInfile.open( i_strFilePath.c_str(), std::ios::binary );
        if( cInfile.is_open() )
        {
          MD5 cMD5( cInfile );
          const char* strHash{ cMD5.hex_digest() };
          if( i_strHash.compare( strHash ) == 0 )
          {
            bExists = true;
          }

          cInfile.close();
        }
      }
    }

    return bExists;
  }


  std::string GetCleanFilePath( const std::string& i_strFilePath )
  {
    const std::filesystem::path fsPath( i_strFilePath );
    return fsPath.lexically_normal().u8string();
  }


  std::string GetCurrentDirectory()
  {
    return std::filesystem::current_path().u8string();
  }


  std::vector<std::string> GetDriveList()
  {
    std::vector<std::string> cDriveVector;
    char* strDrives{ new char[MAX_PATH]() };
#ifdef WIN32
    if( GetLogicalDriveStringsA( MAX_PATH, strDrives ) )
#else
#error DEFINE FOR YOUR OS
#endif
    {
      for( int i = 0; i < 104; i += 4 )
      {
        if( strDrives[i] != '\0' )
        {
          cDriveVector.push_back( std::string{ strDrives[i], strDrives[i + 1], strDrives[i + 2] } );
        }
      }
    }

    delete[] strDrives;
    return cDriveVector;
  }
 

  Sqrat::Array GetDriveListForScript()
  {
    const auto cDriveVector{ GetDriveList() };

    HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

    // Convert the vector to something scripts can use
    Sqrat::Array sqArray( pcVM, cDriveVector.size() );

    int32_t iIndex{ 0 };
    for( const auto& strFolder : cDriveVector )
    {
      sqArray.SetValue( iIndex++, strFolder.c_str() );
    }

    return sqArray;
  }


  std::string GetExtension( const std::string& i_strFilePath )
  {
    const std::filesystem::path fsPath( i_strFilePath );
    return fsPath.extension().u8string();
  }


  std::string GetFilename( const std::string& i_strFilePath )
  {
    const std::filesystem::path fsPath( i_strFilePath );
    return fsPath.filename().u8string();
  }


  std::vector<std::string> GetFilesFromPath( const std::string& i_strPath )
  {
    const std::filesystem::path fsPath( i_strPath );
    if( !std::filesystem::exists( fsPath ) || !std::filesystem::is_directory( fsPath ) )
    {
      std::string strError{ "The path " };
      strError += fsPath.generic_string();
      strError += " is invalid";
      Logger::LogStandard( strError );

      return {};
    }

    std::vector<std::string> cFileVector;

    std::filesystem::directory_iterator iter( fsPath, std::filesystem::directory_options::skip_permission_denied );
    const std::filesystem::directory_iterator end;
    while( iter != end )
    {
      if( std::filesystem::is_regular_file( iter->status() ) )
      {
        std::string strDirectory( iter->path().string() );
        if( !( IsHiddenFile( strDirectory ) || IsSystemFile( strDirectory ) ) )
        {
          cFileVector.push_back( iter->path().filename().u8string() );
        }
      }

      ++iter;
    }

    std::sort( cFileVector.begin(), cFileVector.end() );

    return cFileVector;
  }


  Sqrat::Array GetFilesFromPathForScript( const std::string& i_strPath )
  {
    const auto cFileVector{ GetFilesFromPath( i_strPath ) };

    HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

    // Convert the vector to something scripts can use
    Sqrat::Array sqArray( pcVM, cFileVector.size() );

    int32_t iIndex{ 0 };
    for( const auto& strFile : cFileVector )
    {
      sqArray.SetValue( iIndex++, strFile.c_str() );
    }

    return sqArray;
  }


  std::vector<std::string> GetFoldersFromPath( const std::string& i_strPath )
  {
    const std::filesystem::path fsPath( i_strPath );
    if( !std::filesystem::exists( fsPath ) || !std::filesystem::is_directory( fsPath ) )
    {
      std::string strError{ "The path " };
      strError += fsPath.generic_string();
      strError += " is invalid";
      Logger::LogStandard( strError );

      return {};
    }

    std::vector<std::string> cFolderVector;

    std::filesystem::directory_iterator iter( fsPath, std::filesystem::directory_options::skip_permission_denied );
    const std::filesystem::directory_iterator end;
    while( iter != end )
    {
      if( std::filesystem::is_directory( iter->status() ) )
      {
        std::string strDirectory( iter->path().string() );
        if( !( IsHiddenFile( strDirectory ) || IsSystemFile( strDirectory ) ) )
        {
          cFolderVector.push_back( iter->path().filename().u8string() );
        }
      }

      ++iter;
    }

    std::sort( cFolderVector.begin(), cFolderVector.end() );

    return cFolderVector;
  }


  Sqrat::Array GetFoldersFromPathForScript( const std::string& i_strPath )
  {
    const auto cFolderVector{ GetFoldersFromPath( i_strPath ) };

    HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

    // Convert the vector to something scripts can use
    Sqrat::Array sqArray( pcVM, cFolderVector.size() );

    int32_t iIndex{ 0 };
    for( const auto& strFolder : cFolderVector )
    {
      sqArray.SetValue( iIndex++, strFolder.c_str() );
    }

    return sqArray;
  }


  std::string GetParentPath( const std::string& i_strFilePath )
  {
    const std::filesystem::path fsPath( i_strFilePath );
    return fsPath.parent_path().u8string();
  }


  std::string GetPath( const std::string& i_strFilePath )
  {
    const std::filesystem::path fsPath( i_strFilePath );
    return fsPath.u8string();
  }


  std::string GetPathSeparator()
  {
    std::filesystem::path fsPath;
    fsPath += std::filesystem::path::preferred_separator;
    return fsPath.lexically_normal().string();
  }


  bool HasParentPath( const std::string& i_strPath )
  {
    const std::filesystem::path fsPath( i_strPath );
    return fsPath.compare( fsPath.root_path() ) != 0;
  }


  bool IsFile( const std::string& i_strFilePath )
  {
    const std::filesystem::path fsPath( i_strFilePath );
    return std::filesystem::is_regular_file( fsPath );
  }


  bool IsFolder( const std::string& i_strPath )
  {
    const std::filesystem::path fsPath( i_strPath );
    return std::filesystem::is_directory( fsPath );
  }


  bool IsHiddenFile( const std::string& i_strPath )
  {
#ifdef WIN32
    const auto ulAttributes{ GetFileAttributes( i_strPath.c_str() ) };
    return ( ulAttributes & FILE_ATTRIBUTE_HIDDEN ) != 0;
#else
#error DEFINE FOR YOUR OS
#endif
  }


  bool IsSystemFile( const std::string& i_strPath )
  {
#ifdef WIN32
    const auto ulAttributes{ GetFileAttributes( i_strPath.c_str() ) };
    return ( ulAttributes & FILE_ATTRIBUTE_SYSTEM ) != 0;
#else
#error DEFINE FOR YOUR OS
#endif
  }
}


namespace rumNumberUtils
{
  static std::mt19937 s_cMersenneTwisterRand( std::mt19937::default_seed );
  static std::mt19937_64 s_cMersenneTwisterRand64( std::mt19937::default_seed );
  static float* s_fSqrtArray{ nullptr };

  void FreeSqrtTable()
  {
    SAFE_ARRAY_DELETE( s_fSqrtArray );
  }


  float GetSqrtValue( uint32_t iTaxiCabDistance )
  {
    return s_fSqrtArray[iTaxiCabDistance];
  }


  float GetRandomFloat32()
  {
    return static_cast<float>( GetRandomUInt32() ) / static_cast<float>( 0xFFFFFFFF );
  }


  float GetRandomFloat32_Range( float i_fMin, float i_fMax )
  {
    const uint32_t uiRandomValue{ GetRandomUInt32() };
    return i_fMin + ( static_cast<float>( uiRandomValue ) * ( i_fMax - i_fMin ) / 4294967295.0f );
  }


  uint32_t GetRandomUInt32()
  {
    return s_cMersenneTwisterRand();
  }


  uint64_t GetRandomUInt64()
  {
    return s_cMersenneTwisterRand64();
  }


  void InitSqrtTable( uint32_t i_uiSize )
  {
    FreeSqrtTable();

    s_fSqrtArray = new float[i_uiSize];
    for( uint32_t i = 0; i < i_uiSize; ++i )
    {
      s_fSqrtArray[i] = sqrt( float( i ) );
    }
  }

  void SetRandomSeed( const uint32_t i_uiSeed )
  {
    s_cMersenneTwisterRand.seed( static_cast<std::mt19937::result_type>( i_uiSeed ) );
  }
} // namespace rumNumberUtils


namespace rumStringUtils
{
  void CopyTextToClipboard( const std::string& i_strText )
  {
#ifdef WIN32
    const size_t iSize{ sizeof( TCHAR ) * ( 1 + i_strText.length() ) };
    const HGLOBAL hResult{ GlobalAlloc( GMEM_MOVEABLE, iSize ) };
    LPTSTR strCopy{ (LPTSTR)GlobalLock( hResult ) };
    memcpy( strCopy, i_strText.c_str(), iSize );
    GlobalUnlock( hResult );

    // Set clipboard data
    if( OpenClipboard( NULL ) )
    {
      EmptyClipboard();

      if( !SetClipboardData( CF_TEXT, hResult ) )
      {
        GlobalFree( hResult );
      }

      CloseClipboard();
    }
#else
#error provide clipboard implementation for your platform here
#endif
  }


  void ScriptCopyTextToClipboard( const SQChar* i_strText )
  {
    CopyTextToClipboard( std::string( i_strText ) );
  }


  const char* CopyTextFromClipboard()
  {
    static std::string strText{};

#ifdef WIN32
    if( IsClipboardFormatAvailable( CF_TEXT ) )
    {
      if( OpenClipboard( NULL ) )
      {
        const HGLOBAL hResult{ GetClipboardData( CF_TEXT ) };
        if( hResult != NULL )
        {
          const LPTSTR strCopy{ (LPTSTR)GlobalLock( hResult ) };
          if( strCopy != NULL )
          {
            strText = std::string( strCopy );
            GlobalUnlock( hResult );
          }
        }

        CloseClipboard();
      }
    }
#else
#error provide clipboard implementation for your platform here
#endif

    return strText.c_str();
  }


  const std::string& NullString()
  {
    static std::string strNull;
    return strNull;
  }


  StringVector ParseCSVRow( const std::string& i_strRow )
  {
    bool bCanEndField{ true };

    StringVector cFieldVector{ "" };
    size_t iIndex{ 0 };

    for( const char c : i_strRow )
    {
      switch( c )
      {
        case '"':
          bCanEndField = !bCanEndField;
          cFieldVector[iIndex].push_back( c );
          break;

        case ',':
          if( bCanEndField )
          {
            cFieldVector.push_back( "" );
            ++iIndex;
          }
          else
          {
            cFieldVector[iIndex].push_back( c );
          }
          break;

        default:
          cFieldVector[iIndex].push_back( c );
          break;
      }
    }

    for( auto& iter : cFieldVector )
    {
      // If this is a quoted field (starts and ends with quotation marks), remove them
      if( ( iter.find_first_of( '"' ) == 0 ) && ( iter.rfind( '"' ) == iter.length() - 1 ) )
      {
        iter.erase( iter.begin() );
        iter.erase( iter.end() - 1 );
      }

      // Convert any remaining double-quotation marks with a single-quotation mark
      rumStringUtils::Replace( iter, "\"\"", "\"" );
    }

    return cFieldVector;
  }


  void Replace( std::string& i_strContext, const std::string& i_strSource, const std::string& i_strDest )
  {
    size_t iSearchOffset{ 0 };
    size_t iFoundOffset{ 0 };
    while( ( iFoundOffset = i_strContext.find( i_strSource, iSearchOffset ) ) != std::string::npos )
    {
      i_strContext.replace( iFoundOffset, i_strSource.size(), i_strDest );
      iSearchOffset = iFoundOffset + i_strDest.size();
    }
  }


  void Split( StringVector& io_cStringVector, const std::string& i_strText, const std::string& i_strDelimiter )
  {
    rumAssert( !i_strText.empty() );
    rumAssert( !i_strDelimiter.empty() );

    if( i_strDelimiter.empty() )
    {
      io_cStringVector.push_back( i_strText );
    }
    else if( !i_strText.empty() )
    {
      size_t iStartOffset{ 0 }, iEndOffset{ 0 };

      while( iEndOffset != std::string::npos )
      {
        iEndOffset = i_strText.find( i_strDelimiter, iStartOffset );

        // If at end use length=maxLength, else use length=end-start
        const auto strToken
        {
          i_strText.substr( iStartOffset, ( ( std::string::npos == iEndOffset )
                                            ? std::string::npos
                                            : iEndOffset - iStartOffset ) )
        };

        io_cStringVector.push_back( strToken );

        // If at end use start=maxSize, else use start=end+delimiter
        iStartOffset = ( ( iEndOffset > ( std::string::npos - i_strDelimiter.size() ) )
                         ? std::string::npos
                         : iEndOffset + i_strDelimiter.size() );
      }
    }
  }


  bool ToBool( std::string i_strVal )
  {
    bool bVal{ false };

    rumStringUtils::ToLower( i_strVal );

    std::istringstream cStream( i_strVal );

    // First, try simple integer conversion
    cStream >> bVal;
    if( cStream.fail() )
    {
      // Try boolean
      cStream.clear();
      cStream >> std::boolalpha >> bVal;
    }

    return bVal;
  }


  float ToFloat( const std::string& i_strVal )
  {
    float fResult;
    std::from_chars( i_strVal.data(), i_strVal.data() + i_strVal.size(), fResult );
    return fResult;
  }


  const char* ToFloatString( float i_fVal )
  {
    static char strBuffer[64];
    snprintf( strBuffer, sizeof strBuffer, "%.5g", i_fVal );
    return strBuffer;
  }


  const char* ToHexString( int32_t i_iVal )
  {
    static char strBuffer[11] = { '0', 'x', '\0' };
    ltoa( i_iVal, &strBuffer[2], 16 );
    return strBuffer;
  }


  const char* ToHexString64( int64_t i_iVal )
  {
    static char strBuffer[19] = { '0', 'x', '\0' };
    lltoa( i_iVal, &strBuffer[2], 16 );
    return strBuffer;
  }


  int32_t ToInt( const std::string& i_strVal )
  {
    if( i_strVal.rfind( "0x", 0 ) == 0 )
    {
      return strtol( i_strVal.c_str(), nullptr, 16 );
    }

    return strtol( i_strVal.c_str(), nullptr, 10 );
  }


  int64_t ToInt64( const std::string& i_strVal )
  {
    return atoi64( i_strVal.c_str() );
  }


  uint32_t ToUInt( const std::string& i_strVal )
  {
    if( i_strVal.rfind( "0x", 0 ) == 0 )
    {
      return strtoul( i_strVal.c_str(), nullptr, 16 );
    }

    return strtoul( i_strVal.c_str(), nullptr, 10 );
  }


  void ToLower( std::string& io_strText )
  {
    std::transform( io_strText.begin(), io_strText.end(), io_strText.begin(), ::tolower );
  }


  void ToUpper( std::string& io_strText )
  {
    std::transform( io_strText.begin(), io_strText.end(), io_strText.begin(), ::toupper );
  }


  const char* ToString( int32_t i_iVal, int32_t i_iBase )
  {
    static char strBuffer[16];
    return ( ltoa( i_iVal, strBuffer, i_iBase ) );
  }


  const char* ToString64( int64_t i_iVal, int32_t i_iBase )
  {
    static char strBuffer[24];
    return lltoa( i_iVal, strBuffer, i_iBase );
  }
} // namespace rumStringUtils



namespace rumUtility
{
  void ScriptBind()
  {
    HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

    Sqrat::RootTable( pcVM )
      .Func( "rumAppendPathSeparator", rumFileUtils::AppendPathSeparator )
      .Func( "rumBitAllOn", rumBitUtils::AllOn )
      .Func( "rumBitAnyOn", rumBitUtils::AnyOn )
      .Func( "rumBitClear", rumBitUtils::Clear )
      .Func( "rumBitOn", rumBitUtils::IsSet )
      .Func( "rumBitSet", rumBitUtils::Set )
      .Func( "rumBitToggle", rumBitUtils::Toggle )
      .Func( "rumBitToggleAll", rumBitUtils::ToggleAll )
      .Func( "rumCopyTextToClipboard", rumStringUtils::ScriptCopyTextToClipboard )
      .Func( "rumCopyTextFromClipboard", rumStringUtils::CopyTextFromClipboard )
      .Func( "rumGetCleanFilePath", rumFileUtils::GetCleanFilePath )
      .Func( "rumGetCurrentDirectory", rumFileUtils::GetCurrentDirectory )
      .Func( "rumGetExtension", rumFileUtils::GetExtension )
      .Func( "rumGetDriveArray", rumFileUtils::GetDriveListForScript )
      .Func( "rumGetFileName", rumFileUtils::GetFilename )
      .Func( "rumGetFilesFromPath", rumFileUtils::GetFilesFromPathForScript )
      .Func( "rumGetFoldersFromPath", rumFileUtils::GetFoldersFromPathForScript )
      .Func( "rumGetParentPath", rumFileUtils::GetParentPath )
      .Func( "rumGetPath", rumFileUtils::GetPath )
      .Func( "rumGetPathSeparator", rumFileUtils::GetPathSeparator )
      .Func( "rumGetRandom32", rumNumberUtils::GetRandomUInt32 )
      .Func( "rumGetRandom64", rumNumberUtils::GetRandomUInt64 )
      .Func( "rumHasParentPath", rumFileUtils::HasParentPath )
      .Func( "rumIsFile", rumFileUtils::IsFile )
      .Func( "rumIsFolder", rumFileUtils::IsFolder )
      .Overload<bool(*)( const std::string& )>( "rumVerifyFile", rumFileUtils::FileExists )
      .Overload<bool(*)( const std::string&, const std::string& )>( "rumVerifyFile", rumFileUtils::FileExists );
  }
}
