#include <u_sound_asset.h>

#include <u_resource.h>
#include <u_sound.h>
#include <u_zlib.h>

#include <filesystem>

#define ASSET_NATIVE_CLASS "rumSoundAsset"
#define ASSET_STORAGE_NAME "sound"
#define ASSET_TYPE_SUFFIX  "_Sound"

// Static initializations
std::unordered_map< rumAssetID, rumSoundAsset* > rumSoundAsset::s_hashAssets;
rumAssetID rumSoundAsset::s_eNextFreeID{ FULL_ASSET_ID( Sound_AssetType, 0 ) };
std::string rumSoundAsset::s_strPathHint;


void rumSoundAsset::DetermineSoundDataType()
{
  rumSoundDataType eType{ Invalid_SoundDataType };
  const rumByte* pcData{ (const rumByte*)m_pcData };

  if( pcData && m_uiAllocationSize > 4 )
  {
    if( pcData[0] == 'M' && pcData[1] == 'T' && pcData[2] == 'h' && pcData[3] == 'd' )
    {
      eType = MIDI_SoundDataType;
    }
    else if( pcData[0] == 'R' && pcData[1] == 'I' && pcData[2] == 'F' && pcData[3] == 'F' )
    {
      eType = WAV_SoundDataType;
    }
    else if( m_uiAllocationSize > 48 &&
             pcData[0] == 'O' && pcData[1] == 'g' && pcData[2] == 'g' && pcData[3] == 'S' )
    {
      eType = OGG_SoundDataType;
    }
    else if( m_uiAllocationSize > 12 &&
             pcData[0] == 'F' && pcData[1] == 'O' && pcData[2] == 'R' && pcData[3] == 'M' &&
             pcData[0] == 'A' && pcData[1] == 'I' && pcData[2] == 'F' && pcData[3] == 'F' )
    {
      eType = AIFF_SoundDataType;
    }
    else
    {
      if( pcData[0] == 'I' && pcData[1] == 'D' && pcData[2] == '3' && m_uiAllocationSize > 10 )
      {
        bool bFooterPresent = false;
        uint32_t eFlags = pcData[5];

        // The ID3v2 tag size is stored as a 32 bit synchsafe integer, making a total of 28 effective bits
        // (representing up to 256MB)
        // Pattern: A*2^21 + B*2^14 + C*2^7 + D
        uint32_t uiOffset = pcData[6] * 2097152 + pcData[7] * 16384 + pcData[8] * 128 + pcData[9];
        if( eFlags & 0x10 )
        {
          // A 10-byte footer exists at the end of the ID3 data
          uiOffset += 10;
          m_uiAllocationSize -= 10;
        }

        // Subtract off the ID3 header and metadata size (optional footer is already accounted for)
        m_uiAllocationSize -= ( uiOffset + 10 );
      }

      if( m_uiAllocationSize > 3 )
      {
        if( pcData[0] == 0xff && ( pcData[2] & 0xe0 ) == 0xe0 )
        {
          // Ideally, more than one frame would be tested
          eType = MP3_SoundDataType;
        }
      }
    }
  }

  m_eDataType = eType;
}


// final
void rumSoundAsset::ExportCSVFile( std::ofstream& o_rcOutfile ) const
{
  super::ExportCSVFile( o_rcOutfile );

  // Done
  o_rcOutfile << '\n';
}


// static
void rumSoundAsset::ExportCSVFiles( std::ofstream& o_rcOutfile, std::ofstream& o_rcPropertyOutfile )
{
  // Sort the hash
  using SortedHash = std::map< rumAssetID, rumSoundAsset* >;
  SortedHash hashAssets( s_hashAssets.begin(), s_hashAssets.end() );

  // Export csv values
  for( const auto iter : hashAssets )
  {
    const rumSoundAsset* pcAsset = iter.second;
    pcAsset->ExportCSVFile( o_rcOutfile );
    pcAsset->ExportCSVPropertyFile( o_rcPropertyOutfile );
  }
}


// final
void rumSoundAsset::ExportDBTable( std::string& io_strQuery ) const
{
  io_strQuery = "INSERT INTO " ASSET_STORAGE_NAME " (type_id,name,baseclass,filename) VALUES (";
  super::ExportDBTable( io_strQuery );
  io_strQuery += ");";
}


// static
void rumSoundAsset::ExportDBTables( ServiceType i_eServiceType )
{
  if( i_eServiceType != Client_ServiceType )
  {
    return;
  }

  // Sort the hash
  using SortedHash = std::map< rumAssetID, rumSoundAsset* >;
  const SortedHash hashAssets( s_hashAssets.begin(), s_hashAssets.end() );

  for( const auto& iter : hashAssets )
  {
    std::string strQuery;
    const rumSoundAsset* pcAsset{ iter.second };
    pcAsset->ExportDBTable( strQuery );
    pcAsset->ExportDBPropertyTable( strQuery, ASSET_STORAGE_NAME, i_eServiceType );

    rumDatabase::Query( rumDatabase::Assets_DatabaseID, strQuery );
  }
}


