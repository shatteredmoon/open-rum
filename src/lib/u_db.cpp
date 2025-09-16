#include <u_db.h>

#include <u_log.h>
#include <u_property_asset.h>

#include <filesystem>
#include <fstream>

// Static initializers
bool rumDatabase::s_bInitialized{ false };
uint32_t rumDatabase::s_uiIDStoreReserveInterval{ 200 };
uint32_t rumDatabase::s_uiIDStoreUpdateThreshold{ 100 };
rumDatabase::DatabaseContainer rumDatabase::s_hashSynchronousDBs;
rumDatabase::IDStoreTableContainer rumDatabase::s_hashIDStore;

rumAsyncDatabase::AsyncDatabaseContainer rumAsyncDatabase::s_cAsynchronousDatabaseMap;
rumAsyncDatabase::QueryStagingContainer rumAsyncDatabase::s_cStagingQueue;
std::mutex rumAsyncDatabase::s_mtxQueue;


// static
void rumDatabase::CheckpointAll()
{
  // Release all databases
  for( const auto& iter : s_hashSynchronousDBs )
  {
    sqlite3_wal_checkpoint( iter.second->m_pcHandle, nullptr );
  }
}


void rumDatabase::Close()
{
  if( m_bConnected )
  {
    sqlite3_wal_checkpoint( m_pcHandle, nullptr );

    sqlite3_close( m_pcHandle );
    m_bConnected = false;
    m_strFilename.clear();
    m_eDatabaseID = DatabaseID::Invalid_DatabaseID;
  }
}


// static
void rumDatabase::CloseConnection( DatabaseID i_eDatabaseID )
{
  rumDatabase* pcDatabase{ GetSynchronousDB( i_eDatabaseID ) };
  if( pcDatabase )
  {
    pcDatabase->Close();
    s_hashSynchronousDBs.erase( i_eDatabaseID );
  }
}


// Attempt to connect to a DB
bool rumDatabase::Connect( const std::string& i_strFilename, DatabaseID i_eDatabaseID )
{
  // Close the current handle
  Close();

  if( i_strFilename.empty() )
  {
    return false;
  }

  int32_t eResult{ sqlite3_open( i_strFilename.c_str(), &m_pcHandle ) };
  if( SQLITE_OK == eResult )
  {
    // Set a 10ms timeout for busy (locked) tables in SQLite
    //sqlite3_busy_timeout(m_Handle, 10);

    m_bConnected = true;
    m_strFilename = i_strFilename;
    m_eDatabaseID = i_eDatabaseID;

    sqlite3_exec( m_pcHandle, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr );
    //sqlite3_exec( m_pcHandle, "PRAGMA encoding=\"UTF-8\";", nullptr, nullptr, nullptr );

    //sqlite3_exec( m_pcHandle, "PRAGMA wal_autocheckpoint = 0;", nullptr, nullptr, nullptr );
    //sqlite3_exec( m_pcHandle, "PRAGMA synchronous = FULL;", nullptr, nullptr, nullptr );

    // Callback when a delete, insert, or update are performed
    // The third parameter will be the first parameter of the callback
    //sqlite3_update_hook( m_pcHandle, dbUpdate, nullptr );
  }
  else
  {
    Logger::LogStandard( sqlite3_errmsg( m_pcHandle ), Logger::LOG_ERROR );
  }

  return m_bConnected;
}


