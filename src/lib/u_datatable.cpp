#include <u_datatable.h>

#include <u_broadcast_asset.h>
#include <u_creature_asset.h>
#include <u_custom_asset.h>
#include <u_db.h>
#include <u_graphic_asset.h>
#include <u_inventory_asset.h>
#include <u_log.h>
#include <u_map_asset.h>
#include <u_portal_asset.h>
#include <u_rum.h>
#include <u_sound_asset.h>
#include <u_tile_asset.h>
#include <u_utility.h>
#include <u_widget_asset.h>

#include <filesystem>
#include <fstream>

std::unordered_map<rumDataTable::DataTableID, rumDataTable> rumDataTable::s_hashDataTables;
std::string rumDataTable::s_strPath;
rumDataTable::DataTableID rumDataTable::s_uiNextDataTableID{ 0 };

#define DATA_TABLE_NAME "datatable"
#define EXTRACT_DATA_TABLE_ID( x ) ( (rumDataTable::DataTableID)( ( x ) >> 24 ) )


rumDataTableColumn& rumDataTable::AccessColumnData( uint32_t i_uiColumn )
{
  static rumDataTableColumn cInvalidColumn;

  if( i_uiColumn < m_cDataTableColumnVector.size() )
  {
    return m_cDataTableColumnVector.at( i_uiColumn );
  }

  return cInvalidColumn;
}


uint32_t rumDataTable::AddDataColumn( std::string i_strName, PropertyValueType i_eValueType )
{
  const uint32_t uiNumRows{ GetNumEntries() };
  const uint32_t uiColumnIndex{ static_cast<uint32_t>(m_cDataTableColumnVector.size()) };

  rumDataTableColumn cColumn;
  cColumn.m_strName = i_strName;
  cColumn.m_eValueType = i_eValueType;
  cColumn.m_cValueVector.resize( uiNumRows );

  Sqrat::Object sqValue{ rumPropertyAsset::GetDefaultValueForType( i_eValueType ) };

  for( size_t i = 0; i < uiNumRows; ++i )
  {
    cColumn.m_cValueVector[i] = sqValue;
  }

  m_cDataTableColumnVector.push_back( std::move( cColumn ) );

  return uiColumnIndex;
}


bool rumDataTable::AddDataEntry()
{
  for( auto& cDataColumn : m_cDataTableColumnVector )
  {
    Sqrat::Object sqValue{ rumPropertyAsset::GetDefaultValueForType( cDataColumn.m_eValueType ) };
    cDataColumn.m_cValueVector.push_back( sqValue );
  }

  return true;
}


// static
bool rumDataTable::AddDataTable( DataTableID i_uiDataTableID, const std::string& i_strName, ServiceType i_eServiceType,
                                 bool i_bNewTable )
{
  if( i_uiDataTableID >= s_uiNextDataTableID )
  {
    s_uiNextDataTableID = i_uiDataTableID + 1;
  }

  rumDataTable cDataTable;
  cDataTable.m_uiDataTableID = i_uiDataTableID;
  cDataTable.m_strName = i_strName;
  cDataTable.m_eServiceType = i_eServiceType;
  cDataTable.m_bLoaded = i_bNewTable;

  // Add the new table to the data table hash
  const auto cPair{ s_hashDataTables.insert( std::make_pair( i_uiDataTableID, cDataTable ) ) };
  if( cPair.second )
  {
    // Expose the data table to scripts
    HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };
    const std::string strDataTableID{ i_strName + "_DataTableID" };
    Sqrat::ConstTable( pcVM ).Const( strDataTableID.c_str(), i_uiDataTableID );
  }

  return cPair.second;
}


