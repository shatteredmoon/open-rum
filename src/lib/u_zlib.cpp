#include <u_zlib.h>

#include <u_assert.h>
#include <u_log.h>
#include <u_rum.h>
#include <u_utility.h>
#include <u_script.h>

#include <chrono>

#define RUM_ZLIB_ARCHIVE_HEADER      "rum_z"
#define RUM_ZLIB_TEMP_ARCHIVE_HEADER "rumtz"
#define RUM_ZLIB_ARCHIVE_HEADER_SIZE 0x5
#define RUM_ZLIB_ARCHIVE_VERSION     0x1
#define RUM_ZLIB_CHUNK_SIZE          32768 // 32 * 1024

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

namespace zlib
{
  // This builds a toc from the path, ignoring the file specified by the ignore parameter
  int32_t BuildTOC( const std::filesystem::path& i_fsCurrentPath, const std::filesystem::path& i_fsIgnorePath,
                    std::list<rumFileInfo>& io_rcTOC )
  {
    RUM_COUT( "Building TOC for " << i_fsCurrentPath.string() << '\n' );

    int32_t eResult{ RESULT_SUCCESS };

    std::filesystem::directory_iterator iter( i_fsCurrentPath );
    const std::filesystem::directory_iterator end;
    while( ( RESULT_SUCCESS == eResult ) && iter != end )
    {
      try
      {
        // If a directory encountered, recurse into the directory
        if( std::filesystem::is_directory( iter->status() ) )
        {
          // Recurse into directory
          eResult = BuildTOC( iter->path(), i_fsIgnorePath, io_rcTOC );
        }
        else if( std::filesystem::is_regular_file( iter->status() ) )
        {
          // Do not add the target archive to the toc
          if( i_fsIgnorePath != iter->path() )
          {
            bool bTempfile{ false };

            // Open current file
            FILE* pcFileHandle{ fopen( iter->path().string().c_str(), "rb" ) };
            if( pcFileHandle )
            {
              char strBuffer[RUM_ZLIB_ARCHIVE_HEADER_SIZE + 1];

              // Read header to determine if this is a temp file
              fread( strBuffer, sizeof( char ), RUM_ZLIB_ARCHIVE_HEADER_SIZE, pcFileHandle );
              if( strncmp( reinterpret_cast<char *>( strBuffer ), RUM_ZLIB_TEMP_ARCHIVE_HEADER,
                           sizeof( char ) * RUM_ZLIB_ARCHIVE_HEADER_SIZE ) == 0 )
              {
                bTempfile = true;
              }

              fclose( pcFileHandle );
            }

            // Do not add temp files to the toc
            if( !bTempfile )
            {
              //cout << "Checking " << c_itor->path().string() << std::endl;

              // Create TOC entry
              rumFileInfo cFileInfo;
              cFileInfo.m_strFilename = iter->path().string();

              // Get the last time the file was written
              auto fTime{ last_write_time( iter->path() ) };

              rumAssert( false ); // FIXME
              //info.m_tTimestamp = decltype( ftime )::clock::to_time_t( ftime );

              // Get the size of the file (currently unnecessary)
              //uintmax_t uiSize{ file_size( iter->path() ) };

              // Add a copy
              io_rcTOC.push_back( cFileInfo );
            }
            //else std::cout << "Skipping " << iter->path().string() << std::endl;
          }
          //else std::cout << "Skipping " << iter->path().string() << std::endl;
        }
      }
      catch( ... /*const boost::exception &ex*/ )
      {
        RUM_COUT( iter->path() << " " /*<< ex.what()*/ << '\n' );
        eResult = RESULT_FAILED;
      }

      ++iter;
    }

    return eResult;
  }


  template <typename TimeType>
  std::time_t ConvertTime( TimeType tp )
  {
    using namespace std::chrono;
    const auto cTimePoint{ time_point_cast<system_clock::duration>( tp - TimeType::clock::now() +
                                                                    system_clock::now() ) };
    return system_clock::to_time_t( cTimePoint );
  }


