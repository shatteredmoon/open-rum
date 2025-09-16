#include <u_asset.h>

#include <u_log.h>
#include <u_property_asset.h>
#include <u_registry.h>
#include <u_rum.h>
#include <u_utility.h>

#include <filesystem>
#include <fstream>


// static
template< typename T >
T* rumAsset::CreateAsset( const rumStringUtils::StringVector& i_rvFields )
{
  enum{ COL_ID, COL_NAME, COL_BASECLASS };

  const rumAssetID eAssetID{ (rumAssetID)rumStringUtils::ToUInt( i_rvFields.at( COL_ID ) ) };
  rumAssert( eAssetID != INVALID_ASSET_ID );

  const std::string& strName{ i_rvFields.at( COL_NAME ) };
  const std::string& strBaseClass{ i_rvFields.at( COL_BASECLASS ) };

  return CreateAsset<T>( eAssetID, strName, strBaseClass, i_rvFields );
}


// static
template< typename T >
T* rumAsset::CreateAsset( const rumAssetID i_eAssetID, const std::string& i_strName, const std::string& i_strBaseClass,
                          const rumStringUtils::StringVector& i_rvFields )
{
  T* pcAsset{ new T };

  pcAsset->m_eAssetID = i_eAssetID;
  pcAsset->m_strName = i_strName;
  pcAsset->m_strBaseClassOverride = i_strBaseClass;

  pcAsset->OnCreated( i_rvFields );

  return pcAsset;
}


// static
template< typename T >
void rumAsset::ExportCSVFiles( const std::string& i_strPath )
{
  std::filesystem::path cFilepath( std::filesystem::path( i_strPath ) / CSV_FOLDER_NAME / T::GetStorageName() );
  std::filesystem::path cPropertyFilepath( cFilepath );

  cFilepath += CSV_EXTENSION;

  cPropertyFilepath += "_property";
  cPropertyFilepath += CSV_EXTENSION;

  std::filesystem::create_directories( cFilepath.parent_path() );

  std::ofstream cOutfile( cFilepath, std::ios::out | std::ios::trunc );
  std::ofstream cPropertyOutfile( cPropertyFilepath, std::ios::out | std::ios::trunc );
  if( cOutfile.is_open() && cPropertyOutfile.is_open() )
  {
    T::ExportCSVFiles( cOutfile, cPropertyOutfile );
    cOutfile.close();
    cPropertyOutfile.close();
  }

  // Remove the file stub if nothing was written
  if( std::filesystem::file_size( cFilepath ) == 0U )
  {
    std::filesystem::remove( cFilepath );
  }

  // Remove the file stub if nothing was written
  if( std::filesystem::file_size( cPropertyFilepath ) == 0U )
  {
    std::filesystem::remove( cPropertyFilepath );
  }
}


// static
template< typename T >
bool rumAsset::ExportDBTables( ServiceType i_eServiceType )
{
  const auto strTableNameView{ T::GetStorageName() };
  const std::string strTableName{ strTableNameView.begin(), strTableNameView.end() };

  std::string strQuery{ "DROP TABLE " + strTableName };
  rumDatabase::Query( rumDatabase::Assets_DatabaseID, strQuery );
  rumDatabase::Query( rumDatabase::Assets_DatabaseID, T::GetTableCreateQuery() );

  const std::string& strPropertyTableCreateQuery{ T::GetPropertyTableCreateQuery() };
  if( !strPropertyTableCreateQuery.empty() )
  {
    strQuery = "DROP TABLE " + strTableName + "_properties";
    rumDatabase::Query( rumDatabase::Assets_DatabaseID, strQuery );
    rumDatabase::Query( rumDatabase::Assets_DatabaseID, strPropertyTableCreateQuery );
  }

  rumDatabase::Query( rumDatabase::Assets_DatabaseID, "BEGIN TRANSACTION" );

  T::ExportDBTables( i_eServiceType );

  const QueryPtr pcQuery{ rumDatabase::Query( rumDatabase::Assets_DatabaseID, "COMMIT" ) };
  if( !pcQuery || pcQuery->IsError() )
  {
    rumDatabase::Query( rumDatabase::Assets_DatabaseID, "ROLLBACK" );
    return RESULT_FAILED;
  }

  return RESULT_SUCCESS;
}