// static
bool rumDatabase::CreateConnection( DatabaseID i_eDatabaseID, const std::string& i_strFilename )
{
  // Failure here means Init was never called
  rumAssert( s_bInitialized );
  rumAssert( !i_strFilename.empty() );

  // Make sure the filename is valid
  if( i_strFilename.empty() )
  {
    return false;
  }

  // Make sure the id doesn't already exist
  if( s_hashSynchronousDBs.find( i_eDatabaseID ) != s_hashSynchronousDBs.end() )
  {
    std::string strWarning{ "Already requested a connection for database " };
    strWarning += i_strFilename;
    Logger::LogStandard( strWarning, Logger::LOG_WARNING );
    return true;
  }

  std::filesystem::path fsPath( i_strFilename );
  std::string strPath{ fsPath.generic_string() };

  Logger::LogStandard( "Connecting to database " + strPath );

  rumDatabase* pcDatabase{ new rumDatabase };
  rumAssert( pcDatabase );
  bool bConnected{ pcDatabase->Connect( strPath, i_eDatabaseID ) };
  if( bConnected )
  {
    // Does the database have any tables?
    if( !pcDatabase->HasTables() )
    {
      pcDatabase->InitTables();
    }

    sqlite3_wal_checkpoint( pcDatabase->m_pcHandle, nullptr );

    // Save connections to each Database
    s_hashSynchronousDBs.insert( std::make_pair( i_eDatabaseID, pcDatabase ) );
  }

  return bConnected;
}


// static
void rumDatabase::CreateIDStore( IDStoreTableType i_eTableType )
{
  rumIDStore cIDStore;
  cIDStore.m_eTableType = i_eTableType;

  UpdateIDStore( cIDStore, true /* creating */ );

  const auto cPair{ s_hashIDStore.insert( std::make_pair( i_eTableType, cIDStore ) ) };
  if( !cPair.second )
  {
    std::string strError{ "Error: Failed to create ID store for table type " };
    strError += rumStringUtils::ToString( (int32_t)i_eTableType );
    rumAssertMsg( false, strError );
  }
}


// static
void rumDatabase::DumpTableNames( DatabaseID i_eDatabaseID )
{
  const std::string strQuery{ "SELECT name FROM sqlite_master WHERE type='table' ORDER BY name" };

  std::string strLog{ "Database " };
  strLog += rumStringUtils::ToString( (int32_t)i_eDatabaseID );
  strLog += " tables:\n";

  const QueryPtr pcQuery{ Query( i_eDatabaseID, strQuery ) };
  if( pcQuery && !( pcQuery->IsError() ) )
  {
    for( int32_t i = 0; i < pcQuery->GetNumRows(); ++i )
    {
      strLog += pcQuery->FetchString( i, 0 );
      strLog += '\n';
    }
  }

  Logger::LogStandard( strLog );
}


bool rumDatabase::HasTables()
{
  rumAssert( s_bInitialized );
  rumAssert( m_pcHandle );

    // Does the database have any tables?
  QueryPtr pcQuery{ Query( "SELECT count(*) FROM sqlite_master WHERE type = 'table'" ) };
  if( pcQuery && !pcQuery->IsError() )
  {
    int32_t iNumTables{ pcQuery->FetchInt( 0, 0 ) };
    return iNumTables > 0;
  }

  return false;
}


// static
const std::string& rumDatabase::GetFilename( DatabaseID i_eDatabaseID )
{
  rumDatabase* pcDatabase{ GetSynchronousDB( i_eDatabaseID ) };
  if( pcDatabase )
  {
    return pcDatabase->GetFilename();
  }
  else
  {
    pcDatabase = rumAsyncDatabase::GetAsynchronousDB( i_eDatabaseID );
    if( pcDatabase )
    {
      return pcDatabase->GetFilename();
    }
  }

  return rumStringUtils::NullString();
}


// static
uint64_t rumDatabase::GetNextIDFromIDStore( IDStoreTableType i_eTableType )
{
  uint64_t uiNextID{ INVALID_GAME_ID };

  const IDStoreTableContainer::iterator iter{ s_hashIDStore.find( i_eTableType ) };
  if( iter != s_hashIDStore.end() )
  {
    rumIDStore& rcIDStore{ iter->second };

    // Build the ID with the table type embedded
    uiNextID = ( ( (rumUniqueID)i_eTableType ) << 60 );
    uiNextID |= rcIDStore.m_uiNextID;

    // Have we ran out of IDs?
    if( rcIDStore.m_uiLastID - ++( rcIDStore.m_uiNextID ) < s_uiIDStoreUpdateThreshold )
    {
      // Reserve more IDs
      UpdateIDStore( rcIDStore, false /* not creating */ );
    }
  }
  else
  {
    std::string strError{ "Error: Failed to find ID Store table type " };
    strError += rumStringUtils::ToString( (int32_t)i_eTableType );
    Logger::LogStandard( strError );
  }

  return uiNextID;
}


