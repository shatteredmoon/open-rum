#ifndef _U_ZLIB_H_
#define _U_ZLIB_H_

#include <platform.h>

#include <filesystem>
#include <list>
#include <string>

#include <zlib.h>

struct rumRequiredFileList
{
  std::string m_strPath;
  uint32_t m_uiCRC{ 0 };
};

typedef std::list<rumRequiredFileList> RequiredFileList;


namespace zlib
{
  int32_t InitCompression( z_stream& o_rcStream, int32_t i_iCompressionLevel = Z_DEFAULT_COMPRESSION );
  uint32_t GetCRC( const std::string& i_strArchive );
  void ScriptBind();
  int32_t VerifyArchive( const std::string& i_strArchive );

  struct rumFileInfo
  {
    std::string m_strFilename;
    uint32_t m_uiSizeCompressed{ 0 };
    uint32_t m_uiSizeUncompressed{ 0 };
    time_t m_tTimestamp{ 0 };
  };

} // namespace zlib


class ArchiveReader
{
public:

  ArchiveReader( const std::string& i_strArchive );
  ~ArchiveReader();

  int32_t ExtractToFile();

  // Returns buffer size
  int32_t ExtractToBuffer( char* o_strBuffer );

  // Returns true if the file was found and the contents of find stored in param 'info'
  bool FindFile( const std::string& i_strFile, zlib::rumFileInfo& io_rcFileInfo );

  int32_t GetNumFiles() const
  {
    return m_iNumFiles;
  }

  const zlib::rumFileInfo& GetNextFileInfo();

  bool HasMoreFileInfos() const
  {
    return ( IsValid() && ( m_iNumFileInfosRead < m_iNumFiles ) );
  }

  bool IsValid() const
  {
    return m_pcFile;
  }

private:

  ArchiveReader() = delete;

  // This builds a toc from the path, ignoring the file specified by the ignore parameter
  int32_t BuildTOC( const std::filesystem::path& i_fsCurrentPath, const std::filesystem::path& i_fsIgnorePath,
                    std::list<zlib::rumFileInfo>& io_rcTOC );

  int32_t GetNumFileInfosRead() const
  {
    return m_iNumFileInfosRead;
  }

  int32_t Open( const std::string& i_StrArchive );
  int32_t SkipCompressedData();

  z_stream m_cStream;

  std::string m_strFilename;
  zlib::rumFileInfo m_cFileInfo;

  FILE* m_pcFile{ nullptr };

  bool m_bPeekActive{ false };

  int32_t m_iDataOffset{ 0 };
  int32_t m_iVersion{ -1 };
  int32_t m_iNumFiles{ 0 };
  int32_t m_iNumFileInfosRead{ 0 };
};


class ArchiveWriter
{
public:

  bool AddFile( const std::string& i_strFile );
  bool AddFolder( const std::string& i_strPath );

  bool CreateArchive( const std::string& i_strFile );

private:
  bool AddFileInternal( const std::string& i_strFile );
  bool AddFolderInternal( const std::string& i_strPath );

  int32_t CompressFiles( z_stream& io_rcStream, FILE* const io_pcOutfile );

  std::list<zlib::rumFileInfo> m_cFileInfoList;
};

#endif // _U_ZLIB_H_
