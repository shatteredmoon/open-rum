#include <u_tile_asset.h>

#define ASSET_NATIVE_CLASS "rumTileAsset"
#define ASSET_STORAGE_NAME "tile"
#define ASSET_TYPE_SUFFIX  "_Tile"


// Static initializations
uint32_t rumTileAsset::m_scDefaultPixelSize{ 32 };
uint32_t rumTileAsset::m_sPixelHeight{ rumTileAsset::m_scDefaultPixelSize };
uint32_t rumTileAsset::m_sPixelWidth{ rumTileAsset::m_scDefaultPixelSize };
std::unordered_map< rumAssetID, rumTileAsset* > rumTileAsset::s_hashAssets;
rumAssetID rumTileAsset::s_eNextFreeID{ FULL_ASSET_ID( Tile_AssetType, 0 ) };


// final
void rumTileAsset::ExportCSVFile( std::ofstream& o_rcOutfile ) const
{
  super::ExportCSVFile( o_rcOutfile );

  o_rcOutfile << ',' << rumStringUtils::ToHexString( GetGraphicID() );
  o_rcOutfile << ',' << GetWeight();
  o_rcOutfile << ',' << ( GetBlocksLOS() ? '1' : '0' );
  o_rcOutfile << ',' << rumStringUtils::ToHexString( GetCollisionFlags() );

  // Done
  o_rcOutfile << '\n';
}


// static
void rumTileAsset::ExportCSVFiles( std::ofstream& o_rcOutfile, std::ofstream& o_rcPropertyOutfile )
{
  // Sort the hash
  using SortedHash = std::map< rumAssetID, rumTileAsset* >;
  const SortedHash hashAssets( s_hashAssets.begin(), s_hashAssets.end() );

  // Export csv values
  for( const auto& iter : hashAssets )
  {
    const rumTileAsset* pcAsset{ iter.second };
    pcAsset->ExportCSVFile( o_rcOutfile );
    pcAsset->ExportCSVPropertyFile( o_rcPropertyOutfile );
  }
}


// final
void rumTileAsset::ExportDBTable( std::string& io_strQuery ) const
{
  io_strQuery = "INSERT INTO " ASSET_STORAGE_NAME " (type_id,name,baseclass,graphic_type_id_fk,weight,blocks_los,collision_flags) VALUES (";

  super::ExportDBTable( io_strQuery );

  io_strQuery += ',';
  io_strQuery += rumStringUtils::ToHexString( GetGraphicID() );

  io_strQuery += ',';
  io_strQuery += rumStringUtils::ToFloatString( GetWeight() );

  io_strQuery += ',';
  io_strQuery += ( GetBlocksLOS() ? '1' : '0' );

  io_strQuery += ',';
  io_strQuery += rumStringUtils::ToHexString( GetCollisionFlags() );

  io_strQuery += ");";
}


// static
void rumTileAsset::ExportDBTables( ServiceType i_eServiceType )
{
  // Sort the hash
  using SortedHash = std::map< rumAssetID, rumTileAsset* >;
  const SortedHash hashAssets( s_hashAssets.begin(), s_hashAssets.end() );

  for( const auto& iter : hashAssets )
  {
    std::string strQuery;
    const rumTileAsset* pcAsset{ iter.second };
    pcAsset->ExportDBTable( strQuery );
    pcAsset->ExportDBPropertyTable( strQuery, ASSET_STORAGE_NAME, i_eServiceType );

    rumDatabase::Query( rumDatabase::Assets_DatabaseID, strQuery );
  }
}


// static
rumTileAsset* rumTileAsset::Fetch( rumAssetID i_eAssetID )
{
  const AssetHash::iterator iter{ s_hashAssets.find( i_eAssetID ) };
  return ( iter != s_hashAssets.end() ) ? iter->second : nullptr;
}


// static
const std::string rumTileAsset::GetAssetClassName()
{
  return ASSET_NATIVE_CLASS;
}


// static
const std::string rumTileAsset::GetAssetTypeSuffix()
{
  return ASSET_TYPE_SUFFIX;
}


// static
const std::string rumTileAsset::GetNativeClassName()
{
  // Tiles are never instanced!
  return "";
}


// static
const std::string rumTileAsset::GetPropertyTableCreateQuery()
{
  return "CREATE TABLE [" ASSET_STORAGE_NAME "_properties]("
    "[" ASSET_STORAGE_NAME "_id_fk] INTEGER NOT NULL,"
    "[property_id_fk] INTEGER NOT NULL,"
    "[value] BLOB_TEXT)";
}


