/*

R/U/M Construction Kit Server

MIT License

Copyright 2015 Jonathon Blake Wood-Brooks

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#define PROGRAM_SHORT_DESC  "RUM Server"
#define PROGRAM_LONG_DESC   "R/U/M Server"

#define DEFAULT_FPS         10
#define DEFAULT_CONFIG      "rum_server"
#define DEFAULT_GAME_CONFIG "server"

#define DEFAULT_CHAT_LOG    "chat.log"
#define DEFAULT_PLAYER_LOG  "player.log"

#define DEFAULT_LISTEN_PORT 58888

#define DEFAULT_PATCH_SERVER  "127.0.0.1"
#define DEFAULT_PATCH_PATH    ""
#define DEFAULT_PATCH_PORT    80

#define SERVICE_NAME "RUM Server"

#define NETIMGUI_PORT 8889U

#include <s_rum.h>

#include <filesystem>

#ifdef WIN32
#include <mmsystem.h>
#include <Ws2tcpip.h>
#else
#include <signal.h>
#endif

#include <string>
#include <fstream>
#include <iomanip>
#include <map>

#include <thread>

#include <s_resource.h>

#include <md5.h>
#include <iniparser.h>

#include <network/u_connection.h>
#include <network/u_patcher.h>

#include <u_asset.h>
#include <u_broadcast.h>
#include <u_custom.h>
#include <u_datatable.h>
#include <u_db.h>
#include <u_enum.h>
#include <u_log.h>
#include <u_resource.h>
#include <u_scheduler.h>
#include <u_strings.h>
#include <u_structs.h>
#include <u_timer.h>
#include <u_utility.h>
#include <u_zlib.h>

#include <s_account.h>
#include <s_inventory.h>
#include <s_map.h>
#include <s_pawn.h>
#include <s_player.h>

#include <sqstdblob.h>

#include <d_interface.h>
#include <d_vm.h>


struct rumConfig
{
  std::string m_strServerIni;

  std::string m_strServerLog;
  std::string m_strChatLog;
  std::string m_strPlayerLog;

  std::string m_strGamePath;
  std::string m_strGameTitle;
  std::string m_strGameGuid;

  std::string m_strPatchChecksum;
  std::string m_strPatchServer;
  std::string m_strPatchPath;

  int32_t m_iPatchPort{ 0 };

  int32_t m_iServerPort{ 0 };
  int32_t m_iServerFPS{ 0 };

  bool m_bService{ false };
  bool m_bPatch{ false };

#ifdef _DEBUG
  bool m_bScriptDebug{ false };
#endif // _DEBUG
};

uint64_t g_uiFrameIndex{ 0UL };

// Global variables
rumConfig g_cConfigStruct;

rumTimer g_cMainTimer;

bool g_bShutdown{ false };

// Function prototypes
void BindScripts();

int32_t ConfigInit( const std::string& i_strFile, bool i_bUseAlternate = true );
int32_t ConfigUpdate();

#ifdef WIN32
BOOL CtrlHandler( DWORD i_eCtrlType );
#else
void signalCallback( int32_t iSigNum );
#endif

int32_t GameInit();

bool InEditor();

int32_t InitDatabase();

void Log( const SQChar* i_strText );
void LogChat( const SQChar* i_strText );
int32_t LogInit( const std::string& i_strStandardLog = Logger::DEFAULT_LOG,
                 const std::string& i_strChatLog = DEFAULT_CHAT_LOG,
                 const std::string& i_strPlayerLog = DEFAULT_PLAYER_LOG );

int32_t MainGameLoop();

char* md5( const char* i_strText );

void NetworkThread( SOCKET i_iSocket, const bool& i_rbShutdown );

int32_t ParseArgs( int32_t i_iArgc, char* i_pcArgv[] );

int32_t ReadNetworkMessages();

void ResetDescriptors( SOCKET i_iListenSocket, fd_set* i_pcFdSet, rumNetwork::ConnectionHash& i_rcConnectionHash );

void ScriptDebuggerThread( const std::string& i_strPath, const bool& i_rbShutdown );

int32_t ScriptLoad( const std::string& i_strFilePath );
int32_t ScriptRequire( const std::string& i_strFilePath );

Sqrat::Object ScriptReadConfig();
void ScriptWriteConfig( Sqrat::Object i_sqTable );

#ifdef WIN32
void ServiceInstall();
void ServiceRun();
void ServiceSetStatus( DWORD i_iCurrentState, DWORD i_eWin32ExitCode = NO_ERROR, DWORD i_iWaitHint = 0 );
void ServiceUninstall();
#endif // WIN32

void SetWindowTitle( const char* i_strTitle = nullptr );

void ShowUsage();

void Shutdown();

#ifdef WIN32
// TODO - do these need any special cleanup on exit?
SERVICE_STATUS g_cServiceStatus;
SERVICE_STATUS_HANDLE g_hServiceStatusHandle{ 0 };
HANDLE g_hStopServiceEvent{ 0 };
#endif


int32_t main( int32_t i_iArgc, char* i_pcArgv[], char* i_pcEnvp[] )
{
  // If this assert is failing, it is time to bump the header size type in network packets from a byte to a word
  rumAssert( rumNetwork::NUM_PACKET_HEADERS < 256 );

  // -----------------------------------------------------------------------
  // NOTE: Any logging at this point will go to the default log file
  // -----------------------------------------------------------------------

  SetWindowTitle();

#ifdef WIN32
  // Early check to see if the server is running as a service
  for( int32_t i = 1; i < i_iArgc; ++i )
  {
    if( strcasecmp( "-service", i_pcArgv[i] ) == 0 )
    {
      // Windows services default to the System32 as the current working folder, so switch it
      const std::filesystem::path fsPath( i_pcArgv[0] );
      SetCurrentDirectory( fsPath.parent_path().string().c_str() );
    }
  }
#endif // WIN32

  // Load ini settings
  if( ConfigInit( DEFAULT_CONFIG DEFAULT_CONFIG_EXT ) != RESULT_SUCCESS )
  {
    return RESULT_FAILED;
  }

  // Handle command line early for usage requests
  if( i_iArgc > 1 )
  {
    // Parse command line arguments, store arguments in config struct
    if( ParseArgs( i_iArgc, i_pcArgv ) != RESULT_SUCCESS )
    {
      return RESULT_FAILED;
    }
  }

  // -----------------------------------------------------------------------
  // NOTE: Any logging before this point will go to the default log file
  // -----------------------------------------------------------------------
  // This should be done as early in the program as possible. Here, the log initialization is delayed until we at least
  // load possible log settings from the ini file or command line.
  if( LogInit( g_cConfigStruct.m_strServerLog,
               g_cConfigStruct.m_strChatLog,
               g_cConfigStruct.m_strPlayerLog ) != RESULT_SUCCESS )
  {
    return RESULT_FAILED;
  }

  // Check for valid fps
  if( g_cConfigStruct.m_iServerFPS <= 0 )
  {
    Logger::LogStandard( "Error: The server framerate must be set to a value greater than 0" );
    return RESULT_FAILED;
  }

  // Cache the project file structure
  rumResource::CreateFileCache( g_cConfigStruct.m_strGamePath );

  if( InitDatabase() != RESULT_SUCCESS )
  {
    Logger::LogStandard( "Error: Failed initializing one or more game databases", Logger::LOG_ERROR );
    return RESULT_FAILED;
  }

  if( GameInit() != RESULT_SUCCESS )
  {
    Logger::LogStandard( "Error: Failed game initialization", Logger::LOG_ERROR );
    return RESULT_FAILED;
  }

  if( g_cConfigStruct.m_bService )
  {
    // This will restart for the service thread
    rumDatabase::Shutdown();
  }

  Logger::LogStandard( "Updating config" );

  // Save config ini settings
  if( ConfigUpdate() != RESULT_SUCCESS )
  {
    Logger::LogStandard( "Warning: Failed to update initialization file" );
  }

  // Show the name of the game that is being hosted
  RUM_COUT( '\n' << LINE_BREAK << '\n' << "Initializing game: " << g_cConfigStruct.m_strGameTitle << '\n' <<
            LINE_BREAK << "\n\n" );

#ifdef WIN32
  SetConsoleCtrlHandler( (PHANDLER_ROUTINE)CtrlHandler, TRUE );
#else
  sigignore( SIGPIPE );
  signal( SIGTERM, signalCallback );
  signal( SIGINT, signalCallback );
  signal( SIGABRT, signalCallback );
#endif

  if( g_cConfigStruct.m_bService )
  {
    ServiceRun();
  }
  else
  {
    int32_t eResult{ MainGameLoop() };
    if( eResult != RESULT_SUCCESS )
    {
      return eResult;
    }
  }

  Logger::SetOutputColor( COLOR_STANDARD );
  Shutdown();
  Logger::LogStandard( "Server shutdown complete" );

  return RESULT_SUCCESS;
}


void BindScripts()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::Table sqTable( pcVM );
  Sqrat::RootTable( pcVM ).Bind( "SERVER", sqTable );

  Sqrat::RootTable( pcVM )
    .Func( "rumLoadScript", ScriptLoad )
    .Func( "rumLoadFolder", ScriptLoad )
    .Func( "rumRequireScript", ScriptRequire )
    .Func( "rumRequireFolder", ScriptRequire )
    .Func( "rumSetWindowTitle", SetWindowTitle )
    .Func( "rumReadConfig", ScriptReadConfig )
    .Func( "rumWriteConfig", ScriptWriteConfig )
    .Func( "rumInEditor", InEditor )
    .SquirrelFunc( "rumDebugAttachVM", &rumDebugVM::AttachVM )
    .SquirrelFunc( "rumDebugDetachVM", &rumDebugVM::DetachVM )
    .SquirrelFunc( "rumDebuggerAttached", &rumDebugVM::IsDebuggerAttached );

  rumLanguage::ScriptBind();
  rumStringTable::ScriptBind();
  rumDataTable::ScriptBind();
  Logger::ScriptBind();
  rumUtility::ScriptBind();
  rumTimer::ScriptBind();
  rumPropertyContainer::ScriptBind();
  rumAsset::ScriptBind();
  rumEnum::ScriptBind();
  rumStructs::ScriptBind();
  rumScheduler::ScriptBind();
  rumGameObject::ScriptBind();
  rumCustom::ScriptBind();
  rumServerMap::ScriptBind();
  rumResource::ScriptBind();
  rumServerPawn::ScriptBind();
  rumServerPlayer::ScriptBind();
  rumServerInventory::ScriptBind();
  rumBroadcast::ScriptBind();
}


void ClientServerInfo( const rumNetwork::rumInboundPacket& i_rcPacket )
{
  // Send patch info to the requesting client
  auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
  rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_SERVER_INFO )
    .Write( static_cast<rumByte>( g_cConfigStruct.m_bPatch ) )
    .Write( g_cConfigStruct.m_strPatchChecksum )
    .Write( g_cConfigStruct.m_strPatchServer )
    .Write( g_cConfigStruct.m_strPatchPath )
    .Write( static_cast<rumWord>( g_cConfigStruct.m_iPatchPort ) )
    .Send( i_rcPacket.GetSocket() );
}


int32_t ConfigInit( const std::string& i_strFile, bool i_bUseAlternate )
{
  // Check first the current working directory then the default etc directory

  // Note: Any logging at this point will go to the default log file

  int32_t eResult{ RESULT_SUCCESS };

  // Use boost for platform independent file path
  const std::filesystem::path fsPath( i_strFile );

  // Create config dictionary
  dictionary* pcDictionary{ iniparser_new( fsPath.string().c_str() ) };
  if( pcDictionary )
  {
    try
    {
      // Save the file used
      g_cConfigStruct.m_strServerIni = fsPath.string();
      RUM_COUT( "Parsing " << g_cConfigStruct.m_strServerIni << '\n' );

      g_cConfigStruct.m_strGamePath = iniparser_getstring( pcDictionary, "server:game", DEFAULT_GAME_PATH );
      g_cConfigStruct.m_iServerPort = iniparser_getint( pcDictionary, "server:port", DEFAULT_LISTEN_PORT );
      g_cConfigStruct.m_iServerFPS = iniparser_getint( pcDictionary, "server:fps", DEFAULT_FPS );
      g_cConfigStruct.m_strServerLog = iniparser_getstring( pcDictionary, "server:log",
                                                            const_cast<char*>( Logger::DEFAULT_LOG.c_str() ) );
      g_cConfigStruct.m_strChatLog = iniparser_getstring( pcDictionary, "server:chatlog", DEFAULT_CHAT_LOG );
      g_cConfigStruct.m_strPlayerLog = iniparser_getstring( pcDictionary, "server:playerlog", DEFAULT_PLAYER_LOG );

      int32_t bPatch{ iniparser_getboolean( pcDictionary, "patch:patch", false ) };
      g_cConfigStruct.m_bPatch = ( bPatch != 0 ) ? true : false;
      g_cConfigStruct.m_strPatchServer = iniparser_getstring( pcDictionary, "patch:server", DEFAULT_PATCH_SERVER );
      g_cConfigStruct.m_strPatchPath = iniparser_getstring( pcDictionary, "patch:path", DEFAULT_PATCH_PATH );
      g_cConfigStruct.m_iPatchPort = iniparser_getint( pcDictionary, "patch:port", DEFAULT_PATCH_PORT );
    } catch( ... )
    {
      RUM_COUT( "Error encountered while parsing " << g_cConfigStruct.m_strServerIni << '\n' );
      eResult = RESULT_FAILED;
    }

    iniparser_free( pcDictionary );
  }
  else if( i_bUseAlternate )
  {
    // Try ETC directory ...  (if applicable)
    if( DEFAULT_PATH_ETC != "" )
    {
      eResult = ConfigInit( DEFAULT_PATH_ETC DEFAULT_CONFIG DEFAULT_CONFIG_EXT, false );
    }
  }

  return eResult;
}


int32_t ConfigUpdate()
{
  int32_t eResult{ RESULT_FAILED };

  RUM_COUT( "Updating " << g_cConfigStruct.m_strServerIni << '\n' );

  // The ini filename should already be in the native format
  dictionary* pcDictionary{ iniparser_new( g_cConfigStruct.m_strServerIni.c_str() ) };
  if( pcDictionary )
  {
    try
    {
      iniparser_setstr( pcDictionary, "server:game", g_cConfigStruct.m_strGamePath.c_str() );
      iniparser_setstr( pcDictionary, "server:port", rumStringUtils::ToString( g_cConfigStruct.m_iServerPort ) );
      iniparser_setstr( pcDictionary, "server:fps", rumStringUtils::ToString( g_cConfigStruct.m_iServerFPS ) );
      iniparser_setstr( pcDictionary, "server:log", g_cConfigStruct.m_strServerLog.c_str() );
      iniparser_setstr( pcDictionary, "server:chatlog", g_cConfigStruct.m_strChatLog.c_str() );
      iniparser_setstr( pcDictionary, "server:playerlog", g_cConfigStruct.m_strPlayerLog.c_str() );

      iniparser_setstr( pcDictionary, "patch:server", g_cConfigStruct.m_strPatchServer.c_str() );
      iniparser_setstr( pcDictionary, "patch:path", g_cConfigStruct.m_strPatchPath.c_str() );
      iniparser_setstr( pcDictionary, "patch:port", rumStringUtils::ToString( g_cConfigStruct.m_iPatchPort ) );

      if( iniparser_dump_ini_file( pcDictionary, g_cConfigStruct.m_strServerIni.c_str() ) == 0 )
      {
        eResult = RESULT_SUCCESS;
      }
    } catch( ... )
    {
    }

    iniparser_free( pcDictionary );
  }

  if( RESULT_FAILED == eResult )
  {
    Logger::LogStandard( "Failed to update " + g_cConfigStruct.m_strServerIni );
  }

  return RESULT_SUCCESS;
}


#ifdef WIN32

BOOL CtrlHandler( DWORD i_eCtrlType )
{
  switch( i_eCtrlType )
  {
    case CTRL_C_EVENT:
      RUM_COUT( "Ctrl-C Pressed\n" );
      g_bShutdown = true;
      return TRUE;

    case CTRL_CLOSE_EVENT:
      RUM_COUT( "Application shutdown\n" );
      g_bShutdown = true;
      ExitThread( 0 );
      return TRUE;

    case CTRL_BREAK_EVENT:
      RUM_COUT( "Ctrl-Break Pressed\n" );
      return FALSE;

    case CTRL_LOGOFF_EVENT:
      RUM_COUT( "Ctrl-Logoff event\n" );
      return FALSE;

    case CTRL_SHUTDOWN_EVENT:
      RUM_COUT( "Ctrl-Shutdown event\n" );
      return FALSE;

    default:
      return FALSE;
  }
}

#else

void signalCallback( int32_t i_iSigNum )
{
  RUM_COUT( "Signal " << i_iSigNum << " received\n" );
  bShutdown = true;
}

#endif // WIN32


int32_t GameInit()
{
  // Store the game path
  const std::filesystem::path fsPath( g_cConfigStruct.m_strGamePath );

  // Store the full path including info file
  const std::filesystem::path fsInfoFile( fsPath / DEFAULT_GAME_FILE );

  // Sanity check the game db
  if( exists( fsInfoFile ) == false || is_regular_file( fsInfoFile ) == false )
  {
    std::string strError{ "The game file: " };
    strError += fsInfoFile.string();
    strError += " does not exist";
    Logger::LogStandard( strError, Logger::LOG_ERROR );
    return RESULT_FAILED;
  }

  // Store the path in the platform native version
  g_cConfigStruct.m_strGamePath = fsPath.string();

  bool dbError{ false };

  // Connect to the game database file (the .rum file)
  const QueryPtr pcQuery{ rumDatabase::Query( rumDatabase::DatabaseID::Game_DatabaseID, "SELECT key,value FROM settings" ) };
  if( pcQuery && !pcQuery->IsError() )
  {
    // Retrieve the game info
    enum game_info_types
    {
      COL_KEY, COL_VALUE
    };

    for( int32_t i = 0; i < pcQuery->GetNumRows(); ++i )
    {
      const std::string& strKey{ pcQuery->FetchString( i, COL_KEY ) };
      const std::string& strValue{ pcQuery->FetchString( i, COL_VALUE ) };

      if( strKey.compare( "title" ) == 0 )
      {
        g_cConfigStruct.m_strGameTitle = strValue;
      }
      else if( strKey.compare( "uuid" ) == 0 )
      {
        g_cConfigStruct.m_strGameGuid = strValue;
      }
    }
  }
  else
  {
    dbError = true;
  }

  // Exit if there is anything wrong with the game database
  if( dbError )
  {
    Logger::LogStandard( "Error: The game database failed to open", Logger::LOG_ERROR );
    return RESULT_FAILED;
  }
  else if( g_cConfigStruct.m_strGameTitle.empty() || g_cConfigStruct.m_strGameGuid.empty() )
  {
    Logger::LogStandard( "Error: The game database is uninitialized. Please run the editor to correct the problem",
                         Logger::LOG_ERROR );
    return RESULT_FAILED;
  }

  return RESULT_SUCCESS;
}


double GetElapsedTime()
{
  return g_cMainTimer.GetElapsedSeconds();
}


uint64_t GetFrameIndex()
{
  return g_uiFrameIndex;
}


const std::string& GetProjectPath()
{
  return( g_cConfigStruct.m_strGamePath );
}


bool InEditor()
{
  return false;
}


int32_t InitDatabase()
{
  if( rumDatabase::Init() != RESULT_SUCCESS )
  {
    return RESULT_FAILED;
  }

  std::filesystem::path cPath( g_cConfigStruct.m_strGamePath );

  using DatabaseID = rumDatabase::DatabaseID;

  struct EditorDatabases
  {
    std::string m_strFilename;
    bool m_bAsync{ false };
    DatabaseID m_eDatabaseID{ DatabaseID::Invalid_DatabaseID };
  };

  std::vector<EditorDatabases> vDatabases
  {
    { "game.rum",      false, DatabaseID::Game_DatabaseID },
    { "assets.db",     false, DatabaseID::Assets_DatabaseID },
    { "datatables.db", false, DatabaseID::DataTable_DatabaseID },
    { "player.db",     true,  DatabaseID::Player_DatabaseID },
    { "strings.db",    false, DatabaseID::Strings_DatabaseID },
  };

  // Load each database
  for( const auto& iter : vDatabases )
  {
    const std::filesystem::path cDBPath( cPath / iter.m_strFilename );
    if( !rumDatabase::CreateConnection( iter.m_eDatabaseID, cDBPath.string() ) )
    {
      std::string strError{ "Failed to connect to database " };
      strError += cDBPath.string();
      Logger::LogStandard( strError );
      rumAssertMsg( false, strError );
      return RESULT_FAILED;
    }

    if( iter.m_bAsync )
    {
      if( !rumAsyncDatabase::CreateAsyncConnection( iter.m_eDatabaseID, cDBPath.string() ) )
      {
        std::string strError{ "Failed to connect to database " };
        strError += cDBPath.string();
        Logger::LogStandard( strError );
        rumAssertMsg( false, strError );
        return RESULT_FAILED;
      }
    }
  }

  const std::filesystem::path fsAssetsPath( cPath / "assets.db" );
  const std::filesystem::path fsPlayerPath( cPath / "player.db" );

  // Attach the assets db to the account db
  std::string strQuery{ "ATTACH DATABASE '" };
  strQuery += fsAssetsPath.string();
  strQuery += "' AS assets";
  QueryPtr pcQuery{ rumDatabase::Query( rumDatabase::DatabaseID::Player_DatabaseID, strQuery ) };
  if( !pcQuery || pcQuery->GetErrorMsg() )
  {
    std::string strError{ "Failed to attach database " };
    strError += fsAssetsPath.string();
    strError += " to ";
    strError += fsPlayerPath.string();
    Logger::LogStandard( strError );
    rumAssertMsg( false, strError );
    return RESULT_FAILED;
  }

  // Reset all accounts to inactive in case of previous crash
  strQuery = "UPDATE player SET status=0 WHERE status>0";
  pcQuery = rumDatabase::Query( rumDatabase::DatabaseID::Player_DatabaseID, strQuery );
  if( !pcQuery || pcQuery->GetErrorMsg() )
  {
    std::string strError{ "Failed to reset status on all players" };
    Logger::LogStandard( strError );
    rumAssertMsg( false, strError );
    return RESULT_FAILED;
  }

  rumDatabase::CreateIDStore( rumDatabase::IDStoreTableType::Account_IDStoreTableType );
  rumDatabase::CreateIDStore( rumDatabase::IDStoreTableType::Player_IDStoreTableType );
  rumDatabase::CreateIDStore( rumDatabase::IDStoreTableType::Inventory_IDStoreTableType );

  // Determine the patch database checksum
  const std::filesystem::path fsPatchPath( cPath / rumPatcher::GetPatchDatabaseName() );

  std::ifstream cInfile;
  cInfile.open( fsPatchPath.c_str(), std::ios::binary );
  if( cInfile.is_open() )
  {
    MD5 cMD5( cInfile );
    const char* strHash{ cMD5.hex_digest() };

    g_cConfigStruct.m_strPatchChecksum = strHash;
    RUM_COUT( "Patch database checksum: " << strHash << '\n' );

    cInfile.close();
  }

  return RESULT_SUCCESS;
}


int32_t LogInit( const std::string& i_strStandardLog, const std::string& i_strChatLog,
                 const std::string& i_strPlayerLog )
{
  Logger::Archive( i_strStandardLog );
  Logger::Archive( i_strChatLog );
  Logger::Archive( i_strPlayerLog );

  const std::filesystem::path fsLog( i_strStandardLog );
  Logger::strStandardLog = fsLog.string();

  const std::filesystem::path fsChatLog( i_strChatLog );
  Logger::strChatLog = fsChatLog.string();

  const std::filesystem::path fsPlayerLog( i_strPlayerLog );
  Logger::strPlayerLog = fsPlayerLog.string();

  RUM_COUT( "Using standard log: " << i_strStandardLog << '\n' );
  RUM_COUT( "Using chat log: " << i_strChatLog << '\n' );
  RUM_COUT( "Using player log: " << i_strPlayerLog << '\n' );

  // Save echo status
  const bool bEcho{ Logger::IsEchoEnabled() };
  Logger::EnableEcho( false );

  // Test the log location and specify that the mod is starting
  int32_t eResult{ Logger::LogStandard( PROGRAM_LONG_DESC " version " QUOTED( PROGRAM_VERSION ) ) };
  if( RESULT_FAILED == eResult )
  {
    RUM_COUT( "Warning: Log file " << Logger::strStandardLog << " is invalid.\n" );

    if( i_strStandardLog.compare( Logger::DEFAULT_LOG ) != 0 )
    {
      // Switch back to the default log file
      eResult = LogInit( Logger::DEFAULT_LOG, i_strChatLog, i_strPlayerLog );
    }
    else
    {
      // Disable logging
      RUM_COUT( "Warning: Logging will not be available for this run\n" );
      Logger::EnableEcho( false );
    }
  }

  Logger::LogStandard( LINE_BREAK );

  // Restore echo status
  Logger::EnableEcho( bEcho );

  return eResult;
}


int32_t MainGameLoop()
{
  if( g_cConfigStruct.m_bService )
  {
    if( InitDatabase() != RESULT_SUCCESS )
    {
      Logger::LogStandard( "Failed to initialize databases. Terminating program." );
      return RESULT_FAILED;
    }
  }

  std::filesystem::path fsScriptPath( std::filesystem::path( GetProjectPath() ) / rumScript::GetFolderName() );
  if( rumScript::Init( rumScript::VM_SERVER, fsScriptPath.generic_string() ) != RESULT_SUCCESS )
  {
    Logger::LogStandard( "A fatal error occurred during virtual machine initialization. Terminating program." );
    return RESULT_FAILED;
  }

  // #TODO - change this to Load
  if( rumAsset::Init( g_cConfigStruct.m_strGamePath ) != RESULT_SUCCESS )
  {
    Logger::LogStandard( "Failed to initialize assets. Terminating program." );
    return RESULT_FAILED;
  }

  if( rumStringTable::Init( g_cConfigStruct.m_strGamePath, rumStringUtils::NullString() ) != RESULT_SUCCESS )
  {
    Logger::LogStandard( "Failed to initialize string tables. Terminating program." );
    return RESULT_FAILED;
  }

  if( rumDataTable::Init( g_cConfigStruct.m_strGamePath ) != RESULT_SUCCESS )
  {
    Logger::LogStandard( "Failed to initialize data tables. Terminating program." );
    return RESULT_FAILED;
  }

  BindScripts();

#ifdef _DEBUG
  std::thread cScriptDebuggerThread;

  if( g_cConfigStruct.m_bScriptDebug )
  {
    // Attach the VM to the debugger and create the debugger ImGui thread
    RUM_COUT( "Connecting to script debugger, port " << NETIMGUI_PORT << '\n' );
    const std::filesystem::path fsPath( std::filesystem::path( GetProjectPath() ) / rumScript::GetFolderName() );
    cScriptDebuggerThread = std::thread( &ScriptDebuggerThread, fsPath.generic_string(), std::ref( g_bShutdown ) );

    auto pcVM{ Sqrat::DefaultVM::Get() };
    rumDebugVM::EnableDebugInfo( pcVM );
    rumDebugVM::RegisterVM( pcVM, "rumServer" );
  }
#endif // _DEBUG

  // Compile and execute scripts
  if( rumScript::ExecuteStartupScript( rumScript::VM_SERVER ) != RESULT_SUCCESS )
  {
    Logger::LogStandard( "A fatal error occurred during virtual machine execution. Terminating program." );
    return RESULT_FAILED;
  }

  rumAsset::RegisterClasses();

  rumServerPawn::Init();
  rumServerInventory::Init();
  rumServerMap::Init();

  if( rumServerPlayer::Init() != RESULT_SUCCESS )
  {
    Logger::LogStandard( "Failed to initialize players. Terminating program." );
    return RESULT_FAILED;
  }

  rumCustom::Init();
  rumBroadcast::Init();

  if( !rumScript::CallInitFunction( "OnGameInit" ) )
  {
    Logger::LogStandard( "A fatal error occurred during virtual machine initialization. Terminating program." );
    return RESULT_FAILED;
  }

  constexpr uint32_t uiInboundPacketSize{ 100 };
  constexpr uint32_t uiOutboundPacketSize{ 300 };
  if( rumNetwork::Init( uiInboundPacketSize, uiOutboundPacketSize ) != RESULT_SUCCESS )
  {
    Logger::LogStandard( "Failed to initialize network. Terminating program." );
    rumNetwork::Shutdown();
    return RESULT_FAILED;
  }

  const SOCKET iSocket{ rumNetwork::HostTCP( g_cConfigStruct.m_iServerPort ) };

  std::thread cDatabaseThread( rumAsyncDatabase::DatabaseThread, std::ref( g_bShutdown ) );
  std::thread cNetworkHandlerThread( NetworkThread, iSocket, std::ref( g_bShutdown ) );

  // Show program banner and description
  RUM_COUT( RUM_BANNER << '\n' << PROGRAM_LONG_DESC << " version " << QUOTED( PROGRAM_MAJOR_VERSION ) << '.' <<
            QUOTED( PROGRAM_MINOR_VERSION ) << "\n\n" );

  int32_t iFrameCounter{ 0 };

  rumTimer cFPSDisplayTimer;
  rumTimer cFrameTimer;
  rumTimer cIdlePlayerTimer;
  rumTimer cStatDisplayTimer;
  rumTimer cSystemsTimer;
  rumTimer cGarbageCollectionTimer;

  constexpr double fDisplayStatInterval{ 15.0 };
  constexpr double fIdlePlayerInterval{ 5.0 };

  const double fDesiredFrameTime{ 1.0 / g_cConfigStruct.m_iServerFPS };
  double fOversleepTime{ 0.0 };
  double fSleepTime{ 0.0 };

  double fPerfTotal{ 0.0 };
  double fPerfScheduler{ 0.0 };
  double fPerfNetwork{ 0.0 };
  double fPerfPlayerCreation{ 0.0 };
  double fPerfScriptTick{ 0.0 };
  double fPerfMapTick{ 0.0 };
  uint32_t iIntervalFrames{ 0 };

  Logger::SetOutputColor( COLOR_SERVER );
#ifdef WIN32
  // Determine and set the shortest sleep granularity
  TIMECAPS cTimeCaps;
  timeGetDevCaps( &cTimeCaps, sizeof( TIMECAPS ) );
  RUM_COUT( "Shortest sleep granularity: " << cTimeCaps.wPeriodMin << "ms\n" );
  rumAssert( timeBeginPeriod( cTimeCaps.wPeriodMin ) == TIMERR_NOERROR );
#else
  // Set OS specific sleep granularity here
#endif

  RUM_COUT( "Frame info displayed every: " << fDisplayStatInterval << "s\n" );
  Logger::SetOutputColor( COLOR_STANDARD );

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  while( !g_bShutdown )
  {
    cFrameTimer.Restart();
    cSystemsTimer.Restart();

#ifdef _DEBUG
    rumDebugVM::Update();
    cSystemsTimer.Restart();
#endif // _DEBUG

    rumScheduler::Run();
    fPerfScheduler += cSystemsTimer.GetElapsedSeconds();
    cSystemsTimer.Restart();

    ReadNetworkMessages();
    fPerfNetwork += cSystemsTimer.GetElapsedSeconds();
    cSystemsTimer.Restart();

    // Check pending character creations
    rumServerPlayer::CheckPlayerCreations();
    fPerfPlayerCreation += cSystemsTimer.GetElapsedSeconds();
    cSystemsTimer.Restart();

    // Check pending player logouts
    rumServerPlayer::CheckPlayerLogouts();
    cSystemsTimer.Restart();

    if( cIdlePlayerTimer.GetElapsedSeconds() >= fIdlePlayerInterval )
    {
      rumServerPlayer::CheckIdlePlayers();
      cIdlePlayerTimer.Restart();
    }
    cSystemsTimer.Restart();

    // Check pending account creations
    rumServerAccount::ProcessPendingAccounts();
    cSystemsTimer.Restart();

    // Run the server script update function
    rumScript::ExecOptionalFunc( Sqrat::RootTable( pcVM ), "OnFrameStart", g_cMainTimer.GetElapsedSeconds() );
    fPerfScriptTick += cSystemsTimer.GetElapsedSeconds();
    cSystemsTimer.Restart();

    rumServerPawn::Update();
    cSystemsTimer.Restart();

    // Call this last in the game loop to relinquish any checked out outbound packets that have been sent
    rumNetwork::rumOutboundPacketPool::Update();
    fPerfNetwork += cSystemsTimer.GetElapsedSeconds();

    rumScript::ExecOptionalFunc( Sqrat::RootTable( pcVM ), "OnFrameEnd", g_cMainTimer.GetElapsedSeconds() );

    ++iFrameCounter;
    ++iIntervalFrames;

    // Report fps the server is achieving
    if( cFPSDisplayTimer.GetElapsedSeconds() >= 1.0 )
    {
      // Display and reset the frame statistics
      cFPSDisplayTimer.Restart();

      SetWindowTitle( rumStringUtils::ToString( iFrameCounter ) );

      iFrameCounter = 0;
    }

    const double fElapsedTime{ cStatDisplayTimer.GetElapsedSeconds() };
    const double fWorkThisFrame{ cFrameTimer.GetElapsedSeconds() };
    fPerfTotal += fWorkThisFrame;
    if( fElapsedTime >= fDisplayStatInterval )
    {
      // Display and reset the frame statistics
      cStatDisplayTimer.Restart();

      Logger::SetOutputColor( COLOR_SERVER );

      RUM_COUT( '\n' << setiosflags( ios::fixed ) << setprecision( 1 ) << "FPS: " << iIntervalFrames / fElapsedTime <<
                '/' << g_cConfigStruct.m_iServerFPS );

      const double fTotalWorkTime{ fPerfTotal / fElapsedTime };
      RUM_COUT( setprecision( 3 ) << " | Busy: " << fTotalWorkTime << "s " << setprecision( 1 ) <<
                fTotalWorkTime / fElapsedTime * 100 << '%' );

      const double fIdleTime{ fElapsedTime - fTotalWorkTime };
      RUM_COUT( setprecision( 3 ) << " | Idle: " << fIdleTime << "s " << setprecision( 1 ) <<
                fIdleTime / fElapsedTime * 100 << "%\n" );

      const double fSchedulerTime{ fPerfScheduler / fElapsedTime };
      RUM_COUT( setprecision( 3 ) << "Sched: " << fSchedulerTime << "s " << setprecision( 1 ) <<
                fSchedulerTime / fElapsedTime * 100 << '%' );

      const double fNetworkTime{ fPerfNetwork / fElapsedTime };
      RUM_COUT( setprecision( 3 ) << " | Network: " << fNetworkTime << "s " << setprecision( 1 ) <<
                fNetworkTime / fElapsedTime * 100 << '%' );

      /*const double fChargenTime{ fPerfPlayerCreation / fElapsedTime };
      RUM_COUT( setprecision(3) << " | Chargen: " << fChargenTime << ' ' << setprecision(1) <<
                fChargenTime / fElapsedTime * 100 << '%' );*/

      const double fMapTickTime{ fPerfMapTick / fElapsedTime };
      RUM_COUT( setprecision( 3 ) << " | MapTick: " << fMapTickTime << "s " << setprecision( 1 ) <<
                fMapTickTime / fElapsedTime * 100 << "%\n" );

      RUM_COUT( "Active Players: " << rumPlayer::GetNumPlayers() );

      const auto& ciMapHash{ rumMap::GetMaps() };
      RUM_COUT( " | Loaded Maps: " << ciMapHash.size() << '\n' );

      Logger::SetOutputColor( COLOR_STANDARD );

      iIntervalFrames = 0;
      fPerfTotal = 0.0;
      fPerfScheduler = 0.0;
      fPerfNetwork = 0.0;
      fPerfPlayerCreation = 0.0;
      fPerfScriptTick = 0.0;
      fPerfMapTick = 0.0;
    }

    // Determine if the frame has some leftover time
    fSleepTime = fDesiredFrameTime - cFrameTimer.GetElapsedSeconds() - fOversleepTime;
    if( fSleepTime > 0.0 )
    {
      if( cGarbageCollectionTimer.GetElapsedSeconds() > 10.0 )
      {
        sq_collectgarbage( pcVM );
        cGarbageCollectionTimer.Restart();
      }

      fSleepTime = fDesiredFrameTime - cFrameTimer.GetElapsedSeconds() - fOversleepTime;
      if( fSleepTime > 0.0 )
      {
        try
        {
          std::this_thread::sleep_for( std::chrono::duration<double>( fSleepTime ) );
        }
        catch( ... )
        {
          RUM_COUT( "Error encountered during this_thread::sleep_for " << fSleepTime << "s\n" );
        }
      }

      // Calculate overage cause by this system's sleep granularity
      fOversleepTime = cFrameTimer.GetElapsedSeconds() - fSleepTime;
    }
    else
    {
      if( cGarbageCollectionTimer.GetElapsedSeconds() > 30.0 )
      {
        sq_collectgarbage( pcVM );
        cGarbageCollectionTimer.Restart();
      }

      fOversleepTime = 0.0;
    }

    ++g_uiFrameIndex;
  }

