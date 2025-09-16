#ifndef _U_UTILITY_H_
#define _U_UTILITY_H_

#include <platform.h>
#include <u_assert.h>

#include <string>
#include <vector>

#define APPEND( x, y ) x ## y

#define BIT( x ) ( 0x1 << x )

#define MAKE4CC( a, b, c, d ) ( (a) | ( (b) << 8 ) | ( (c) << 16) | ( (d) << 24 ) )

#define SQR( x ) ( ( x ) * ( x ) )

#define _QUOTED( x ) # x
#define QUOTED( x )  _QUOTED( x )

#define SAFE_DELETE( a ) { delete ( a ); ( a ) = nullptr; }
#define SAFE_ARRAY_DELETE( a ) { delete [] ( a ); ( a ) = nullptr; }


namespace rumBitUtils
{
  // Returns true if all bits in the mask are set on the value
  inline bool AllOn( uint32_t i_uiVal, uint32_t i_uiMask )
  {
    return ( ( i_uiVal & i_uiMask ) == i_uiMask );
  }

  // Returns true if any of the bits in the mask are set on the value
  inline bool AnyOn( uint32_t i_uiVal, uint32_t i_uiMask )
  {
    return ( ( i_uiVal & i_uiMask ) != 0 );
  }

  // Clears the bit on the value at the specified position
  inline uint32_t Clear( uint32_t i_uiVal, uint32_t i_uiPos )
  {
    return ( i_uiVal & ( ~( 1 << i_uiPos ) ) );
  }

  // Returns true if a bit in the specified position is set
  inline bool IsSet( uint32_t i_uiVal, uint32_t i_uiPos )
  {
    return ( ( i_uiVal & ( 1 << i_uiPos ) ) != 0 );
  }

  // Sets the bit on the value at the specified position
  inline uint32_t Set( uint32_t i_uiVal, uint32_t i_uiPos )
  {
    return ( i_uiVal | ( 1 << i_uiPos ) );
  }

  // Returns the value with the bit at the specified position toggled
  inline uint32_t Toggle( uint32_t i_uiVal, uint32_t i_uiPos )
  {
    return ( i_uiVal ^ ( 1 << i_uiPos ) );
  }

  // Returns the value with all bits toggled
  inline uint32_t ToggleAll( uint32_t i_uiVal )
  {
    return ~i_uiVal;
  }
} // namespace rumBitUtils


namespace rumFileUtils
{
  std::string AppendPathSeparator( const std::string& i_strPath );

  bool FileExists( const std::string& i_strFilePath, const std::string& i_strHash );
  bool FileExists( const std::string& i_strFilePath );

  std::string GetCleanFilePath( const std::string& i_strPath );

  std::string GetCurrentDirectory();

  std::vector<std::string> GetDriveList();

  std::string GetExtension( const std::string& i_strFilePath );
  std::string GetFilename( const std::string& i_strFilePath );

  std::vector<std::string> GetFilesFromPath( const std::string& i_strPath );

  std::vector<std::string> GetFoldersFromPath( const std::string& i_strPath );

  std::string GetParentPath( const std::string& i_strPath );
  bool HasParentPath( const std::string& i_strPath );

  std::string GetPath( const std::string& i_strFilePath );
  std::string GetPathSeparator();

  bool IsFile( const std::string& i_strFilePath );
  bool IsFolder( const std::string& i_strPath );

  bool IsHiddenFile( const std::string& i_strPath );
  bool IsSystemFile( const std::string& i_strPath );
} // namespace rumFileUtils


namespace rumNumberUtils
{
  template<typename T>
  void Clamp( T& o_rTVal, const T i_TMin, const T i_TMax )
  {
    rumAssertMsg( i_TMax > i_TMin, "Clamp parameters are reversed" );
    o_rTVal < i_TMin ? o_rTVal = i_TMin : ( ( o_rTVal > i_TMax ) ? o_rTVal = i_TMax : 0 );
  }

  void FreeSqrtTable();
  float GetSqrtValue( uint32_t i_uiTaxiCabDistance );
  void InitSqrtTable( uint32_t i_uiSize );

  uint32_t GetRandomUInt32();
  uint64_t GetRandomUInt64();

  float GetRandomFloat32();
  float GetRandomFloat32_Range( float i_fMin, float i_fMax );

  void SetRandomSeed( const uint32_t i_uiSeed );

  template<typename T>
  bool ValuesEqual( const T& i_rTVal1, const T& i_rTVal2 )
  {
    return std::fabs( i_rTVal1 - i_rTVal2 ) < std::numeric_limits<T>::epsilon();
  }

  template<typename T>
  bool ValueZero( const T& i_rTVal )
  {
    return std::fabs( i_rTVal - T{} ) < std::numeric_limits<T>::epsilon();
  }
}


namespace rumSerializationUtils
{
  inline char ReadByte( const char* i_pcString );

  // Big Endian integer based read of byte buffer
  template<typename T>
  const char* Read( T& o_TVal, const char* i_pcString )
  {
    o_TVal = 0;
    static constexpr size_t iSize{ sizeof( T ) };
    for( size_t i = 0; i < iSize; ++i )
    {
      o_TVal |= ( static_cast<T>( static_cast<uint8_t>( ReadByte( i_pcString++ ) ) ) <<
                  ( ( iSize - ( i + 1 ) ) << 3 ) );
    }

    return i_pcString;
  }