  int32_t ExtractArchive( const std::string& i_strArchive )
  {
    int32_t eResult{ RESULT_SUCCESS };

    ArchiveReader cArchive( i_strArchive );
    while( ( RESULT_SUCCESS == eResult ) && cArchive.HasMoreFileInfos() )
    {
      eResult = cArchive.ExtractToFile();
    }

    return eResult;
  }


  int32_t InitCompression( z_stream& o_rcStream, int32_t i_iCompressionLevel )
  {
    // If m_cStream has already been used, reset it
    //deflateReset(&m_cStream);

    // Init zlib compression
    o_rcStream.zalloc = Z_NULL;
    o_rcStream.zfree = Z_NULL;
    o_rcStream.opaque = Z_NULL;

    return deflateInit( &o_rcStream, i_iCompressionLevel );
  }


  uint32_t GetCRC( const std::string& i_strArchive )
  {
    uLong uiCRC{ crc32( 0L, Z_NULL, 0 ) };

    // Use boost to convert to native file format
    const std::filesystem::path bfs_archive( i_strArchive );

    FILE* pcFileHandle{ fopen( bfs_archive.string().c_str(), "rb" ) };
    if( pcFileHandle )
    {
      constexpr int32_t BUFFER_SIZE{ 4096 };

      Bytef strBuffer[BUFFER_SIZE];

      while( !feof( pcFileHandle ) )
      {
        size_t iBytesRead{ fread( strBuffer, sizeof( Bytef ), BUFFER_SIZE, pcFileHandle ) };
        uiCRC = crc32( uiCRC, strBuffer, (uint32_t)iBytesRead );
      }

      fclose( pcFileHandle );
    }

    return uiCRC;
  }


  void ScriptBind()
  {
    // Todo: This is something the server would never want to do. Perhaps the ScriptBind mechanism should be placed in
    // c_zlib or s_zlib files to expand only what is necessary? Also, this shouldn't be easily available to players.
    HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };
    Sqrat::RootTable( pcVM ).Func( "rumExtractArchive", ExtractArchive );
  }


  int32_t verifyArchive( const std::string& i_strArchive )
  {
    // Out of date if:
    // 1) TOC file count does not match number of files stored
    // 2) A stored timestamp does not match the actual file timestamp

    int32_t eResult{ RESULT_FAILED };

    const std::filesystem::path fsArchive( i_strArchive );

    // Build a TOC of the specified path
    std::list<rumFileInfo> cTOCList;
    BuildTOC( fsArchive.parent_path(), fsArchive, cTOCList );

    // The TOC
    rumFileInfo cFileInfo;

    ArchiveReader cArchive( i_strArchive );

    // Make sure the toc and archive are reporting the same number of files
    if( cArchive.GetNumFiles() != cTOCList.size() )
    {
      RUM_COUT( "File mismatch detected in archive, archive must be updated\n" );
      return RESULT_FAILED;
    }

    while( cArchive.HasMoreFileInfos() == true )
    {
      cFileInfo = cArchive.GetNextFileInfo();
      RUM_COUT( cFileInfo.m_strFilename << '\n' );

      // Compare timestamps and make sure all toc files are touched
      std::list<rumFileInfo>::iterator iter{ cTOCList.begin() };
      const std::list<rumFileInfo>::iterator end{ cTOCList.end() };
      while( ( RESULT_SUCCESS == eResult ) && iter != end )
      {
        // Make sure filename's match
        if( iter->m_strFilename.compare( cFileInfo.m_strFilename ) != 0 )
        {
          RUM_COUT( "File mismatch detected in archive, archive must be updated\n" );
          eResult = RESULT_FAILED;
        }

        // Make sure timestamps match
        else if( iter->m_tTimestamp != cFileInfo.m_tTimestamp )
        {
          RUM_COUT( "The file " << iter->m_strFilename << " is out of date in the archive.\n" );
          eResult = RESULT_FAILED;
        }

        ++iter;
      }
    }

    if( RESULT_SUCCESS == eResult )
    {
      RUM_COUT( "Archive is up to date.\n" );
    }

    return eResult;
  }

} // namespace zlib


