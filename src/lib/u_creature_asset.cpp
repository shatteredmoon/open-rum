#include <u_creature_asset.h>

#define ASSET_NATIVE_CLASS "rumCreatureAsset"
#define ASSET_STORAGE_NAME "creature"
#define ASSET_TYPE_SUFFIX  "_Creature"


// Static initializations
std::unordered_map< rumAssetID, rumCreatureAsset* > rumCreatureAsset::s_hashAssets;
rumAssetID rumCreatureAsset::s_eNextFreeID{ FULL_ASSET_ID( Creature_AssetType, 0 ) };


// final
void rumCreatureAsset::ExportCSVFile( std::ofstream& o_rcOutfile ) const
{
  super::ExportCSVFile( o_rcOutfile );

  // Done
  o_rcOutfile << '\n';
}


// static
void rumCreatureAsset::ExportCSVFiles( std::ofstream& o_rcOutfile, std::ofstream& o_rcPropertyOutfile )
{
  // Sort the hash
  using SortedHash = std::map< rumAssetID, rumCreatureAsset* >;
  const SortedHash hashAssets( s_hashAssets.begin(), s_hashAssets.end() );

  // Export csv values
  for( const auto& iter : hashAssets )
  {
    const rumPawnAsset* pcAsset{ iter.second };
    pcAsset->ExportCSVFile( o_rcOutfile );
    pcAsset->ExportCSVPropertyFile( o_rcPropertyOutfile );
  }
}


// final
void rumCreatureAsset::ExportDBTable( std::string& io_strQuery ) const
{
  io_strQuery = "INSERT INTO " ASSET_STORAGE_NAME " (type_id,name,baseclass,graphic_type_id_fk,move_flags,blocks_los,"
                "collision_flags,light_range,render_priority,service_type) VALUES (";
  super::ExportDBTable( io_strQuery );
  io_strQuery += ");";
}


// static
void rumCreatureAsset::ExportDBTables( ServiceType i_eServiceType )
{
  // Sort the hash
  using SortedHash = std::map< rumAssetID, rumCreatureAsset* >;
  const SortedHash hashAssets( s_hashAssets.begin(), s_hashAssets.end() );

  for( const auto& iter : hashAssets )
  {
    std::string strQuery;
    const rumCreatureAsset* pcAsset{ iter.second };
    if( ( pcAsset->GetServiceType() == i_eServiceType ) || ( pcAsset->GetServiceType() == Shared_ServiceType ) )
    {
      pcAsset->ExportDBTable( strQuery );
      pcAsset->ExportDBPropertyTable( strQuery, ASSET_STORAGE_NAME, i_eServiceType );

      rumDatabase::Query( rumDatabase::Assets_DatabaseID, strQuery );
    }
  }
}


// static
rumCreatureAsset* rumCreatureAsset::Fetch( rumAssetID i_eAssetID )
{
  const AssetHash::iterator iter{ s_hashAssets.find( i_eAssetID ) };
  return ( iter != s_hashAssets.end() ) ? iter->second : nullptr;
}


// static
const std::string rumCreatureAsset::GetAssetClassName()
{
  return ASSET_NATIVE_CLASS;
}


// static
const std::string rumCreatureAsset::GetAssetTypeSuffix()
{
  return ASSET_TYPE_SUFFIX;
}


// static
const std::string rumCreatureAsset::GetNativeClassName()
{
  return SCRIPT_CREATURE_NATIVE_CLASS;
}


// static
const std::string rumCreatureAsset::GetPropertyTableCreateQuery()
{
  return "CREATE TABLE [" ASSET_STORAGE_NAME "_properties]("
    "[" ASSET_STORAGE_NAME "_id_fk] INTEGER NOT NULL,"
    "[property_id_fk] INTEGER NOT NULL,"
    "[value] BLOB_TEXT)";
}


// static
const std::string rumCreatureAsset::GetPropertyTableSelectQuery()
{
  return "SELECT " ASSET_STORAGE_NAME "_id_fk,property_id_fk,value FROM " ASSET_STORAGE_NAME "_properties";
}


// static
std::string_view rumCreatureAsset::GetStorageName()
{
  return ASSET_STORAGE_NAME;
}


// static
const std::string rumCreatureAsset::GetTableCreateQuery()
{
  return "CREATE TABLE[" ASSET_STORAGE_NAME "]("
    "[type_id] INTEGER NOT NULL UNIQUE,"
    "[name] TEXT NOT NULL,"
    "[baseclass] TEXT,"
    "[graphic_type_id_fk] INTEGER NOT NULL DEFAULT 0,"
    "[move_flags] INTEGER NOT NULL DEFAULT 0,"
    "[blocks_los] INTEGER NOT NULL DEFAULT 0,"
    "[collision_flags] INTEGER NOT NULL DEFAULT 0,"
    "[light_range] INTEGER NOT NULL DEFAULT 0,"
    "[render_priority] FLOAT NOT NULL DEFAULT(-1.0),"
    "[service_type] INTEGER NOT NULL DEFAULT 0)";
}

// static
const std::string rumCreatureAsset::GetTableSelectQuery()
{
  return "SELECT type_id,name,baseclass,graphic_type_id_fk,move_flags,blocks_los,collision_flags,light_range,"
         "render_priority,service_type FROM " ASSET_STORAGE_NAME;
}


// override
const std::string_view rumCreatureAsset::GetTypeName() const
{
  return ASSET_STORAGE_NAME;
}


// override
const std::string_view rumCreatureAsset::GetTypeSuffix() const
{
  return ASSET_TYPE_SUFFIX;
}


// static
int32_t rumCreatureAsset::Init( const std::string& i_strPath )
{
  rumAssert( s_hashAssets.empty() );

  // Build the asset hash from any available resource
  LoadAssets<rumCreatureAsset>( i_strPath );

  return RESULT_SUCCESS;
}


void rumCreatureAsset::OnCreated( const std::vector<std::string>& i_rvFields )
{
  super::OnCreated( i_rvFields );

  s_hashAssets.insert( std::make_pair( GetAssetID(), this ) );

  if( GetAssetID() >= s_eNextFreeID )
  {
    s_eNextFreeID = GetAssetID() + 1;
  }
}


// static
void rumCreatureAsset::ScriptBind()
{
  rumScript::GetOrCreateClassRegistry( GetClassRegistryID() );

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::RootTable( pcVM ).Func( "rumGetCreatureAsset", Fetch );

  Sqrat::DerivedClass<rumCreatureAsset, rumPawnAsset> cCreatureAsset( pcVM, ASSET_NATIVE_CLASS );
  Sqrat::RootTable( pcVM ).Bind( ASSET_NATIVE_CLASS, cCreatureAsset );

  // Bind all available assets to script
  for( const auto& iter : s_hashAssets )
  {
    BindAssetID( iter.first, iter.second->GetName(), ASSET_TYPE_SUFFIX );
  }
}


// static
void rumCreatureAsset::Shutdown()
{
  auto* pcRegistry{ rumScript::GetOrCreateClassRegistry( GetClassRegistryID() ) };

  for( const auto& iter : s_hashAssets )
  {
    rumCreatureAsset* pcAsset{ iter.second };
    pcRegistry->UnregisterScriptClass( pcAsset->GetAssetID() );
    delete pcAsset;
  }

  s_hashAssets.clear();
}
