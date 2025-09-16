#include <u_custom_asset.h>

#include <u_custom.h>
#include <u_rum.h>

#define ASSET_NATIVE_CLASS "rumCustomAsset"
#define ASSET_STORAGE_NAME "custom"
#define ASSET_TYPE_SUFFIX  "_Custom"


// Static initializations
std::unordered_map< rumAssetID, rumCustomAsset* > rumCustomAsset::s_hashAssets;
rumAssetID rumCustomAsset::s_eNextFreeID{ FULL_ASSET_ID( Custom_AssetType, 0 ) };


// final
void rumCustomAsset::ExportCSVFile( std::ofstream& o_rcOutfile ) const
{
  super::ExportCSVFile( o_rcOutfile );

  // Done
  o_rcOutfile << '\n';
}


// static
void rumCustomAsset::ExportCSVFiles( std::ofstream& o_rcOutfile, std::ofstream& o_rcPropertyOutfile )
{
  // Sort the hash
  using SortedHash = std::map< rumAssetID, rumCustomAsset* >;
  const SortedHash hashAssets( s_hashAssets.begin(), s_hashAssets.end() );

  // Export csv values
  for( const auto& iter : hashAssets )
  {
    const rumCustomAsset* pcAsset{ iter.second };
    pcAsset->ExportCSVFile( o_rcOutfile );
    pcAsset->ExportCSVPropertyFile( o_rcPropertyOutfile );
  }
}


// final
void rumCustomAsset::ExportDBTable( std::string& io_strQuery ) const
{
  io_strQuery = "INSERT INTO " ASSET_STORAGE_NAME " (type_id,name,baseclass) VALUES (";
  super::ExportDBTable( io_strQuery );
  io_strQuery += ");";
}


// static
void rumCustomAsset::ExportDBTables( ServiceType i_eServiceType )
{
  // Sort the hash
  using SortedHash = std::map< rumAssetID, rumCustomAsset* >;
  const SortedHash hashAssets( s_hashAssets.begin(), s_hashAssets.end() );

  for( const auto& iter : hashAssets )
  {
    std::string strQuery;
    const rumCustomAsset* pcAsset{ iter.second };
    pcAsset->ExportDBTable( strQuery );
    pcAsset->ExportDBPropertyTable( strQuery, ASSET_STORAGE_NAME, i_eServiceType );

    rumDatabase::Query( rumDatabase::Assets_DatabaseID, strQuery );
  }
}


// static
rumCustomAsset* rumCustomAsset::Fetch( rumAssetID i_eAssetID )
{
  const AssetHash::iterator iter{ s_hashAssets.find( i_eAssetID ) };
  return ( iter != s_hashAssets.end() ) ? iter->second : nullptr;
}


// static
const std::string rumCustomAsset::GetAssetClassName()
{
  return ASSET_NATIVE_CLASS;
}


// static
const std::string rumCustomAsset::GetAssetTypeSuffix()
{
  return ASSET_TYPE_SUFFIX;
}


// static
const std::string rumCustomAsset::GetNativeClassName()
{
  return SCRIPT_CUSTOM_NATIVE_CLASS;
}


// static
const std::string rumCustomAsset::GetPropertyTableCreateQuery()
{
  return "CREATE TABLE [" ASSET_STORAGE_NAME "_properties]("
    "[" ASSET_STORAGE_NAME "_id_fk] INTEGER NOT NULL,"
    "[property_id_fk] INTEGER NOT NULL,"
    "[value] BLOB_TEXT)";
}


// static
const std::string rumCustomAsset::GetPropertyTableSelectQuery()
{
  return "SELECT " ASSET_STORAGE_NAME "_id_fk,property_id_fk,value FROM " ASSET_STORAGE_NAME "_properties";
}


// static
std::string_view rumCustomAsset::GetStorageName()
{
  return ASSET_STORAGE_NAME;
}


// static
const std::string rumCustomAsset::GetTableCreateQuery()
{
  return "CREATE TABLE[" ASSET_STORAGE_NAME "]("
    "[type_id] INTEGER NOT NULL UNIQUE,"
    "[name] TEXT NOT NULL,"
    "[baseclass] TEXT)";
}


// static
const std::string rumCustomAsset::GetTableSelectQuery()
{
  return "SELECT type_id,name,baseclass FROM " ASSET_STORAGE_NAME;
}


// override
const std::string_view rumCustomAsset::GetTypeName() const
{
  return ASSET_STORAGE_NAME;
}


// override
const std::string_view rumCustomAsset::GetTypeSuffix() const
{
  return ASSET_TYPE_SUFFIX;
}


// static
int32_t rumCustomAsset::Init( const std::string& i_strPath )
{
  rumAssert( s_hashAssets.empty() );

  // Build the asset hash from any available resource
  LoadAssets<rumCustomAsset>( i_strPath );

  return RESULT_SUCCESS;
}


void rumCustomAsset::OnCreated( const std::vector<std::string>& i_rvFields )
{
  s_hashAssets.insert( std::make_pair( GetAssetID(), this ) );

  if( GetAssetID() >= s_eNextFreeID )
  {
    s_eNextFreeID = GetAssetID() + 1;
  }
}


// static
void rumCustomAsset::ScriptBind()
{
  rumScript::GetOrCreateClassRegistry( GetClassRegistryID() );

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::RootTable( pcVM ).Func( "rumGetCustomAsset", Fetch );

  Sqrat::DerivedClass<rumCustomAsset, rumAsset> cCustomAsset( pcVM, ASSET_NATIVE_CLASS );
  Sqrat::RootTable( pcVM ).Bind( ASSET_NATIVE_CLASS, cCustomAsset );

  // Bind all available assets to script
  for( const auto& iter : s_hashAssets )
  {
    BindAssetID( iter.first, iter.second->GetName(), ASSET_TYPE_SUFFIX );
  }
}


// static
void rumCustomAsset::Shutdown()
{
  auto* pcRegistry{ rumScript::GetOrCreateClassRegistry( GetClassRegistryID() ) };

  for( const auto& iter : s_hashAssets )
  {
    rumCustomAsset* pcAsset{ iter.second };
    pcRegistry->UnregisterScriptClass( pcAsset->GetAssetID() );
    delete pcAsset;
  }

  s_hashAssets.clear();
}
