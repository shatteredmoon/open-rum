#include <u_resource.h>

#include <u_asset.h>
#include <u_assert.h>
#include <u_log.h>
#include <u_object.h>
#include <u_structs.h>
#include <u_utility.h>
#include <u_zlib.h>

#include <filesystem>
#include <fstream>

// Static initializers
rumResource::FolderTreeContainer rumResource::s_cFileCacheMap;


rumResource::~rumResource()
{
  Release();
}


// static
int32_t rumResource::CreateFileCache( const std::string& i_strPath )
{
  int32_t numFiles{ 0 };

  // Throw away the current cache
  s_cFileCacheMap.clear();

  std::string strInfo{ "Caching project path: " };
  strInfo += i_strPath;
  Logger::LogStandard( strInfo );

  const std::filesystem::path fsPath( i_strPath );
  if( !std::filesystem::is_directory( fsPath ) )
  {
    std::string strWarning{ "Failed to created file cache for folder: " };
    strWarning += i_strPath;
    Logger::LogStandard( strWarning );
    return 0;
  }

  for( const auto& iter : std::filesystem::recursive_directory_iterator( fsPath ) )
  {
    try
    {
#ifdef RESOURCE_DEBUG
      Logger::LogStandard( iter.path().string() );
#endif // RESOURCE_DEBUG

      if( !std::filesystem::is_directory( iter.path() ) )
      {
        ++numFiles;
        s_cFileCacheMap.insert( std::make_pair( iter.path().filename().generic_string(),
                                                iter.path().generic_string() ) );
      }
    }
    catch( const std::exception& rcException )
    {
      std::string strError{ "Error: " };
      strError += iter.path().string();
      strError += " ";
      strError += rcException.what();
      Logger::LogStandard( strError );
    }
  }

  return numFiles;
}


// static
bool rumResource::FileCacheFind( const std::string& i_strFile, std::vector<std::string>& o_rcResultsVector )
{
  std::filesystem::path fsFile( i_strFile );
  if( fsFile.has_parent_path() )
  {
    // The path cache only caches filenames, not path info
    fsFile = fsFile.filename();
  }

  auto cRange{ s_cFileCacheMap.equal_range( fsFile.string() ) };
  for( auto& iter{ cRange.first }; iter != cRange.second; ++iter )
  {
    o_rcResultsVector.push_back( iter->second );
  }

  return !s_cFileCacheMap.empty();
}


// static
bool rumResource::FileExists( const std::string& i_strPath )
{
  bool bExists{ false };

  std::ifstream infile( i_strPath.c_str(), std::ios::in );
  if( infile.is_open() )
  {
    bExists = true;
    infile.close();
  }

  return bExists;
}


// static
bool rumResource::FindFile( const std::string& i_strFile, std::string& o_strResult )
{
  o_strResult.clear();

  std::vector<std::string> vResults;
  const bool bSearched{ FileCacheFind( i_strFile, vResults ) };
  if( vResults.size() > 0 )
  {
    o_strResult = vResults[0];

    if( vResults.size() > 1 )
    {
      std::string strWarn{ "Warning: Multiple occurrences of the file " };
      strWarn += i_strFile;
      strWarn += " were found in the path cache. Using the first encountered file: ";
      strWarn += vResults[0];
      Logger::LogStandard( strWarn, Logger::LOG_WARNING );
    }
  }

  if( !bSearched )
  {
    RecursiveFind( i_strFile, GetProjectPath(), o_strResult );
  }

  return !o_strResult.empty();
}


// static
bool rumResource::FindFile( const std::string& i_strFile, std::vector<std::string>& o_rcResultsVector )
{
  o_rcResultsVector.clear();

  if( !FileCacheFind( i_strFile, o_rcResultsVector ) )
  {
    RecursiveFind( i_strFile, GetProjectPath(), o_rcResultsVector );
  }

  return !o_rcResultsVector.empty();
}


