#pragma once

#include <u_script.h>
#include <u_utility.h>

#include <sqlite3.h>

#include <queue>
#include <mutex>
#include <set>
#include <unordered_map>
#include <vector>

class rumDatabaseQuery;
class rumPropertyAsset;

typedef std::shared_ptr<rumDatabaseQuery> QueryPtr;

// Sqlite datatypes are: Null, Integer, Real, Text, Blob


class rumDatabase
{
public:

  virtual ~rumDatabase()
  {
    Close();
  }

  enum DatabaseID
  {
    Invalid_DatabaseID = -1,
    Default_DatabaseID,
    Game_DatabaseID,
    Player_DatabaseID,
    Patch_DatabaseID,
    Strings_DatabaseID,
    Assets_DatabaseID,
    DataTable_DatabaseID
  };

  // ID Store tables
  enum IDStoreTableType
  {
    Invalid_IDStoreTableType   = 0x0,
    Account_IDStoreTableType   = 0x1,
    Player_IDStoreTableType    = 0x2,
    Inventory_IDStoreTableType = 0x3,
  };

  struct rumIDStore
  {
    uint64_t m_uiNextID{ 0 };
    uint64_t m_uiLastID{ 0 };
    IDStoreTableType m_eTableType{ IDStoreTableType::Invalid_IDStoreTableType };
  };

  DatabaseID GetDatabaseID() const
  {
    return m_eDatabaseID;
  }

  void InitTables();

  bool IsConnected() const
  {
    return m_bConnected;
  }

  bool HasTables();
  bool TableExists( const std::string& i_strTable );

  bool TableExport( const std::string& i_strTable, const std::string& i_strFilename ) const;
  bool TableImport( const std::string& i_strTable, const std::string& i_strFilename );

  static bool CreateConnection( DatabaseID i_eDatabaseID, const std::string& i_strFilename );
  static void CloseConnection( DatabaseID i_eDatabaseID );

  static void CheckpointAll();

  // Creates and initializes an ID Store for a given table
  static void CreateIDStore( IDStoreTableType i_eTable );

  static void DumpTableNames( DatabaseID i_eDatabaseID );

  static const std::string& GetFilename( DatabaseID i_eDatabaseID );

  // Gets the next available ID from an ID Store
  static uint64_t GetNextIDFromIDStore( IDStoreTableType i_eTable );

  static rumDatabase* GetSynchronousDB( DatabaseID i_eDatabaseID );

  static void GetTableNames( DatabaseID i_eDatabaseID, std::set<std::string>& o_hTables );
  static void GetTableColumnInfo( DatabaseID i_eDatabaseID, const std::string& i_strTable,
                                  std::vector<std::string>& o_vColumnNames, std::vector<std::string>& o_vColumnTypes );
  static void GetTableColumnNames( DatabaseID i_eDatabaseID, const std::string& i_strTable,
                                   std::vector<std::string>& o_vColumnNames );
  static void GetTableColumnTypes( DatabaseID i_eDatabaseID, const std::string& i_strTable,
                                   const std::vector<std::string>& o_vColumnNames,
                                   std::vector<std::string>& o_vColumnTypes );

  static int32_t Init();

  static QueryPtr Query( DatabaseID i_eDatabaseID, const std::string& i_strQuery );

  static void Shutdown();

protected:

  // Closes the database
  void Close();

  // Performs a connection attempt
  bool Connect( const std::string& i_strFilename, DatabaseID i_eDatabaseID );

  // Performs a query and returns a result class
  QueryPtr Query( const std::string& i_strQuery );

  void SetDatabaseID( DatabaseID i_eDatabaseID )
  {
    m_eDatabaseID = i_eDatabaseID;
  }

  // Reserves more IDs
  static void RefundIDStore();
  static void UpdateIDStore( rumIDStore& io_rcIDStore, bool i_bCreate );

private:

  const std::string& GetFilename() const
  {
    return m_strFilename;
  }

  // Returns handle
  const sqlite3* GetHandle() const
  {
    return m_pcHandle;
  }

  typedef std::unordered_map<DatabaseID, rumDatabase*> DatabaseContainer;
  static DatabaseContainer s_hashSynchronousDBs;

  typedef std::unordered_map<IDStoreTableType, rumIDStore> IDStoreTableContainer;
  static IDStoreTableContainer s_hashIDStore;

  // When IDs are fetched from the ID store, this is the number the database will be incremented by, effectively
  // reserving this amount of IDs prior to the next needed update
  static uint32_t s_uiIDStoreReserveInterval;