// static
rumSoundAsset* rumSoundAsset::Fetch( rumAssetID i_eAssetID )
{
  const AssetHash::iterator iter{ s_hashAssets.find( i_eAssetID ) };
  return ( iter != s_hashAssets.end() ) ? iter->second : nullptr;
}


// static
rumSoundAsset* rumSoundAsset::FetchByName( const std::string& i_strName )
{
  // This is a reverse lookup, so walk the entire hash looking for a sound matching the provided name
  for( const auto& iter : s_hashAssets )
  {
    rumSoundAsset* pcAsset{ iter.second };
    const std::string& strName{ pcAsset->GetName() };
    if( strName.compare( i_strName ) == 0 )
    {
      return pcAsset;
    }
  }

  return nullptr;
}


// static
rumSoundAsset* rumSoundAsset::FetchByFilename( const std::string& i_strFilename )
{
  // This is a reverse lookup, so walk the entire hash looking for a sound matching the provided name
  for( const auto& iter : s_hashAssets )
  {
    rumSoundAsset* pcAsset{ iter.second };
    const std::string& strFilename{ pcAsset->GetFilename() };
    if( strFilename.compare( i_strFilename ) == 0 )
    {
      return pcAsset;
    }
  }

  return nullptr;
}


// static
const std::string rumSoundAsset::GetAssetClassName()
{
  return ASSET_NATIVE_CLASS;
}


// static
const std::string rumSoundAsset::GetAssetTypeSuffix()
{
  return ASSET_TYPE_SUFFIX;
}


// static
const std::string rumSoundAsset::GetNativeClassName()
{
  return SCRIPT_SOUND_NATIVE_CLASS;
}


// static
const std::string rumSoundAsset::GetPropertyTableCreateQuery()
{
  return "CREATE TABLE [" ASSET_STORAGE_NAME "_properties]("
    "[" ASSET_STORAGE_NAME "_id_fk] INTEGER NOT NULL,"
    "[property_id_fk] INTEGER NOT NULL,"
    "[value] BLOB_TEXT)";
}


// static
const std::string rumSoundAsset::GetPropertyTableSelectQuery()
{
  return "SELECT " ASSET_STORAGE_NAME "_id_fk,property_id_fk,value FROM " ASSET_STORAGE_NAME "_properties";
}


// static
std::string_view rumSoundAsset::GetStorageName()
{
  return ASSET_STORAGE_NAME;
}


// static
const std::string rumSoundAsset::GetTableCreateQuery()
{
  return "CREATE TABLE[" ASSET_STORAGE_NAME "]("
    "[type_id] INTEGER NOT NULL UNIQUE,"
    "[name] TEXT NOT NULL UNIQUE,"
    "[baseclass] TEXT,"
    "[filename] TEXT NOT NULL)";
}


// static
const std::string rumSoundAsset::GetTableSelectQuery()
{
  return "SELECT type_id,name,baseclass,filename FROM " ASSET_STORAGE_NAME;
}


// override
const std::string_view rumSoundAsset::GetTypeName() const
{
  return ASSET_STORAGE_NAME;
}


// override
const std::string_view rumSoundAsset::GetTypeSuffix() const
{
  return ASSET_TYPE_SUFFIX;
}


// static
int32_t rumSoundAsset::Init( const std::string& i_strPath )
{
  rumAssert( rumScript::GetCurrentVMType() != rumScript::VM_SERVER );
  rumAssert( s_hashAssets.empty() );

  // Build the asset hash from any available resource
  LoadAssets<rumSoundAsset>( i_strPath );

  return RESULT_SUCCESS;
}


bool rumSoundAsset::LoadFileData()
{
  SAFE_ARRAY_DELETE( m_pcData );
  m_eDataType = Invalid_SoundDataType;
  m_uiAllocationSize = 0;

  std::filesystem::path cFilePath{ std::filesystem::path( s_strPathHint ) / m_strFilename };
  if( !std::filesystem::exists( cFilePath ) )
  {
    std::vector<std::string> vResults;
    rumResource::FindFile( m_strFilename, vResults );
    rumAssert( !vResults.empty() );
    if( vResults.empty() )
    {
      std::string strError{ "Error: Failed to load data for sound asset " };
      strError += cFilePath.string();
      Logger::LogStandard( strError, Logger::LOG_ERROR );
      return false;
    }

    bool bFound{ false };

    std::filesystem::path cFirstFound;

    for( size_t i{ 0 }; i < vResults.size() && !bFound; ++i )
    {
      const std::string& strResult{ vResults.at( i ) };
      if( strResult.find( s_strPathHint ) != std::string::npos )
      {
        cFilePath = std::filesystem::path( strResult );
        bFound = true;
      }
      else if( cFirstFound.empty() )
      {
        cFirstFound = std::filesystem::path( strResult );
      }
    }

    if( !bFound && !cFirstFound.empty() )
    {
      cFilePath = cFirstFound;
    }
  }

  rumResourceLoader cResource;
  const int32_t iNumBytes{ cResource.LoadFile( cFilePath.string() ) };
  if( iNumBytes > 0 )
  {
    const rumByte* pcData{ (const rumByte*)cResource.GetMemFile() };
    SetData( pcData, iNumBytes );
    return true;
  }

  return false;
}