// static
bool rumResource::RecursiveFind( const std::string& i_strFile, const std::string& i_strPath, std::string& o_strResult )
{
#ifdef RESOURCE_DEBUG
  std::string strInfo{ "Starting recursive find of file " };
  strInfo += strFile;
  strInfo += " starting at folder ";
  strInfo += strPath;
  Logger::LogStandard( strInfo );
#endif // RESOURCE_DEBUG

  std::filesystem::path fsFile( i_strFile );
  if( fsFile.has_parent_path() )
  {
    // Only strPath should include path info
    fsFile = fsFile.filename();
  }

  const std::filesystem::path fsPath( fsFile );
  if( !std::filesystem::is_directory( fsPath ) )
  {
    return false;
  }

  Logger::LogStandard( "Performance warning: Recursive file search", Logger::LOG_WARNING );

  bool bFound{ false };

  std::filesystem::recursive_directory_iterator end, iter( fsPath );
  while( !bFound && iter != end )
  {
    try
    {
#ifdef RESOURCE_DEBUG
      Logger::LogStandard( iter->path().filename().string() );
#endif // RESOURCE_DEBUG

      if( !std::filesystem::is_directory( iter->path() ) )
      {
        if( i_strFile.compare( iter->path().filename().string() ) == 0 )
        {
          bFound = true;
          o_strResult = iter->path().string();
        }
      }
    }
    catch( const std::exception& rcException )
    {
      std::string strError{ "Error: " };
      strError += iter->path().string();
      strError += " ";
      strError += rcException.what();
      Logger::LogStandard( strError );
    }

    ++iter;
  }

  return bFound;
}


// static
bool rumResource::RecursiveFind( const std::string& i_strFile, const std::string& i_strPath,
                                 std::vector<std::string>& o_rcResultsVector )
{
#ifdef RESOURCE_DEBUG
  std::string strInfo{ "Starting recursive find of file " };
  strInfo += strFile;
  strInfo += " starting at folder ";
  strInfo += strPath;
  Logger::LogStandard( strInfo );
#endif // RESOURCE_DEBUG

  std::filesystem::path fsFile( i_strFile );
  if( fsFile.has_parent_path() )
  {
    // Only strPath should include path info
    fsFile = fsFile.filename();
  }

  const std::filesystem::path fsPath( i_strPath );
  if( !std::filesystem::is_directory( fsPath ) )
  {
    return false;
  }

  Logger::LogStandard( "Performance warning: Recursive file search", Logger::LOG_WARNING );

  bool bFound{ false };

  for( const auto& iter : std::filesystem::recursive_directory_iterator( fsPath ) )
  {
    try
    {
#ifdef RESOURCE_DEBUG
      Logger::LogStandard( dir->path().filename().string() );
#endif // RESOURCE_DEBUG

      if( !std::filesystem::is_directory( iter.path() ) )
      {
        if( i_strFile.compare( iter.path().filename().string() ) == 0 )
        {
          bFound = true;
          o_rcResultsVector.push_back( iter.path().string() );
        }
      }
    }
    catch( const std::exception& rcException )
    {
      std::string strError{ "Error: " };
      strError += iter.path().string();
      strError += " ";
      strError += rcException.what();
      Logger::LogStandard( strError );
    }
  }

  return bFound;
}


void rumResource::Release()
{
  if( m_pcMemFile != nullptr )
  {
    delete[] m_pcMemFile;
    m_pcLoadOffset = m_pcSaveOffset = m_pcMemFile = nullptr;
    m_uiMemSize = 0;
  }
}


void rumResource::ScriptBind()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::RootTable( pcVM )
    .Func( "rumWriteStringToFile", &WriteStringToFile );

  Sqrat::Class<rumResource, Sqrat::NoConstructor<rumResource>> cResource( pcVM, "rumResource" );
  cResource
    .Func( "IsLoading", &IsLoading )
    .Func( "IsSaving", &IsSaving )
    .Func( "Serialize", &ScriptSerialize )
    .Func( "SerializeBool", &ScriptSerializeBool )
    .Func( "SerializeFloat", &ScriptSerializeFloat )
    .Func( "SerializeInt", &ScriptSerializeInt )
    .Func( "SerializeString", &ScriptSerializeString );
  Sqrat::RootTable( pcVM ).Bind( "rumResource", cResource );

  Sqrat::DerivedClass<rumResourceLoader, rumResource> cResourceLoader( pcVM, "rumResourceLoader" );
  Sqrat::RootTable( pcVM ).Bind( "rumResourceLoader", cResourceLoader );

  Sqrat::DerivedClass<rumResourceSaver, rumResource> cResourceSaver( pcVM, "rumResourceSaver" );
  Sqrat::RootTable( pcVM ).Bind( "rumResourceSaver", cResourceSaver );
}