// static
rumDatabase* rumDatabase::GetSynchronousDB( DatabaseID i_eDatabaseID )
{
  DatabaseContainer::iterator iter( s_hashSynchronousDBs.find( i_eDatabaseID ) );
  return iter != s_hashSynchronousDBs.end() ? iter->second : nullptr;
}


// static
void rumDatabase::GetTableColumnInfo( DatabaseID i_eDatabaseID,
                                      const std::string& i_strTable,
                                      std::vector<std::string>& o_rcColumnNames,
                                      std::vector<std::string>& o_rcColumnTypes )
{
  GetTableColumnNames( i_eDatabaseID, i_strTable, o_rcColumnNames );
  GetTableColumnTypes( i_eDatabaseID, i_strTable, o_rcColumnNames, o_rcColumnTypes );
}


// static
void rumDatabase::GetTableColumnNames( DatabaseID i_eDatabaseID,
                                       const std::string& i_strTable,
                                       std::vector<std::string>& o_rcColumnNames )
{
  std::string strQuery{ "PRAGMA table_info(" };
  strQuery += i_strTable;
  strQuery += ")";

  QueryPtr pcQuery{ Query( i_eDatabaseID, strQuery ) };
  if( pcQuery && !( pcQuery->IsError() ) )
  {
    for( int32_t i = 0; i < pcQuery->GetNumRows(); ++i )
    {
      o_rcColumnNames.push_back( pcQuery->FetchString( i, 1 ) );
    }
  }
}


// static
void rumDatabase::GetTableColumnTypes( DatabaseID i_eDatabaseID,
                                       const std::string& i_strTable,
                                       const std::vector<std::string>& i_rcColumnNames,
                                       std::vector<std::string>& o_rcColumnTypes )
{
  /*
  #define SQLITE_INTEGER  1
  #define SQLITE_FLOAT    2
  #define SQLITE_TEXT     3
  #define SQLITE3_TEXT    3
  #define SQLITE_BLOB     4
  #define SQLITE_NULL     5
  */

  if( i_rcColumnNames.size() == 0 )
  {
    return;
  }

  std::string strQuery{ "SELECT " };

  for( size_t i = 0; i < i_rcColumnNames.size(); ++i )
  {
    strQuery += "TYPEOF(";
    strQuery += i_rcColumnNames[i];
    strQuery += ")";

    if( i + 1 < i_rcColumnNames.size() )
    {
      strQuery += ",";
    }
  }

  strQuery += " FROM ";
  strQuery += i_strTable;
  strQuery += " LIMIT 1";

  QueryPtr pcQuery{ Query( i_eDatabaseID, strQuery ) };
  if( pcQuery && !( pcQuery->IsError() ) )
  {
    for( uint32_t i = 0; i < i_rcColumnNames.size(); ++i )
    {
      o_rcColumnTypes.push_back( pcQuery->FetchString( 0, i ) );
    }
  }
}


// static
void rumDatabase::GetTableNames( DatabaseID i_eDatabaseID, std::set<std::string>& o_rcTableNames )
{
  std::string strQuery{ "SELECT name FROM sqlite_master WHERE type='table' ORDER BY name" };
  QueryPtr pcQuery{ Query( i_eDatabaseID, strQuery ) };
  if( pcQuery && !( pcQuery->IsError() ) )
  {
    for( int32_t i = 0; i < pcQuery->GetNumRows(); ++i )
    {
      o_rcTableNames.insert( pcQuery->FetchString( i, 0 ) );
    }
  }
}


// static
int32_t rumDatabase::Init()
{
  //setOutputColor(COLOR_SERVER);
  RUM_COUT( "\nInitializing SQLite version " << SQLITE_VERSION );
  if( sqlite3_threadsafe() == 0 )
  {
    RUM_COUT( "(not threadsafe)\n" );

    // Recompile your version of SQLite with SQLITE_THREADSAFE defined
    assert( false );
  }
  else
  {
    RUM_COUT( "(threadsafe)\n" );
  }

  // Simply tracks that Init was called
  s_bInitialized = true;

  //setOutputColor(COLOR_STANDARD);

  return RESULT_SUCCESS;
}


