#ifndef _U_RESOURCE_H_
#define _U_RESOURCE_H_

#include <u_assert.h>
#include <u_script.h>

#include <map>
#include <vector>

// Uncomment for extra resource logging
//#define RESOURCE_DEBUG

class rumResource
{
public:

  virtual ~rumResource();

  static void ScriptBind();

  const char* GetMemFile() const
  {
    return m_pcMemFile;
  }

  virtual bool IsLoading() const = 0;
  virtual bool IsSaving() const = 0;

  // Rewind to the beginning of the data
  virtual void Reset() = 0;

  template <typename T>
  inline rumResource& operator<<( T& io_rcVar )
  {
    return Serialize( io_rcVar );
  }

  template <typename T>
  inline rumResource& operator<<( const T& i_rcVar )
  {
    return Serialize( i_rcVar );
  }

  template <typename T>
  rumResource& Serialize( T& io_rcVar )
  {
    if( IsLoading() )
    {
      if( !IsEndOfFile() )
      {
        m_pcLoadOffset = rumSerializationUtils::Read( io_rcVar, m_pcLoadOffset );
      }
    }
    else
    {
      if( ( sizeof( io_rcVar ) + GetPos() ) > m_uiMemSize )
      {
        if( !IsEndOfFile() )
        {
          m_pcSaveOffset = rumSerializationUtils::Write( io_rcVar, m_pcSaveOffset );
        }
      }
      else
      {
        m_pcSaveOffset = rumSerializationUtils::Write( io_rcVar, m_pcSaveOffset );
      }
    }

    return *this;
  }

  template <typename T>
  rumResource& Serialize( const T& i_rcVar )
  {
    rumAssertMsg( IsSaving(), "Serialize expected to be saving, not loading" );
    if( ( sizeof( i_rcVar ) + GetPos() ) > m_uiMemSize )
    {
      if( !IsEndOfFile() )
      {
        m_pcSaveOffset = rumSerializationUtils::Write( i_rcVar, m_pcSaveOffset );
      }
    }
    else
    {
      m_pcSaveOffset = rumSerializationUtils::Write( i_rcVar, m_pcSaveOffset );
    }

    return *this;
  }

  rumResource& Serialize( const std::string& i_strText );
  inline rumResource& operator<<( const std::string& i_strText )
  {
    return Serialize( i_strText );
  }

  rumResource& Serialize( const char* i_strText );
  inline rumResource& operator<<( const char* i_strText )
  {
    return Serialize( i_strText );
  }

  Sqrat::Object ScriptSerialize( Sqrat::Object i_sqParam );
  virtual Sqrat::Object ScriptSerializeArray( Sqrat::Object i_sqParam ) = 0;
  virtual Sqrat::Object ScriptSerializeBool( Sqrat::Object i_sqParam ) = 0;
  virtual Sqrat::Object ScriptSerializeClass( Sqrat::Object i_sqParam ) = 0;
  virtual Sqrat::Object ScriptSerializeFloat( Sqrat::Object i_sqParam ) = 0;
  virtual Sqrat::Object ScriptSerializeInstance( Sqrat::Object i_sqParam ) = 0;
  virtual Sqrat::Object ScriptSerializeInt( Sqrat::Object i_sqParam ) = 0;
  virtual Sqrat::Object ScriptSerializeNull( Sqrat::Object i_sqParam ) = 0;
  virtual Sqrat::Object ScriptSerializeString( Sqrat::Object i_sqParam ) = 0;
  virtual Sqrat::Object ScriptSerializeTable( Sqrat::Object i_sqParam ) = 0;

  virtual bool IsEndOfFile() = 0;
  virtual size_t GetPos() const = 0;
  virtual void SeekPos( size_t ) = 0;
  size_t GetSize() const
  {
    return m_uiMemSize;
  }

  // Caches the project folder tree using the provided path.
  // Returns the number of files cached.
  static int32_t CreateFileCache( const std::string& i_strPath );

  // Finds a file using the file cache only. Note, this call will not lazily create a file cache.
  // Returns true if the cache exists and the search was successful. The found results are stored in o_rcResultsVector.
  static bool FileCacheFind( const std::string& i_strFile, std::vector<std::string>& o_rcResultsVector );

  // Checks for filename existence using an optional path. This function makes no attempt to search for the file in any
  // other location than what is provided.
  // Returns true if the file was found.
  static bool FileExists( const std::string& i_strPath );

  // Attempts to find a single occurrence of the specified file. The algorithm will first use the file cache if it is
  // available, falling back on recursive directory searching otherwise.
  // Returns true if an occurrence of the file was found. If found, the filename is stored in o_strResult.
  static bool FindFile( const std::string& i_strFile, std::string& o_strResult );

  // Attempts to discover all occurrences of the provided filename. The algorithm will first use the file cache if it
  // is available, falling back on recursive directory searching otherwise.
  // Returns true if any occurrence of the file was found. The found results are stored in o_rcResultsVector.
  static bool FindFile( const std::string& i_strFile, std::vector<std::string>& o_rcResultsVector );