Sqrat::Object rumResource::ScriptSerialize( Sqrat::Object i_sqParam )
{
  switch( i_sqParam.GetType() )
  {
    case OT_ARRAY:    i_sqParam = ScriptSerializeArray( i_sqParam );    break;
    case OT_BOOL:     i_sqParam = ScriptSerializeBool( i_sqParam );     break;
    case OT_CLASS:    i_sqParam = ScriptSerializeClass( i_sqParam );    break;
    case OT_FLOAT:    i_sqParam = ScriptSerializeFloat( i_sqParam );    break;
    case OT_INSTANCE: i_sqParam = ScriptSerializeInstance( i_sqParam ); break;
    case OT_INTEGER:  i_sqParam = ScriptSerializeInt( i_sqParam );      break;
    case OT_NULL:     i_sqParam = ScriptSerializeNull( i_sqParam );     break;
    case OT_STRING:   i_sqParam = ScriptSerializeString( i_sqParam );   break;
    case OT_TABLE:    i_sqParam = ScriptSerializeTable( i_sqParam );    break;
    case OT_CLOSURE:
    case OT_NATIVECLOSURE: /* do nothing */ break;
    default:
    {
      std::string strInfo{ "Serialization attempted on unsupported script type: " };
      strInfo += rumScript::GetObjectTypeName( i_sqParam );
      Logger::LogStandard( strInfo );
    }

    rumScript::SetValue( i_sqParam, nullptr );
    break;
  }

  return i_sqParam;
}


rumResource& rumResource::Serialize( const std::string& i_strOutput )
{
  rumAssertMsg( IsSaving(), "Serialize expected to be saving, not loading" );

  if( ( i_strOutput.length() + GetPos() ) > m_uiMemSize )
  {
    if( !IsEndOfFile() )
    {
      m_pcSaveOffset = rumSerializationUtils::Write( i_strOutput, m_pcSaveOffset );
    }
  }
  else
  {
    m_pcSaveOffset = rumSerializationUtils::Write( i_strOutput, m_pcSaveOffset );
  }

  return *this;
}


rumResource& rumResource::Serialize( const char* i_strOutput )
{
  rumAssertMsg( IsSaving(), "Serialize expected to be saving, not loading" );

  if( ( strlen( i_strOutput ) + GetPos() ) > m_uiMemSize )
  {
    if( !IsEndOfFile() )
    {
      m_pcSaveOffset = rumSerializationUtils::Write( i_strOutput, m_pcSaveOffset );
    }
  }
  else
  {
    m_pcSaveOffset = rumSerializationUtils::Write( i_strOutput, m_pcSaveOffset );
  }

  return *this;
}


// static
bool rumResource::WriteStringToFile( const char* i_strFilePath, const char* i_strOutput )
{
  bool bWritten{ false };

  std::ofstream cOutFile( i_strFilePath, std::ios::out | std::ios::trunc );
  if( cOutFile.is_open() )
  {
    cOutFile << i_strOutput;
    cOutFile.close();

    bWritten = true;
  }

  return bWritten;
}


rumResourceLoader::rumResourceLoader( size_t i_uiSize )
{
  AllocateBuffer( i_uiSize );
}


void rumResourceLoader::AllocateBuffer( size_t i_uiSize )
{
  rumAssert( i_uiSize > 0 );

  m_pcLoadOffset = m_pcMemFile = new char[i_uiSize];
  if( m_pcMemFile != nullptr )
  {
    m_uiMemSize = i_uiSize;
  }
}


/*int32_t rumResourceLoader::loadArchive(const std::string& archive)
{
    int32_t size = 0;

    release();

    zlib::archive_reader ar(archive);
    zlib::file_info info = ar.findFile(filename);
    if (info.m_lSizeUncompressed > 0)
    {
        // Create a buffer large enough to hold the entire file
        m_pcOffset = m_pcMemFile = new char[info.m_lSizeUncompressed];
        assert(m_pcMemFile != NULL);
        size = ar.extractToBuffer(m_pcMemFile);
        assert(size == info.m_lSizeUncompressed);
    }

    return size;
}*/