void rumDatabase::InitTables()
{
  rumAssert( s_bInitialized );
  rumAssert( m_pcHandle );
}


QueryPtr rumDatabase::Query( const std::string& i_strQuery )
{
  std::shared_ptr<rumDatabaseQuery> pcQuery( new rumDatabaseQuery );
  if( pcQuery )
  {
    pcQuery->Execute( m_pcHandle, i_strQuery );
  }

  return pcQuery;
}


// static
QueryPtr rumDatabase::Query( DatabaseID i_eDatabaseID, const std::string& i_strQuery )
{
  // Perform synchronous query from the specified database
  DatabaseContainer::iterator iter( s_hashSynchronousDBs.find( i_eDatabaseID ) );
  return s_hashSynchronousDBs.end() != iter ? iter->second->Query( i_strQuery ) : nullptr;
}


// static
void rumDatabase::RefundIDStore()
{
  for( const auto& iter : s_hashIDStore )
  {
    std::string strQuery{ "UPDATE id_store SET next_id=" };
    strQuery += rumStringUtils::ToString64( iter.second.m_uiNextID );
    strQuery += " WHERE table_id=";
    strQuery += rumStringUtils::ToString( iter.first );
    Query( DatabaseID::Player_DatabaseID, strQuery );
  }
}


// static
void rumDatabase::Shutdown()
{
  // Write the ID Store results back to the db so reserved numbers aren't wasted
  RefundIDStore();

  // Release all databases
  for( auto& iter : s_hashSynchronousDBs )
  {
    // Disconnect each database connection
    iter.second->Close();

    // Free memory
    delete iter.second;
  }

  s_hashSynchronousDBs.clear();

  rumAsyncDatabase::ShutdownAsync();
}


bool rumDatabase::TableExists( const std::string& i_strTableName )
{
  bool bExists{ false };

  std::string strQuery{ "SELECT name FROM sqlite_master WHERE type='table' AND name='" };
  strQuery += i_strTableName;
  strQuery += "' LIMIT 1";

  QueryPtr pcQuery{ Query( strQuery ) };
  if( !pcQuery )
  {
    Logger::LogStandard( "DB::tableExists query failed" );
  }
  else if( pcQuery->IsError() )
  {
    Logger::LogStandard( pcQuery->GetErrorMsg() );
  }
  else if( pcQuery->GetNumRows() > 0 )
  {
    bExists = true;
  }

  return bExists;
}


bool rumDatabase::TableExport( const std::string& i_strTableName, const std::string& i_strFile ) const
{
  // Create the output file
  std::ofstream cOutfile( i_strFile.c_str(), std::ios::out | std::ios::trunc );
  if( !cOutfile.is_open() )
  {
    return false;
  }

  std::vector<std::string> cColumnNames;
  std::vector<std::string> cColumnTypes;

  // Output the table columns
  GetTableColumnInfo( m_eDatabaseID, i_strTableName, cColumnNames, cColumnTypes );

  std::string strQuery = "SELECT ";

  for( size_t i = 0; i < cColumnNames.size(); ++i )
  {
    cOutfile << "\"" << cColumnNames[i] << "\"";
    strQuery += cColumnNames[i];

    if( i + 1 < cColumnNames.size() )
    {
      cOutfile << ",";
      strQuery += ",";
    }
  }

  cOutfile << std::endl;

  strQuery += " FROM ";
  strQuery += i_strTableName;

  QueryPtr pcQuery{ Query( m_eDatabaseID, strQuery ) };
  if( pcQuery && !( pcQuery->IsError() ) )
  {
    for( int32_t i = 0; i < pcQuery->GetNumRows(); ++i )
    {
      for( size_t j = 0; j < cColumnNames.size(); ++j )
      {
        if( cColumnTypes[j].compare( "text" ) == 0 ||
            cColumnTypes[j].compare( "blob" ) == 0 )
        {
          // Add qoutation marks around the data and delimit data as needed
          std::string strTemp( pcQuery->FetchString( i, (uint32_t)j ) );
          rumStringUtils::Replace( strTemp, "\"", "\"\"" );

          cOutfile << "\"" << strTemp << "\"";
        }
        else
        {
          // Output the value as is
          cOutfile << pcQuery->FetchString( i, (uint32_t)j );
        }

        if( j + 1 < cColumnNames.size() )
        {
          cOutfile << ",";
        }
      }

      cOutfile << std::endl;
    }
  }

  cOutfile.close();

  return true;
}