  // Big Endian bool based read of byte buffer (template override)
  inline const char* Read( bool& o_bVal, const char* i_pcString )
  {
    char cVal;
    i_pcString = Read( cVal, i_pcString );
    o_bVal = ( cVal != '\0' );
    return i_pcString;
  }


  // Big Endian float based read of byte buffer (template override)
  inline const char* Read( float& o_fVal, const char* i_pcString )
  {
    rumDWord iVal;
    i_pcString = Read( iVal, i_pcString );
    o_fVal = *( reinterpret_cast<float*>( &iVal ) );
    return i_pcString;
  }


  // Big Endian double based read of byte buffer (template override)
  inline const char* Read( double& o_fVal, const char* i_pcString )
  {
    rumQWord iVal;
    i_pcString = Read( iVal, i_pcString );
    o_fVal = *( reinterpret_cast<double*>( &iVal ) );
    return i_pcString;
  }


  // Big Endian string based read of byte buffer (template override)
  inline const char* Read( std::string& i_strVal, const char* i_pcString )
  {
    i_strVal = i_pcString;
    return i_pcString + i_strVal.length() + 1;
  }


  // Big Endian const char string based read of byte buffer (template override)
  inline const char* Read( char* o_strVal, const char* i_pcString )
  {
    strcpy( o_strVal, i_pcString );
    return i_pcString + strlen( o_strVal ) + 1;
  }


  inline char ReadByte( const char* i_pcString )
  {
    static_assert( sizeof( char ) == sizeof( uint8_t ) );
    return static_cast<char>( *i_pcString );
  }


  // Big Endian integer based write to byte buffer
  template<typename T>
  char* Write( const T i_TVal, char* io_pcString )
  {
    static constexpr size_t iSize{ sizeof( T ) };
    for( size_t i = 0; i < iSize; ++i )
    {
      *io_pcString++ = static_cast<char>( ( i_TVal >> ( ( iSize - ( i + 1 ) ) << 3 ) ) & 0xff );
    }

    return io_pcString;
  }


  // Big Endian bool based write to byte buffer (template override)
  inline char* Write( bool i_bVal, char* io_pcString )
  {
    return Write( char( i_bVal ), io_pcString );
  }


  // Big Endian float based write to byte buffer (template override)
  inline char* Write( float i_fVal, char* io_pcString )
  {
    static_assert( sizeof( float ) == sizeof( rumDWord ) );
    const rumDWord iVal{ *reinterpret_cast<rumDWord*>( &i_fVal ) };
    return Write( iVal, io_pcString );
  }


  // Big Endian double based write to byte buffer (template override)
  inline char* Write( double i_fVal, char* io_pcString )
  {
    static_assert( sizeof( double ) == sizeof( rumQWord ) );
    const rumQWord iVal{ *( reinterpret_cast<rumQWord*>( &i_fVal ) ) };
    return Write( iVal, io_pcString );
  }


  // Big Endian string based write to byte buffer (template override)
  inline char* Write( const std::string& i_strVal, char* io_pcString )
  {
    const size_t iSize{ i_strVal.length() + 1 };
    memcpy( io_pcString, i_strVal.c_str(), iSize );
    return io_pcString + iSize;
  }


  // Big Endian const char string based write to byte buffer (template override)
  inline char* Write( const char* i_strVal, char* io_pcString )
  {
    const size_t iSize{ strlen( i_strVal ) + 1 };
    memcpy( io_pcString, i_strVal, iSize );
    return io_pcString + iSize;
  }
} // namespace rumSerializationUtils


namespace rumStringUtils
{
  const char* CopyTextFromClipboard();
  void CopyTextToClipboard( const std::string& i_strText );

  const std::string& NullString();

  using StringVector = std::vector<std::string>;
  StringVector ParseCSVRow( const std::string& i_strRow );

  void Replace( std::string& i_strContext, const std::string& i_strSource, const std::string& i_strDest );

  void Split( StringVector& o_cStringVector, const std::string& i_strText, const std::string& i_strDelimiter );

  bool ToBool( std::string i_strVal );

  float ToFloat( const std::string& i_strVal );
  const char* ToFloatString( float i_fVal );

  const char* ToHexString( int32_t i_iVal );
  const char* ToHexString64( int64_t i_iVal );

  int32_t ToInt( const std::string& i_strVal );
  int64_t ToInt64( const std::string& i_strVal );
  uint32_t ToUInt( const std::string& i_strVal );

  void ToLower( std::string& io_strText );
  void ToUpper( std::string& io_strText );

  const char* ToString( int32_t i_iVal, int32_t i_iBase = 10 );
  const char* ToString64( int64_t i_iVal, int32_t i_iBase = 10 );
}


namespace rumUtility
{
  void ScriptBind();

  template <typename T>
  constexpr auto ToUnderlyingType( T enumerator ) noexcept
  {
    return static_cast<std::underlying_type_t<T>>( enumerator );
  }
}

#endif // _U_UTILITY_H_