// static
const std::string rumTileAsset::GetPropertyTableSelectQuery()
{
  return "SELECT " ASSET_STORAGE_NAME "_id_fk,property_id_fk,value FROM " ASSET_STORAGE_NAME "_properties";
}


// static
std::string_view rumTileAsset::GetStorageName()
{
  return ASSET_STORAGE_NAME;
}


// static
const std::string rumTileAsset::GetTableCreateQuery()
{
  return "CREATE TABLE[" ASSET_STORAGE_NAME "]("
    "[type_id] INTEGER NOT NULL UNIQUE,"
    "[name] TEXT NOT NULL UNIQUE,"
    "[baseclass] TEXT,"
    "[graphic_type_id_fk] INTEGER NOT NULL,"
    "[weight] FLOAT,"
    "[blocks_los] INTEGER DEFAULT 0,"
    "[collision_flags] INTEGER DEFAULT 0)";
}


// static
const std::string rumTileAsset::GetTableSelectQuery()
{
  return "SELECT type_id,name,baseclass,graphic_type_id_fk,weight,blocks_los,collision_flags FROM " ASSET_STORAGE_NAME;
}


// override
const std::string_view rumTileAsset::GetTypeName() const
{
  return ASSET_STORAGE_NAME;
}


// override
const std::string_view rumTileAsset::GetTypeSuffix() const
{
  return ASSET_TYPE_SUFFIX;
}


// static
int32_t rumTileAsset::Init( const std::string& i_strPath )
{
  rumAssert( s_hashAssets.empty() );

  // Build the asset hash from any available resource
  LoadAssets<rumTileAsset>( i_strPath );

  return RESULT_SUCCESS;
}


void rumTileAsset::OnCreated( const std::vector<std::string>& i_rvFields )
{
  s_hashAssets.insert( std::make_pair( GetAssetID(), this ) );

  if( GetAssetID() >= s_eNextFreeID )
  {
    s_eNextFreeID = GetAssetID() + 1;
  }

  if( !i_rvFields.empty() )
  {
    enum { COL_ID, COL_NAME, COL_BASECLASS, COL_GRAPHIC_ID, COL_WEIGHT, COL_BLOCKS_LOS, COL_COLLISION_FLAGS };

    m_eGraphicID = (rumAssetID)rumStringUtils::ToUInt( i_rvFields.at( COL_GRAPHIC_ID ) );
    m_fWeight = strtof( i_rvFields.at( COL_WEIGHT ).c_str(), nullptr );
    m_bBlocksLOS = rumStringUtils::ToBool( i_rvFields.at( COL_BLOCKS_LOS ) );
    m_uiCollisionFlags = rumStringUtils::ToUInt( i_rvFields.at( COL_COLLISION_FLAGS ) );
  }
}


// static
void rumTileAsset::ScriptBind()
{
  rumScript::GetOrCreateClassRegistry( GetClassRegistryID() );

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::RootTable( pcVM )
    .Func( "rumGetTileAsset", Fetch )
    .Func( "rumGetTilePixelHeight", GetPixelHeight )
    .Func( "rumSetTilePixelHeight", SetPixelHeight )
    .Func( "rumGetTilePixelWidth", GetPixelWidth )
    .Func( "rumSetTilePixelWidth", SetPixelWidth );

  Sqrat::DerivedClass<rumTileAsset, rumAsset> cTileAsset( pcVM, ASSET_NATIVE_CLASS );
  cTileAsset
    .Func( "GetBlocksLOS", &GetBlocksLOS )
    .Func( "GetCollisionFlags", &GetCollisionFlags )
    .Func( "IsCollision", &IsCollision )
    .Func( "GetWeight", &GetWeight );
  Sqrat::RootTable( pcVM ).Bind( ASSET_NATIVE_CLASS, cTileAsset );

  // Bind all available assets to script
  for( const auto& iter : s_hashAssets )
  {
    BindAssetID( iter.first, iter.second->GetName(), ASSET_TYPE_SUFFIX );
  }
}


// static
void rumTileAsset::Shutdown()
{
  auto* pcRegistry{ rumScript::GetOrCreateClassRegistry( GetClassRegistryID() ) };

  for( const auto& iter : s_hashAssets )
  {
    rumTileAsset* pcAsset{ iter.second };
    pcRegistry->UnregisterScriptClass( pcAsset->GetAssetID() );
    delete pcAsset;
  }

  s_hashAssets.clear();
}