#ifdef WIN32
  rumAssert( timeEndPeriod( cTimeCaps.wPeriodMin ) == TIMERR_NOERROR );
#endif

  // Join threads
#ifdef _DEBUG
  rumDebugVM::DetachVM( Sqrat::DefaultVM::Get() );

  if( cScriptDebuggerThread.joinable() )
  {
    cScriptDebuggerThread.join();
  }
#endif // _DEBUG

  if( cNetworkHandlerThread.joinable() )
  {
    cNetworkHandlerThread.join();
  }

  if( cDatabaseThread.joinable() )
  {
    cDatabaseThread.join();
  }

#ifdef WIN32
  if( g_cConfigStruct.m_bService )
  {
    SetEvent( g_hStopServiceEvent );
  }
#endif // WIN32

  return RESULT_SUCCESS;
}


char* md5( const char* i_strText )
{
  if( i_strText )
  {
    MD5 context;
    context.update( (unsigned char*)i_strText, (uint32_t)strlen( i_strText ) );
    context.finalize();
    return context.hex_digest();
  }

  return nullptr;
}


// Threaded
void NetworkThread( SOCKET i_iListenSocket, const bool& i_rbShutdown )
{
  rumAssert( i_iListenSocket != INVALID_SOCKET );

  sockaddr_in cClientAddr;
  socklen_t iAddrSize{ sizeof( cClientAddr ) };

  fd_set cFdSet[rumNetwork::NUM_FD_SETS];

  timeval cTimeVal;
  cTimeVal.tv_sec = 0;
  cTimeVal.tv_usec = 0;

  //Create list of connect sockets and add the listenSocket to the list
  rumNetwork::ConnectionHash cConnectionHash;
  rumNetwork::rumConnection* pcConnection{ nullptr };

  while( !i_rbShutdown )
  {
    ResetDescriptors( i_iListenSocket, cFdSet, cConnectionHash );

    // Handle the recv and send packet queues
    rumNetwork::UpdatePackets( cConnectionHash );

    // Determine what sockets are ready to send or recv
    if( select( 0, &cFdSet[rumNetwork::FD_SET_READ], &cFdSet[rumNetwork::FD_SET_WRITE],
                &cFdSet[rumNetwork::FD_SET_EXCEPT], &cTimeVal ) > 0 )
    {
      // See if the client listening socket received data
      if( FD_ISSET( i_iListenSocket, &cFdSet[rumNetwork::FD_SET_READ] ) != 0 )
      {
        // Accept the connection request and create a new connection
        const SOCKET iClientSocket{ accept( i_iListenSocket, (PSOCKADDR)&cClientAddr, &iAddrSize ) };
        if( iClientSocket != INVALID_SOCKET )
        {
          char strIP[INET_ADDRSTRLEN];

          // Tell user we accepted the socket, and add it to our connecition list
          std::string strInfo{ "Accepted connection from " };
          strInfo += inet_ntop( cClientAddr.sin_family, &( cClientAddr.sin_addr ), strIP, sizeof( strIP ) );
          strInfo += ":";
          strInfo += rumStringUtils::ToString( ntohs( cClientAddr.sin_port ) );
          strInfo += ", socket ";
          strInfo += rumStringUtils::ToString64( iClientSocket );
          Logger::LogStandard( strInfo );

          // Add the client connection
          pcConnection = new rumNetwork::rumConnection( iClientSocket );
          if( pcConnection )
          {
            cConnectionHash.insert( make_pair( iClientSocket, pcConnection ) );
            /*if ((cConnectionHash.size() + 1) > 64)
            {
                RUM_COUT( "WARNING: More than 63 client connections accepted. This will not work reliably on some "
                          "Winsock stacks\n" );
            }*/

            // debug
            RUM_COUT( "Clients connected: " << cConnectionHash.size() << '\n' );

            // Mark the socket as non-blocking, for safety
            u_long ulNoBlock{ 1 };
            ioctlsocket( iClientSocket, FIONBIO, &ulNoBlock );
          }
        }
        else
        {
          RUM_COUT( "Error while accepting incoming connection\n" );
        }
      }

      // See if any client sockets received data
      rumNetwork::ConnectionHash::iterator iter( cConnectionHash.begin() );
      const rumNetwork::ConnectionHash::iterator end( cConnectionHash.end() );
      while( iter != end )
      {
        bool bDisconnect{ false };

        SOCKET iSocket{ iter->first };
        pcConnection = iter->second;

        // See if this socket's flag is set in any of the FD sets
        if( FD_ISSET( iSocket, &cFdSet[rumNetwork::FD_SET_EXCEPT] ) )
        {
          bDisconnect = true;
          FD_CLR( iSocket, &cFdSet[rumNetwork::FD_SET_EXCEPT] );
        }
        else
        {
          // Are there any sockets waiting to recv?
          if( FD_ISSET( iSocket, &cFdSet[rumNetwork::FD_SET_READ] ) )
          {
            // Perform a winsock recv
            if( pcConnection->ReceivePackets() == SOCKET_ERROR )
            {
              bDisconnect = true;
            }

            FD_CLR( iSocket, &cFdSet[rumNetwork::FD_SET_READ] );
          }

          // Are there any sockets waiting to send?
          if( FD_ISSET( iSocket, &cFdSet[rumNetwork::FD_SET_WRITE] ) )
          {
            // Perform a winsock send
            if( pcConnection->SendPackets() == SOCKET_ERROR )
            {
              bDisconnect = true;
            }

            FD_CLR( iSocket, &cFdSet[rumNetwork::FD_SET_WRITE] );
          }
        }

        // Handle winsock errors and disconnects
        if( bDisconnect )
        {
          // Something bad happened on the socket, or the client closed its half of the connection. Shut the connection
          // down and remove it from the list.
          int32_t iError;
          int32_t iErrorLength{ sizeof( iError ) };
          getsockopt( iSocket, SOL_SOCKET, SO_ERROR, (char*)&iError, &iErrorLength );
          if( WSAEWOULDBLOCK != iError )
          {
            RUM_COUT( "Closing client connection " << iSocket << '\n' );

            pcConnection->CloseSocket();
            delete pcConnection;
            iter = cConnectionHash.erase( iter );

            RUM_COUT( "Clients connected: " << cConnectionHash.size() << '\n' );
          }
          else
          {
            // Go on to next connection
            ++iter;
          }
        }
        else
        {
          // Go on to next connection
          ++iter;
        }
      }
    }

    std::this_thread::yield();
  }
}