ArchiveReader::ArchiveReader( const std::string& i_strArchive )
{
  // Allocate inflate state
  m_cStream.zalloc = Z_NULL;
  m_cStream.zfree = Z_NULL;
  m_cStream.opaque = Z_NULL;
  m_cStream.next_in = Z_NULL;
  m_cStream.avail_in = 0;

  if( inflateInit( &m_cStream ) == Z_OK )
  {
    Open( i_strArchive );
  }
  else
  {
    std::string strError{ "Failed to init archive reader for " };
    strError += i_strArchive;
    Logger::LogStandard( strError, Logger::LOG_ERROR );
  }
}


ArchiveReader::~ArchiveReader()
{
  if( m_pcFile )
  {
    fclose( m_pcFile );
  }

  inflateEnd( &m_cStream );
}


int ArchiveReader::Open( const std::string& i_strArchive )
{
  m_strFilename = std::filesystem::path( i_strArchive ).string();

  m_pcFile = fopen( m_strFilename.c_str(), "rb" );
  if( !m_pcFile )
  {
    return RESULT_FAILED;
  }

  char strBuffer[RUM_ZLIB_CHUNK_SIZE];

  // Read header to determine if this is an archive
  fread( strBuffer, sizeof( char ), RUM_ZLIB_ARCHIVE_HEADER_SIZE, m_pcFile );
  if( strncmp( reinterpret_cast<char *>( strBuffer ), RUM_ZLIB_ARCHIVE_HEADER,
               sizeof( char ) * RUM_ZLIB_ARCHIVE_HEADER_SIZE ) != 0 )
  {
    return RESULT_FAILED;
  }

  // Archive version
  m_iVersion = fgetc( m_pcFile );
  if( m_iVersion != RUM_ZLIB_ARCHIVE_VERSION )
  {
    return RESULT_FAILED;
  }

  // Get the number of files stored in the archive
  strBuffer[0] = fgetc( m_pcFile );
  strBuffer[1] = fgetc( m_pcFile );
  rumWord iVal;
  rumSerializationUtils::Read( iVal, strBuffer );
  m_iNumFiles = iVal;

  // Save the offset location of the first file
  m_iDataOffset = ftell( m_pcFile );

  return RESULT_SUCCESS;
}


const zlib::rumFileInfo& ArchiveReader::GetNextFileInfo()
{
  // By default, skip the file data
  if( m_bPeekActive )
  {
    SkipCompressedData();
  }

  char strBuffer[RUM_ZLIB_CHUNK_SIZE];
  char* pcBufferOffset{ strBuffer };

  // Find the next NULL terminator
  while( ( *pcBufferOffset++ = fgetc( m_pcFile ) ) != '\0' && !feof( m_pcFile ) );

  // Use Boost to create a native filename
  m_cFileInfo.m_strFilename = std::filesystem::path( reinterpret_cast<char *>( strBuffer ) ).string();

  rumDWord iVal{ 0 };

  // Retrieve the last write time of this file
  fread( strBuffer, sizeof( char ), 4, m_pcFile );
  rumSerializationUtils::Read( iVal, strBuffer );
  m_cFileInfo.m_tTimestamp = static_cast<time_t>( iVal );

  // Retrieve the number bytes of compressed data
  fread( strBuffer, sizeof( char ), 4, m_pcFile );
  rumSerializationUtils::Read( iVal, strBuffer );
  m_cFileInfo.m_uiSizeCompressed = static_cast<time_t>( iVal );

  // Retrieve the number bytes of uncompressed data
  fread( strBuffer, sizeof( char ), 4, m_pcFile );
  rumSerializationUtils::Read( iVal, strBuffer );
  m_cFileInfo.m_uiSizeUncompressed = static_cast<time_t>( iVal );

  // Data should next be extracted or skipped
  m_bPeekActive = true;

  ++m_iNumFileInfosRead;

  return m_cFileInfo;
}