int32_t rumResourceLoader::LoadFile( const std::string& i_strFilePath )
{
  std::ifstream::pos_type iOffset{ 0 };

  std::ifstream cInFile( i_strFilePath.c_str(), std::ios::in | std::ios::binary | std::ios::ate );
  if( cInFile.is_open() )
  {
    // Get file size
    iOffset = cInFile.tellg();
    const auto uiSize{ (size_t)iOffset };
    cInFile.seekg( 0, std::ios::beg );

    if( m_uiMemSize < uiSize )
    {
      Release();

      m_pcMemFile = new char[uiSize];
      m_uiMemSize = uiSize;
    }

    m_pcLoadOffset = m_pcMemFile;

    // Create a buffer large enough to hold the entire file
    if( m_pcMemFile != nullptr )
    {
      cInFile.read( m_pcMemFile, iOffset );
    }

    m_uiDataSize = uiSize;

    cInFile.close();
  }

  return (int32_t)iOffset;
}


int32_t rumResourceLoader::LoadFileFromArchive( const std::string& i_strFilePath, const std::string& i_strArchive )
{
  ArchiveReader cArchive( i_strArchive );
  if( cArchive.IsValid() )
  {
    zlib::rumFileInfo cFileInfo;
    const bool bFound{ cArchive.FindFile( i_strFilePath, cFileInfo ) };
    if( bFound && cFileInfo.m_uiSizeUncompressed > 0 )
    {
      if( m_uiMemSize < cFileInfo.m_uiSizeUncompressed )
      {
        Release();

        // Create a buffer large enough to hold the entire file
        m_pcMemFile = new char[cFileInfo.m_uiSizeUncompressed];
        rumAssert( m_pcMemFile != nullptr );

        m_uiMemSize = cFileInfo.m_uiSizeUncompressed;
      }

      m_pcLoadOffset = m_pcMemFile;

      if( m_pcMemFile )
      {
        const auto uiSizeExtracted{ cArchive.ExtractToBuffer( m_pcMemFile ) };
        rumAssert( uiSizeExtracted <= m_uiMemSize );
        m_uiDataSize = uiSizeExtracted;
      }
    }
  }

  return (int32_t)m_uiMemSize;
}


void rumResourceLoader::LoadMemoryFile( const char* i_strBuffer, size_t i_uiSize )
{
  if( m_uiMemSize < i_uiSize )
  {
    // Not enough memory to hold the input buffer, so resize
    Release();
    m_pcMemFile = new char[i_uiSize];
    m_uiMemSize = i_uiSize;
  }

  m_uiDataSize = i_uiSize;

  memcpy( m_pcMemFile, i_strBuffer, i_uiSize );
  m_pcLoadOffset = m_pcMemFile + ( sizeof( char ) * i_uiSize );
}


void rumResourceLoader::Release()
{
  m_uiDataSize = 0;
  super::Release();
}


void rumResourceLoader::Reset()
{
  m_pcLoadOffset = m_pcMemFile;
}


Sqrat::Object rumResourceLoader::ScriptSerializeArray( Sqrat::Object i_sqParam )
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  // The number of items in the array
  rumDWord uiNumItems{ 0 };
  Serialize( uiNumItems );

  Sqrat::Array sqArray( pcVM, (int32_t)uiNumItems );
  i_sqParam = sqArray;

  // Get the parent class for iteration
  rumDWord eTypeID{ 0 };
  for( rumDWord i = 0; i < uiNumItems && !IsEndOfFile(); ++i )
  {
    // Get the variable type
    Serialize( eTypeID );

    Sqrat::Object sqValue;

    // Get the value
    switch( eTypeID )
    {
      case OT_BOOL:    sqValue = ScriptSerializeBool( sqValue );   break;
      case OT_CLASS:   sqValue = ScriptSerializeClass( sqValue );  break;
      case OT_FLOAT:   sqValue = ScriptSerializeFloat( sqValue );  break;
      case OT_INTEGER: sqValue = ScriptSerializeInt( sqValue );    break;
      case OT_NULL:    sqValue = ScriptSerializeNull( sqValue );   break;
      case OT_STRING:  sqValue = ScriptSerializeString( sqValue ); break;
      case NULL:       break;

        // If it's not above, it's not supported and therefore very dangerous
      default:
        rumAssertArgs( false, "Unsupported script type id: ", eTypeID );
        break;
    }

    sqArray.SetValue( i, sqValue );
  }

  i_sqParam = sqArray;

  return sqArray;
}


Sqrat::Object rumResourceLoader::ScriptSerializeBool( Sqrat::Object i_sqParam )
{
  bool bVal{ false };
  Serialize( bVal );

  rumScript::SetValue( i_sqParam, bVal );
  return i_sqParam;
}