int32_t ParseArgs( int32_t i_argc, char* i_argv[] )
{
  // The config struct should already contain values loaded from ini file.
  // Arguments passed via the command line should override the stored value.

  bool bShowUsage{ false };
  int32_t eResult{ RESULT_SUCCESS };

  for( int32_t i = 1; bShowUsage == false && i < i_argc; ++i )
  {
    if( strcasecmp( "-game", i_argv[i] ) == 0 )
    {
      g_cConfigStruct.m_strGamePath = i_argv[++i];
    }
    else if( strcasecmp( "-port", i_argv[i] ) == 0 )
    {
      g_cConfigStruct.m_iServerPort = atoi( i_argv[++i] );
    }
    else if( strcasecmp( "-fps", i_argv[i] ) == 0 )
    {
      g_cConfigStruct.m_iServerFPS = atoi( i_argv[++i] );
    }
    else if( strcasecmp( "-log", i_argv[i] ) == 0 )
    {
      g_cConfigStruct.m_strServerLog = i_argv[++i];
    }
#ifdef _DEBUG
    else if( strcasecmp( "-debug", i_argv[i] ) == 0 )
    {
      g_cConfigStruct.m_bScriptDebug = true;
    }
#endif // _DEBUG
#ifdef WIN32
    else if( strcasecmp( "-install", i_argv[i] ) == 0 )
    {
      ServiceInstall();

      // Not an actual failure, but we don't want to continue execution
      eResult = RESULT_FAILED;
    }
    else if( strcasecmp( "-uninstall", i_argv[i] ) == 0 )
    {
      ServiceUninstall();

      // Not an actual failure, but we don't want to continue execution
      eResult = RESULT_FAILED;
    }
#endif // WIN32
    else if( strcasecmp( "-service", i_argv[i] ) == 0 )
    {
      g_cConfigStruct.m_bService = true;
    }
    else if( strcmp( "-?", i_argv[i] ) == 0 ||
             strcasecmp( "-h", i_argv[i] ) == 0 ||
             strcasecmp( "-help", i_argv[i] ) == 0 )
    {
      bShowUsage = true;
    }
    else
    {
      RUM_COUT( "Unknown parameter: " << i_argv[i] << '\n' );
      bShowUsage = true;
    }
  }

  if( bShowUsage )
  {
    ShowUsage();
    eResult = RESULT_FAILED;
  }

  return eResult;
}