bool rumDatabase::TableImport( const std::string& i_strTable, const std::string& i_strFile )
{
  // Open the input file
  std::ifstream cInfile( i_strFile.c_str(), std::ios::in );
  if( !cInfile.is_open() )
  {
    return false;
  }

  // Remove all the contents of this table
  std::string strQuery{ "DELETE FROM " };
  strQuery += i_strTable;

  QueryPtr pcQuery{ Query( m_eDatabaseID, strQuery ) };
  if( !pcQuery || pcQuery->IsError() )
  {
    return false;
  }

  std::string strLine;
  std::vector<std::string> strResults;

  // The first line contains column names
  std::getline( cInfile, strLine, '\n' );

  // Remove all quotation marks and tokenize
  rumStringUtils::Replace( strLine, "\"", "" );
  rumStringUtils::Split( strResults, strLine, "," );

  const uint32_t uiColumns{ (uint32_t)strResults.size() };

  // Build our insert string
  std::string strInsert{ "INSERT INTO " };
  strInsert += i_strTable;
  strInsert += " (";

  for( uint32_t i = 0; i < uiColumns; ++i )
  {
    strInsert += strResults[i];
    if( i + 1 < uiColumns )
    {
      strInsert += ",";
    }
  }

  strInsert += ") VALUES (";

  strQuery = "BEGIN;";

  while( cInfile.good() )
  {
    strLine.clear();
    strResults.clear();

    // Parse and insert each line
    std::getline( cInfile, strLine, '\n' );
    if( strLine.empty() )
    {
      continue;
    }

    rumStringUtils::Split( strResults, strLine, "," );

    strQuery += strInsert;

    for( uint32_t i = 0, j = 0; i < uiColumns; ++i, ++j )
    {
      if( strResults[j].size() == 0 )
      {
        strQuery += "null";
      }

      // If the std::string starts with a quotation mark, it's either a string or a blob value and we must handle it
      // differently
      else if( strResults[j][0] == '"' )
      {
        // We must now find a single quotation mark in the last column or all bets are off. This can of course occur in
        // blobs by random chance, but they're currently unused in any database table.

        std::string strTemp{ strResults[j] };

        uint32_t uiNumQuotes{ (uint32_t)std::count( strTemp.begin(), strTemp.end(), '"' ) };
        while( uiNumQuotes % 2 != 0 && strTemp.at( strTemp.length() - 1 ) != '"' && j < strResults.size() )
        {
          // Advance to next token and append it
          strTemp += ",";
          strTemp += strResults[++j];
          uiNumQuotes += (uint32_t)std::count( strTemp.begin(), strTemp.end(), '"' );
        }

        // We should have at this point:
        // 1. An even number of quotation marks in the string
        // 2. A std::string beginning and ending with quotation marks
        // 3. A value of j <= number of elements in strResults

        // Copy the std::string without quotation marks
        strTemp.replace( 0, 1, "'" );
        strTemp.replace( strTemp.length() - 1, 1, "'" );

        strQuery += strTemp;
      }
      else
      {
        strQuery += "'";
        strQuery += strResults[j];
        strQuery += "'";
      }

      if( i + 1 < uiColumns )
      {
        strQuery += ",";
      }
    }

    strQuery += ");";
  }

  strQuery += "END;";

  cInfile.close();

  pcQuery = Query( m_eDatabaseID, strQuery );
  return ( pcQuery && !( pcQuery->IsError() ) );
}