  // Finds a file by recursively walking the project tree on disk.
  // Returns true if the file was found. The final path is returned in o_strResult.
  static bool RecursiveFind( const std::string& i_strFile,
                             const std::string& i_strPath,
                             std::string& o_strResult );

  // Finds all files by recursively walking the project tree on disk.
  // Returns true if any occurrences of the filename were found. The final paths are returned in o_rcResultsVector.
  static bool RecursiveFind( const std::string& i_strFile,
                             const std::string& i_strPath,
                             std::vector<std::string>& o_rcResultsVector );

  static bool WriteStringToFile( const char* i_strFile, const char* i_strOutput );

protected:

  virtual void Release();

#pragma message("Why does the base class have all of these?")
  char* m_pcMemFile{ nullptr };
  char* m_pcSaveOffset{ nullptr };
  const char* m_pcLoadOffset{ nullptr };

  size_t m_uiMemSize{ 0 };

  // Used for caching the project file tree for faster file lookup. Since filename collisions can be expected, a
  // multimap is used here. The key represents the filename, data represents the path to the file.
  typedef std::multimap<std::string, std::string> FolderTreeContainer;
  static FolderTreeContainer s_cFileCacheMap;
};


class rumResourceLoader : public rumResource
{
public:

  rumResourceLoader() = default;
  rumResourceLoader( size_t i_uiBufferSize );

  bool IsLoading() const override
  {
    return true;
  }

  bool IsSaving() const override
  {
    return false;
  }

  Sqrat::Object ScriptSerializeArray( Sqrat::Object i_sqParam ) override;
  Sqrat::Object ScriptSerializeBool( Sqrat::Object i_sqParam ) override;
  Sqrat::Object ScriptSerializeClass( Sqrat::Object i_sqParam ) override;
  Sqrat::Object ScriptSerializeFloat( Sqrat::Object i_sqParam ) override;
  Sqrat::Object ScriptSerializeInstance( Sqrat::Object i_sqParam ) override;
  Sqrat::Object ScriptSerializeInt( Sqrat::Object i_sqParam ) override;
  Sqrat::Object ScriptSerializeNull( Sqrat::Object i_sqParam ) override;
  Sqrat::Object ScriptSerializeString( Sqrat::Object i_sqParam ) override;
  Sqrat::Object ScriptSerializeTable( Sqrat::Object i_sqParam ) override;

  //int loadArchive(const std::string& strArchive);
  int32_t LoadFile( const std::string& i_strFilePath );
  int32_t LoadFileFromArchive( const std::string& i_strFilePath, const std::string& i_strArchive );
  void LoadMemoryFile( const char* i_strBuffer, size_t i_iSize );

  bool IsEndOfFile() override
  {
    return ( GetPos() >= m_uiDataSize ? true : false );
  }

  size_t GetPos() const override
  {
    return m_pcLoadOffset - m_pcMemFile;
  }

  // Rewind to the beginning of the data
  void Reset() override;

  void SeekPos( size_t i_uiPos ) override;

private:

  void Release() override;

  void AllocateBuffer( size_t i_uiSize );

  // The actual size of the loaded data. Note that the memory buffer can be larger than the loaded data size.
  size_t m_uiDataSize{ 0 };

  typedef rumResource super;
};


class rumResourceSaver : public rumResource
{
public:

  rumResourceSaver();
  rumResourceSaver( size_t i_uiSize );

  bool IsLoading() const override
  {
    return false;
  }

  bool IsSaving() const override
  {
    return true;
  }

  bool IsEndOfFile() override;
  size_t GetPos() const override
  {
    return m_pcSaveOffset - m_pcMemFile;
  }

  Sqrat::Object ScriptSerializeArray( Sqrat::Object i_sqParam ) override;
  Sqrat::Object ScriptSerializeBool( Sqrat::Object i_sqParam ) override;
  Sqrat::Object ScriptSerializeClass( Sqrat::Object i_sqParam ) override;
  Sqrat::Object ScriptSerializeFloat( Sqrat::Object i_sqParam ) override;
  Sqrat::Object ScriptSerializeInstance( Sqrat::Object i_sqParam ) override;
  Sqrat::Object ScriptSerializeInt( Sqrat::Object i_sqParam ) override;
  Sqrat::Object ScriptSerializeNull( Sqrat::Object i_sqParam ) override;
  Sqrat::Object ScriptSerializeString( Sqrat::Object i_sqParam ) override;
  Sqrat::Object ScriptSerializeTable( Sqrat::Object i_sqParam ) override;

  // Rewind to the beginning of the data
  void Reset() override;

  int32_t SaveFile( const std::string& i_strFilename );

  void SeekPos( size_t i_uiPos ) override;

  void SetRawData( const char* i_pcData, uint32_t i_uiNumBytes );

private:

  void AllocateBuffer( size_t i_uiSize );
};

#endif // _U_RESOURCE_H_
