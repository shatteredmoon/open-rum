#include <u_broadcast_asset.h>

#include <u_broadcast.h>

#define ASSET_NATIVE_CLASS "rumBroadcastAsset"
#define ASSET_STORAGE_NAME "broadcast"
#define ASSET_TYPE_SUFFIX  "_Broadcast"


// Static initializations
std::unordered_map< rumAssetID, rumBroadcastAsset* > rumBroadcastAsset::s_hashAssets;
rumAssetID rumBroadcastAsset::s_eNextFreeID{ FULL_ASSET_ID( Broadcast_AssetType, 0 ) };


// final
void rumBroadcastAsset::ExportCSVFile( std::ofstream& o_rcOutfile ) const
{
  super::ExportCSVFile( o_rcOutfile );

  // Done
  o_rcOutfile << '\n';
}


// static
void rumBroadcastAsset::ExportCSVFiles( std::ofstream& o_rcOutfile, std::ofstream& o_rcPropertyOutfile )
{
  // Sort the hash
  using SortedHash = std::map< rumAssetID, rumBroadcastAsset* >;
  const SortedHash hashAssets( s_hashAssets.begin(), s_hashAssets.end() );

  // Export csv values
  for( const auto& iter : hashAssets )
  {
    const rumBroadcastAsset* pcAsset{ iter.second };
    pcAsset->ExportCSVFile( o_rcOutfile );
    pcAsset->ExportCSVPropertyFile( o_rcPropertyOutfile );
  }
}


// final
void rumBroadcastAsset::ExportDBTable( std::string& io_strQuery ) const
{
  io_strQuery = "INSERT INTO " ASSET_STORAGE_NAME " (type_id,name,baseclass) VALUES (";
  super::ExportDBTable( io_strQuery );
  io_strQuery += ");";
}


// static
void rumBroadcastAsset::ExportDBTables( ServiceType i_eServiceType )
{
  // Sort the hash
  using SortedHash = std::map< rumAssetID, rumBroadcastAsset* >;
  const SortedHash hashAssets( s_hashAssets.begin(), s_hashAssets.end() );

  for( const auto& iter : hashAssets )
  {
    std::string strQuery;
    const rumBroadcastAsset* pcAsset{ iter.second };
    pcAsset->ExportDBTable( strQuery );
    pcAsset->ExportDBPropertyTable( strQuery, ASSET_STORAGE_NAME, i_eServiceType );

    rumDatabase::Query( rumDatabase::Assets_DatabaseID, strQuery );
  }
}


// static
rumBroadcastAsset* rumBroadcastAsset::Fetch( rumAssetID i_eAssetID )
{
  const AssetHash::iterator iter{ s_hashAssets.find( i_eAssetID ) };
  return ( iter != s_hashAssets.end() ) ? iter->second : nullptr;
}


// static
const std::string rumBroadcastAsset::GetAssetClassName()
{
  return ASSET_NATIVE_CLASS;
}


// static
const std::string rumBroadcastAsset::GetAssetTypeSuffix()
{
  return ASSET_TYPE_SUFFIX;
}


// static
const std::string rumBroadcastAsset::GetNativeClassName()
{
  return SCRIPT_BROADCAST_NATIVE_CLASS;
}


// static
const std::string rumBroadcastAsset::GetPropertyTableCreateQuery()
{
  return "CREATE TABLE [" ASSET_STORAGE_NAME "_properties]("
    "[" ASSET_STORAGE_NAME "_id_fk] INTEGER NOT NULL,"
    "[property_id_fk] INTEGER NOT NULL,"
    "[value] BLOB_TEXT)";
}


// static
const std::string rumBroadcastAsset::GetPropertyTableSelectQuery()
{
  return "SELECT " ASSET_STORAGE_NAME "_id_fk,property_id_fk,value FROM " ASSET_STORAGE_NAME "_properties";
}


// static
std::string_view rumBroadcastAsset::GetStorageName()
{
  return ASSET_STORAGE_NAME;
}


// static
const std::string rumBroadcastAsset::GetTableCreateQuery()
{
  return "CREATE TABLE[" ASSET_STORAGE_NAME "]("
    "[type_id] INTEGER NOT NULL UNIQUE,"
    "[name] TEXT NOT NULL,"
    "[baseclass] TEXT)";
}


// static
const std::string rumBroadcastAsset::GetTableSelectQuery()
{
  return "SELECT type_id,name,baseclass FROM " ASSET_STORAGE_NAME;
}


// override
const std::string_view rumBroadcastAsset::GetTypeName() const
{
  return ASSET_STORAGE_NAME;
}


// override
const std::string_view rumBroadcastAsset::GetTypeSuffix() const
{
  return ASSET_TYPE_SUFFIX;
}


// static
int32_t rumBroadcastAsset::Init( const std::string& i_strPath )
{
  rumAssert( s_hashAssets.empty() );

  // Build the asset hash from any available resource
  LoadAssets<rumBroadcastAsset>( i_strPath );

  return RESULT_SUCCESS;
}


void rumBroadcastAsset::OnCreated( const std::vector<std::string>& i_rvFields )
{
  s_hashAssets.insert( std::make_pair( GetAssetID(), this ) );

  if( GetAssetID() >= s_eNextFreeID )
  {
    s_eNextFreeID = GetAssetID() + 1;
  }
}


// static
void rumBroadcastAsset::ScriptBind()
{
  rumScript::GetOrCreateClassRegistry( GetClassRegistryID() );

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::RootTable( pcVM ).Func( "rumGetBroadcastAsset", Fetch );

  Sqrat::DerivedClass<rumBroadcastAsset, rumAsset> cBroadcastAsset( pcVM, ASSET_NATIVE_CLASS );
  Sqrat::RootTable( pcVM ).Bind( ASSET_NATIVE_CLASS, cBroadcastAsset );

  // Bind all available assets to script
  for( const auto& iter : s_hashAssets )
  {
    BindAssetID( iter.first, iter.second->GetName(), ASSET_TYPE_SUFFIX );
  }
}


// static
void rumBroadcastAsset::Shutdown()
{
  auto* pcRegistry{ rumScript::GetOrCreateClassRegistry( GetClassRegistryID() ) };

  for( const auto& iter : s_hashAssets )
  {
    rumBroadcastAsset* pcAsset{ iter.second };
    pcRegistry->UnregisterScriptClass( pcAsset->GetAssetID() );
    delete pcAsset;
  }

  s_hashAssets.clear();
}