Sqrat::Object rumResourceLoader::ScriptSerializeFloat( Sqrat::Object i_sqParam )
{
  float fVal{ 0.0f };
  Serialize( fVal );

  rumScript::SetValue( i_sqParam, fVal );
  return i_sqParam;
}


Sqrat::Object rumResourceLoader::ScriptSerializeClass( Sqrat::Object i_sqParam )
{
  // Serializing a class is no longer supported
  rumAssert( false );

  return Sqrat::Object();
}


Sqrat::Object rumResourceLoader::ScriptSerializeInt( Sqrat::Object i_sqParam )
{
  rumQWord uiVal{ 0 };
  Serialize( uiVal );

  rumScript::SetValue( i_sqParam, (uint64_t)uiVal );
  return i_sqParam;
}


Sqrat::Object rumResourceLoader::ScriptSerializeNull( Sqrat::Object i_sqParam )
{
  Sqrat::Object sqObject;
  return sqObject;
}


Sqrat::Object rumResourceLoader::ScriptSerializeString( Sqrat::Object i_sqParam )
{
  std::string strVal;
  Serialize( strVal );

  rumScript::SetValue( i_sqParam, strVal );
  return i_sqParam;
}


Sqrat::Object rumResourceLoader::ScriptSerializeTable( Sqrat::Object i_sqParam )
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  // The number of items in the table
  rumDWord uiNumItems{ 0 };
  Serialize( uiNumItems );

  Sqrat::Table sqTable( pcVM );

  rumDWord eTypeID{ 0 };
  for( rumDWord i = 0; i < uiNumItems && !IsEndOfFile(); ++i )
  {
    Sqrat::Object sqKey, sqValue;

    // Get the key type
    Serialize( eTypeID );

    // Read the key's value
    switch( eTypeID )
    {
      case OT_BOOL:    sqKey = ScriptSerializeBool( sqKey );   break;
      case OT_CLASS:   sqKey = ScriptSerializeClass( sqKey );  break;
      case OT_FLOAT:   sqKey = ScriptSerializeFloat( sqKey );  break;
      case OT_INTEGER: sqKey = ScriptSerializeInt( sqKey );    break;
      case OT_STRING:  sqKey = ScriptSerializeString( sqKey ); break;

        // If it's not above, it's not supported and therefore very dangerous
      default: rumAssertArgs( false, "Unsupported script type id: ", eTypeID ); break;
    }

    // Get the value type
    Serialize( eTypeID );

    // Read the value
    switch( eTypeID )
    {
      case OT_BOOL:    sqValue = ScriptSerializeBool( sqValue );   break;
      case OT_CLASS:   sqValue = ScriptSerializeClass( sqValue );  break;
      case OT_FLOAT:   sqValue = ScriptSerializeFloat( sqValue );  break;
      case OT_INTEGER: sqValue = ScriptSerializeInt( sqValue );    break;
      case OT_NULL:    sqValue = ScriptSerializeNull( sqValue );   break;
      case OT_STRING:  sqValue = ScriptSerializeString( sqValue ); break;
      case NULL:       break;

        // If it's not above, it's not supported and therefore very dangerous
      default:
        rumAssertArgs( false, "Unsupported script type id: ", eTypeID );
        break;
    }

    rumScript::SetSlot( sqTable, sqKey, sqValue );
  }

  i_sqParam = sqTable;

  return sqTable;
}


