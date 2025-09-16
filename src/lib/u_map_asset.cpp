#include <u_map_asset.h>

#include <u_map.h>

#define ASSET_NATIVE_CLASS "rumMapAsset"
#define ASSET_STORAGE_NAME "map"
#define ASSET_TYPE_SUFFIX  "_Map"


// Static initializations
std::unordered_map< rumAssetID, rumMapAsset* > rumMapAsset::s_hashAssets;
rumAssetID rumMapAsset::s_eNextFreeID{ FULL_ASSET_ID( Map_AssetType, 0 ) };


// final
void rumMapAsset::ExportCSVFile( std::ofstream& o_rcOutfile ) const
{
  super::ExportCSVFile( o_rcOutfile );

  // Done
  o_rcOutfile << '\n';
}


// static
void rumMapAsset::ExportCSVFiles( std::ofstream& o_rcOutfile, std::ofstream& o_rcPropertyOutfile )
{
  // Sort the hash
  using SortedHash = std::map< rumAssetID, rumMapAsset* >;
  const SortedHash hashAssets( s_hashAssets.begin(), s_hashAssets.end() );

  // Export csv values
  for( const auto& iter : hashAssets )
  {
    const rumMapAsset* pcAsset{ iter.second };
    pcAsset->ExportCSVFile( o_rcOutfile );
    pcAsset->ExportCSVPropertyFile( o_rcPropertyOutfile );
  }
}


// final
void rumMapAsset::ExportDBTable( std::string& io_strQuery ) const
{
  io_strQuery = "INSERT INTO " ASSET_STORAGE_NAME " (type_id,name,baseclass,filename) VALUES (";

  super::ExportDBTable( io_strQuery );

  io_strQuery += ");";
}


// static
void rumMapAsset::ExportDBTables( ServiceType i_eServiceType )
{
  // Sort the hash
  using SortedHash = std::map< rumAssetID, rumMapAsset* >;
  const SortedHash hashAssets( s_hashAssets.begin(), s_hashAssets.end() );

  for( const auto& iter : hashAssets )
  {
    std::string strQuery;
    const rumMapAsset* pcAsset{ iter.second };
    pcAsset->ExportDBTable( strQuery );
    pcAsset->ExportDBPropertyTable( strQuery, ASSET_STORAGE_NAME, i_eServiceType );

    rumDatabase::Query( rumDatabase::Assets_DatabaseID, strQuery );
  }
}


// static
rumMapAsset* rumMapAsset::Fetch( rumAssetID i_eAssetID )
{
  const AssetHash::iterator iter{ s_hashAssets.find( i_eAssetID ) };
  return ( iter != s_hashAssets.end() ) ? iter->second : nullptr;
}


// static
const std::string rumMapAsset::GetAssetClassName()
{
  return ASSET_NATIVE_CLASS;
}


// static
const std::string rumMapAsset::GetAssetTypeSuffix()
{
  return ASSET_TYPE_SUFFIX;
}


// static
const std::string rumMapAsset::GetNativeClassName()
{
  return SCRIPT_MAP_NATIVE_CLASS;
}


// static
const std::string rumMapAsset::GetPropertyTableCreateQuery()
{
  return "CREATE TABLE [" ASSET_STORAGE_NAME "_properties]("
    "[" ASSET_STORAGE_NAME "_id_fk] INTEGER NOT NULL,"
    "[property_id_fk] INTEGER NOT NULL,"
    "[value] BLOB_TEXT)";
}


// static
const std::string rumMapAsset::GetPropertyTableSelectQuery()
{
  return "SELECT " ASSET_STORAGE_NAME "_id_fk,property_id_fk,value FROM " ASSET_STORAGE_NAME "_properties";
}


// static
std::string_view rumMapAsset::GetStorageName()
{
  return ASSET_STORAGE_NAME;
}


// static
const std::string rumMapAsset::GetTableCreateQuery()
{
  return "CREATE TABLE[" ASSET_STORAGE_NAME "]("
    "[type_id] INTEGER NOT NULL UNIQUE,"
    "[name] TEXT NOT NULL UNIQUE,"
    "[baseclass] TEXT,"
    "[filename] TEXT NOT NULL)";
}


// static
const std::string rumMapAsset::GetTableSelectQuery()
{
  return "SELECT type_id,name,baseclass,filename FROM " ASSET_STORAGE_NAME;
}


// override
const std::string_view rumMapAsset::GetTypeName() const
{
  return ASSET_STORAGE_NAME;
}


// override
const std::string_view rumMapAsset::GetTypeSuffix() const
{
  return ASSET_TYPE_SUFFIX;
}


// static
int32_t rumMapAsset::Init( const std::string& i_strPath )
{
  rumAssert( s_hashAssets.empty() );

  // Build the asset hash from any available resource
  LoadAssets<rumMapAsset>( i_strPath );

  return RESULT_SUCCESS;
}


void rumMapAsset::OnCreated( const std::vector<std::string>& i_rvFields )
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
void rumMapAsset::ScriptBind()
{
  rumScript::GetOrCreateClassRegistry( GetClassRegistryID() );

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::RootTable( pcVM ).Func( "rumGetMapAsset", Fetch );

  Sqrat::DerivedClass<rumMapAsset, rumAsset> cMapAsset( pcVM, ASSET_NATIVE_CLASS );
  Sqrat::RootTable( pcVM ).Bind( ASSET_NATIVE_CLASS, cMapAsset );

  // Bind all available assets to script
  for( const auto& iter : s_hashAssets )
  {
    BindAssetID( iter.first, iter.second->GetName(), ASSET_TYPE_SUFFIX );
  }
}


// static
void rumMapAsset::Shutdown()
{
  auto* pcRegistry{ rumScript::GetOrCreateClassRegistry( GetClassRegistryID() ) };

  for( const auto& iter : s_hashAssets )
  {
    rumMapAsset* pcAsset{ iter.second };
    pcRegistry->UnregisterScriptClass( pcAsset->GetAssetID() );
    delete pcAsset;
  }

  s_hashAssets.clear();
}
