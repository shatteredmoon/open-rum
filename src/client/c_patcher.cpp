#include <c_patcher.h>

#include <u_db.h>
#include <u_events.h>
#include <u_log.h>

#include <filesystem>
#include <fstream>

#include <md5.h>


#ifdef WIN32
  #define PLATFORM_ID 1
#else
  #error DEFINE YOUR PLATFORM ID HERE
#endif

// Static initializers
std::vector<size_t> rumClientPatcher::s_uiEventHandles;
std::mutex rumClientPatcher::s_cMutex;


void rumClientPatcher::CheckPatchDatabase()
{
  m_uiNumFilesToPatch = 0;

  std::set<std::string> cConditionalFilesSet;

  // Patch database location
  const std::filesystem::path fsPath( m_strUUID );

  const std::filesystem::path fsPatchDatabase( fsPath / GetPatchDatabaseName() );
  if( !rumDatabase::CreateConnection( rumDatabase::DatabaseID::Patch_DatabaseID, fsPatchDatabase.string() ) )
  {
    std::string strError{ "Failed to create connection to database " };
    strError += fsPatchDatabase.string();
    Logger::LogStandard( strError, Logger::LOG_ERROR );
  }

  // Build the set of editable files from the patch database
  std::string strQuery{ "SELECT filepath FROM conditional" };

  //strQuery += "WHERE platform = '" };
  //strQuery += rumStringUtils::ToString( PLATFORM_ID );
  //strQuery += "'";

  QueryPtr pcQuery{ rumDatabase::Query( rumDatabase::DatabaseID::Patch_DatabaseID, strQuery ) };
  if( pcQuery && !pcQuery->IsError() )
  {
    enum { DB_FILEPATH, DB_CRC, DB_SIZE };

    for( int32_t i = 0; i < pcQuery->GetNumRows(); ++i )
    {
      cConditionalFilesSet.insert( pcQuery->FetchString( i, DB_FILEPATH ) );
    }
  }

  // Build the queue of patch files from the patch database
  strQuery = "SELECT filepath,crc,size FROM standard";

  //strQuery += WHERE platform = '";
  //strQuery += rumStringUtils::ToString( PLATFORM_ID );
  //strQuery += "'";

  pcQuery = rumDatabase::Query( rumDatabase::DatabaseID::Patch_DatabaseID, strQuery );
  if( pcQuery && !pcQuery->IsError() )
  {
    enum { DB_FILEPATH, DB_CRC, DB_SIZE };

    const std::set<std::string>::iterator end{ cConditionalFilesSet.end() };

    std::vector<rumNetwork::rumDownloadInfo> cFilesVector;
    cFilesVector.reserve( pcQuery->GetNumRows() );

    for( int32_t i = 0; i < pcQuery->GetNumRows(); i++ )
    {
      const std::string strFilePath{ pcQuery->FetchString( i, DB_FILEPATH ) };
      const std::string strFileCRC{ pcQuery->FetchString( i, DB_CRC ) };
      const uint64_t uiSize{ static_cast<uint64_t>( pcQuery->FetchInt64( i, DB_SIZE ) ) };

      const std::filesystem::path fsClientPath( std::filesystem::path( m_strUUID ) / strFilePath );
      const std::filesystem::path fsServerPath( std::filesystem::path( m_strServerPath ) / strFilePath );

      // If the file is conditional, don't overwrite the client-side modification
      const std::set<std::string>::iterator iter{ cConditionalFilesSet.find( strFilePath ) };

      bool bExists{ std::filesystem::exists( fsClientPath ) };
      bool bEditable{ end != iter };
      if( !( bEditable && bExists ) )
      {
        // Okay to download and overwrite the local file if needed; check CRC to see if the file needs to be downloaded
        if( !rumFileUtils::FileExists( fsClientPath.string(), strFileCRC ) )
        {
          rumNetwork::rumDownloadInfo cDownloadInfo;
          cDownloadInfo.m_strServer = m_strServerAddress;
          cDownloadInfo.m_iPort = m_iServerPort;
          cDownloadInfo.m_strClientPath = fsClientPath.generic_string();
          cDownloadInfo.m_strServerPath = fsServerPath.generic_string();
          cDownloadInfo.m_uiExpectedBytes = uiSize;

          cFilesVector.emplace_back( std::move( cDownloadInfo ) );
        }
      }
    }

    // Request download for any files that need updating
    if( !cFilesVector.empty() )
    {
      m_uiNumFilesToPatch = static_cast<uint32_t>( cFilesVector.size() );
      m_eStatus = Status::Patching;

      std::string strInfo{ rumStringUtils::ToString( m_uiNumFilesToPatch ) };
      strInfo += " file(s) are missing or need to be updated.";
      Logger::LogStandard( strInfo );

      rumDatabase::Shutdown();

      rumNetwork::EnqueueFiles( cFilesVector );
    }
    else
    {
      m_eStatus = Status::Finished;
    }
  }
  else
  {
    m_eStatus = Status::Failed;
  }

  // TODO - Remove files specified in the 'remove' folder

  rumDatabase::CloseConnection( rumDatabase::DatabaseID::Patch_DatabaseID );
}