Sqrat::Object rumResourceLoader::ScriptSerializeInstance( Sqrat::Object i_sqParam )
{
  rumAssetID eAssetID{ INVALID_ASSET_ID };
  Serialize( eAssetID );

#ifdef RESOURCE_DEBUG
  rumAsset* pcAsset{ rumAsset::Fetch( eAssetID ) };
  std::string strInfo{ "Reading script instance " };
  strInfo += pcAsset ? pcAsset->GetName() : " <unknown> ";
  Logger::LogStandard( strInfo );
#endif // RESOURCE_DEBUG

  // Create an instance of this class
  Sqrat::Object sqInstance{ rumGameObject::Create( eAssetID ) };
  if( sqInstance.GetType() == OT_INSTANCE )
  {
    std::string strKey;
    Sqrat::Object sqValue;
    rumDWord eTypeID{ 0 };

    // Determine the next object we're going to serialize
    Serialize( eTypeID );
    while( !IsEndOfFile() && eTypeID != 0 )
    {
      // Get the key name
      Serialize( strKey );

      bool bSerializable{ true };

      // Get the value
      switch( eTypeID )
      {
        case OT_ARRAY:    sqValue = ScriptSerializeArray( sqValue );    break;
        case OT_BOOL:     sqValue = ScriptSerializeBool( sqValue );     break;
          // Note - only registered classes can be serialized! If the value comes back NULL, the class is not
          // being derived from a RUM class
        case OT_CLASS:    sqValue = ScriptSerializeClass( sqValue );    break;
        case OT_FLOAT:    sqValue = ScriptSerializeFloat( sqValue );    break;
        case OT_INSTANCE: sqValue = ScriptSerializeInstance( sqValue ); break;
        case OT_INTEGER:  sqValue = ScriptSerializeInt( sqValue );      break;
        case OT_NULL:     sqValue = ScriptSerializeNull( sqValue );     break;
        case OT_STRING:   sqValue = ScriptSerializeString( sqValue );   break;
        case OT_TABLE:    sqValue = ScriptSerializeTable( sqValue );    break;

          // If it's not above, it's not supported and therefore very dangerous
        default:
          bSerializable = false;
          break;
          /*{
              SQUserPointer typeTag;
              sqParam.GetTypeTag(&typeTag);

              if (UniversalID::GetScriptTypeTag() == typeTag)
              {
                  UniversalID *pcUniversalID = UniversalID::GetNativePointer(sqParam);
                  pcUniversalID->Serialize(*this);
              }
              else
              {
                  // Not handled
                  //assert(false);
              }
          }*/
      }

      if( bSerializable )
      {
        // Store the result
        Sqrat::Object sqKey;
        rumScript::SetValue( sqKey, strKey.c_str() );
        rumScript::SetSlot( sqInstance, sqKey, sqValue );
      }

#ifdef RESOURCE_DEBUG
      strInfo = "  Key = ";
      strInfo += strKey;
      strInfo += ", Value = ";
      strInfo += ToString( sqValue );
      Logger::LogStandard( strInfo );
#endif // RESOURCE_DEBUG

      // Get the next type
      Serialize( eTypeID );
    }
  }

  return sqInstance;
}


// virtual
void rumResourceLoader::SeekPos( size_t i_uiPos )
{
  if( i_uiPos < 0 )
  {
    // Seek to front
    Reset();
  }
  else if( i_uiPos <= m_uiMemSize )
  {
    // Seek to pos
    m_pcLoadOffset = m_pcMemFile + i_uiPos;
  }
  else
  {
    // Seek to end
    m_pcLoadOffset = m_pcMemFile + m_uiMemSize;
  }
}


rumResourceSaver::rumResourceSaver()
{
  // Default to 1 megabyte of storage space
  static size_t uiSize{ sizeof( char ) * 1024 * 1024 * 1 };
  AllocateBuffer( uiSize );
}


rumResourceSaver::rumResourceSaver( size_t i_uiSize )
{
  AllocateBuffer( i_uiSize );
}


void rumResourceSaver::AllocateBuffer( size_t i_uiSize )
{
  rumAssert( i_uiSize > 0 );

  m_pcSaveOffset = m_pcMemFile = new char[i_uiSize];
  if( m_pcMemFile != nullptr )
  {
    m_uiMemSize = i_uiSize;
  }
}


// virtual
bool rumResourceSaver::IsEndOfFile()
{
  bool bStopWriting{ true };

  // This function does not request a check to see if we are at the end of a file, since we are currently building a
  // buffer. This function tells the saver that we have reached the end of available memory and need more.

#ifdef RESOURCE_DEBUG
  Logger::LogStandard( "Save buffer is out of room, allocating more space..." );
#endif

  const size_t iUsedSize{ GetPos() };

  // Double the size of available memory
  const size_t uiNewSize{ m_uiMemSize * 2 };
  rumAssert( m_uiMemSize > 0 && uiNewSize > m_uiMemSize );

  char* strBuffer{ new char[uiNewSize] };
  if( strBuffer != nullptr )
  {
#ifdef RESOURCE_DEBUG
    std::string strInfo{ "Allocated " };
    strInfo += ToString( uiNewSize );
    strInfo += " bytes";
    Logger::LogStandard( strInfo );
#endif

    m_uiMemSize = uiNewSize;
    memcpy( strBuffer, m_pcMemFile, sizeof( char ) * iUsedSize );
    Release();
    m_pcMemFile = strBuffer;
    m_pcSaveOffset = m_pcMemFile + iUsedSize;
    m_uiMemSize = uiNewSize;

    // More memory was allocated, we are no longer potentially eof
    bStopWriting = false;
  }

  return bStopWriting;
}