  // When the current ID falls within this threshold of the last ID, more IDs are fetched from the ID store
  static uint32_t s_uiIDStoreUpdateThreshold;

  DatabaseID m_eDatabaseID{ DatabaseID::Invalid_DatabaseID };

  std::string m_strFilename;
  bool m_bConnected{ false };

protected:

  // Whether or not the database system has been initialized
  static bool s_bInitialized;

  sqlite3* m_pcHandle{ nullptr };
};


class rumAsyncDatabase : public rumDatabase
{
  friend class rumDatabase;

public:

  ~rumAsyncDatabase()
  {
    rumAssert( s_cExecutionQueue.empty() );
  }

  static void CheckpointAll();

  static bool CreateAsyncConnection( DatabaseID i_eDatabaseID, const std::string& i_strFilename );

  static void DatabaseThread( const bool& i_rbShutdown );

  static void QueryAsync( DatabaseID i_eDatabaseID, const std::string& i_strQuery );

private:

  struct rumAsyncQuery
  {
    rumAsyncQuery( DatabaseID i_eDatabaseID, const std::string& i_strQuery )
      : m_eDatabaseID( i_eDatabaseID ), m_strQuery( i_strQuery )
    {}

    DatabaseID m_eDatabaseID;
    const std::string m_strQuery;

  private:

    rumAsyncQuery(); // No default construction allowed
  };

  static rumAsyncDatabase* GetAsynchronousDB( DatabaseID i_eDatabaseID );

  static void ShutdownAsync();

  typedef std::unordered_map<DatabaseID, rumAsyncDatabase*> AsyncDatabaseContainer;
  static AsyncDatabaseContainer s_cAsynchronousDatabaseMap;

  // Query queue used by the main thread
  typedef std::queue<rumAsyncQuery*> QueryStagingContainer;
  static QueryStagingContainer s_cStagingQueue;

  static std::mutex s_mtxQueue;

  // Queries moved to the database thread, ready for execution
  typedef std::queue<std::string> QueryExecutionContainer;
  QueryExecutionContainer s_cExecutionQueue;
};


// The DB result contains the result of the query. On a low level, it executes the query and does the cleanup for it,
// but should not be constructed except by the DB class.
class rumDatabaseQuery
{
public:

  friend class rumDatabase;

  ~rumDatabaseQuery();

  int32_t GetNumCols() const
  {
    return m_iNumCols;
  }

  int32_t GetNumRows() const
  {
    return m_iNumRows;
  }

  // Fetch a particular result element as a string. You should copy it if you want to keep it. Row/col starts at 0.
  const char* Fetch( uint32_t i_uiRow, uint32_t i_uiCol ) const;
  std::string FetchString( uint32_t i_uiRow, uint32_t i_uiCol ) const
  {
    const char* strResult{ Fetch( i_uiRow, i_uiCol ) };
    return strResult ? strResult : rumStringUtils::NullString();
  }

  // Process the result element as a bool
  bool FetchBool( uint32_t i_uiRow, uint32_t i_uiCol ) const;

  // Process the result element as a float
  float FetchFloat( uint32_t i_uiRow, uint32_t i_uiCol ) const;

  // Process the result element as an int
  int32_t FetchInt( uint32_t i_uiRow, uint32_t i_uiCol ) const;
  int64_t FetchInt64( uint32_t i_uiRow, uint32_t i_uiCol ) const;

  const char* GetErrorMsg() const
  {
    return m_strErrorMsg;
  }

  // Helper to return a script object with a value read from a provided type
  Sqrat::Object GetValueFromProperty( uint32_t i_uiRow, uint32_t i_uiColumn,
                                      const rumPropertyAsset* i_pcProperty ) const;

  // Was this an error?
  bool IsError() const
  {
    return m_strErrorMsg;
  }

  // Helper to format a script object as a string for writing to a database table
  static std::string FormatProperty( const rumPropertyAsset* i_pcProperty, Sqrat::Object i_sqValue );

private:

  int32_t Execute( sqlite3* i_pcHandle, const std::string& i_strQuery );

  // Called by the query callback to add new row results
  void UpdateQuery( int32_t i_iArgc, char** i_strArgv );

  // The query callback
  static int32_t ExecuteCallback( void* i_pcUserData, int32_t i_iArgc, char** i_strArgv, char** i_strColumnNames );

  // The matrix of row results from the query
  std::vector<std::vector<std::string>> m_pcResultVector;
  char* m_strErrorMsg{ nullptr };
  int32_t m_iNumCols{ 0 };
  int32_t m_iNumRows{ 0 };
  bool m_bError{ false };
};