int ArchiveReader::ExtractToFile()
{
  if( !m_bPeekActive )
  {
    GetNextFileInfo();
  }

  char* strBuffer{ new char[m_cFileInfo.m_uiSizeUncompressed] };
  if( !strBuffer )
  {
    return RESULT_FAILED;
  }

  const int32_t iSize{ ExtractToBuffer( strBuffer ) };
  if( iSize != m_cFileInfo.m_uiSizeUncompressed )
  {
    std::string strWarning{ "Extract to file expected to store " };
    strWarning += rumStringUtils::ToString( m_cFileInfo.m_uiSizeUncompressed );
    strWarning += " bytes, but stored ";
    strWarning += rumStringUtils::ToString( iSize );
    strWarning += " bytes instead for file ";
    strWarning += m_strFilename;
    Logger::LogStandard( strWarning );
  }

  std::filesystem::path fsPath( m_strFilename );
  fsPath.remove_filename();
  fsPath /= m_cFileInfo.m_strFilename;
  if( fsPath.has_parent_path() )
  {
    std::filesystem::create_directories( fsPath.parent_path() );
  }

  // Create the file
  FILE* pcFileHandle{ fopen( fsPath.string().c_str(), "wb" ) };
  if( !pcFileHandle )
  {
    delete[] strBuffer;
    return RESULT_FAILED;
  }

  if( fwrite( strBuffer, sizeof( char ), iSize, pcFileHandle ) != iSize || ferror( pcFileHandle ) )
  {
    inflateEnd( &m_cStream );
    delete[] strBuffer;
    fclose( pcFileHandle );
    return RESULT_FAILED;
  }

  if( strBuffer )
  {
    delete[] strBuffer;
  }

  fclose( pcFileHandle );

  return RESULT_SUCCESS;
}


int ArchiveReader::ExtractToBuffer( char* o_strBuffer )
{
  if( !o_strBuffer )
  {
    return 0;
  }

  if( !m_bPeekActive )
  {
    GetNextFileInfo();
  }

  uint32_t uiTotalBytesRead{ 0 };
  uint32_t uiExtractedBytes{ 0 };

  char strBuffer[RUM_ZLIB_CHUNK_SIZE];

  do
  {
    // Decompress until deflate stream ends or end of file
    // Should we read an entire chunk, or is there fewer bytes still to go?
    const uint32_t uiBytesToRead{ std::min<uint32_t>( m_cFileInfo.m_uiSizeCompressed - uiTotalBytesRead,
                                                      RUM_ZLIB_CHUNK_SIZE ) };

    m_cStream.avail_in = (uint32_t)fread( strBuffer, sizeof( char ), uiBytesToRead, m_pcFile );
    uiTotalBytesRead += m_cStream.avail_in;

    if( ferror( m_pcFile ) )
    {
      inflateEnd( &m_cStream );
      return Z_ERRNO;
    }
    if( m_cStream.avail_in == 0 )
    {
      break;
    }

    m_cStream.next_in = (Bytef*)strBuffer;

    do
    {
      // Run inflate() on input until output buffer not full
      m_cStream.avail_out = uiBytesToRead;
      m_cStream.next_out = (Bytef*)o_strBuffer + uiExtractedBytes;
      int32_t eResult{ inflate( &m_cStream, Z_NO_FLUSH ) };
      switch( eResult )
      {
        case Z_NEED_DICT:
          eResult = Z_DATA_ERROR; // fall through
        case Z_DATA_ERROR:
        case Z_MEM_ERROR:
        case Z_STREAM_ERROR:
          inflateEnd( &m_cStream );
          return uiExtractedBytes;
      }

      uiExtractedBytes += uiBytesToRead - m_cStream.avail_out;

    } while( m_cStream.avail_out == 0 );

    // done when inflate() says it's done
  } while( uiTotalBytesRead < m_cFileInfo.m_uiSizeCompressed );

  m_bPeekActive = false;
  inflateReset( &m_cStream );

  return uiExtractedBytes;
}