int32_t ReadNetworkMessages()
{
  rumNetwork::PacketQueue qPackets;

  // Dequeue all pending outgoing packets (causes mutex lock)
  rumNetwork::DequeuePackets( rumNetwork::PACKET_QUEUE_RECV, qPackets );

#if NETWORK_DEBUG
  if( packets.size() > 0 )
  {
    RUM_COUT_IFDEF( NETWORK_DEBUG, "Reading " << packets.size() << " packet(s)\n" );
  }
#endif // NETWORK_DEBUG

  // Send packets
  while( !qPackets.empty() )
  {
    auto pcPacket{ qPackets.front() };
    rumNetwork::rumInboundPacket* pcPacketTemp{ (rumNetwork::rumInboundPacket*)pcPacket };
    auto& cPacket{ *pcPacketTemp };
    qPackets.pop();

    // Update the associated player's keep-alive timer
    auto* pcPlayer{ rumServerPlayer::FetchBySocket( cPacket.GetSocket() ) };
    if( pcPlayer )
    {
      pcPlayer->KeepAlive();
    }

    const PACKET_HEADER_TYPE eHeaderType{ static_cast<PACKET_HEADER_TYPE>( cPacket.GetHeaderType() ) };
    if( rumNetwork::PACKET_HEADER_CLIENT_REQ_SERVER_INFO == eHeaderType )
    {
      ClientServerInfo( cPacket );
    }
    else if( rumNetwork::PACKET_HEADER_CLIENT_ACCOUNT_LOGIN == eHeaderType )
    {
      std::string strAccountName;
      std::string strPassword;

      cPacket << strAccountName << strPassword;

      rumServerAccount::AccountLogin( strAccountName, strPassword, cPacket.GetSocket() );
    }
    else if( rumNetwork::PACKET_HEADER_CLIENT_ACCOUNT_LOGOUT == eHeaderType )
    {
      rumServerAccount::AccountLogout( cPacket.GetSocket() );
    }
    else if( rumNetwork::PACKET_HEADER_CONNECTION_TERMINATED == eHeaderType )
    {
      rumServerAccount::AccountLogout( cPacket.GetSocket() );
    }
    else if( rumNetwork::PACKET_HEADER_CLIENT_ACCOUNT_CREATE == eHeaderType )
    {
      std::string strAccountName;
      std::string strEmail;
      std::string strPassword;

      cPacket << strAccountName << strEmail << strPassword;

      rumServerAccount::CreateAccount( strAccountName, strEmail, strPassword, cPacket.GetSocket() );
    }
    else if( rumNetwork::PACKET_HEADER_CLIENT_PLAYER_CREATE == eHeaderType )
    {
      std::string strPlayerName;
      cPacket << strPlayerName;

      rumServerPlayer::ProcessPlayerCreateRequest( cPacket.GetSocket(), strPlayerName );
    }
    else if( rumNetwork::PACKET_HEADER_CLIENT_PLAYER_LOGIN == eHeaderType )
    {
      std::string strPlayerName;
      cPacket << strPlayerName;

      rumServerPlayer::ProcessPlayerLoginRequest( cPacket.GetSocket(), strPlayerName, /* i_bRestoreDB */ true );
    }
    else if( rumNetwork::PACKET_HEADER_CLIENT_PLAYER_LOGOUT == eHeaderType )
    {
      rumServerPlayer::ProcessPlayerLogoutRequest( cPacket.GetSocket() );
    }
    else if( rumNetwork::PACKET_HEADER_CLIENT_PLAYER_DELETE == eHeaderType )
    {
      std::string strPlayerName;
      cPacket << strPlayerName;

      rumServerPlayer::ProcessPlayerDeleteRequest( cPacket.GetSocket(), strPlayerName );
    }
    else if( rumNetwork::PACKET_HEADER_SCRIPT_DEFINED == eHeaderType )
    {
      Sqrat::Object sqInstance{ cPacket.ScriptRead() };
      rumAssert( sqInstance.GetType() == OT_INSTANCE );

#if NETWORK_DEBUG
      rumBroadcast* pcBroadcast{ sqInstance.Cast<rumBroadcast*>() };
      rumAssert( pcBroadcast );
      RUM_COUT( "Received PACKET_HEADER_SCRIPT_DEFINED, broadcast: " << pcBroadcast->GetName() << '\n' );
#endif // NETWORK_DEBUG

      rumServerPlayer::ProcessBroadcast( cPacket.GetSocket(), sqInstance );
    }
    else
    {
#ifdef _DEBUG
      std::string strError{ "Error: Unhandled packet type " };
      strError += rumStringUtils::ToString( (int32_t)eHeaderType );
      Logger::LogStandard( strError, Logger::LOG_ERROR );
      rumAssert( false );
#endif
    }

    // TODO - is this necessary?
    //delete pcPacket;
  }

  return RESULT_SUCCESS;
}