// static
template< typename T >
void rumAsset::LoadAssets( const std::string& i_strPath )
{
  // Try CSV files first
  std::filesystem::path fsFilepath( std::filesystem::path( i_strPath ) / CSV_FOLDER_NAME / T::GetStorageName() );
  std::filesystem::path fsPropertyFilepath( fsFilepath );

  fsFilepath += CSV_EXTENSION;

  std::ifstream cFile( fsFilepath, std::ios::in );
  if( cFile.is_open() )
  {
    std::string strRow;

    while( !cFile.eof() )
    {
      std::getline( cFile, strRow );
      if( !strRow.empty() )
      {
        auto vFields{ rumStringUtils::ParseCSVRow( strRow ) };
        CreateAsset<T>( vFields );
      }
    }

    cFile.close();

    fsPropertyFilepath += "_property";
    fsPropertyFilepath += CSV_EXTENSION;

    std::ifstream cPropertyFile( fsPropertyFilepath, std::ios::in );
    if( cPropertyFile.is_open() )
    {
      while( !cPropertyFile.eof() )
      {
        std::getline( cPropertyFile, strRow );
        if( !strRow.empty() )
        {
          auto vFields{ rumStringUtils::ParseCSVRow( strRow ) };
          ParsePropertyFields<T>( std::move( vFields ) );
        }
      }
    }
  }
  else
  {
    QueryPtr pcQuery{ rumDatabase::Query( rumDatabase::Assets_DatabaseID, T::GetTableSelectQuery() ) };
    if( pcQuery && !pcQuery->IsError() )
    {
      for( int32_t i = 0; i < pcQuery->GetNumRows(); ++i )
      {
        rumStringUtils::StringVector vFields;
        vFields.resize( pcQuery->GetNumCols() );

        for( int32_t j = 0; j < pcQuery->GetNumCols(); ++j )
        {
          vFields[j] = pcQuery->FetchString( i, j );
        }

        CreateAsset<T>( vFields );
      }

      pcQuery = rumDatabase::Query( rumDatabase::Assets_DatabaseID, T::GetPropertyTableSelectQuery() );
      if( pcQuery && !pcQuery->IsError() )
      {
        for( int32_t i = 0; i < pcQuery->GetNumRows(); ++i )
        {
          rumStringUtils::StringVector vFields;
          vFields.resize( pcQuery->GetNumCols() );

          for( int32_t j = 0; j < pcQuery->GetNumCols(); ++j )
          {
            vFields[j] = pcQuery->FetchString( i, j );
          }

          ParsePropertyFields<T>( std::move( vFields ) );
        }
      }
    }
  }
}


// static
template< typename T >
void rumAsset::ParsePropertyFields( rumStringUtils::StringVector&& i_rvFields )
{
  enum { COL_ASSET_ID, COL_PROPERTY_ID, COL_PROPERTY_VALUE };

  const rumAssetID eAssetID{ (rumAssetID)rumStringUtils::ToUInt( i_rvFields.at( COL_ASSET_ID ) ) };
  rumAssert( eAssetID != INVALID_ASSET_ID );

  T* pcAsset{ T::Fetch( eAssetID ) };
  rumAssert( pcAsset );
  if( pcAsset )
  {
    pcAsset->ParseProperty( i_rvFields.at( COL_PROPERTY_ID ), i_rvFields.at( COL_PROPERTY_VALUE ) );
  }
}


// static
template< typename T >
static void rumAsset::RegisterClass( const rumAsset* i_pcAsset )
{
  rumAssert( i_pcAsset != nullptr );
  if( nullptr == i_pcAsset )
  {
    return;
  }

  auto* pcRegistry{ rumScript::GetOrCreateClassRegistry( T::GetClassRegistryID() ) };
  rumAssert( pcRegistry != nullptr );
  if( nullptr == pcRegistry )
  {
    return;
  }

  const auto& strNativeClassName{ T::GetNativeClassName() };
  const auto& strAssetTypeSuffix{ T::GetAssetTypeSuffix() };

  // Append the suffix to the class name
  const std::string strName{ i_pcAsset->GetName() + strAssetTypeSuffix };

  // Use the override if one is provided, otherwise use the native class name
  std::string strBaseClass{ i_pcAsset->GetBaseClassOverride().empty() ? strNativeClassName
                                                                      : i_pcAsset->GetBaseClassOverride() };
  if( !strBaseClass.empty() )
  {
    // Make sure the base class exists before proceeding
    Sqrat::Object sqBaseClass{ Sqrat::RootTable().GetSlot( strBaseClass.c_str() ) };
    if( sqBaseClass.GetType() != OT_CLASS )
    {
      std::string strWarning{ "Warning: class " };
      strWarning += strName;
      strWarning += " extends ";
      strWarning += strBaseClass;
      strWarning += ", but ";
      strWarning += strBaseClass;
      strWarning += " does not exist! Skipping asset!";
      Logger::LogStandard( strWarning, Logger::LOG_WARNING );

      return;
    }
  }

  // Ensure the class object exists
  Sqrat::Object sqClass{ Sqrat::RootTable().GetSlot( strName.c_str() ) };
  if( sqClass.GetType() != OT_CLASS )
  {
    // Class doesn't exist, so try to create it
    sqClass = rumScript::CreateClassScript( strName, strBaseClass );
  }

  if( sqClass.GetType() == OT_CLASS )
  {
    pcRegistry->RegisterScriptClass( i_pcAsset->GetAssetID(), sqClass );
  }
  else
  {
    std::string strWarning{ "Warning: Failed to register class " };
    strWarning += strName;
    Logger::LogStandard( strWarning, Logger::LOG_WARNING );
  }
}


// static
template< typename T >
void rumAsset::RegisterClasses()
{
  for( const auto& iter : T::GetAssetHash() )
  {
    RegisterClass<T>( iter.second );
  }
}