void rumResourceSaver::Reset()
{
  m_pcSaveOffset = m_pcMemFile;
}


int32_t rumResourceSaver::SaveFile( const std::string& i_strFilename )
{
  int32_t eResult{ RESULT_FAILED };

  std::ofstream cOutFile( i_strFilename.c_str(), std::ios::out | std::ios::binary | std::ios::trunc );
  if( cOutFile.is_open() )
  {
    cOutFile.write( m_pcMemFile, GetPos() );
    cOutFile.close();

    eResult = RESULT_SUCCESS;
  }

  return eResult;
}


Sqrat::Object rumResourceSaver::ScriptSerializeArray( Sqrat::Object i_sqParam )
{
  rumAssertMsg( i_sqParam.GetType() == OT_ARRAY, "sqParam expected to be a script array" );

  if( i_sqParam.GetType() != OT_ARRAY )
  {
    return Sqrat::Object();
  }

  Sqrat::Array sqArray( i_sqParam );

  // The number of items in the array
  uint32_t uiNumItems{ (uint32_t)sqArray.GetSize() };
  Serialize( uiNumItems );

  for( uint32_t i = 0; i < uiNumItems; ++i )
  {
    // Get the next array value
    Sqrat::SharedPtr<Sqrat::Object> sqValue{ sqArray.GetValue<Sqrat::Object>( i ) };

    // Write the value type id
    Serialize( (rumDWord)sqValue->GetType() );

    // Serialize the value
    ScriptSerialize( *sqValue );
  }

  return i_sqParam;
}


Sqrat::Object rumResourceSaver::ScriptSerializeClass( Sqrat::Object i_sqParam )
{
  // Serializing a class is no longer supported
  rumAssert( false );

  return i_sqParam;
}


Sqrat::Object rumResourceSaver::ScriptSerializeBool( Sqrat::Object i_sqParam )
{
  bool bVal{ i_sqParam.Cast<bool>() };
  Serialize( bVal );

  Sqrat::Object sqObject;
  rumScript::SetValue( sqObject, bVal );

  return sqObject;
}


Sqrat::Object rumResourceSaver::ScriptSerializeFloat( Sqrat::Object i_sqParam )
{
  float fVal{ i_sqParam.Cast<float>() };
  Serialize( fVal );

  Sqrat::Object sqObject;
  rumScript::SetValue( sqObject, fVal );

  return sqObject;
}


Sqrat::Object rumResourceSaver::ScriptSerializeInt( Sqrat::Object i_sqParam )
{
  rumQWord iVal{ (rumQWord)i_sqParam.Cast<int64_t>() };
  Serialize( iVal );

  Sqrat::Object sqObject;
  rumScript::SetValue( sqObject, iVal );

  return sqObject;
}


Sqrat::Object rumResourceSaver::ScriptSerializeNull( Sqrat::Object i_sqParam )
{
  return Sqrat::Object();
}


Sqrat::Object rumResourceSaver::ScriptSerializeString( Sqrat::Object i_sqParam )
{
  std::string strVal{ i_sqParam.Cast<std::string>() };
  Serialize( strVal );

  Sqrat::Object sqObject;
  rumScript::SetValue( sqObject, strVal );

  return sqObject;
}