// Threaded
void ResetDescriptors( SOCKET i_iListenSocket, fd_set* i_pcFdSet, rumNetwork::ConnectionHash& i_rcConnectionHash )
{
  rumAssert( i_pcFdSet );

  FD_ZERO( &i_pcFdSet[rumNetwork::FD_SET_READ] );
  FD_ZERO( &i_pcFdSet[rumNetwork::FD_SET_WRITE] );
  FD_ZERO( &i_pcFdSet[rumNetwork::FD_SET_EXCEPT] );

  // Add the listener socket to the read and except FD sets
  FD_SET( i_iListenSocket, &i_pcFdSet[rumNetwork::FD_SET_READ] );
  FD_SET( i_iListenSocket, &i_pcFdSet[rumNetwork::FD_SET_EXCEPT] );

  SOCKET iSocket{ INVALID_SOCKET };
  rumNetwork::rumConnection* pcConnection{ nullptr };

  // Add client connections
  for( const auto& iter : i_rcConnectionHash )
  {
    iSocket = iter.first;
    pcConnection = iter.second;

    FD_SET( iSocket, &i_pcFdSet[rumNetwork::FD_SET_EXCEPT] );

    if( pcConnection->CanReceivePackets() )
    {
      // There's space in the read buffer, so pay attention to incoming data
      FD_SET( iSocket, &i_pcFdSet[rumNetwork::FD_SET_READ] );
    }

    if( pcConnection->HasQueuedPackets( rumNetwork::PACKET_QUEUE_SEND ) )
    {
      // The send buffer contains data, so allow the send
      FD_SET( iSocket, &i_pcFdSet[rumNetwork::FD_SET_WRITE] );
    }
  }
}