void rumDataTable::ExportCSVFile( const std::string& i_strPath ) const
{
  if( !m_bLoaded )
  {
    return;
  }

  const std::filesystem::path fsPath( std::filesystem::path( i_strPath ) / CSV_FOLDER_NAME );
  std::filesystem::create_directories( fsPath );

  std::filesystem::path fsFilepath( fsPath / DATA_TABLE_NAME );
  fsFilepath += '_';
  fsFilepath += GetName();
  fsFilepath += CSV_EXTENSION;

  std::ofstream cOutfile( fsFilepath, std::ios::out | std::ios::trunc );
  if( cOutfile.is_open() )
  {
    ExportCSVFile( cOutfile );
    cOutfile.close();
  }

  // Remove the file stub if nothing was written
  if( std::filesystem::file_size( fsFilepath ) == 0U )
  {
    std::filesystem::remove( fsFilepath );
  }
}


void rumDataTable::ExportCSVFile( std::ofstream& o_rcOutfile ) const
{
  if( !m_bLoaded )
  {
    return;
  }

  std::string strOutput;

  const uint32_t uiNumRows{ GetNumEntries() };

  if( ( 0 == uiNumRows ) || m_cDataTableColumnVector.empty() )
  {
    // Nothing to export
    return;
  }

  // Export the column headings
  for( const auto& rcColumnVector : m_cDataTableColumnVector )
  {
    strOutput += rcColumnVector.m_strName + ',';
  }

  strOutput.pop_back();
  strOutput += '\n';

  // Export the column data types
  for( const auto& rcColumnVector : m_cDataTableColumnVector )
  {
    strOutput += rumStringUtils::ToHexString( rumUtility::ToUnderlyingType( rcColumnVector.m_eValueType ) );
    strOutput += ',';
  }

  strOutput.pop_back();
  strOutput += '\n';

  // Export the table
  for( uint32_t uiRow{ 0 }; uiRow < uiNumRows; ++uiRow )
  {
    for( const auto& rcColumn : m_cDataTableColumnVector )
    {
      auto& sqValue{ rcColumn.m_cValueVector[uiRow] };
      strOutput += rumScript::ToSerializationString( rcColumn.m_eValueType, sqValue ) + ',';
    }

    strOutput.pop_back();
    strOutput += '\n';
  }

  o_rcOutfile << strOutput;
}


// static
void rumDataTable::ExportCSVFiles( const std::string& i_strPath )
{
  if( s_hashDataTables.empty() )
  {
    return;
  }

  // Sort the hash
  using SortedHash = std::map< DataTableID, rumDataTable >;
  const SortedHash cSortedDataTablesHash( s_hashDataTables.begin(), s_hashDataTables.end() );

  std::filesystem::path fsPath( std::filesystem::path( i_strPath ) / CSV_FOLDER_NAME );
  std::filesystem::create_directories( fsPath );

  std::filesystem::path fsFilepath( fsPath / DATA_TABLE_NAME );
  fsFilepath += CSV_EXTENSION;

  std::ofstream cOutfile( fsFilepath, std::ios::out | std::ios::trunc );
  if( cOutfile.is_open() )
  {
    for( const auto& iter : cSortedDataTablesHash )
    {
      const auto& rcDataTable( iter.second );

      cOutfile << rumStringUtils::ToHexString( rcDataTable.GetID() ) << ','
               << rcDataTable.GetName() << ','
               << rcDataTable.GetServiceType() << '\n';

      rcDataTable.ExportCSVFile( i_strPath );
    }

    cOutfile.close();
  }
}


// static
void rumDataTable::ExportDB( const std::string& i_strPath, std::vector<ServiceType>&& i_rcServiceTypeVector )
{
  rumDatabase::Query( rumDatabase::DataTable_DatabaseID, "DROP TABLE data_table" );
  rumDatabase::Query( rumDatabase::DataTable_DatabaseID, "CREATE TABLE[data_table]("
                      "[id] INTEGER NOT NULL UNIQUE,"
                      "[name] TEXT NOT NULL UNIQUE);" );

  for( auto& iter : s_hashDataTables )
  {
    const DataTableID uiDataTableID{ iter.first };
    rumDataTable& rcDataTable{ iter.second };
    const ServiceType eServiceType{ rcDataTable.GetServiceType() };
    if( std::find( i_rcServiceTypeVector.begin(), i_rcServiceTypeVector.end(), eServiceType ) !=
        i_rcServiceTypeVector.end() )
    {
      std::string strQuery{ "INSERT INTO data_table (id,name) VALUES (" };
      strQuery += rumStringUtils::ToString( uiDataTableID );
      strQuery += ",'";
      strQuery += rcDataTable.GetName();
      strQuery += "')";

      rumDatabase::Query( rumDatabase::DataTable_DatabaseID, strQuery );

      rcDataTable.Load();
      rcDataTable.ExportDBTable();
    }
  }

  rumDatabase::Query( rumDatabase::DataTable_DatabaseID, "VACUUM" );
}