Sqrat::Object rumResourceSaver::ScriptSerializeTable( Sqrat::Object i_sqParam )
{
  rumAssertMsg( i_sqParam.GetType() == OT_TABLE, "sqParam expected to be a script table" );

  if( i_sqParam.GetType() != OT_TABLE )
  {
    return Sqrat::Object();
  }

  Sqrat::Array sqArray( i_sqParam );

  // The number of items in the array
  rumDWord uiNumItems{ (rumDWord)sqArray.GetSize() };
  Serialize( uiNumItems );

  Sqrat::Object sqKey, sqValue;
  Sqrat::Object::iterator iter;
  while( sqArray.Next( iter ) )
  {
    sqKey = iter.getKey();
    sqValue = iter.getValue();

#ifdef RESOURCE_DEBUG
    std::string strInfo{ "  Key = " };
    strInfo += rumScript::ToString( sqKey );
    strInfo += ", Value = ";
    strInfo += rumScript::ToString( sqValue );
    Logger::LogStandard( strInfo );
#endif

    // Write the key's type id
    Serialize( (rumDWord)sqKey.GetType() );

    // Write the key
    ScriptSerialize( sqKey );

    // Write the value's type id
    Serialize( (rumDWord)sqValue.GetType() );

    // Write the value
    ScriptSerialize( sqValue );
  }

  return i_sqParam;
}


Sqrat::Object rumResourceSaver::ScriptSerializeInstance( Sqrat::Object i_sqParam )
{
  const auto cPair{ rumScript::EvalRequiredFunc( i_sqParam, "GetAssetID", INVALID_ASSET_ID ) };
  const rumAssetID eAssetID{ cPair.second };
  if( !cPair.first || ( INVALID_ASSET_ID == cPair.second ) )
  {
    return Sqrat::Object();
  }

  // Get the class for member iteration
  Sqrat::Object sqClass{ rumScript::GetClass( i_sqParam ) };

#ifdef RESOURCE_DEBUG
  rumAsset* pcAsset{ rumAsset::Fetch( eAssetID ) };
  std::string strInfo{ "Writing script instance " };
  strInfo += pcAsset ? pcAsset->GetName() : " <unknown> ";
  Logger::LogStandard( strInfo );
  rumScript::sq_print_iteration( sqClass );
#endif // RESOURCE_DEBUG

  std::string strKey;

  Serialize( (rumDWord)eAssetID );

  if( sqClass.GetType() == OT_CLASS )
  {
    Sqrat::Object sqKey, sqValue;

    Sqrat::Object::iterator iter;
    while( sqClass.Next( iter ) )
    {
      sqKey = iter.getKey();
      sqValue = iter.getValue();

      // Never attempt to serialize closures of any kind
      if( ( sqKey.GetType() == OT_STRING ) &&
          sqValue.GetType() != OT_NATIVECLOSURE &&
          sqValue.GetType() != OT_CLOSURE )
      {
        strKey = sqKey.Cast<std::string>();

        // Never package anything starting with double underscores
        if( !( ( strKey[0] == '_' ) && ( strKey[1] == '_'  ) ) )
        {
          // Get the value from the instance
          sqValue = i_sqParam.GetSlot( strKey.c_str() );

#ifdef RESOURCE_DEBUG
          strInfo = "  Key = ";
          strInfo += strKey;
          strInfo += ", Value = ";
          strInfo += ToString( sqValue );
          Logger::LogStandard( strInfo );
#endif // RESOURCE_DEBUG

          // Write the value's type id
          Serialize( (rumDWord)sqValue.GetType() );

          // Write the key name
          Serialize( strKey );

          // Write the value
          ScriptSerialize( sqValue );
        }
      }
    }
  }

  // Maybe someday - serialize the native portion of a registered class
  /*SQUserPointer typeTag;
  sqParam.GetTypeTag(&typeTag);

  if (UniversalID::GetScriptTypeTag() == typeTag)
  {
      UniversalID *pcUniversalID = UniversalID::GetNativePointer(sqParam);
      pcUniversalID->Serialize(*this);
  }*/

  // Write NULL type so the deserializer knows to stop
  Serialize( NULL );

  return i_sqParam;
}


// virtual
void rumResourceSaver::SeekPos( size_t i_uiPos )
{
  if( i_uiPos < 0 )
  {
    // Seek to front
    Reset();
  }
  else if( i_uiPos <= m_uiMemSize )
  {
    // Seek to pos
    m_pcSaveOffset = m_pcMemFile + i_uiPos;
  }
  else
  {
    // Seek to end
    m_pcSaveOffset = m_pcMemFile + m_uiMemSize;
  }
}

void rumResourceSaver::SetRawData( const char* i_strData, uint32_t i_uiNumBytes )
{
  Release();
  AllocateBuffer( i_uiNumBytes );
  memcpy( m_pcMemFile, i_strData, sizeof( rumByte ) * i_uiNumBytes );
  m_pcSaveOffset = m_pcMemFile + i_uiNumBytes;
}