void ScriptDebuggerThread( const std::string& i_strPath, const bool& i_rbShutdown )
{
  rumDebugInterface::Init( DEFAULT_CONFIG, NETIMGUI_PORT, i_strPath );

  while( !i_rbShutdown )
  {
    rumDebugInterface::Update();
  }

  rumDebugInterface::Shutdown();
}


int32_t ScriptLoad( const std::string& i_strFilePath )
{
#ifdef _DEBUG
  return rumScript::Load( i_strFilePath );
#else
  return RESULT_FAILED;
#endif
}


Sqrat::Object ScriptReadConfig()
{
  const std::filesystem::path fsPath
  {
    std::filesystem::path( g_cConfigStruct.m_strGamePath ) / DEFAULT_GAME_CONFIG DEFAULT_CONFIG_EXT
  };
  return rumScript::ReadConfig( fsPath.string() );
}


int32_t ScriptRequire( const std::string& i_strFilePath )
{
#ifdef _DEBUG
  return rumScript::Require( i_strFilePath );
#else
  return RESULT_FAILED;
#endif
}


void ScriptWriteConfig( Sqrat::Object sqTable )
{
  const std::filesystem::path fsPath
  {
    std::filesystem::path( g_cConfigStruct.m_strGamePath ) / DEFAULT_GAME_CONFIG DEFAULT_CONFIG_EXT
  };
  rumScript::WriteConfig( fsPath.string(), sqTable );
}