// static
void rumDatabase::UpdateIDStore( rumIDStore& io_rcIDStore, bool i_bCreate )
{
  std::string strQuery{ "SELECT next_id FROM id_store WHERE table_id=" };
  strQuery += rumStringUtils::ToString( (int32_t)io_rcIDStore.m_eTableType );
  strQuery += " LIMIT 1";

  const QueryPtr pcQuery{ Query( DatabaseID::Player_DatabaseID, strQuery ) };
  if( pcQuery && !( pcQuery->IsError() ) )
  {
    if( i_bCreate )
    {
      io_rcIDStore.m_uiNextID = (uint64_t)pcQuery->FetchInt64( 0, 0 );
      io_rcIDStore.m_uiLastID = io_rcIDStore.m_uiNextID + s_uiIDStoreReserveInterval - 1;
    }
    else
    {
      uint64_t uiNextID{ (uint64_t)pcQuery->FetchInt64( 0, 0 ) };
      io_rcIDStore.m_uiLastID = uiNextID - 1;
    }

#pragma message("Should this be asynchronous?")
    // Update the database table to reflect the reservation - note that this update is not asynchronous!
    strQuery = "UPDATE id_store SET next_id=next_id+";
    strQuery += rumStringUtils::ToString( s_uiIDStoreReserveInterval );
    strQuery += " WHERE table_id=";
    strQuery += rumStringUtils::ToString( (int32_t)io_rcIDStore.m_eTableType );
    Query( DatabaseID::Player_DatabaseID, strQuery );
  }
}


// static
void rumAsyncDatabase::CheckpointAll()
{
  for( const auto& iter : s_cAsynchronousDatabaseMap )
  {
    sqlite3_wal_checkpoint( iter.second->m_pcHandle, nullptr );
  }
}


// static
bool rumAsyncDatabase::CreateAsyncConnection( DatabaseID i_eDatabaseID, const std::string& i_strFilename )
{
  // Failure here means Init was never called
  assert( s_bInitialized );

  // Make sure the file exists
  if( i_strFilename.empty() )
  {
    return false;
  }

  // Make sure the id doesn't already exist
  if( s_cAsynchronousDatabaseMap.find( i_eDatabaseID ) != s_cAsynchronousDatabaseMap.end() )
  {
    assert( false );
  }

  const std::filesystem::path fsPath( i_strFilename );
  const std::string strPath{ fsPath.string() };

  Logger::LogStandard( "Connecting to database (asynchronously) " + strPath );

  bool bConnected{ false };

  rumAsyncDatabase* pcAsyncDatabase{ new rumAsyncDatabase };
  assert( pcAsyncDatabase );
  if( pcAsyncDatabase )
  {
    bConnected = pcAsyncDatabase->Connect( strPath, i_eDatabaseID );
    if( bConnected )
    {
      sqlite3_wal_checkpoint( pcAsyncDatabase->m_pcHandle, nullptr );

      // Save connections to each Database
      s_cAsynchronousDatabaseMap.insert( std::make_pair( i_eDatabaseID, pcAsyncDatabase ) );
    }
  }

  return bConnected;
}


// static
void rumAsyncDatabase::DatabaseThread( const bool& i_rbShutdown )
{
  // Failure here means Init was never called
  assert( s_bInitialized );

  while( !i_rbShutdown )
  {
    if( !s_cStagingQueue.empty() )
    {
      std::lock_guard<std::mutex> cLockGuard( s_mtxQueue );

      // Bulk move everything from the staging queue to the execution queue
      do
      {
        const rumAsyncQuery* pcQuery{ s_cStagingQueue.front() };
        s_cStagingQueue.pop();
        const AsyncDatabaseContainer::iterator iter( s_cAsynchronousDatabaseMap.find( pcQuery->m_eDatabaseID ) );
        if( iter != s_cAsynchronousDatabaseMap.end() )
        {
          iter->second->s_cExecutionQueue.push( pcQuery->m_strQuery );
        }
      } while( !s_cStagingQueue.empty() );
    }

    for( const auto& iter : s_cAsynchronousDatabaseMap )
    {
      // Empty each queue
      rumAsyncDatabase* pcAsyncDatabase{ iter.second };

      if( !pcAsyncDatabase->s_cExecutionQueue.empty() )
      {
        std::string strQuery;

        {
          do
          {
            strQuery += pcAsyncDatabase->s_cExecutionQueue.front() + ";";
            pcAsyncDatabase->s_cExecutionQueue.pop();
          } while( !pcAsyncDatabase->s_cExecutionQueue.empty() );
        }

        // Commit all queries as a transaction
        pcAsyncDatabase->Query( strQuery );
      }
    }

    using namespace std::literals::chrono_literals;
    std::this_thread::sleep_for( 15ms );
  }
}