void rumDataTable::ExportDBTable() const
{
  if( m_cDataTableColumnVector.empty() )
  {
    return;
  }

  std::string strDataTableName{ DATA_TABLE_NAME };
  strDataTableName += "_";
  strDataTableName += GetName();

  std::string strDataTableHeadingsName{ DATA_TABLE_NAME };
  strDataTableHeadingsName += "_";
  strDataTableHeadingsName += GetName();
  strDataTableHeadingsName += "_";
  strDataTableHeadingsName += "headings";

  std::string strDataTableTypesName{ DATA_TABLE_NAME };
  strDataTableTypesName += "_";
  strDataTableTypesName += GetName();
  strDataTableTypesName += "_";
  strDataTableTypesName += "types";

  rumDatabase::Query( rumDatabase::DataTable_DatabaseID, "DROP TABLE " + strDataTableName );
  rumDatabase::Query( rumDatabase::DataTable_DatabaseID, "DROP TABLE " + strDataTableHeadingsName );
  rumDatabase::Query( rumDatabase::DataTable_DatabaseID, "DROP TABLE " + strDataTableTypesName );

  // Create the headings table
  std::string strQuery{ "CREATE TABLE[" + strDataTableHeadingsName + "]([name] TEXT)" };
  rumDatabase::Query( rumDatabase::DataTable_DatabaseID, strQuery );

  // Write the headings
  for( uint32_t i = 0; i < m_cDataTableColumnVector.size(); ++i )
  {
    strQuery = "INSERT INTO " + strDataTableHeadingsName + " (name) VALUES ('";
    strQuery += m_cDataTableColumnVector.at( i ).m_strName;
    strQuery += "')";
    const QueryPtr pcQuery{ rumDatabase::Query( rumDatabase::DataTable_DatabaseID, strQuery ) };
    rumAssert( !pcQuery->IsError() );
  }

  // Create the column types table
  strQuery = "CREATE TABLE[" + strDataTableTypesName + "]([type] INTEGER);";
  rumDatabase::Query( rumDatabase::DataTable_DatabaseID, strQuery );

  // Write the column types
  for( uint32_t i = 0; i < m_cDataTableColumnVector.size(); ++i )
  {
    const auto uiValue{ rumUtility::ToUnderlyingType( m_cDataTableColumnVector.at( i ).m_eValueType ) };
    strQuery = "INSERT INTO " + strDataTableTypesName + " (type) VALUES (";
    strQuery += rumStringUtils::ToHexString( uiValue );
    strQuery += ")";

    const QueryPtr pcQuery{ rumDatabase::Query( rumDatabase::DataTable_DatabaseID, strQuery ) };
    rumAssert( !pcQuery->IsError() );
  }

  // Create the data table
  strQuery = "CREATE TABLE[" + strDataTableName + "](";
  for( uint32_t i = 0; i < m_cDataTableColumnVector.size(); ++i )
  {
    const auto& rcColumnVector{ m_cDataTableColumnVector.at( i ) };

    strQuery += "[col";
    strQuery += rumStringUtils::ToString( i );
    strQuery += "] ";

    if( rcColumnVector.m_eValueType == PropertyValueType::Float )
    {
      strQuery += "REAL";
    }
    else if( rcColumnVector.m_eValueType == PropertyValueType::Null )
    {
      strQuery += "NULL";
    }
    else if( rcColumnVector.m_eValueType == PropertyValueType::String )
    {
      strQuery += "TEXT";
    }
    else
    {
      strQuery += "INTEGER";
    }

    strQuery += ',';
  }

  strQuery.pop_back();
  strQuery += ")";

  rumDatabase::Query( rumDatabase::DataTable_DatabaseID, strQuery );

  // Write the data table data
  const auto uiNumRows{ GetNumEntries() };
  const auto uiNumCols{ GetNumColumns() };

  rumDatabase::Query( rumDatabase::DataTable_DatabaseID, "BEGIN TRANSACTION" );

  for( uint32_t i = 0; i < uiNumRows; ++i )
  {
    strQuery = "INSERT INTO " + strDataTableName + " (";
    for( uint32_t j = 0; j < uiNumCols; ++j )
    {
      strQuery += "col";
      strQuery += rumStringUtils::ToString( j );
      strQuery += ',';
    }
    strQuery.pop_back();
    strQuery += ") VALUES (";
    for( uint32_t j = 0; j < uiNumCols; ++j )
    {
      const auto& rcDataColumn{ GetColumnData( j ) };
      Sqrat::Object sqValue{ rcDataColumn.m_cValueVector.at( i ) };
      strQuery += rumScript::ToSerializationString( rcDataColumn.m_eValueType, sqValue );
      strQuery += ',';
    }
    strQuery.pop_back();
    strQuery += ");";

    rumDatabase::Query( rumDatabase::DataTable_DatabaseID, strQuery );
  }

  rumDatabase::Query( rumDatabase::DataTable_DatabaseID, "COMMIT" );
}


