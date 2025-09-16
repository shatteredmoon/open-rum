#include <u_graphic_asset.h>

#include <u_graphic.h>
#include <u_resource.h>
#include <u_zlib.h>

#include <filesystem>

#define ASSET_NATIVE_CLASS "rumGraphicAsset"
#define ASSET_STORAGE_NAME "graphic"
#define ASSET_TYPE_SUFFIX  "_Graphic"

//#define SCRIPT_GRAPHIC_CLASS            "rumGraphic"
//#define SCRIPT_GRAPHIC_FLIPPED          "rumFlipped"
//#define SCRIPT_GRAPHIC_MIRRORED         "rumMirrored"

// Static initializations
std::unordered_map< rumAssetID, rumGraphicAsset* > rumGraphicAsset::s_hashAssets;
rumAssetID rumGraphicAsset::s_eNextFreeID{ FULL_ASSET_ID( Graphic_AssetType, 0 ) };
std::string rumGraphicAsset::s_strPathHint;


// final
void rumGraphicAsset::ExportCSVFile( std::ofstream& o_rcOutfile ) const
{
  super::ExportCSVFile( o_rcOutfile );

  o_rcOutfile << ',' << GetNumAnimFrames();
  o_rcOutfile << ',' << GetNumAnimStates();
  o_rcOutfile << ',' << GetAnimType();
  o_rcOutfile << ',' << GetAnimInterval();
  o_rcOutfile << ',' << ( IsClientRendered() ? '1' : '0' );

  // Done
  o_rcOutfile << '\n';
}


// static
void rumGraphicAsset::ExportCSVFiles( std::ofstream& o_rcOutfile, std::ofstream& o_rcPropertyOutfile )
{
  // Sort the hash
  using SortedHash = std::map< rumAssetID, rumGraphicAsset* >;
  const SortedHash hashAssets( s_hashAssets.begin(), s_hashAssets.end() );

  // Export csv values
  for( const auto& iter : hashAssets )
  {
    rumGraphicAsset* pcAsset{ iter.second };
    pcAsset->ExportCSVFile( o_rcOutfile );
    pcAsset->ExportCSVPropertyFile( o_rcPropertyOutfile );
  }
}


// final
void rumGraphicAsset::ExportDBTable( std::string& io_strQuery ) const
{
  io_strQuery = "INSERT INTO " ASSET_STORAGE_NAME
                " (type_id,name,baseclass,filename,num_frames,num_sets,anim_type,anim_interval,client_rendered)"
                " VALUES (";

  super::ExportDBTable( io_strQuery );

  io_strQuery += ',';
  io_strQuery += rumStringUtils::ToString( GetNumAnimFrames() );

  io_strQuery += ',';
  io_strQuery += rumStringUtils::ToString( GetNumAnimStates() );

  io_strQuery += ',';
  io_strQuery += rumStringUtils::ToString( GetAnimType() );

  io_strQuery += ',';
  io_strQuery += rumStringUtils::ToFloatString( GetAnimInterval() );

  io_strQuery += ',';
  io_strQuery += ( IsClientRendered() ? '1' : '0' );

  io_strQuery += ");";
}


// static
void rumGraphicAsset::ExportDBTables( ServiceType i_eServiceType )
{
  if( i_eServiceType != Client_ServiceType )
  {
    return;
  }

  // Sort the hash
  using SortedHash = std::map< rumAssetID, rumGraphicAsset* >;
  const SortedHash hashAssets( s_hashAssets.begin(), s_hashAssets.end() );

  for( const auto& iter : hashAssets )
  {
    std::string strQuery;
    const rumGraphicAsset* pcAsset{ iter.second };
    pcAsset->ExportDBTable( strQuery );
    pcAsset->ExportDBPropertyTable( strQuery, ASSET_STORAGE_NAME, i_eServiceType );

    rumDatabase::Query( rumDatabase::Assets_DatabaseID, strQuery );
  }
}


// static
rumGraphicAsset* rumGraphicAsset::Fetch( rumAssetID i_eAssetID )
{
  const AssetHash::iterator iter{ s_hashAssets.find( i_eAssetID ) };
  return ( iter != s_hashAssets.end() ) ? iter->second : nullptr;
}


// static
rumGraphicAsset* rumGraphicAsset::FetchByFilename( const std::string& i_strName )
{
  // This is a reverse lookup, so walk the entire hash looking for a graphic matching the provided name
  for( const auto& iter : s_hashAssets )
  {
    rumGraphicAsset* pcAsset{ iter.second };
    const std::string& strName{ pcAsset->GetName() };
    if( strName.compare( i_strName ) == 0 )
    {
      return pcAsset;
    }
    else
    {
      const std::string& strFilename{ pcAsset->GetFilename() };
      if( strFilename.compare( i_strName ) == 0 )
      {
        return pcAsset;
      }
    }
  }

  return nullptr;
}


