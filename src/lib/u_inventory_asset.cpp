#include <u_inventory_asset.h>

#include <u_inventory.h>

#define ASSET_NATIVE_CLASS "rumInventoryAsset"
#define ASSET_STORAGE_NAME "inventory"
#define ASSET_TYPE_SUFFIX  "_Inventory"


// Static initializations
std::unordered_map< rumAssetID, rumInventoryAsset* > rumInventoryAsset::s_hashAssets;
rumAssetID rumInventoryAsset::s_eNextFreeID{ FULL_ASSET_ID( Inventory_AssetType, 0 ) };


// final
void rumInventoryAsset::ExportCSVFile( std::ofstream& o_rcOutfile ) const
{
  super::ExportCSVFile( o_rcOutfile );

  o_rcOutfile << ',' << rumStringUtils::ToString( GetClientReplicationType() );
  o_rcOutfile << ',' << ( IsPersistent() ? '1' : '0' );

  // Done
  o_rcOutfile << '\n';
}


// static
void rumInventoryAsset::ExportCSVFiles( std::ofstream& o_rcOutfile, std::ofstream& o_rcPropertyOutfile )
{
  // Sort the hash
  using SortedHash = std::map< rumAssetID, rumInventoryAsset* >;
  const SortedHash hashAssets( s_hashAssets.begin(), s_hashAssets.end() );

  // Export csv values
  for( const auto& iter : hashAssets )
  {
    const rumInventoryAsset* pcAsset{ iter.second };
    pcAsset->ExportCSVFile( o_rcOutfile );
    pcAsset->ExportCSVPropertyFile( o_rcPropertyOutfile );
  }
}


// final
void rumInventoryAsset::ExportDBTable( std::string& io_strQuery ) const
{
  io_strQuery = "INSERT INTO " ASSET_STORAGE_NAME " (type_id,name,baseclass,replication_type,persistent) VALUES (";

  super::ExportDBTable( io_strQuery );

  io_strQuery += ',';
  io_strQuery += rumStringUtils::ToString( GetClientReplicationType() );

  io_strQuery += ',';
  io_strQuery += ( IsPersistent() ? '1' : '0' );

  io_strQuery += ");";
}


// static
void rumInventoryAsset::ExportDBTables( ServiceType i_eServiceType )
{
  // Sort the hash
  using SortedHash = std::map< rumAssetID, rumInventoryAsset* >;
  const SortedHash hashAssets( s_hashAssets.begin(), s_hashAssets.end() );

  for( const auto& iter : hashAssets )
  {
    std::string strQuery;
    const rumInventoryAsset* pcAsset{ iter.second };
    pcAsset->ExportDBTable( strQuery );
    pcAsset->ExportDBPropertyTable( strQuery, ASSET_STORAGE_NAME, i_eServiceType );

    rumDatabase::Query( rumDatabase::Assets_DatabaseID, strQuery );
  }
}


// static
rumInventoryAsset* rumInventoryAsset::Fetch( rumAssetID i_eAssetID )
{
  const AssetHash::iterator iter{ s_hashAssets.find( i_eAssetID ) };
  return ( iter != s_hashAssets.end() ) ? iter->second : nullptr;
}


// static
const std::string rumInventoryAsset::GetAssetClassName()
{
  return ASSET_NATIVE_CLASS;
}


// static
const std::string rumInventoryAsset::GetAssetTypeSuffix()
{
  return ASSET_TYPE_SUFFIX;
}


// static
const std::string rumInventoryAsset::GetNativeClassName()
{
  return SCRIPT_INVENTORY_NATIVE_CLASS;
}


// static
const std::string rumInventoryAsset::GetPropertyTableCreateQuery()
{
  return "CREATE TABLE [" ASSET_STORAGE_NAME "_properties]("
    "[" ASSET_STORAGE_NAME "_id_fk] INTEGER NOT NULL,"
    "[property_id_fk] INTEGER NOT NULL,"
    "[value] BLOB_TEXT)";
}


// static
const std::string rumInventoryAsset::GetPropertyTableSelectQuery()
{
  return "SELECT " ASSET_STORAGE_NAME "_id_fk,property_id_fk,value FROM " ASSET_STORAGE_NAME "_properties";
}


// static
std::string_view rumInventoryAsset::GetStorageName()
{
  return ASSET_STORAGE_NAME;
}


// static
const std::string rumInventoryAsset::GetTableCreateQuery()
{
  return "CREATE TABLE[" ASSET_STORAGE_NAME "]("
    "[type_id] INTEGER NOT NULL UNIQUE,"
    "[name] TEXT NOT NULL UNIQUE,"
    "[baseclass] TEXT,"
    "[replication_type] INTEGER DEFAULT 1,"
    "[persistent] INTEGER DEFAULT 1)";
}


// static
const std::string rumInventoryAsset::GetTableSelectQuery()
{
  return "SELECT type_id,name,baseclass,replication_type,persistent FROM " ASSET_STORAGE_NAME;
}


// override
const std::string_view rumInventoryAsset::GetTypeName() const
{
  return ASSET_STORAGE_NAME;
}


// override
const std::string_view rumInventoryAsset::GetTypeSuffix() const
{
  return ASSET_TYPE_SUFFIX;
}


// static
int32_t rumInventoryAsset::Init( const std::string& i_strPath )
{
  rumAssert( s_hashAssets.empty() );

  // Build the asset hash from any available resource
  LoadAssets<rumInventoryAsset>( i_strPath );

  return RESULT_SUCCESS;
}


void rumInventoryAsset::OnCreated( const std::vector<std::string>& i_rvFields )
{
  s_hashAssets.insert( std::make_pair( GetAssetID(), this ) );

  if( GetAssetID() >= s_eNextFreeID )
  {
    s_eNextFreeID = GetAssetID() + 1;
  }

  if( !i_rvFields.empty() )
  {
    enum { COL_ID, COL_NAME, COL_BASECLASS, COL_REPLICATION_TYPE, COL_PERSISTENT };

    m_eClientReplicationType = (ClientReplicationType)rumStringUtils::ToInt( i_rvFields.at( COL_REPLICATION_TYPE ) );
    m_bPersistent = rumStringUtils::ToBool( i_rvFields.at( COL_PERSISTENT ) );
  }
}


// static
void rumInventoryAsset::ScriptBind()
{
  rumScript::GetOrCreateClassRegistry( GetClassRegistryID() );

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::RootTable( pcVM ).Func( "rumGetInventoryAsset", Fetch );

  Sqrat::DerivedClass<rumInventoryAsset, rumAsset> cInventoryAsset( pcVM, ASSET_NATIVE_CLASS );
  Sqrat::RootTable( pcVM ).Bind( ASSET_NATIVE_CLASS, cInventoryAsset );

  // Bind all available assets to script
  for( const auto& iter : s_hashAssets )
  {
    BindAssetID( iter.first, iter.second->GetName(), ASSET_TYPE_SUFFIX );
  }
}


// static
void rumInventoryAsset::Shutdown()
{
  auto* pcRegistry{ rumScript::GetOrCreateClassRegistry( GetClassRegistryID() ) };

  for( const auto& iter : s_hashAssets )
  {
    rumInventoryAsset* pcAsset{ iter.second };
    pcRegistry->UnregisterScriptClass( pcAsset->GetAssetID() );
    delete pcAsset;
  }

  s_hashAssets.clear();
}