bool ArchiveReader::FindFile( const std::string& i_strFile, zlib::rumFileInfo& io_rcFileInfo )
{
  bool bFound{ false };

  // Rewind to the beginning of the data
  m_bPeekActive = false;
  m_iNumFileInfosRead = 0;
  fseek( m_pcFile, m_iDataOffset, SEEK_SET );

  // Build an alternate file string with just the filename - see TODO notes below
  const std::filesystem::path fsFile( i_strFile );
  const std::string strFileAlt{ fsFile.filename().string() };

  while( !bFound && HasMoreFileInfos() )
  {
    io_rcFileInfo = GetNextFileInfo();

    if( io_rcFileInfo.m_strFilename.compare( i_strFile ) == 0 )
    {
      bFound = true;
    }
    else
    {
      // TODO: This is a hack. There are conditions where a filename won't be found in the archive because it has a
      // path in front of it, i.e. u4/u4_britannia.map. This section will attempt a lookup of "u4_britannia.map"
      // without the path "u4" which of course can lead to all kinds of false-positives. A strong script-to-filename
      // relationship is desirable.

      // Remove path info
      const std::filesystem::path fsArchive( io_rcFileInfo.m_strFilename );
      const std::string strArchiveFile{ fsArchive.filename().string() };
      if( strArchiveFile.compare( strFileAlt ) == 0 )
      {
        bFound = true;
      }
    }
  }

  return bFound;
}


int ArchiveReader::SkipCompressedData()
{
  if( m_bPeekActive )
  {
    if( fseek( m_pcFile, m_cFileInfo.m_uiSizeCompressed, SEEK_CUR ) != 0 )
    {
      return RESULT_FAILED;
    }
  }

  m_bPeekActive = false;

  return RESULT_SUCCESS;
}


bool ArchiveWriter::AddFile( const std::string& i_strFile )
{
  return AddFileInternal( i_strFile );
}


bool ArchiveWriter::AddFileInternal( const std::string& i_strFile )
{
  const std::filesystem::path fsFile( i_strFile );
  if(std::filesystem::exists( fsFile ) && std::filesystem::is_regular_file( fsFile ) )
  {
    // Create file info entry
    zlib::rumFileInfo cFileInfo;
    cFileInfo.m_strFilename = fsFile.string();

    // Get the last time the file was written
    const auto fTime{ last_write_time( fsFile ) };
    cFileInfo.m_tTimestamp = zlib::ConvertTime( fTime );

    // Get the size of the file (currently unnecessary)
    uintmax_t uiSize{ file_size( fsFile ) };

    // Add a copy
    m_cFileInfoList.push_back( cFileInfo );

    return true;
  }

  return false;
}


bool ArchiveWriter::AddFolder( const std::string& i_strPath )
{
  return AddFolderInternal( i_strPath );
}


bool ArchiveWriter::AddFolderInternal( const std::string& i_strPath )
{
  bool bResult{ true };

  const std::filesystem::path fsPath( i_strPath );
  for( const auto& iter : std::filesystem::directory_iterator( fsPath ) )
  {
    // If directory encountered, recurse into the directory
    if(std::filesystem::is_directory( iter.status() ) )
    {
      // Recurse into directory
      bResult = AddFolderInternal( iter.path().string() );
    }
    else if(std::filesystem::is_regular_file( iter.status() ) )
    {
      bResult = AddFileInternal( iter.path().string() );
    }
  }

  return bResult;
}