const rumDataTableColumn& rumDataTable::GetColumnData( uint32_t i_uiColumn ) const
{
  static rumDataTableColumn cInvalidColumn;

  if( i_uiColumn < m_cDataTableColumnVector.size() )
  {
    return m_cDataTableColumnVector.at( i_uiColumn );
  }

  return cInvalidColumn;
}


// static
Sqrat::Array rumDataTable::GetColumnData( DataTableID i_uiDataTableID, uint32_t i_uiCol )
{
  const auto& rcDataTable{ GetDataTable( i_uiDataTableID ) };
  const auto& rcColumnData{ rcDataTable.GetColumnData( i_uiCol ) };

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };
  int32_t iCount{ static_cast<int32_t>( rcColumnData.m_cValueVector.size() ) };

  // It's okay to have zero length squirrel arrays
  Sqrat::Array sqArray( pcVM, iCount );

  for( int32_t i = 0; i < iCount; ++i )
  {
    sqArray.SetValue( i, rcColumnData.m_cValueVector[i] );
  }

  return sqArray;
}


// static
rumDataTable& rumDataTable::GetDataTable( DataTableID i_uiDataTableID )
{
  static rumDataTable sDataTable;

  const auto& iter{ s_hashDataTables.find( i_uiDataTableID ) };
  if( iter != s_hashDataTables.end() )
  {
    return iter->second;
  }

  return sDataTable;
}


// static
std::vector<rumDataTable::DataTableID> rumDataTable::GetDataTableIDs()
{
  std::vector<DataTableID> cDataTableIDVector;
  for( const auto& iter : s_hashDataTables )
  {
    cDataTableIDVector.push_back( iter.first );
  }

  return cDataTableIDVector;
}


// static
Sqrat::Array rumDataTable::GetRowData( DataTableID i_uiDataTableID, uint32_t i_uiRow )
{
  const auto& rcDataTable{ GetDataTable( i_uiDataTableID ) };

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };
  uint32_t iCount{ rcDataTable.GetNumColumns() };

  // It's okay to have zero length squirrel arrays
  Sqrat::Array sqArray( pcVM, iCount );

  for( uint32_t i = 0; i < iCount; ++i )
  {
    const auto& rcColumnData{ rcDataTable.GetColumnData( i ) };
    if( i_uiRow < rcColumnData.m_cValueVector.size() )
    {
      sqArray.SetValue( i, rcColumnData.m_cValueVector[i_uiRow] );
    }
    else
    {
      sqArray.SetValue( i, 0 );
    }
  }

  return sqArray;
}