// static
bool rumSoundAsset::LoadData( rumAssetID i_eAssetID )
{
  rumSoundAsset* pcAsset{ Fetch( i_eAssetID ) };
  if( !pcAsset )
  {
    std::string strError{ "Error: Failed to load data for rumSoundAsset type " };
    strError += rumStringUtils::ToString( i_eAssetID );
    Logger::LogStandard( strError, Logger::LOG_ERROR );
    return false;
  }

  return pcAsset->LoadFileData();
}


// static
bool rumSoundAsset::LoadArchive( const std::string& i_strArchive )
{
  std::string strArchive( i_strArchive );

  if( !rumResource::FileExists( i_strArchive ) && !rumResource::FindFile( i_strArchive, strArchive ) )
  {
    std::string strError{ "Error: Could not find archive: " };
    strError += i_strArchive;
    Logger::LogStandard( strError, Logger::LOG_ERROR );

    return RESULT_FAILED;
  }

  zlib::rumFileInfo cFileInfo;
  ArchiveReader cArchive( strArchive );
  if( !cArchive.IsValid() )
  {
    std::string strError{ "Error: Failed to open sound archive '" };
    strError += strArchive;
    strError += "'";
    Logger::LogStandard( strError );
  }

  while( cArchive.HasMoreFileInfos() )
  {
    cFileInfo = cArchive.GetNextFileInfo();

    rumByte* pcData{ new rumByte[cFileInfo.m_uiSizeUncompressed] };
    const uint32_t uiNumBytes{ static_cast<uint32_t>( cArchive.ExtractToBuffer( (char*)pcData ) ) };

    // Get the filename from the filepath
    const std::filesystem::path fsPath( cFileInfo.m_strFilename );
    const std::string strFile{ fsPath.filename().string() };

    // Visit all sounds and update each sound that matches the current filename
    for( const auto& iter : s_hashAssets )
    {
      rumSoundAsset* pcAsset{ iter.second };
      if( pcAsset->GetFilename().compare( strFile ) == 0 )
      {
        pcAsset->SetData( pcData, uiNumBytes );
      }
    }

    // Free the extracted buffer
    SAFE_ARRAY_DELETE( pcData );
  }

  return RESULT_SUCCESS;
}


void rumSoundAsset::OnCreated( const std::vector<std::string>& i_rvFields )
{
  s_hashAssets.insert( std::make_pair( GetAssetID(), this ) );

  if( GetAssetID() >= s_eNextFreeID )
  {
    s_eNextFreeID = GetAssetID() + 1;
  }

  if( !i_rvFields.empty() )
  {
    enum { COL_ID, COL_NAME, COL_BASECLASS, COL_FILENAME };

    m_strFilename = i_rvFields.at( COL_FILENAME );
  }
}


// static
void rumSoundAsset::RefreshFileData()
{
  for( const auto& iter : s_hashAssets )
  {
    iter.second->LoadFileData();
  }
}


// static
void rumSoundAsset::ScriptBind()
{
  rumAssert( rumScript::GetCurrentVMType() != rumScript::VM_SERVER );

  rumScript::GetOrCreateClassRegistry( GetClassRegistryID() );

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::RootTable( pcVM )
    .Func( "rumGetSoundAsset", Fetch )
    .Func( "rumLoadSoundArchive", LoadArchive )
    .Func( "rumLoadSound", LoadData );

  Sqrat::DerivedClass<rumSoundAsset, rumFileAsset> cSoundAsset( pcVM, ASSET_NATIVE_CLASS );
  Sqrat::RootTable( pcVM ).Bind( ASSET_NATIVE_CLASS, cSoundAsset );

  // Bind all available assets to script
  for( const auto& iter : s_hashAssets )
  {
    BindAssetID( iter.first, iter.second->GetName(), ASSET_TYPE_SUFFIX );
  }
}


void rumSoundAsset::SetData( const rumByte* i_pcData, uint32_t i_uiNumBytes )
{
  super::SetData( i_pcData, i_uiNumBytes );

  DetermineSoundDataType();

  rumSound::OnAssetDataChanged( *this );
}


// static
void rumSoundAsset::Shutdown()
{
  rumAssert( rumScript::GetCurrentVMType() != rumScript::VM_SERVER );

  auto* pcRegistry{ rumScript::GetOrCreateClassRegistry( GetClassRegistryID() ) };

  for( const auto& iter : s_hashAssets )
  {
    rumSoundAsset* pcAsset{ iter.second };
    SAFE_ARRAY_DELETE( pcAsset->m_pcData );
    pcRegistry->UnregisterScriptClass( pcAsset->GetAssetID() );
    delete pcAsset;
  }

  s_hashAssets.clear();
}