#ifdef WIN32

void WINAPI ServiceControlHandler( DWORD i_iControlCode )
{
  switch( i_iControlCode )
  {
    case SERVICE_CONTROL_STOP:
      ServiceSetStatus( SERVICE_STOP_PENDING );

      g_bShutdown = true;

      if( WaitForSingleObject( g_hStopServiceEvent, INFINITE ) != WAIT_OBJECT_0 )
      {
        throw GetLastError();
      }

      ServiceSetStatus( SERVICE_STOPPED );
      return;

    case SERVICE_CONTROL_SHUTDOWN:
      ServiceSetStatus( SERVICE_STOPPED );
      break;

    case SERVICE_CONTROL_PAUSE:
      ServiceSetStatus( SERVICE_PAUSE_PENDING );
      ServiceSetStatus( SERVICE_PAUSED );
      break;

    case SERVICE_CONTROL_CONTINUE:
      ServiceSetStatus( SERVICE_CONTINUE_PENDING );
      ServiceSetStatus( SERVICE_RUNNING );
      break;

    case SERVICE_CONTROL_INTERROGATE:
      break;
  }
}


void ServiceInstall()
{
  RUM_COUT( "Installing Windows service\n" );

  // Open the local default service control manager database
  const SC_HANDLE hManager{ OpenSCManager( nullptr, nullptr, SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE ) };
  if( hManager )
  {
    char strPath[MAX_PATH];
    if( GetModuleFileName( NULL, strPath, sizeof( char ) * MAX_PATH ) > 0 )
    {
      // Append the "-service" arg to the service executable
      char strPathParams[MAX_PATH + 16];
      sprintf( strPathParams, "%s -service", &strPath );

      // Install the service into SCM by calling CreateService
      const SC_HANDLE hService{ CreateService( hManager, SERVICE_NAME, SERVICE_NAME, SERVICE_QUERY_STATUS,
                                               SERVICE_WIN32_OWN_PROCESS, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
                                               strPathParams, nullptr, NULL, nullptr, nullptr, nullptr ) };
      if( hService )
      {
        RUM_COUT( "Successfully installed Windows service " << SERVICE_NAME << '\n' );
        CloseServiceHandle( hService );
      }
      else
      {
        std::string strError{ "InstallService failed CreateService(), error " };
        strError += rumStringUtils::ToString( GetLastError() );
        Logger::LogStandard( strError );
      }
    }
    else
    {
      std::string strError{ "InstallService failed GetModuleFileName(), error " };
      strError += rumStringUtils::ToString( GetLastError() );
      Logger::LogStandard( strError );
    }

    CloseServiceHandle( hManager );
  }
  else
  {
    std::string strError{ "InstallService failed OpenSCManager(), error " };
    strError += rumStringUtils::ToString( GetLastError() );
    Logger::LogStandard( strError );
  }
}