// static
int32_t rumDataTable::Init( const std::string& i_strPath )
{
  s_hashDataTables.clear();
  s_strPath = i_strPath;

  return LoadDataTableHash();
}


int32_t rumDataTable::Load()
{
  if( m_bLoaded )
  {
    // Already loaded
    return GetID();
  }

  // Try CSV files first
  std::filesystem::path fsFilepath( std::filesystem::path( s_strPath ) / CSV_FOLDER_NAME / DATA_TABLE_NAME );
  fsFilepath += '_';
  fsFilepath += m_strName;
  fsFilepath += CSV_EXTENSION;

  if( std::filesystem::exists( fsFilepath ) )
  {
    return LoadFromFile( fsFilepath.string() );
  }

  return LoadFromDatabase();
}


// static
int32_t rumDataTable::LoadDataTable( DataTableID i_uiTableID )
{
  const auto& iter{ s_hashDataTables.find( i_uiTableID ) };
  if( iter == s_hashDataTables.end() )
  {
    return RESULT_FAILED;
  }

  rumDataTable& rcDataTable{ iter->second };
  return rcDataTable.Load();
}


// static
int32_t rumDataTable::LoadDataTableHash()
{
  // Try CSV first
  std::filesystem::path fsFilepath{ std::filesystem::path( s_strPath ) / CSV_FOLDER_NAME / DATA_TABLE_NAME };
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
        const auto vFields{ rumStringUtils::ParseCSVRow( strRow ) };
        ParseFields( vFields );
      }
    }

    cFile.close();
  }
  else
  {
    const std::string strQuery{"SELECT id,name FROM data_table"};
    const QueryPtr pcQuery{ rumDatabase::Query( rumDatabase::DataTable_DatabaseID, strQuery ) };
    if( pcQuery && !pcQuery->IsError() )
    {
      enum{ COL_ID, COL_NAME };

      for( int32_t i = 0; i < pcQuery->GetNumRows(); ++i )
      {
        const DataTableID uiDataTableID{ (DataTableID)pcQuery->FetchInt( i, COL_ID ) };
        const std::string strName{ pcQuery->FetchString( i, COL_NAME ) };

        if( !AddDataTable( uiDataTableID, strName, Shared_ServiceType, false /* not new */) )
        {
          const std::string strError{ "Error: Failed to create entry for data table '" + strName + '"' };
          Logger::LogStandard( strError, Logger::LOG_ERROR );
        }
      }
    }
    else
    {
      const std::string strError{ "Error: Failed to query data_table from datatable.db" };
      Logger::LogStandard( strError, Logger::LOG_ERROR );
    }
  }

  return( s_hashDataTables.size() > 0 ? RESULT_SUCCESS : RESULT_FAILED );
}