// static
rumAsyncDatabase* rumAsyncDatabase::GetAsynchronousDB( DatabaseID i_eDatabaseID )
{
  AsyncDatabaseContainer::iterator iter( s_cAsynchronousDatabaseMap.find( i_eDatabaseID ) );
  return ( iter != s_cAsynchronousDatabaseMap.end() ? iter->second : nullptr );
}


// static
void rumAsyncDatabase::QueryAsync( DatabaseID i_eDatabaseID, const std::string& i_strQuery )
{
  // Queue for asynchronous query
  std::lock_guard<std::mutex> cLockGuard( s_mtxQueue );
  s_cStagingQueue.push( new rumAsyncQuery( i_eDatabaseID, i_strQuery ) );
}


// static
void rumAsyncDatabase::ShutdownAsync()
{
  for( const auto& iter : s_cAsynchronousDatabaseMap )
  {
    // Disconnect each database connection
    iter.second->Close();

    assert( iter.second->s_cExecutionQueue.empty() );

    // Free memory
    delete iter.second;
  }

  s_cAsynchronousDatabaseMap.clear();
}


rumDatabaseQuery::~rumDatabaseQuery()
{
  m_pcResultVector.clear();

  if( m_strErrorMsg )
  {
    sqlite3_free( m_strErrorMsg );
  }
}


int32_t rumDatabaseQuery::Execute( sqlite3* i_pcHandle, const std::string& i_strQuery )
{
#if DATABASE_DEBUG
  RUM_COUT( DATABASE_DEBUG, "Executing query " << i_strQuery << '\n' );
  rumTimer cTimer;
#endif

  int32_t eResult{ sqlite3_exec( i_pcHandle, i_strQuery.c_str(), ExecuteCallback, this, &m_strErrorMsg ) };
  if( eResult != SQLITE_OK )
  {
    m_bError = true;
    Logger::LogStandard( m_strErrorMsg );
  }

#if DATABASE_DEBUG
  RUM_COUT( "Query execution time :" << cTimer.GetExecutionTime() << "s\n" );
#endif

  return eResult;
}


int32_t rumDatabaseQuery::ExecuteCallback( void* io_pcQuery, int32_t i_iArgc, char** i_strArgv, char** i_strColumnNames )
{
  if( io_pcQuery && i_iArgc > 0 )
  {
    ( (rumDatabaseQuery*)io_pcQuery )->UpdateQuery( i_iArgc, i_strArgv );
  }

  return 0;
}


const char* rumDatabaseQuery::Fetch( uint32_t i_uiRow, uint32_t i_uiCol ) const
{
  if( m_bError || ( 0 == m_iNumRows ) || ( (int32_t)i_uiRow >= m_iNumRows ) || ( (int32_t)i_uiCol >= m_iNumCols ) )
  {
    return (const char*)nullptr;
  }

  return m_pcResultVector.at(i_uiRow).at(i_uiCol).c_str();
}


bool rumDatabaseQuery::FetchBool( uint32_t i_uiRow, uint32_t i_uiCol ) const
{
  const int32_t iValue{ FetchInt( i_uiRow, i_uiCol ) };
  return iValue ? true : false;
}


float rumDatabaseQuery::FetchFloat( uint32_t i_uiRow, uint32_t i_uiCol ) const
{
  const char* strValue{ Fetch( i_uiRow, i_uiCol ) };
  return strValue ? (float)atof( strValue ) : 0.f;
}