void WINAPI ServiceMain( DWORD i_iArgc, LPSTR* i_pcArgv )
{
  g_cServiceStatus.dwServiceType = SERVICE_WIN32;
  g_cServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;

  // Register the handler function for the service
  g_hServiceStatusHandle = RegisterServiceCtrlHandler( SERVICE_NAME, ServiceControlHandler );
  if( g_hServiceStatusHandle )
  {
    try
    {
      ServiceSetStatus( SERVICE_START_PENDING );

      Sleep( 1000 );

      // TODO - is this thread handle leaked?
      // The main game loop running on a separate thread
      std::thread cNetworkThread( MainGameLoop );

      Sleep( 1000 );

      ServiceSetStatus( SERVICE_RUNNING );

      Sleep( 1000 );
    } catch( ... )
    {
      Logger::LogStandard( "Exception when starting service", Logger::LOG_ERROR );
    }

    g_hStopServiceEvent = CreateEvent( 0, FALSE, FALSE, 0 );
  }
}


void ServiceRun()
{
  g_hStopServiceEvent = CreateEvent( nullptr, TRUE, FALSE, nullptr );

  const SERVICE_TABLE_ENTRY aServiceTable[] =
  {
      { (char*)SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
      { NULL, NULL }
  };

  // Connects the main thread of a service process to the service control manager, which causes the thread to be the
  // service control dispatcher thread for the calling process. This call returns when the service has stopped. The
  // process should simply terminate when the call returns.
  if( !StartServiceCtrlDispatcher( aServiceTable ) )
  {
    std::string strError{ "Failed to start service " };
    strError += SERVICE_NAME;
    strError += ", error ";
    strError += rumStringUtils::ToString( GetLastError() );
    Logger::LogStandard( strError, Logger::LOG_ERROR, true );
  }
}


void ServiceSetStatus( DWORD i_iCurrentState, DWORD i_eWin32ExitCode, DWORD i_iWaitHint )
{
  static DWORD iCheckPoint{ 1 };

  // Fill in the SERVICE_STATUS structure of the service
  g_cServiceStatus.dwCurrentState = i_iCurrentState;
  g_cServiceStatus.dwWin32ExitCode = i_eWin32ExitCode;
  g_cServiceStatus.dwServiceSpecificExitCode = 0;
  g_cServiceStatus.dwWaitHint = i_iWaitHint;

  const bool bRunningOrStopped{ ( SERVICE_RUNNING == i_eWin32ExitCode ) || ( SERVICE_STOPPED == i_iCurrentState ) };
  g_cServiceStatus.dwCheckPoint = bRunningOrStopped ? 0 : ++iCheckPoint;

  // Report the status of the service to the SCM
  ::SetServiceStatus( g_hServiceStatusHandle, &g_cServiceStatus );
}


void ServiceUninstall()
{
  RUM_COUT( "Uninstalling Windows service\n" );

  const SC_HANDLE hManager{ OpenSCManager( nullptr, nullptr, SC_MANAGER_CONNECT ) };
  if( hManager )
  {
    // Open the service with delete, stop, and query status permissions
    const SC_HANDLE hService{ OpenService( hManager, SERVICE_NAME, SERVICE_STOP | SERVICE_QUERY_STATUS | DELETE ) };
    if( hService )
    {
      SERVICE_STATUS cServiceStatus;

      // Try to stop the service
      if( ControlService( hService, SERVICE_CONTROL_STOP, &cServiceStatus ) )
      {
        RUM_COUT( "Stopping service " << SERVICE_NAME << '\n' );

        while( QueryServiceStatus( hService, &cServiceStatus ) &&
               SERVICE_STOP_PENDING != cServiceStatus.dwCurrentState )
        {
          Sleep( 1000 );
        }

        if( cServiceStatus.dwCurrentState == SERVICE_STOPPED )
        {
          RUM_COUT( SERVICE_NAME << " service is stopped\n" );
        }
        else
        {
          RUM_COUT( SERVICE_NAME << " service failed to stop\n" );
        }
      }

      // Now remove the service by
      if( DeleteService( hService ) )
      {
        RUM_COUT( "Successfully uninstalled Windows service " << SERVICE_NAME << '\n' );
      }
      else
      {
        std::string strError{ "UninstallService failed DeleteService(), error " };
        strError += rumStringUtils::ToString( GetLastError() );
        Logger::LogStandard( strError );
      }

      CloseServiceHandle( hService );
    }
    else
    {
      std::string strError{ "UninstallService failed OpenService(), error " };
      strError += rumStringUtils::ToString( GetLastError() );
      Logger::LogStandard( strError );
    }

    CloseServiceHandle( hManager );
  }
  else
  {
    std::string strError{ "UninstallService failed OpenSCManager(), error " };
    strError += rumStringUtils::ToString( GetLastError() );
    Logger::LogStandard( strError );
  }
}

#endif // WIN32


void SetWindowTitle( const char* i_strTitle )
{
  std::string strTitle{ PROGRAM_SHORT_DESC };

  // Append program version
  strTitle += " v";
  strTitle += QUOTED( PROGRAM_MAJOR_VERSION );
  strTitle += ".";
  strTitle += QUOTED( PROGRAM_MINOR_VERSION );

#ifdef _DEBUG
  strTitle += " DEBUG";
#endif

  // Append passed in description
  if( i_strTitle )
  {
    strTitle += " ";
    strTitle += i_strTitle;
  }

#ifdef WIN32
  SetConsoleTitle( strTitle.c_str() );
#else
  printf( "\033]2;%s\007", strTitle.c_str() );
#endif
}


void ShowUsage()
{
  RUM_COUT( '\n' << "Launches " << PROGRAM_LONG_DESC << "\n\n" );

#ifdef WIN32
  RUM_COUT( "RUM_SERVER [-?|-h|-help] | [-game path][-port number][-fps number][-log file]\n" );
  RUM_COUT( '\n' );
  RUM_COUT( "-?, -h, -help\n" );
  RUM_COUT( "   Displays program execution requirements and command-line parameter help\n" );
  RUM_COUT( '\n' );
  RUM_COUT( "-game path\n" );
  RUM_COUT( "   Specifies location of game to serves\n" );
  RUM_COUT( "   Default is " << DEFAULT_GAME_PATH << '\n' );
  RUM_COUT( '\n' );
  RUM_COUT( "-port number\n" );
  RUM_COUT( "   Specifies the network port for client connections\n" );
  RUM_COUT( "   Default is " << DEFAULT_LISTEN_PORT << '\n' );
  RUM_COUT( '\n' );
  RUM_COUT( "-fps number\n" );
  RUM_COUT( "   Specifies the number of frames the server should process per second\n" );
  RUM_COUT( "   Default is " << DEFAULT_FPS << '\n' );
  RUM_COUT( '\n' );
  RUM_COUT( "-log file\n" );
  RUM_COUT( "   Specifies output file the server will use for logging events and errors\n" );
  RUM_COUT( "   Default is " << Logger::DEFAULT_LOG << '\n' );
  RUM_COUT( "-install\n" );
  RUM_COUT( "   Installs the server as a Windows service\n" );
  RUM_COUT( "-uninstall\n" );
  RUM_COUT( "   Uninstalls the server Windows service entry if it exists\n" );
  RUM_COUT( "-service\n" );
  RUM_COUT( "   Runs the server as a Windows service (run -install to install the service first)\n" );

  // add service stuff

  RUM_COUT( '\n' );
#else
#error UNHANDLED PLATFORM
#endif
}


void Shutdown()
{
  RUM_COUT( "Main thread is terminating\n" );

  rumScript::CallShutdownFunction();

  rumServerPlayer::CheckPlayerLogouts();

  rumScheduler::Shutdown();
  rumNetwork::Shutdown();

  rumServerMap::Shutdown();
  rumCustom::Shutdown();
  rumServerPlayer::Shutdown();
  rumPawn::Shutdown();
  rumInventory::Shutdown();
  rumBroadcast::Shutdown();
  rumGameObject::Shutdown();

  rumAsset::Shutdown();

  RUM_COUT( "Terminating mod ...\n" );
  rumScript::Shutdown();

  rumDatabase::Shutdown();
}