// static
rumClientPatcher* rumClientPatcher::GetInstance()
{
  static rumClientPatcher* pcInstance{ new rumClientPatcher };
  return pcInstance;
}


// static
void rumClientPatcher::Init( const std::string& i_strFilePath )
{
  s_uiEventHandles.push_back( rumEvent::s_cFileDownloadedEvent.connect( OnFileDownloaded ) );

  auto* pcInstance{ GetInstance() };
  if( pcInstance )
  {
    pcInstance->m_strUUID = i_strFilePath;
  }

  super::Init( i_strFilePath );
}


// static
void rumClientPatcher::NotifyScripts( const rumNetwork::rumDownloadInfo& i_rcDownloadInfo )
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  const uint32_t uiPercent{
    i_rcDownloadInfo.m_uiExpectedBytes
      ? static_cast<uint32_t>( i_rcDownloadInfo.m_uiDownloadedBytes * 100 / i_rcDownloadInfo.m_uiExpectedBytes )
      : 0 };

  rumScript::ExecOptionalFunc( Sqrat::RootTable( pcVM ), "OnPatchFile", i_rcDownloadInfo.m_strClientPath.c_str(),
                               uiPercent );
}


// static
void rumClientPatcher::OnFileDownloaded( const rumNetwork::rumDownloadInfo& i_rcDownloadInfo )
{
  auto* pcInstance{ GetInstance() };
  if( pcInstance )
  {
    std::lock_guard<std::mutex> cLockGuard( s_cMutex );
    pcInstance->m_uiFileUpdatesQueue.push( i_rcDownloadInfo );
  }
}


void rumClientPatcher::Reset()
{
  m_uiNumFilesToPatch = 0;
  m_eStatus = Status::Idle;
}


void rumClientPatcher::SetConnectionInfo( bool bPatchRequired,
                                          const std::string& i_strPatchChecksum,
                                          const std::string& i_strAddress,
                                          const std::string& i_strPath,
                                          int32_t i_iPort )
{
  std::string strInfo{ "Server info received:\nPatch server: " };
  strInfo += i_strAddress;
  if( !i_strPath.empty() )
  {
    strInfo += i_strPath;
  }
  strInfo += ":";
  strInfo += rumStringUtils::ToString( i_iPort );
  Logger::LogStandard( strInfo, Logger::LOG_INFO, true );

  m_bPatchRequired = bPatchRequired;
  m_strPatchChecksum = i_strPatchChecksum;
  m_strServerAddress = i_strAddress;
  m_strServerPath = i_strPath;
  m_iServerPort = i_iPort;
}