// static
const std::string rumGraphicAsset::GetAssetClassName()
{
  return ASSET_NATIVE_CLASS;
}


// static
const std::string rumGraphicAsset::GetAssetTypeSuffix()
{
  return ASSET_TYPE_SUFFIX;
}


// static
const std::string rumGraphicAsset::GetNativeClassName()
{
  return SCRIPT_GRAPHIC_NATIVE_CLASS;
}


// static
const std::string rumGraphicAsset::GetPropertyTableCreateQuery()
{
  return "CREATE TABLE [" ASSET_STORAGE_NAME "_properties]("
    "[" ASSET_STORAGE_NAME "_id_fk] INTEGER NOT NULL,"
    "[property_id_fk] INTEGER NOT NULL,"
    "[value] BLOB_TEXT)";
}


// static
const std::string rumGraphicAsset::GetPropertyTableSelectQuery()
{
  return "SELECT " ASSET_STORAGE_NAME "_id_fk,property_id_fk,value FROM " ASSET_STORAGE_NAME "_properties";
}


// static
std::string_view rumGraphicAsset::GetStorageName()
{
  return ASSET_STORAGE_NAME;
}


// static
const std::string rumGraphicAsset::GetTableCreateQuery()
{
  return "CREATE TABLE[" ASSET_STORAGE_NAME "]("
    "[type_id] INTEGER NOT NULL UNIQUE,"
    "[name] TEXT NOT NULL UNIQUE,"
    "[baseclass] TEXT,"
    "[filename] TEXT NOT NULL,"
    "[num_frames] INTEGER NOT NULL DEFAULT 1,"
    "[num_sets] INTEGER NOT NULL DEFAULT 1,"
    "[anim_type] INTEGER NOT NULL DEFAULT 0,"
    "[anim_interval] REAL NOT NULL DEFAULT 0.1,"
    "[client_rendered] INTEGER NOT NULL DEFAULT 1)";
}


// static
const std::string rumGraphicAsset::GetTableSelectQuery()
{
  return "SELECT type_id,name,baseclass,filename,num_frames,num_sets,anim_type,anim_interval,client_rendered FROM "
         ASSET_STORAGE_NAME;
}


// override
const std::string_view rumGraphicAsset::GetTypeName() const
{
  return ASSET_STORAGE_NAME;
}


// override
const std::string_view rumGraphicAsset::GetTypeSuffix() const
{
  return ASSET_TYPE_SUFFIX;
}


// static
int32_t rumGraphicAsset::Init( const std::string& i_strPath )
{
  rumAssert( rumScript::GetCurrentVMType() != rumScript::VM_SERVER );
  rumAssert( s_hashAssets.empty() );

  // Build the asset hash from any available resource
  LoadAssets<rumGraphicAsset>( i_strPath );

  return RESULT_SUCCESS;
}


bool rumGraphicAsset::LoadFileData()
{
  SAFE_ARRAY_DELETE( m_pcData );
  m_uiAllocationSize = 0;

  std::filesystem::path fsFilePath{ std::filesystem::path( s_strPathHint ) / m_strFilename };
  if( !std::filesystem::exists( fsFilePath ) )
  {
    std::vector<std::string> vResults;
    rumResource::FindFile( m_strFilename, vResults );
    rumAssert( !vResults.empty() );
    if( vResults.empty() )
    {
      std::string strError{ "Error: Failed to load data for graphic asset " };
      strError += fsFilePath.string();
      Logger::LogStandard( strError, Logger::LOG_ERROR );
      return false;
    }

    bool bFound{ false };

    std::filesystem::path fsFirstFound;

    for( size_t i{ 0 }; i < vResults.size() && !bFound; ++i )
    {
      const std::string& strResult{ vResults.at( i ) };
      if( strResult.find( s_strPathHint ) != std::string::npos )
      {
        fsFilePath = std::filesystem::path( strResult );
        bFound = true;
      }
      else if( fsFirstFound.empty() )
      {
        fsFirstFound = std::filesystem::path( strResult );
      }
    }

    if( !bFound && !fsFirstFound.empty() )
    {
      fsFilePath = fsFirstFound;
    }
  }

  rumResourceLoader cResource;
  const int32_t iNumBytes{ cResource.LoadFile( fsFilePath.string() ) };
  if( iNumBytes > 0 )
  {
    const rumByte* pcData{ (const rumByte*)cResource.GetMemFile() };
    SetData( pcData, iNumBytes );
    return true;
  }

  return false;
}