int ArchiveWriter::CompressFiles( z_stream& io_rcStream, FILE* const io_pcOutfile )
{
  int32_t eResult{ RESULT_SUCCESS };

  rumByte strInBuffer[RUM_ZLIB_CHUNK_SIZE];
  rumByte strOutBuffer[RUM_ZLIB_CHUNK_SIZE];

  for( auto& iter : m_cFileInfoList )
  {
    //cout << "Archiving " << bfsFile.string() << std::endl;

    // Begin status in a "not finished" state
    int32_t eStatus{ Z_NO_FLUSH };
    int32_t eResult{ Z_OK };

    const std::filesystem::path fsFile( iter.m_strFilename );

    // Open current file
    FILE* pcSourceFile{ fopen( fsFile.string().c_str(), "rb" ) };
    if( pcSourceFile )
    {
      // Compress until end of file
      do
      {
        // Store the amount of bytes read in avail_in
        io_rcStream.avail_in = (uInt)fread( strInBuffer, sizeof( char ), RUM_ZLIB_CHUNK_SIZE, pcSourceFile );
        if( ferror( pcSourceFile ) )
        {
          deflateEnd( &io_rcStream );
          eResult = Z_ERRNO;
        }
        else
        {
          // Keep track of the uncompressed file size
          iter.m_uiSizeUncompressed += io_rcStream.avail_in;

          // Set next_in to the buffer we just read
          io_rcStream.next_in = (Bytef*)strInBuffer;

          // If the end of file has been encountered, we are done
          if( feof( pcSourceFile ) )
          {
            eStatus = Z_FINISH;
          }

          // Run deflate() on input until output buffer not full, finish compression if all of source has been read in
          do
          {
            io_rcStream.avail_out = RUM_ZLIB_CHUNK_SIZE;
            io_rcStream.next_out = (Bytef*)strOutBuffer;
            eResult = deflate( &io_rcStream, eStatus ); // no bad return value
            rumAssertMsg( eResult != Z_STREAM_ERROR, "zlib stream error" ); // state not clobbered

            if( eResult != Z_STREAM_ERROR )
            {
              uint32_t uiOutputSize{ RUM_ZLIB_CHUNK_SIZE - io_rcStream.avail_out };
              if( fwrite( strOutBuffer, sizeof( char ), uiOutputSize, io_pcOutfile ) !=  uiOutputSize ||
                  ferror( io_pcOutfile ) )
              {
                deflateEnd( &io_rcStream );
                eResult = Z_ERRNO;
              }
            }
          } while( ( io_rcStream.avail_out == 0 ) && eResult != Z_ERRNO );

          // Was all input used?
          rumAssertMsg( io_rcStream.avail_in == 0, "Not all zlib input was processed" );
        }
        // Done when last data in file processed
      } while( eStatus != Z_FINISH && eResult != Z_ERRNO );

      if( eResult != Z_ERRNO )
      {
        rumAssertMsg( Z_STREAM_END == eResult, "" ); // stream will be complete

        // Keep track of how many total bytes have been deflated
        iter.m_uiSizeCompressed = io_rcStream.total_out;
      }
      else
      {
        std::string strError{ "Failed to compress: " };
        strError += iter.m_strFilename;
        Logger::LogStandard( strError );
      }

      // Must reset in order to deflate more data
      deflateReset( &io_rcStream );

      // Close current input file
      fclose( pcSourceFile );
    }
    else
    {
      std::string strError{ "Failed to compress: " };
      strError += iter.m_strFilename;
      Logger::LogStandard( strError );
    }
  }

  return eResult;
}