int32_t rumDataTable::LoadFromDatabase()
{
  int32_t eResult{ RESULT_SUCCESS };

  // Read headings table
  std::string strQuery{ "SELECT name FROM " DATA_TABLE_NAME "_" + m_strName + "_headings ORDER BY rowid" };

  QueryPtr pcQuery{ rumDatabase::Query( rumDatabase::DataTable_DatabaseID, strQuery ) };
  if( pcQuery && !pcQuery->IsError() )
  {
    for( int32_t i = 0; i < pcQuery->GetNumRows(); ++i )
    {
      const std::string strColumnName{ pcQuery->FetchString( i, 0 ) };
      AddDataColumn( strColumnName, PropertyValueType::Null );
    }

    // Read the column types
    strQuery = "SELECT type FROM " DATA_TABLE_NAME "_" + m_strName + "_types ORDER BY rowid";

    pcQuery = rumDatabase::Query( rumDatabase::DataTable_DatabaseID, strQuery );
    if( pcQuery && !pcQuery->IsError() )
    {
      for( int32_t i = 0; i < pcQuery->GetNumRows(); ++i )
      {
        const PropertyValueType eValueType{ static_cast<PropertyValueType>( pcQuery->FetchInt( i, 0 ) ) };

        auto& rcColumnData{ AccessColumnData( i ) };
        rcColumnData.m_eValueType = eValueType;
      }

      const uint32_t uiNumColumns{ GetNumColumns() };

      // Read the data
      strQuery = "SELECT ";

      for( uint32_t i = 0; i < uiNumColumns; ++i )
      {
        strQuery += "col";
        strQuery += rumStringUtils::ToString( i );
        strQuery += ',';
      }

      strQuery.pop_back();
      strQuery += " FROM " DATA_TABLE_NAME "_" + m_strName + " ORDER BY rowid";

      pcQuery = rumDatabase::Query( rumDatabase::DataTable_DatabaseID, strQuery );
      if( pcQuery && !pcQuery->IsError() )
      {
        const int32_t iNumRows{ pcQuery->GetNumRows() };

        // Prepare the column vectors for n rows
        for( uint32_t i = 0; i < uiNumColumns; ++i )
        {
          auto& rcColumnData{ AccessColumnData( i ) };
          rcColumnData.m_cValueVector.resize( iNumRows );
        }

        // Fill each entry with data
        for( int32_t i = 0; i < pcQuery->GetNumRows(); ++i )
        {
          for( uint32_t j = 0; j < uiNumColumns; ++j )
          {
            auto& rcColumnData{ AccessColumnData( j ) };

            if( PropertyValueType::Null == rcColumnData.m_eValueType )
            {
              // Do nothing since the script value should already be Null
            }
            else if( PropertyValueType::Bool == rcColumnData.m_eValueType )
            {
              rumScript::SetValue( rcColumnData.m_cValueVector[i], pcQuery->FetchBool( i, j ) );
            }
            else if( PropertyValueType::Float == rcColumnData.m_eValueType )
            {
              rumScript::SetValue( rcColumnData.m_cValueVector[i], pcQuery->FetchFloat( i, j ) );
            }
            else if( PropertyValueType::String == rcColumnData.m_eValueType )
            {
              rumScript::SetValue( rcColumnData.m_cValueVector[i], pcQuery->FetchString( i, j ) );
            }
            else
            {
              rumScript::SetValue( rcColumnData.m_cValueVector[i], pcQuery->FetchInt64( i, j ) );
            }
          }
        }
      }
      else
      {
        eResult = RESULT_FAILED;
      }
    }
    else
    {
      eResult = RESULT_FAILED;
    }
  }
  else
  {
    eResult = RESULT_FAILED;
  }

  if( RESULT_SUCCESS == eResult )
  {
    m_bLoaded = true;
  }
  else
  {
    std::string strError{ "Error: Failed to load data table " };
    strError += m_strName;
    strError += ", ";
    strError += pcQuery != nullptr ? pcQuery->GetErrorMsg() : "<uknown error>";
    Logger::LogStandard( strError, Logger::LOG_WARNING );
  }

  return eResult;
}