int32_t rumDatabaseQuery::FetchInt( uint32_t i_uiRow, uint32_t i_uiCol ) const
{
  const char* strValue{ Fetch( i_uiRow, i_uiCol ) };
  return strValue ? strtoul( strValue, nullptr, 10 ) : 0;
}


int64_t rumDatabaseQuery::FetchInt64( uint32_t i_uiRow, uint32_t i_uiCol ) const
{
  const char* strValue{ Fetch( i_uiRow, i_uiCol ) };
  if( strValue )
  {
#ifdef WIN32
    return _atoi64( strValue );
#else
    return atoi64( strValue );
#endif
  }

  return 0;
}


// static
std::string rumDatabaseQuery::FormatProperty( const rumPropertyAsset* i_pcProperty, Sqrat::Object i_sqValue )
{
  if( !i_pcProperty )
  {
    return {};
  }

  std::string strValue;
  
  if( i_pcProperty->IsAssetRef() )
  {
    strValue = rumStringUtils::ToString64( i_sqValue.Cast<int64_t>() );
  }
  else
  {
    switch( i_pcProperty->GetValueType() )
    {
      case PropertyValueType::Bitfield:
      case PropertyValueType::Integer:
        strValue = rumStringUtils::ToString64( i_sqValue.Cast<int64_t>() );
        break;

      case PropertyValueType::Float:
        strValue = rumStringUtils::ToFloatString( i_sqValue.Cast<float>() );
        break;

      case PropertyValueType::Bool:
        strValue = i_sqValue.Cast<bool>() ? "1" : "0";
        break;

      case PropertyValueType::String:
        strValue = "'";
        strValue += i_sqValue.Cast<std::string>();
        strValue += "'";
        break;
    }
  }

  return strValue;
}


Sqrat::Object rumDatabaseQuery::GetValueFromProperty( uint32_t i_uiRow, uint32_t i_uiColumn,
                                                      const rumPropertyAsset* i_pcProperty ) const
{
  Sqrat::Object sqValue;

  rumAssert( i_pcProperty );
  if( !i_pcProperty )
  {
    return sqValue;
  }

  if( i_pcProperty->IsAssetRef() )
  {
    rumScript::SetValue( sqValue, FetchInt64( i_uiRow, i_uiColumn ) );
  }
  else
  {
    switch( i_pcProperty->GetValueType() )
    {
      case PropertyValueType::Bitfield:
      case PropertyValueType::Integer:
      case PropertyValueType::StringToken:
        rumScript::SetValue( sqValue, FetchInt64( i_uiRow, i_uiColumn ) );
        break;

      case PropertyValueType::Bool:
        rumScript::SetValue( sqValue, FetchBool( i_uiRow, i_uiColumn ) );
        break;

      case PropertyValueType::Float:
        rumScript::SetValue( sqValue, FetchFloat( i_uiRow, i_uiColumn ) );
        break;

      case PropertyValueType::String:
        rumScript::SetValue( sqValue, Fetch( i_uiRow, i_uiColumn ) );
        break;

      default:
        std::string strError{ "Error: GetValueFromType could not handle type " };
        strError += rumStringUtils::ToHexString( rumUtility::ToUnderlyingType( i_pcProperty->GetValueType() ) );
        Logger::LogStandard( strError, Logger::LOG_ERROR );
        break;
    }
  }

  return sqValue;
}


void rumDatabaseQuery::UpdateQuery( int32_t i_iArgc, char** i_strArgv )
{
  ++m_iNumRows;
  m_iNumCols = i_iArgc;

  // Build the new row
  std::vector<std::string> cRow;
  cRow.reserve( i_iArgc );
  for( int32_t i = 0; i < i_iArgc; ++i )
  {
    if( i_strArgv[i] != nullptr && *i_strArgv[i] != '\0' )
    {
      cRow.push_back( i_strArgv[i] );
    }
    else
    {
      cRow.push_back( {} );
    }
  }

  // Add the row
  m_pcResultVector.push_back( cRow );
}