// static
bool rumGraphicAsset::LoadData( rumAssetID i_eAssetID )
{
  rumGraphicAsset* pcAsset{ Fetch( i_eAssetID ) };
  if( !pcAsset )
  {
    std::string strError{ "Error: Failed to load data for rumGraphicAsset type " };
    strError += rumStringUtils::ToString( i_eAssetID );
    Logger::LogStandard( strError, Logger::LOG_ERROR );
    return false;
  }

  return pcAsset->LoadFileData();
}


// static
bool rumGraphicAsset::LoadArchive( const std::string& i_strArchive )
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
    std::string strError{ "Error: Failed to open graphic archive '" };
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

    // Visit all images and update each image that matches the current filename
    for( const auto& iter : s_hashAssets )
    {
      rumGraphicAsset* pcAsset{ iter.second };
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


void rumGraphicAsset::OnCreated( const std::vector<std::string>& i_rvFields )
{
  s_hashAssets.insert( std::make_pair( GetAssetID(), this ) );

  if( GetAssetID() >= s_eNextFreeID )
  {
    s_eNextFreeID = GetAssetID() + 1;
  }

  if( i_rvFields.empty() )
  {
    return;
  }

  enum { COL_ID, COL_NAME, COL_BASECLASS, COL_FILENAME, COL_NUMFRAMES, COL_NUMSETS, COL_ANIMTYPE, COL_ANIMINTERVAL,
         COL_CLIENTRENDERED };

  m_strFilename = i_rvFields.at( COL_FILENAME );
  m_uiNumFrames = rumStringUtils::ToUInt( i_rvFields.at( COL_NUMFRAMES ) );
  m_uiNumStates = rumStringUtils::ToUInt( i_rvFields.at( COL_NUMSETS ) );
  m_eAnimType = static_cast<rumAnimationType>( rumStringUtils::ToUInt( i_rvFields.at( COL_ANIMTYPE ) ) );
  m_fAnimInterval = rumStringUtils::ToFloat( i_rvFields.at( COL_ANIMINTERVAL ) );
  m_bClientRendered = rumStringUtils::ToBool( i_rvFields.at( COL_CLIENTRENDERED ) );
}


// static
void rumGraphicAsset::RefreshFileData()
{
  for( const auto& iter : s_hashAssets )
  {
    iter.second->LoadFileData();
  }
}


// static
void rumGraphicAsset::ScriptBind()
{
  rumAssert( rumScript::GetCurrentVMType() != rumScript::VM_SERVER );

  rumScript::GetOrCreateClassRegistry( GetClassRegistryID() );

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::RootTable( pcVM )
    .Func( "rumGetGraphicAsset", Fetch )
    .Func( "rumLoadGraphicArchive", LoadArchive )
    .Func( "rumLoadGraphic", LoadData );

  Sqrat::DerivedClass<rumGraphicAsset, rumFileAsset> cGraphicAsset( pcVM, ASSET_NATIVE_CLASS );
  cGraphicAsset
    .Func( "GetAnimType", &GetAnimType )
    .Func( "GetAnimInterval", &GetAnimInterval )
    .Func( "GetNumAnimFrames", &GetNumAnimFrames )
    .Func( "GetNumAnimStates", &GetNumAnimStates );
  Sqrat::RootTable( pcVM ).Bind( ASSET_NATIVE_CLASS, cGraphicAsset );

  // Bind all available assets to script
  for( const auto& iter : s_hashAssets )
  {
    BindAssetID( iter.first, iter.second->GetName(), ASSET_TYPE_SUFFIX );
  }
}


void rumGraphicAsset::SetData( const rumByte* i_pcData, uint32_t i_uiNumBytes )
{
  super::SetData( i_pcData, i_uiNumBytes );
  rumGraphic::OnAssetDataChanged( *this );
}


// static
void rumGraphicAsset::Shutdown()
{
  rumAssert( rumScript::GetCurrentVMType() != rumScript::VM_SERVER );

  auto* pcRegistry{ rumScript::GetOrCreateClassRegistry( GetClassRegistryID() ) };

  for( const auto& iter : s_hashAssets )
  {
    rumGraphicAsset* pcAsset{ iter.second };
    SAFE_ARRAY_DELETE( pcAsset->m_pcData );
    pcRegistry->UnregisterScriptClass( pcAsset->GetAssetID() );
    delete pcAsset;
  }

  s_hashAssets.clear();
}