int32_t rumDataTable::LoadFromFile( const std::string& i_strFilePath )
{
  if( m_bLoaded )
  {
    return RESULT_SUCCESS;
  }

  m_cDataTableColumnVector.clear();

  int32_t eResult{ RESULT_SUCCESS };
  std::filesystem::path fsDataTable( i_strFilePath );

  // Get the token hash for the specified language
  if( std::filesystem::exists( fsDataTable ) )
  {
    std::ifstream cFile( fsDataTable.string(), std::ios::in );
    if( cFile.is_open() )
    {
      std::string strRow;

      // Parse the header row
      std::getline( cFile, strRow );

      const auto cHeaderVector{ rumStringUtils::ParseCSVRow( strRow ) };
      for( const auto& strHeader : cHeaderVector )
      {
        rumDataTableColumn cColumn;
        cColumn.m_strName = strHeader;

        m_cDataTableColumnVector.push_back( std::move( cColumn ) );
      }

      // Parse the type row
      std::getline( cFile, strRow );

      uint32_t uiIndex{ 0 };
      const auto cTypeVector{ rumStringUtils::ParseCSVRow( strRow ) };
      for( const auto& strType : cTypeVector )
      {
        auto& cColumn{ m_cDataTableColumnVector.at( uiIndex++ ) };
        cColumn.m_eValueType = static_cast<PropertyValueType>( rumStringUtils::ToUInt( strType ) );
      }

      // Parse all remaining data rows
      while( !cFile.eof() )
      {
        std::getline( cFile, strRow );
        if( !strRow.empty() )
        {
          uiIndex = 0;
          const auto cDataVector{ rumStringUtils::ParseCSVRow( strRow ) };
          for( const auto& strValue : cDataVector )
          {
            auto& cColumn{ m_cDataTableColumnVector.at( uiIndex++ ) };

            Sqrat::Object sqValue{ rumScript::ToValue( cColumn.m_eValueType, strValue ) };
            cColumn.m_cValueVector.push_back( sqValue );
          }
        }
      }

      m_bLoaded = true;
    }
  }

  if( !m_bLoaded )
  {
    std::string strError{ "Error: Failed to load data table " };
    strError += m_strName;
    strError += ", file ";
    strError += fsDataTable.string();
    strError += " could not be found or opened";
    Logger::LogStandard( strError, Logger::LOG_WARNING );

    eResult = RESULT_FAILED;
  }

  return eResult;
}


// static
void rumDataTable::ParseFields( const std::vector<std::string>& i_rvFields )
{
  enum { COL_ID, COL_NAME, COL_SERVICE_TYPE };

  const DataTableID uiTableID{ (DataTableID)rumStringUtils::ToUInt( i_rvFields.at( COL_ID ) ) };
  const std::string& strName{ i_rvFields.at( COL_NAME ) };
  const ServiceType eServiceType{ (ServiceType)rumStringUtils::ToUInt( i_rvFields.at( COL_SERVICE_TYPE ) ) };
  if( !AddDataTable( uiTableID, strName, eServiceType, false /* not new */) )
  {
    std::string strError{ "Error: Failed to create entry for string table '" };
    strError += strName;
    strError += '\'';
    Logger::LogStandard( strError, Logger::LOG_ERROR );
  }
}


void rumDataTable::RemoveDataColumn( uint32_t i_uiCol )
{
  rumAssert( i_uiCol < m_cDataTableColumnVector.size() );
  if( i_uiCol >= m_cDataTableColumnVector.size() )
  {
    return;
  }

  m_cDataTableColumnVector.erase( m_cDataTableColumnVector.begin() + i_uiCol );
}


void rumDataTable::RemoveDataEntry( uint32_t i_uiRow )
{
  for( auto& rcDataColumn : m_cDataTableColumnVector )
  {
    if( rcDataColumn.m_cValueVector.size() > i_uiRow )
    {
      rcDataColumn.m_cValueVector.erase( rcDataColumn.m_cValueVector.begin() + i_uiRow );
    }
  }
}


bool rumDataTable::RemoveDataTable( DataTableID i_uiDataTableID )
{
  return s_hashDataTables.erase( i_uiDataTableID ) >= 1;
}


// static
void rumDataTable::ScriptBind()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  //Sqrat::RootTable( pcVM )
  Sqrat::RootTable( pcVM )
    .Overload<Sqrat::Array( * )( DataTableID, uint32_t )>( "rumGetDataTableColumn", GetColumnData )
    .Func( "rumGetDataTableRow", GetRowData );

  // Expose all data tables to script
  for( const auto uiDataTableID : GetDataTableIDs() )
  {
    rumDataTable& rcDataTable{ GetDataTable( uiDataTableID ) };
    rcDataTable.Load(); // TODO - lazy load?
    const std::string strDataTable{ rcDataTable.GetName() + "_DataTableID" };
    Sqrat::ConstTable( pcVM ).Const( strDataTable.c_str(), uiDataTableID );
  }
}


// static
void rumDataTable::Shutdown()
{
  // Nothing to do
}