bool ArchiveWriter::CreateArchive( const std::string& i_strArchive )
{
  bool eResult{ RESULT_SUCCESS };

  // Use boost to convert to native file format
  const std::filesystem::path fsArchive( i_strArchive );

  try
  {
    // Init zlib
    z_stream cStream;
    zlib::InitCompression( cStream );

    // Create directories to the specified archive if necessary
    const std::filesystem::path fsPath( fsArchive.parent_path() );
    std::filesystem::create_directories( fsPath );

    // Create temporary file
    const std::string strTempFile{ tmpnam( nullptr ) };
    FILE* pcTempFile{ fopen( strTempFile.c_str(), "wb+" ) };
    if( pcTempFile )
    {
      // Store archive header
      fwrite( RUM_ZLIB_TEMP_ARCHIVE_HEADER, sizeof( char ), RUM_ZLIB_ARCHIVE_HEADER_SIZE, pcTempFile );

      // Compress all files to the archive
      CompressFiles( cStream, pcTempFile );

      // Skip the archive header and version number
      fseek( pcTempFile, RUM_ZLIB_ARCHIVE_HEADER_SIZE, SEEK_SET );

      // Create final archive with a Table of Contents using the temp archive
      FILE* pcOutfile{ fopen( fsArchive.string().c_str(), "wb" ) };
      if( pcOutfile )
      {
        char strBuffer[RUM_ZLIB_CHUNK_SIZE];

        // Store archive header
        fwrite( RUM_ZLIB_ARCHIVE_HEADER, sizeof( char ), RUM_ZLIB_ARCHIVE_HEADER_SIZE, pcOutfile );

        // Archive version
        fputc( RUM_ZLIB_ARCHIVE_VERSION, pcOutfile );

        rumSerializationUtils::Write( (rumWord)m_cFileInfoList.size(), strBuffer );

        // Store number of files in archive
        if( fwrite( strBuffer, sizeof( char ), sizeof( rumWord ), pcOutfile ) != sizeof( rumWord ) ||
            ferror( pcOutfile ) )
        {
          return false; // Z_ERRNO
        }

        // For creating relative paths based off of the project path
        const std::string& strRoot{ fsPath.string() };

        // Iterate through all files, storing a header for each entry and copying the compressed information from the
        // temp archive
        for( const auto& iter : m_cFileInfoList )
        {
          const zlib::rumFileInfo& rcFileInfo{ iter };

          // It would be great if boost had a "relative_to" function
          std::string strFile( rcFileInfo.m_strFilename );
          strFile.erase( 0, strRoot.size() + 1 );

          //debug
          //cout << "Processing " << ptoc->m_strFilename.c_str() << std::endl;

          const int32_t iLength{ (int32_t)strFile.length() + 1 };

          // Store the current filename with null terminator
          if( fwrite( strFile.c_str(), sizeof( char ), iLength, pcOutfile ) != iLength ||
              ferror( pcOutfile ) )
          {
            return false; // Z_ERRNO
          }

          // Store the last write time of this file for archive update checking
          rumSerializationUtils::Write( (rumDWord)rcFileInfo.m_tTimestamp, strBuffer );
          if( fwrite( strBuffer, sizeof( char ), sizeof( rumDWord ), pcOutfile ) != sizeof( rumDWord ) ||
              ferror( pcOutfile ) )
          {
            return false; // Z_ERRNO
          }

          // Store the number of bytes of compressed data
          rumSerializationUtils::Write( (rumDWord)rcFileInfo.m_uiSizeCompressed, strBuffer );
          if( fwrite( strBuffer, sizeof( char ), sizeof( rumDWord ), pcOutfile ) != sizeof( rumDWord ) ||
              ferror( pcOutfile ) )
          {
            return false; // Z_ERRNO
          }

          // Store the number of bytes uncompressed data
          rumSerializationUtils::Write( (rumDWord)rcFileInfo.m_uiSizeUncompressed, strBuffer );
          if( fwrite( strBuffer, sizeof( char ), sizeof( rumDWord ), pcOutfile ) != sizeof( rumDWord ) ||
              ferror( pcOutfile ) )
          {
            return false; // Z_ERRNO
          }

          // Copy compressed bytes at current offset from source to dest
          uint32_t uiTotalBytesRead{ 0 };
          do
          {
            // Should we read an entire chunk, or is there fewer bytes still to go?
            const uint32_t uiBytesToRead{ std::min<uint32_t>( ( rcFileInfo.m_uiSizeCompressed - uiTotalBytesRead ),
                                                              RUM_ZLIB_CHUNK_SIZE ) };
            const uint32_t uiCurrentBytesRead{ (uint32_t)fread( strBuffer, sizeof( char ), uiBytesToRead,
                                                                pcTempFile ) };
            uiTotalBytesRead += uiCurrentBytesRead;
            if( ferror( pcTempFile ) )
            {
              return false; // Z_ERRNO
            }

            // Output the last input chunk
            if( fwrite( strBuffer, sizeof( char ), uiCurrentBytesRead, pcOutfile ) != uiCurrentBytesRead ||
                ferror( pcOutfile ) )
            {
              return false; // Z_ERRNO
            }

          } while( uiTotalBytesRead < rcFileInfo.m_uiSizeCompressed );
        }

        fclose( pcOutfile );

        RUM_COUT( "Compressed " << m_cFileInfoList.size() << " files to " << fsArchive.string() << '\n' );
      }
      else
      {
        Logger::LogStandard( "Error creating archive" );
        eResult = RESULT_FAILED;
      }

      fclose( pcTempFile );

      // Delete the temporary archive
      remove(std::filesystem::path( strTempFile ) );
    }
    else
    {
      Logger::LogStandard( "Error creating temp file" );
      eResult = RESULT_FAILED;
    }
  }
  catch( ... )
  {
    std::string strError{ "An exception occurred while creating archive: " };
    strError += fsArchive.string();
    Logger::LogStandard( strError );
    eResult = RESULT_FAILED;
    throw;
  }

  return ( RESULT_SUCCESS == eResult );
}