// static
void rumClientPatcher::Shutdown()
{
  for( const auto uiHandle : s_uiEventHandles )
  {
    rumEvent::s_cFileDownloadedEvent.disconnect( uiHandle );
  }

  rumPatcher::Shutdown();

  auto* pcInstance{ GetInstance() };
  if( pcInstance )
  {
    SAFE_DELETE( pcInstance );
  }
}


void rumClientPatcher::StartPatch()
{
  if( !m_bPatchRequired )
  {
    m_eStatus = Status::NotApplicable;
    rumDatabase::CloseConnection( rumDatabase::DatabaseID::Patch_DatabaseID );
    return;
  }

  if( m_eStatus != Status::Idle )
  {
    // Patching already underway
    return;
  }

  m_eStatus = Status::Starting;

  // See if the patch database needs to be updated
  const std::filesystem::path fsPath( m_strUUID );
  const std::filesystem::path fsPatchDatabase( fsPath / GetPatchDatabaseName() );

  std::ifstream cInfile;
  cInfile.open( fsPatchDatabase.c_str(), std::ios::binary );
  if( cInfile.is_open() )
  {
    MD5 cMD5( cInfile );
    std::string strHash{ cMD5.hex_digest() };

    RUM_COUT( "Expected Patch database checksum: " << m_strPatchChecksum << '\n' );
    RUM_COUT( "Actual database checksum: " << strHash << '\n' );

    if( strHash.compare( m_strPatchChecksum ) != 0 )
    {
      const std::filesystem::path fsClientPath( std::filesystem::path( m_strUUID ) / GetPatchDatabaseName() );
      const std::filesystem::path fsServerPath( std::filesystem::path( m_strServerPath ) / GetPatchDatabaseName() );

      rumNetwork::rumDownloadInfo cDownloadInfo;
      cDownloadInfo.m_strServer = m_strServerAddress;
      cDownloadInfo.m_iPort = m_iServerPort;
      cDownloadInfo.m_strClientPath = fsClientPath.generic_string();
      cDownloadInfo.m_strServerPath = fsServerPath.generic_string();

      rumNetwork::EnqueueFile( std::move( cDownloadInfo ) );
    }
    else
    {
      CheckPatchDatabase();
    }

    cInfile.close();
  }
}


void rumClientPatcher::Update()
{
  if( !m_uiFileUpdatesQueue.empty() )
  {
    rumNetwork::rumDownloadInfo cDownloadInfo;

    {
      std::lock_guard<std::mutex> cLockGuard( s_cMutex );

      cDownloadInfo = m_uiFileUpdatesQueue.front();

      m_uiFileUpdatesQueue.pop();
    }

    if( cDownloadInfo.m_bFailed )
    {
      m_eStatus = Status::Failed;
    }
    else if( Status::Starting == m_eStatus )
    {
      if( cDownloadInfo.m_bComplete )
      {
        CheckPatchDatabase();
      }

      NotifyScripts( cDownloadInfo );
    }
    else if( Status::Patching == m_eStatus )
    {
      if( cDownloadInfo.m_bComplete )
      {
        --m_uiNumFilesToPatch;
        if( 0 == m_uiNumFilesToPatch )
        {
          m_eStatus = Status::Finished;
        }
      }

      NotifyScripts( cDownloadInfo );
    }
  }

  if( Status::Finished == m_eStatus )
  {
    HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };
    rumScript::ExecRequiredFunc( Sqrat::RootTable( pcVM ), "OnPatchComplete", true /* restart */ );
    Reset();
  }
  else if( Status::NotApplicable == m_eStatus )
  {
    HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };
    rumScript::ExecRequiredFunc( Sqrat::RootTable( pcVM ), "OnPatchComplete", false /* don't restart */ );
    Reset();
  }
#ifdef _DEBUG
  else if( Status::Failed == m_eStatus )
  {
    HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };
    rumScript::ExecRequiredFunc( Sqrat::RootTable( pcVM ), "OnPatchComplete", false /* don't restart */ );
    Reset();
  }
#endif // DEBUG
}
