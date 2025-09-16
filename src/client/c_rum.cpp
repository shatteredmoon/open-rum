/*

R/U/M Construction Kit Client

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

#define PROGRAM_SHORT_DESC  "RUM Client"
#define PROGRAM_LONG_DESC   "R/U/M Client"

#define DEFAULT_CONFIG      "rum_client"
#define DEFAULT_GAME_CONFIG "game"

#define NETIMGUI_PORT 8890U

#include <filesystem>

#include <c_account.h>
#include <c_font.h>
#include <c_inventory.h>
#include <c_map.h>
#include <c_patcher.h>
#include <c_pawn.h>
#include <c_player.h>
#include <c_resource.h>
#include <c_rum.h>
#include <controls/c_control.h>

#ifdef USE_FMOD
#include <c_sound_fmod.h>
#endif // USE_FMOD

#ifdef USE_GLFW
#include <c_graphics_glfw.h>
#include <c_input_glfw.h>
#elif USE_SDL
#include <c_graphics_sdl.h>
#include <c_input_sdl.h>
#else
#error You must include a graphic and input core
#endif

#include <network/u_connection.h>
#include <u_asset.h>
#include <u_broadcast.h>
#include <u_custom.h>
#include <u_datatable.h>
#include <u_db.h>
#include <u_enum.h>
#include <u_log.h>
#include <u_property_container.h>
#include <u_resource.h>
#include <u_scheduler.h>
#include <u_strings.h>
#include <u_timer.h>
#include <u_utility.h>
#include <u_zlib.h>

#include <d_interface.h>
#include <d_vm.h>

#include <fstream>
#include <queue>
#include <thread>

#include <md5.h>
#include <iniparser.h>


uint64_t g_uiFrameIndex{ 0UL };

rumConfig g_cConfigStruct;
bool g_bRestart{ false };

rumTimer g_cMainTimer;

std::thread g_threadNetworkHandler;

void BindScripts();

int32_t ConfigInit( const std::string& i_strFile, bool i_bUseAlternate = true );
int32_t ConfigUpdate();

#ifdef WIN32
BOOL ConsoleCtrlHandler( DWORD i_eCtrlType );
#endif

int32_t InitDatabase();

void Log( const SQChar* i_strText );
int32_t LogInit( const std::string& i_strLog = Logger::DEFAULT_LOG );

void ConnectGameServer( const std::string& i_strAddr, int32_t i_iPort );
void ConnectGameServer_VM( const std::string& i_strServer, const std::string& i_strPort );

bool InEditor();

void NetworkThread( const bool& i_bShutdown );

int32_t ParseArgs( int32_t i_argc, char* i_argv[] );

int32_t ReadNetworkMessages();

void RestartClient();

int32_t ScriptLoad( const std::string& i_strFilePath );
int32_t ScriptRequire( const std::string& i_strFilePath );

void ScriptDebuggerThread( const std::string& i_strPath, const bool& i_bShutdown );

Sqrat::Object ScriptGetLastErrorString();

Sqrat::Object ScriptReadConfig();
void ScriptWriteConfig( Sqrat::Object sqTable );

void SetWindowTitle( const std::string& i_strTitle );

void ShowUsage();

void Shutdown();
void Shutdown_VM();

void StartPatch_VM();

// Notes on mangled main:
// For release modes, use a windows subsystem with an entry point of mainCRTStartup.
// For the Windows solution, this is done through the IDE. It can also be set via #pragma directives:
// #ifdef _DEBUG
//  #pragma comment(linker, "/subsystem:\"console\" /entry:\"mainCRTStartup\"")
// #else
//  #pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
// #endif

//////////////////////////////////////////////////////////////////////////////
// MAIN
//////////////////////////////////////////////////////////////////////////////

int32_t main( int32_t i_iArgc, char* i_pcArgv[], char* i_pcEnvp[] )
{
  // -----------------------------------------------------------------------
  // NOTE: Any logging at this point will go to the default log file
  // -----------------------------------------------------------------------
#ifdef WIN32
//#pragma execution_character_set("utf-8")
  SetConsoleOutputCP( CP_UTF8 );
  RUM_COUT( u8"ÙŠïñ§ ÇöÐê þÅ¶€: UTF8\n" );
#else
#error Change your codepage to UTF8
#endif // WIN32

  ConfigInit( DEFAULT_CONFIG DEFAULT_CONFIG_EXT );

  // Handle command line early for usage requests
  if( i_iArgc > 1 )
  {
    // Parse command line arguments, store arguments in config struct
    if( ParseArgs( i_iArgc, i_pcArgv ) != RESULT_SUCCESS )
    {
      return RESULT_FAILED;
    }
  }
  /*else
  {
      ShowUsage();
      return RESULT_FAILED;
  }*/

  // -----------------------------------------------------------------------
  // NOTE: Any logging before this point will go to the default log file
  // -----------------------------------------------------------------------
  // This should be done as early in the program as possible. Here, the log initialization is delayed until we at least
  // load possible log settings from the ini file or command line.
  if( LogInit( g_cConfigStruct.m_strClientLog ) != RESULT_SUCCESS )
  {
    return RESULT_FAILED;
  }

#ifdef WIN32
  SetConsoleCtrlHandler( (PHANDLER_ROUTINE)ConsoleCtrlHandler, TRUE );
#else
#error Add a signal/control shutdown handler
#endif

  constexpr uint32_t uiInboundPacketSize{ 5 };
  constexpr uint32_t uiOutboundPacketSize{ 5 };
  if( rumNetwork::Init( uiInboundPacketSize, uiOutboundPacketSize ) != RESULT_SUCCESS )
  {
    Logger::LogStandard( "Failed to initialize network. Terminating program." );
    rumNetwork::Shutdown();
    return RESULT_FAILED;
  }

  if( InitDatabase() != RESULT_SUCCESS )
  {
    Logger::LogStandard( "Failed to initialize databases. Terminating program." );
    return RESULT_FAILED;
  }

  rumClientPatcher::Init( g_cConfigStruct.m_strUUID );

  // Cache the project file structure
  rumResource::CreateFileCache( g_cConfigStruct.m_strUUID );

  // Initialize the scripting engine
  std::filesystem::path fsScriptPath( std::filesystem::path( GetProjectPath() ) / rumScript::GetFolderName() );
  if( rumScript::Init( rumScript::VM_CLIENT, fsScriptPath.generic_string() ) != RESULT_SUCCESS )
  {
    Logger::LogStandard( "A fatal error occurred during virtual machine initialization. Terminating program." );
    return RESULT_FAILED;
  }

  if( rumAsset::Init( g_cConfigStruct.m_strUUID ) != RESULT_SUCCESS )
  {
    Logger::LogStandard( "Failed to initialize assets. Terminating program." );
    return RESULT_FAILED;
  }

  if( rumStringTable::Init( g_cConfigStruct.m_strUUID, g_cConfigStruct.m_strLanguage ) != RESULT_SUCCESS )
  {
    Logger::LogStandard( "Failed to initialize string tables. Terminating program." );
    return RESULT_FAILED;
  }

  if( rumDataTable::Init( g_cConfigStruct.m_strUUID ) != RESULT_SUCCESS )
  {
    Logger::LogStandard( "Failed to initialize data tables. Terminating program." );
    return RESULT_FAILED;
  }

  BindScripts();

#ifdef _DEBUG
  std::thread threadScriptDebugger;

  if( g_cConfigStruct.m_bScriptDebug )
  {
    // Attach the VM to the debugger and create the debugger ImGui thread
    RUM_COUT( "Connecting to script debugger, port " << NETIMGUI_PORT << '\n' );
    const std::filesystem::path fsPath( std::filesystem::path( GetProjectPath() ) / rumScript::GetFolderName() );
    threadScriptDebugger = std::thread( &ScriptDebuggerThread, fsPath.generic_string(),
                                        std::ref( g_cConfigStruct.m_bShutdown ) );

    auto pcVM{ Sqrat::DefaultVM::Get() };
    rumDebugVM::EnableDebugInfo( pcVM );
    rumDebugVM::RegisterVM( pcVM, "rumClient" );
  }
#endif // _DEBUG

  // Compile and execute scripts
  if( rumScript::ExecuteStartupScript( rumScript::VM_CLIENT ) != RESULT_SUCCESS )
  {
    Logger::LogStandard( "A fatal error occurred during virtual machine execution. Terminating program." );
    return RESULT_FAILED;
  }

  rumAsset::RegisterClasses();

  rumClientMap::Init();
  rumClientPawn::Init();

  if( rumClientGraphic::Init( g_cConfigStruct ) != RESULT_SUCCESS )
  {
    Logger::LogStandard( "Failed to initialize graphics. Terminating program." );
    return RESULT_FAILED;
  }

  if( rumInput::Init() != 0 )
  {
    Logger::LogStandard( "Failed to initialize input. Terminating program." );
    return RESULT_FAILED;
  }

  if( rumClientSound::Init() != 0 )
  {
    Logger::LogStandard( "Failed to initialize audio. Terminating program." );
    return RESULT_FAILED;
  }

  rumClientInventory::Init();

  SetWindowTitle( PROGRAM_SHORT_DESC );

  if( rumFont::Init() != 0 )
  {
    Logger::LogStandard( "Failed to initialize fonts. Terminating program." );
    return RESULT_FAILED;
  }

  rumCustom::Init();
  rumBroadcast::Init();

  // Save config ini settings
  if( ConfigUpdate() != RESULT_SUCCESS )
  {
    Logger::LogStandard( "Warning: Failed to update initialization file" );
  }

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  // Begin executing game scripts
  const rumAccount& rcAccount{ rumAccount::GetInstance() };

  const auto cPair{ rumScript::EvalRequiredFunc( Sqrat::RootTable( pcVM ),
                                                 "OnGameInit",
                                                 RESULT_FAILED,
                                                 GetElapsedTime(),
                                                 g_cConfigStruct.m_strServerAddr.c_str(),
                                                 rumStringUtils::ToString( g_cConfigStruct.m_uiServerPort ),
                                                 rcAccount.GetAccountName().c_str(),
                                                 g_cConfigStruct.m_strLanguage.c_str() ) };
  if( !cPair.first )
  {
    Logger::LogStandard( "Missing required script function: OnGameInit.\n"
                         "A fatal error occurred during virtual machine initialization. Terminating program." );
    return RESULT_FAILED;
  }

  // Make sure at least one font was created during script initialization
  if( rumFont::GetDefaultFontName().empty() )
  {
    // Attempt to provide a default font
    rumFontAttributes cFontAttributes;
    cFontAttributes.m_uiPixelHeight = 13;
    cFontAttributes.m_cFaceColor = rumColor::s_cWhite;
    cFontAttributes.m_bBlendFace = false;

    // Create the font
    if( !rumFont::Create( "default.ttf", "default", cFontAttributes ) )
    {
      Logger::LogStandard( "At least one font must be created. Terminating program." );
      return RESULT_FAILED;
    }
  }

  //Seed random number generator
  //srand(static_cast<uint32_t>(time(NULL)));

  // Build the sqrt lookup table
  rumNumberUtils::InitSqrtTable( 1024 * 32 ); // 32768 at 4 bytes = 128k

  if( rumClientPlayer::Init() != RESULT_SUCCESS )
  {
    Logger::LogStandard( "Failed to initialize player. Terminating program." );
    return RESULT_FAILED;
  }

  int32_t iFrameCounter{ 0 };

  rumTimer cFPSTimer;
  rumTimer cFrameTimer;
  rumTimer cGarbageCollectionTimer;

  double fDesiredFrameTime{ 1.0 / g_cConfigStruct.m_uiFPS };
  double fOversleepTime{ 0.0 };
  double fSleepTime{ 0.0 };

  while( !g_cConfigStruct.m_bShutdown )
  {
    cFrameTimer.Restart();

    if( rumClientGraphic::WindowClosed() )
    {
      rumClientPlayer::RequestLogout();
      rumClientAccount::AccountLogout();

      std::this_thread::sleep_for( 1s );

      g_cConfigStruct.m_bShutdown = true;
    }

#ifdef _DEBUG
    rumDebugVM::Update();
#endif // _DEBUG

    if( rumNetwork::GetConnectionStatus() == rumNetwork::ConnectionStatus::Failed )
    {
      if( g_threadNetworkHandler.joinable() )
      {
        rumNetwork::Disconnect( INVALID_SOCKET );
        rumScript::ExecRequiredFunc( Sqrat::RootTable( pcVM ), "OnConnectionFailed" );
        g_threadNetworkHandler.join();
      }
    }

    rumScheduler::Run();

    rumInput::Update();
    rumClientSound::Update();

    // Run the server script update function
    rumScript::ExecOptionalFunc( Sqrat::RootTable( pcVM ), "OnFrameStart", g_cMainTimer.GetElapsedSeconds() );

    rumClientGraphic::RenderGame();

    // Process all messages in the queue
    ReadNetworkMessages();

    rumClientPawn::Update();

    // Call this last in the game loop to relinquish any checked out outbound packets that have been sent
    rumNetwork::rumOutboundPacketPool::Update();

    rumClientPatcher* pcPatcher{ rumClientPatcher::GetInstance() };
    if( pcPatcher )
    {
      pcPatcher->Update();
    }

    rumScript::ExecOptionalFunc( Sqrat::RootTable( pcVM ), "OnFrameEnd", g_cMainTimer.GetElapsedSeconds() );

    ++iFrameCounter;

    // Report fps the server is achieving
    if( cFPSTimer.GetElapsedSeconds() >= 1.0 )
    {
      // Display and reset the frame statistics
      cFPSTimer.Restart();
      //RUM_COUT( iFrameCounter << "fps\n" );
      iFrameCounter = 0;
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
        std::this_thread::sleep_for( std::chrono::duration<double>( fSleepTime ) );
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

  // Join threads
#ifdef _DEBUG
  rumDebugVM::DetachVM( Sqrat::DefaultVM::Get() );

  if( threadScriptDebugger.joinable() )
  {
    threadScriptDebugger.join();
  }
#endif // _DEBUG

  // Save config ini settings
  if( ConfigUpdate() != RESULT_SUCCESS )
  {
    Logger::LogStandard( "Warning: Failed to update initialization file" );
  }

  Shutdown();

  if( g_threadNetworkHandler.joinable() )
  {
    g_threadNetworkHandler.join();
  }

  if( g_bRestart )
  {
    // Schedule the client for a restart
    _spawnv( P_NOWAIT, i_pcArgv[0], i_pcArgv );
  }

  Logger::LogStandard( "Client shutdown complete" );

  return 0;
}


void BindScripts()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::Table sqTable( pcVM );
  Sqrat::RootTable( pcVM ).Bind( "CLIENT", sqTable );
  Sqrat::RootTable( pcVM ).Func( "rumStartPatch", StartPatch_VM );

  Sqrat::RootTable( pcVM )
#ifdef _DEBUG
    .SquirrelFunc( "rumDebugAttachVM", &rumDebugVM::AttachVM )
    .SquirrelFunc( "rumDebugDetachVM", &rumDebugVM::DetachVM )
    .SquirrelFunc( "rumDebuggerAttached", &rumDebugVM::IsDebuggerAttached )
#endif // _DEBUG
    .Func( "rumLoadScript", ScriptLoad )
    .Func( "rumLoadFolder", ScriptLoad )
    .Func( "rumRequireScript", ScriptRequire )
    .Func( "rumRequireFolder", ScriptRequire )
    .Func( "rumSetWindowTitle", SetWindowTitle )
    .Func( "rumReadConfig", ScriptReadConfig )
    .Func( "rumWriteConfig", ScriptWriteConfig )
    .Func( "rumInEditor", InEditor )
    .Func( "rumShutdown", Shutdown_VM )
    .Func( "rumServerConnect", ConnectGameServer_VM )
    .Func( "rumRestartClient", RestartClient )
    .Func( "rumGetLastErrorString", ScriptGetLastErrorString );

  Logger::ScriptBind();
  rumLanguage::ScriptBind();
  rumStringTable::ScriptBind();
  rumDataTable::ScriptBind();
  rumNetwork::ScriptBind();
  rumClientAccount::ScriptBind();
  rumUtility::ScriptBind();
  rumTimer::ScriptBind();
  rumPropertyContainer::ScriptBind();
  rumAsset::ScriptBind();
  rumEnum::ScriptBind();
  rumStructs::ScriptBind();
  rumScheduler::ScriptBind();
  rumGameObject::ScriptBind();
  rumCustom::ScriptBind();
  rumClientGraphic::ScriptBind();
  rumClientSound::ScriptBind();
  rumClientMap::ScriptBind();
  rumClientPawn::ScriptBind();
  rumClientPlayer::ScriptBind();
  rumClientInventory::ScriptBind();
  rumResource::ScriptBind();
  rumBroadcast::ScriptBind();
  rumFont::ScriptBind();
  rumClientControl::ScriptBind();
  rumInput::ScriptBind();
  zlib::ScriptBind();
}


int32_t ConfigInit( const std::string& i_strFile, bool i_bUseAlternate )
{
  // Check first the current working directory then the default etc directory

  // Note: Any logging at this point will go to the default log file

  int32_t eResult{ RESULT_FAILED };

  // Use boost for platform independent file path
  const std::filesystem::path fsPath( i_strFile );

  // Create config dictionary
  dictionary* pcDictionary{ iniparser_new( fsPath.string().c_str() ) };
  if( pcDictionary )
  {
    try
    {
      // Save the file used
      g_cConfigStruct.m_strClientIni = fsPath.string();

      int32_t bFullscreen{ iniparser_getboolean( pcDictionary, "client:fullscreen", DEFAULT_FULLSCREEN ) };
      g_cConfigStruct.m_bFullscreen = ( bFullscreen != 0 ) ? true : false;
      g_cConfigStruct.m_uiFullScreenWidth = iniparser_getint( pcDictionary, "client:fullscreen_width",
                                                              DEFAULT_FULLSCREEN_WIDTH );
      g_cConfigStruct.m_uiFullScreenHeight = iniparser_getint( pcDictionary, "client:fullscreen_height",
                                                               DEFAULT_FULLSCREEN_HEIGHT );

      g_cConfigStruct.m_uiWindowedScreenWidth = iniparser_getint( pcDictionary, "client:screen_width",
                                                                  DEFAULT_SCREEN_WIDTH );
      g_cConfigStruct.m_uiWindowedScreenHeight = iniparser_getint( pcDictionary, "client:screen_height",
                                                                   DEFAULT_SCREEN_HEIGHT );
      g_cConfigStruct.m_uiColorDepth = iniparser_getint( pcDictionary, "client:color_depth", DEFAULT_COLOR_DEPTH );

      g_cConfigStruct.m_strUUID = iniparser_getstring( pcDictionary, "client:game", DEFAULT_GAME_PATH );

      g_cConfigStruct.m_strClientLog = iniparser_getstring( pcDictionary, "client:log",
                                                            const_cast<char*>( Logger::DEFAULT_LOG.c_str() ) );

      g_cConfigStruct.m_strLanguage = iniparser_getstring( pcDictionary, "client:language", DEFAULT_LANGUAGE );

      eResult = RESULT_SUCCESS;
    } catch( ... )
    {
      //RUM_COUT( "Error encountered while parsing " << cfg.file << '\n' );
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

  //RUM_COUT( "Updating " << cfg.c_str() << '\n' );

  // The ini filename should already be in the native format
  dictionary* pcDictionary{ iniparser_new( g_cConfigStruct.m_strClientIni.c_str() ) };
  if( pcDictionary )
  {
    try
    {
      iniparser_setstr( pcDictionary, "client:fullscreen", ( rumClientGraphicBase::IsFullscreen() ? "Yes" : "No" ) );
      iniparser_setstr( pcDictionary, "client:fullscreen_width",
                        rumStringUtils::ToString( rumClientGraphicBase::GetFullScreenWidth() ) );
      iniparser_setstr( pcDictionary, "client:fullscreen_height",
                        rumStringUtils::ToString( rumClientGraphicBase::GetFullScreenHeight() ) );
      iniparser_setstr( pcDictionary, "client:screen_width",
                        rumStringUtils::ToString( rumClientGraphicBase::GetWindowedScreenWidth() ) );
      iniparser_setstr( pcDictionary, "client:screen_height",
                        rumStringUtils::ToString( rumClientGraphicBase::GetWindowedScreenHeight() ) );
      iniparser_setstr( pcDictionary, "client:color_depth",
                        rumStringUtils::ToString( g_cConfigStruct.m_uiColorDepth ) );
      iniparser_setstr( pcDictionary, "client:log", g_cConfigStruct.m_strClientLog.c_str() );
      iniparser_setstr( pcDictionary, "client:language", g_cConfigStruct.m_strLanguage.c_str() );

      if( iniparser_dump_ini_file( pcDictionary, g_cConfigStruct.m_strClientIni.c_str() ) == 0 )
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
    Logger::LogStandard( "Failed to update " + g_cConfigStruct.m_strClientIni );
  }

  return RESULT_SUCCESS;
}


void ConnectGameServer( const std::string& i_strAddr, int32_t i_iPort )
{
  if( ( rumNetwork::GetNetworkSocket() == INVALID_SOCKET ) && !g_threadNetworkHandler.joinable() )
  {
    RUM_COUT( "Connecting to game server " << i_strAddr << ':' << i_iPort << '\n' );

    g_cConfigStruct.m_strServerAddr = i_strAddr;
    g_cConfigStruct.m_uiServerPort = i_iPort;

    // Start sending and receiving server communication
    g_threadNetworkHandler = std::thread( &NetworkThread, std::ref( g_cConfigStruct.m_bShutdown ) );
  }
}


void ConnectGameServer_VM( const std::string& i_strServer, const std::string& i_strPort )
{
  int32_t iPort{ DEFAULT_SERVER_PORT };
  if( !i_strPort.empty() )
  {
    iPort = atoi( i_strPort.c_str() );
  }

  ConnectGameServer( i_strServer, iPort );
}


#ifdef WIN32
BOOL ConsoleCtrlHandler( DWORD i_eCtrlType )
{
  switch( i_eCtrlType )
  {
    case CTRL_C_EVENT:
      RUM_COUT( "Ctrl-C Pressed\n" );
      rumClientPlayer::RequestLogout();
      rumClientAccount::AccountLogout();

      std::this_thread::sleep_for( 1s );

      g_cConfigStruct.m_bShutdown = true;
      return TRUE;

    case CTRL_CLOSE_EVENT:
      RUM_COUT( "Application shutdown\n" );

      rumClientPlayer::RequestLogout();
      rumClientAccount::AccountLogout();

      std::this_thread::sleep_for( 1s );

      g_cConfigStruct.m_bShutdown = true;
      ExitThread( 0 );
      return FALSE;

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
#endif // WIN32


const rumConfig& GetConfig()
{
  return g_cConfigStruct;
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
  return g_cConfigStruct.m_strUUID;
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

  const std::filesystem::path fsPath( g_cConfigStruct.m_strUUID );

  using DatabaseID = rumDatabase::DatabaseID;

  struct EditorDatabases
  {
    std::string m_strFilename;
    DatabaseID m_eDatabaseID{ DatabaseID::Invalid_DatabaseID };
  };

  const std::vector<EditorDatabases> vDatabases
  {
    { "assets.db",     DatabaseID::Assets_DatabaseID },
    { "datatables.db", DatabaseID::DataTable_DatabaseID },
    { "patch.db",      DatabaseID::Patch_DatabaseID },
    { "strings.db",    DatabaseID::Strings_DatabaseID }
  };

  // Load each database
  for( const auto& iter : vDatabases )
  {
    const std::filesystem::path fsDatabasePath( fsPath / iter.m_strFilename );
    if( !rumDatabase::CreateConnection( iter.m_eDatabaseID, fsDatabasePath.string() ) )
    {
      std::string strError{ "Failed to connect to database " };
      strError += fsDatabasePath.string();
      Logger::LogStandard( strError );
      rumAssertMsg( false, strError );
      return RESULT_FAILED;
    }
  }

  return RESULT_SUCCESS;
}


void Log( const SQChar* i_strText )
{
  Logger::LogStandard( std::string( i_strText ) );
}


int32_t LogInit( const std::string& i_strLog )
{
  Logger::Archive( i_strLog );

  const std::filesystem::path fsPath( i_strLog );
  Logger::strStandardLog = fsPath.string();

  RUM_COUT( "Using log: " << Logger::strStandardLog << '\n' );

  // Save echo status
  const bool bEcho{ Logger::IsEchoEnabled() };
  Logger::EnableEcho( false );

  // Test the log location and specify that the mod is starting
  int32_t eResult{ Logger::LogStandard( PROGRAM_LONG_DESC " version " QUOTED( PROGRAM_MAJOR_VERSION ) "."
                                        QUOTED( PROGRAM_MINOR_VERSION ) ) };
  if( RESULT_FAILED == eResult )
  {
    RUM_COUT( "Warning: Log file " << Logger::strStandardLog << " is invalid.\n" );

    if( i_strLog.compare( Logger::DEFAULT_LOG ) != 0 )
    {
      // Switch back to the default log file
      eResult = LogInit( Logger::DEFAULT_LOG );
    }
    else
    {
      // Disable logging
      RUM_COUT( "Warning: Logging will not be available for this run\n" );
      Logger::EnableLogging( false );
    }
  }

  Logger::LogStandard( LINE_BREAK );

  // Restore echo status
  Logger::EnableEcho( bEcho );

  return eResult;
}


#ifdef WIN32
int32_t MessageProcedure( HWND i_hWnd, UINT i_uiMsg, WPARAM i_piWParam, LPARAM i_piLParam, int32_t* i_piRetval )
{
  i_piRetval = 0;

  //switch( uMsg )
  //{
  //  case WM_FILE_OPEN_DIALOG_SELECTION:
  //    return 0;
  //}

  return 1;
}
#else
#error Add support for your custom message handler here
#endif // WIN32f


void NetworkThread( const bool& i_rbShutdown )
{
  RUM_COUT( "Connection of socket " << socket << '\n' );

  // Connect to the selected server
  const SOCKET iSocket{ rumNetwork::Connect( g_cConfigStruct.m_strServerAddr, g_cConfigStruct.m_uiServerPort ) };
  if( INVALID_SOCKET == iSocket )
  {
    return;
  }

  // Request server info straight away
  auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
  rcPacket.SetHeader( rumNetwork::PACKET_HEADER_CLIENT_REQ_SERVER_INFO );
  rcPacket.Send( iSocket );

  u_long ulNoBlock{ 1 };
  ioctlsocket( iSocket, FIONBIO, &ulNoBlock );

  int32_t eResult{ RESULT_FAILED };
  bool bDisconnect{ false };

  rumNetwork::rumConnection* pcConnection{ new rumNetwork::rumConnection( iSocket ) };
  assert( pcConnection );

  rumNetwork::ConnectionHash hashConnections;
  hashConnections.insert( make_pair( iSocket, pcConnection ) );

  while( !i_rbShutdown )
  {
    rumNetwork::UpdatePackets( hashConnections );

    rumNetwork::ConnectionHash::iterator iter( hashConnections.begin() );
    const rumNetwork::ConnectionHash::iterator end( hashConnections.end() );
    while( iter != end )
    {
      pcConnection = iter->second;

      eResult = pcConnection->SendPackets();
      if( RESULT_SUCCESS == eResult )
      {
        eResult = pcConnection->ReceivePackets();
      }

      if( SOCKET_ERROR == eResult )
      {
        int32_t eError;
        int32_t iErrorLength{ sizeof( eError ) };

        getsockopt( iter->first, SOL_SOCKET, SO_ERROR, (char*)&eError, &iErrorLength );

        if( !( ( NO_ERROR == eError ) || ( WSAEWOULDBLOCK == eError ) ) )
        {
          bDisconnect = true;
        }
      }

      if( bDisconnect )
      {
        RUM_COUT( "Closing connection " << iter->first << '\n' );

        pcConnection->CloseSocket();
        delete pcConnection;
        iter = hashConnections.erase( iter );

        bDisconnect = false;
      }
      else
      {
        ++iter;
      }
    }

    // Process the download file queue
    rumNetwork::DownloadFiles();

    using namespace std::literals::chrono_literals;
    std::this_thread::sleep_for( 5ms );
  }

  // Clean up network connections
  for( const auto& iter : hashConnections )
  {
    iter.second->CloseSocket();
    delete iter.second;
  }

  hashConnections.clear();
}


int32_t ParseArgs( int32_t i_iArgc, char* i_pcArgv[] )
{
  // The ini struct should already contain values loaded from the .ini file.
  // Arguments passed via the command line should override the stored value.

  int32_t eResult{ RESULT_SUCCESS };
  bool bShowUsage{ false };

  // Cycle through all parameters
  for( int32_t i = 1; !bShowUsage && i < i_iArgc; ++i )
  {
    if( strcasecmp( "-acct", i_pcArgv[i] ) == 0 )
    {
      rumAccount& rcAccount{ rumAccount::GetInstance() };
      rcAccount.SetName( i_pcArgv[++i] );
    }
    else if( strcasecmp( "-game", i_pcArgv[i] ) == 0 )
    {
      g_cConfigStruct.m_strUUID = i_pcArgv[++i];
    }
    else if( strcasecmp( "-server", i_pcArgv[i] ) == 0 )
    {
      g_cConfigStruct.m_strServerAddr = i_pcArgv[++i];
    }
    else if( strcasecmp( "-port", i_pcArgv[i] ) == 0 )
    {
      g_cConfigStruct.m_uiServerPort = atoi( i_pcArgv[++i] );
    }
    else if( strcasecmp( "-language", i_pcArgv[i] ) == 0 )
    {
      g_cConfigStruct.m_strLanguage = i_pcArgv[++i];
    }
#ifdef _DEBUG
    else if( strcasecmp( "-debug", i_pcArgv[i] ) == 0 )
    {
      g_cConfigStruct.m_bScriptDebug = true;
    }
#endif // _DEBUG

    /// TEMP - for string database updates
    /*else if( strcasecmp( "-db", i_pcArgv[i] ) == 0 )
    {
      // Update databases from .csv files
      g_cConfigStruct.m_bDatabaseRefresh = true;
    }*/
    else if( strcmp( "-?", i_pcArgv[i] ) == 0 ||
             strcasecmp( "-h", i_pcArgv[i] ) == 0 ||
             strcasecmp( "-help", i_pcArgv[i] ) == 0 )
    {
      bShowUsage = true;
    }
    else
    {
      bShowUsage = true;
    }
  }

  // The uuid must be passed to the client, there is no such thing as a default server connection uuid
  if( g_cConfigStruct.m_strUUID.empty() )
  {
    ShowUsage();
    eResult = RESULT_FAILED;
  }

  return eResult;
}


int32_t ReadNetworkMessages()
{
  rumNetwork::PacketQueue cPacketQueue;

  // Dequeue all pending outgoing packets (causes mutex lock)
  rumNetwork::DequeuePackets( rumNetwork::PACKET_QUEUE_RECV, cPacketQueue );

#if NETWORK_DEBUG
  if( packets.size() > 0 )
  {
    RUM_COUT_IFDEF( NETWORK_DEBUG, "Reading " << packets.size() << " packet(s)\n" );
  }
#endif // NETWORK_DEBUG

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  // Send packets
  while( !cPacketQueue.empty() )
  {
    auto pcPacket{ cPacketQueue.front() };
    rumNetwork::rumInboundPacket* pcPacketTemp{ (rumNetwork::rumInboundPacket*)pcPacket };
    const auto& cPacket{ *pcPacketTemp };
    cPacketQueue.pop();

    const PACKET_HEADER_TYPE eHeaderType{ static_cast<PACKET_HEADER_TYPE>( cPacket.GetHeaderType() ) };

    if( rumNetwork::PACKET_HEADER_SERVER_SERVER_INFO == eHeaderType )
    {
      rumByte bPatch{ 0 };
      std::string strPatchChecksum;
      std::string strAddress;
      std::string strPath;
      rumWord iPort{ 0 };

      cPacket << bPatch << strPatchChecksum << strAddress << strPath << iPort;

      if( rumClientPatcher::GetInstance() )
      {
        rumClientPatcher::GetInstance()->SetConnectionInfo( ( bPatch != 0 ? true : false ), strPatchChecksum, strAddress, strPath, iPort );
      }

      rumScript::ExecRequiredFunc( Sqrat::RootTable( pcVM ), "OnConnectionComplete" );
    }
    else if( rumNetwork::PACKET_HEADER_SERVER_ACCOUNT_CREATE_RESULT == eHeaderType )
    {
      rumByte iResult{ 0 };

      cPacket << iResult;

      rumClientAccount::CreateResult( (rumAccount::AccountCreateResultType)iResult );
    }
    else if( rumNetwork::PACKET_HEADER_SERVER_ACCOUNT_LOGIN_RESULT == eHeaderType )
    {
      rumByte eResult{ 0 };
      rumQWord iSocket{ 0 };
      rumByte nCharacters{ 0 };

      std::list<std::string> listCharacterNames;

      cPacket << eResult;

      if( (rumByte)rumAccount::AccountLoginResultType::ACCOUNT_LOGIN_SUCCESS == eResult )
      {
        cPacket << iSocket << nCharacters;

        RUM_COUT_IFDEF( NETWORK_DEBUG,
                        "Received ACCOUNT_LOGIN_PASS id: " << iSocket << ", " << (int32_t)nCharacters <<
                        " character(s)\n" );

        std::string strName;

        for( uint32_t i = 0; i < nCharacters; ++i )
        {
          cPacket << strName;
          listCharacterNames.push_back( strName );
        }
      }

      rumClientAccount::LoginResult( (rumAccount::AccountLoginResultType)eResult, iSocket, listCharacterNames );
    }
    else if( rumNetwork::PACKET_HEADER_SERVER_PLAYER_CREATE_RESULT == eHeaderType )
    {
      rumByte iResult{ 0 };
      std::string strPlayerName;

      cPacket << iResult << strPlayerName;

      rumClientPlayer::CreateResult( (rumPlayer::PlayerCreateResultType)iResult, strPlayerName );
    }
    else if( rumNetwork::PACKET_HEADER_SERVER_PLAYER_LOGIN_RESULT == eHeaderType )
    {
      rumQWord uiPlayerID{ INVALID_GAME_ID };
      rumByte iResult{ 0 };

      cPacket << uiPlayerID << iResult;

      rumClientPlayer::LoginResult( (rumUniqueID)uiPlayerID, (rumPlayer::PlayerLoginResultType)iResult );
    }
    else if( rumNetwork::PACKET_HEADER_SERVER_PLAYER_LOGOUT_RESULT == eHeaderType )
    {
      rumQWord uiPlayerID{ INVALID_GAME_ID };

      cPacket << uiPlayerID;

      rumClientPlayer::LogoutResult( (rumUniqueID)uiPlayerID );
    }
    else if( rumNetwork::PACKET_HEADER_CONNECTION_TERMINATED == eHeaderType )
    {
      RUM_COUT_IFDEF( NETWORK_DEBUG, "Connection terminated, socket: " << cPacket.GetSocket() << '\n' );
    }
    else if( rumNetwork::PACKET_HEADER_SERVER_ALL_PLAYER_INFO == eHeaderType )
    {
      RUM_COUT_IFDEF( NETWORK_DEBUG, "Received ALL_PLAYER_INFO\n" );

      // Received when a new player joins the game. This is the list of all other players.
      rumWord uiNumPlayers{ 0 };

      cPacket << uiNumPlayers;

      for( int i = 0; i < uiNumPlayers; ++i )
      {
        std::string strName;
        rumQWord uiPlayerID{ INVALID_GAME_ID };
        rumQWord uiPawnID{ INVALID_GAME_ID };

        cPacket << strName << uiPlayerID << uiPawnID;

        RUM_COUT_IFDEF( NETWORK_DEBUG,
                        "Creating player " << strName << " [" << rumStringUtils::ToHexString64( uiPlayerID ) <<
                        "] pawn [" << rumStringUtils::ToHexString64( uiPawnID ) << "]\n" );

        rumClientPlayer::RecvNewPlayerInfo( strName, (rumUniqueID)uiPlayerID, (rumUniqueID)uiPawnID );
      }
    }
    else if( rumNetwork::PACKET_HEADER_SERVER_NEW_PLAYER_INFO == eHeaderType )
    {
      // Received when a new player joins the game. The player's pawn is created here and the two are linked. This
      // packet is generally followed by a PLAYER_MAP_UPDATE packet due to the server player pawn getting added to a
      // map for the first time.

      std::string strName;
      rumQWord uiPlayerID{ INVALID_GAME_ID };
      rumQWord uiPawnID{ INVALID_GAME_ID };

      cPacket << strName << uiPlayerID << uiPawnID;

      RUM_COUT_IFDEF( NETWORK_DEBUG,
                      "Received NEW_PLAYER_INFO, creating player " << strName << " [" <<
                      rumStringUtils::ToHexString64( uiPlayerID ) << "] pawn [" <<
                      rumStringUtils::ToHexString64( uiPawnID ) << "]\n" );

      rumClientPlayer::RecvNewPlayerInfo( strName, (rumUniqueID)uiPlayerID, (rumUniqueID)uiPawnID );
    }
    // Received when a player transitions to a new map. Immediately followed by a SERVER_PAWN_UPDATES packet.
    else if( rumNetwork::PACKET_HEADER_SERVER_PLAYER_MAP_UPDATE == eHeaderType )
    {
      rumQWord uiPlayerID{ INVALID_GAME_ID };
      rumDWord eMapAssetID{ INVALID_ASSET_ID };
      rumQWord uiMapID{ INVALID_GAME_ID };
      rumDWord iPosX{ 0 };
      rumDWord iPosY{ 0 };

      cPacket << uiPlayerID << eMapAssetID << uiMapID << iPosX << iPosY;

      RUM_COUT_IFDEF( NETWORK_DEBUG,
                      "Received PLAYER_MAP_UPDATE, player [" << rumStringUtils::ToHexString64( uiPlayerID ) <<
                      "] map (" << rumStringUtils::ToHexString( eMapAssetID ) << ")[" <<
                      rumStringUtils::ToHexString64( uiMapID ) << ", position (" << iPosX << ", " << iPosY << ")\n" );

      rumClientPlayer::RecvMapUpdate( (rumUniqueID)uiPlayerID, (rumAssetID)eMapAssetID, (rumUniqueID)uiMapID,
                                      rumPosition( iPosX, iPosY ) );
    }
    else if( rumNetwork::PACKET_HEADER_SERVER_PROPERTY_REMOVED == eHeaderType )
    {
      rumQWord uiGameID{ 0 };
      rumDWord ePropertyID{ INVALID_ASSET_ID };

      cPacket << uiGameID << ePropertyID;

      RUM_COUT_IFDEF( NETWORK_DEBUG || PROPERTY_DEBUG,
                      "Received game Object [" << rumStringUtils::ToHexString64( uiGameID ) <<
                      "] property removal:\n " );

      // Get the affected object
      rumGameObject* pcObject{ rumGameObject::Fetch( (rumUniqueID)uiGameID ) };
      rumAssertMsg( pcObject, "Failed to fetch target object for property update" );
      if( pcObject )
      {
#if NETWORK_DEBUG || PROPERTY_DEBUG
        std::string strName{ "Unknown" };

        const rumPropertyAsset* pcProperty{ rumPropertyAsset::Fetch( (rumAssetID)ePropertyID ) };
        if( pcProperty )
        {
          strName = pcProperty->GetName();
        }

        RUM_COUT( strName << " (" << rumStringUtils::ToHexString( ePropertyID ) << ")\n" );
#endif // NETWORK_DEBUG || PROPERTY_DEBUG

        pcObject->RemoveProperty( (rumAssetID)ePropertyID );
      }
    }
    else if( rumNetwork::PACKET_HEADER_SERVER_PROPERTY_UPDATE == eHeaderType )
    {
      rumQWord uiGameID{ INVALID_GAME_ID };
      rumDWord uiNumProperties{ 0 };
      rumDWord ePropertyID{ INVALID_ASSET_ID };
      Sqrat::Object sqValue;

      cPacket << uiGameID << uiNumProperties;

      RUM_COUT_IFDEF( NETWORK_DEBUG || PROPERTY_DEBUG,
                      "Received game Object [" << rumStringUtils::ToHexString64( uiGameID ) <<
                      "] property updates:\n " );

      // Get the affected object
      rumGameObject* pcObject{ rumGameObject::Fetch( (rumUniqueID)uiGameID ) };
      rumAssertMsg( pcObject, "Failed to fetch target object for property update" );
      if( pcObject )
      {
        for( uint32_t i = 0; i < (uint32_t)uiNumProperties; ++i )
        {
          cPacket << ePropertyID << sqValue;

#if NETWORK_DEBUG || PROPERTY_DEBUG
          std::string strName{ "Unknown" };

          const rumPropertyAsset* pcProperty{ rumPropertyAsset::Fetch( (rumAssetID)ePropertyID ) };
          if( pcProperty )
          {
            strName = pcProperty->GetName();
          }

          RUM_COUT( strName << " (" << rumStringUtils::ToHexString( ePropertyID ) << " value " <<
                    rumStringUtils::ToString( sqValue.Cast<uint32_t>() ) << ")\n" );
#endif // NETWORK_DEBUG || PROPERTY_DEBUG

          pcObject->SetProperty( (rumAssetID)ePropertyID, sqValue );
        }
      }
    }
    else if( rumNetwork::PACKET_HEADER_SERVER_INVENTORY_UPDATE == eHeaderType )
    {
      rumQWord uiPlayerID{ INVALID_GAME_ID };
      rumQWord uiInventoryID{ INVALID_GAME_ID };
      rumDWord eInventoryType{ INVALID_ASSET_ID };
      bool bCreate{ false };

      cPacket << uiPlayerID << uiInventoryID << eInventoryType << bCreate;

      RUM_COUT_IFDEF( NETWORK_DEBUG,
                      "Received inventory for player [" << rumStringUtils::ToHexString64( uiPlayerID ) << "] item [" <<
                      rumStringUtils::ToHexString64( uiInventoryID ) << "] type (" <<
                      rumStringUtils::ToHexString( eInventoryType ) << ") create (" <<
                      rumStringUtils::ToString( bCreate ) << ")\n" );

      rumClientPlayer::RecvInventoryUpdate( (rumUniqueID)uiPlayerID, (rumUniqueID)uiInventoryID,
                                            (rumAssetID)eInventoryType, bCreate );
    }
    else if( rumNetwork::PACKET_HEADER_SERVER_PAWN_REMOVE == eHeaderType )
    {
      rumQWord uiPawnID{ INVALID_GAME_ID };

      cPacket << uiPawnID;

      RUM_COUT_IFDEF( NETWORK_DEBUG,
                      "Received PAWN_REMOVE, object: " << rumStringUtils::ToHexString64( uiPawnID ) << '\n' );

      rumPawn* pcPawn{ rumPawn::Fetch( uiPawnID ) };
      if( pcPawn )
      {
        // Remove the pawn and only release the script object if it's not a player
        rumMap* pcMap{ pcPawn->GetMap() };
        if( pcMap && pcMap->RemovePawn( pcPawn ) && !pcPawn->IsPlayer() )
        {
          rumGameObject::UnmanageScriptObject( pcPawn->GetScriptInstance() );
        }
      }
    }
    // Received after a player has transitioned to a new map. The player pawn should already be created at this point
    // and the new map fully loaded. This should always follow a PLAYER_MAP_UPDATE packet.
    else if( rumNetwork::PACKET_HEADER_SERVER_PAWN_UPDATES == eHeaderType )
    {
      const rumPlayer* pcPlayer{ rumClientPlayer::GetClientPlayer() };
      if( pcPlayer )
      {
        const rumPawn* pcPawn{ pcPlayer->GetPlayerPawn() };
        if( pcPawn )
        {
          rumMap* pcMap{ pcPawn->GetMap() };
          rumAssert( pcMap );
          if( pcMap )
          {
            RUM_COUT_IFDEF( NETWORK_DEBUG, "Received pawn updates for map " << pcMap->GetName() << '\n' );

            rumClientMap::ServerPawnUpdates( pcMap, cPacket );
          }
        }
      }
    }
    // Received any time a pawn has moved
    else if( rumNetwork::PACKET_HEADER_SERVER_PAWN_POSITION_UPDATE == eHeaderType )
    {
      rumQWord uiPawnID{ INVALID_GAME_ID };
      rumDWord iPosX{ 0 };
      rumDWord iPosY{ 0 };
      cPacket << uiPawnID << iPosX << iPosY;

      RUM_COUT_IFDEF( NETWORK_DEBUG,
                      "Received PACKET_HEADER_SERVER_PAWN_POSITION_UPDATE, pawn [" <<
                      rumStringUtils::ToHexString64( uiPawnID ) << "], pos: " << iPosX << ", " << iPosY << '\n' );

      rumPawn* pcPawn{ rumPawn::Fetch( uiPawnID ) };
      if( pcPawn )
      {
        rumMap* pcMap{ pcPawn->GetMap() };
        rumAssert( pcMap );
        if( pcMap )
        {
          const rumMoveFlags eMoveFlags{ (rumMoveFlags)( IgnoreTileCollision_MoveFlag | IgnorePawnCollision_MoveFlag |
                                                         IgnoreDistance_MoveFlag ) };
          pcMap->MovePawn( pcPawn, rumPosition( iPosX, iPosY ), eMoveFlags, 0 );
        }
      }
    }
    // Received any time a pawn's collision flags change
    else if( rumNetwork::PACKET_HEADER_SERVER_PAWN_COLLISIONFLAGS_UPDATE == eHeaderType )
    {
      rumQWord uiPawnID{ INVALID_GAME_ID };
      rumDWord uiCollisionFlags{ 0 };

      cPacket << uiPawnID << uiCollisionFlags;

      rumPawn* pcPawn{ rumPawn::Fetch( uiPawnID ) };
      if( pcPawn )
      {
        pcPawn->SetCollisionFlags( (uint32_t)uiCollisionFlags );
      }
    }
    // Received any time a pawn's move type changes
    else if( rumNetwork::PACKET_HEADER_SERVER_PAWN_MOVETYPE_UPDATE == eHeaderType )
    {
      rumQWord uiPawnID{ INVALID_GAME_ID };
      rumDWord uiMoveType{ 0 };

      cPacket << uiPawnID << uiMoveType;

      rumPawn* pcPawn{ rumPawn::Fetch( uiPawnID ) };
      if( pcPawn )
      {
        pcPawn->SetMoveType( uiMoveType );
      }
    }
    // Received any time a pawn's state changes
    else if( rumNetwork::PACKET_HEADER_SERVER_PAWN_STATE_UPDATE == eHeaderType )
    {
      rumQWord uiPawnID{ INVALID_GAME_ID };
      rumDWord iState{ 0 };

      cPacket << uiPawnID << iState;

      rumPawn* pcPawn{ rumPawn::Fetch( uiPawnID ) };
      if( pcPawn )
      {
        pcPawn->SetState( iState );
      }
    }
    // Received any time a pawn has changed visibility
    else if( rumNetwork::PACKET_HEADER_SERVER_PAWN_VISIBILITY_UPDATE == eHeaderType )
    {
      rumQWord uiPawnID{ INVALID_GAME_ID };
      rumByte uiVisibility{ 0 };

      cPacket << uiPawnID << uiVisibility;

      RUM_COUT_IFDEF( NETWORK_DEBUG,
                      "Received PACKET_HEADER_SERVER_PAWN_VISIBILITY_UPDATE, pawn [" <<
                      rumStringUtils::ToHexString64( uiPawnID ) << "], visibility " <<
                      ( uiVisibility ? "true" : "false" ) << '\n' );

      rumPawn* pcPawn{ rumPawn::Fetch( uiPawnID ) };
      if( pcPawn )
      {
        pcPawn->SetVisibility( 0 == uiVisibility ? false : true );
      }
    }
    // Received any time a pawn's lighting has changed
    else if( rumNetwork::PACKET_HEADER_SERVER_PAWN_LIGHT_UPDATE == eHeaderType )
    {
      rumQWord uiPawnID{ INVALID_GAME_ID };
      rumDWord uiLightRange{ 0 };

      cPacket << uiPawnID << uiLightRange;

      RUM_COUT_IFDEF( NETWORK_DEBUG,
                      "Received PACKET_HEADER_SERVER_PAWN_LIGHT_UPDATE, pawn [" <<
                      rumStringUtils::ToHexString64( uiPawnID ) << ", light range " << uiLightRange << '\n' );

      rumPawn* pcPawn{ rumPawn::Fetch( uiPawnID ) };
      if( pcPawn )
      {
        pcPawn->SetLightRange( uiLightRange );
      }
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

      rumClientPlayer::ProcessBroadcast( sqInstance );
    }
    else
    {
#ifdef _DEBUG
      std::string strError{ "Error: Unhandled packet type " };
      strError += rumStringUtils::ToString( (int32_t)eHeaderType );
      Logger::LogStandard( strError, Logger::LOG_ERROR );
      assert( false );
#endif
    }

    // TODO - is this necessary?
    //delete pcPacket;
  }

  return RESULT_SUCCESS;
}


void RestartClient()
{
  g_cConfigStruct.m_bShutdown = true;
  g_bRestart = true;
}


void ScriptDebuggerThread( const std::string& i_strPath, const bool& i_bShutdown )
{
#ifdef _DEBUG
  rumDebugInterface::Init( DEFAULT_CONFIG, NETIMGUI_PORT, i_strPath );

  while( !i_bShutdown )
  {
    rumDebugInterface::Update();
  }

  rumDebugInterface::Shutdown();
#endif // _DEBUG
}


Sqrat::Object ScriptGetLastErrorString()
{
  Sqrat::Object sqString;
  rumScript::SetValue( sqString, g_pstrLastErrorString );

  return sqString;
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
    std::filesystem::path( g_cConfigStruct.m_strUUID ) / DEFAULT_GAME_CONFIG DEFAULT_CONFIG_EXT
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
    std::filesystem::path( g_cConfigStruct.m_strUUID ) / DEFAULT_GAME_CONFIG DEFAULT_CONFIG_EXT
  };
  rumScript::WriteConfig( fsPath.string(), sqTable );
}


void SetWindowTitle( const std::string& i_strDesc )
{
  rumClientGraphic::SetWindowTitle( i_strDesc );
}


void ShowUsage()
{
  // We really don't want users providing their own parameters to the client
  // since much of the information comes from the portal
#ifdef WIN32
  rumClientGraphic::MessageBox( "Launch rum_portal.exe to start a game." );
#else
#error UNHANDLED PLATFORM
#endif
}


void Shutdown()
{
  rumScript::CallShutdownFunction();

  rumNumberUtils::FreeSqrtTable();

  rumClientPatcher::Shutdown();

  rumScheduler::Shutdown();

  rumClientMap::Shutdown();
  rumCustom::Shutdown();
  rumPlayer::Shutdown();
  rumPawn::Shutdown();
  rumInventory::Shutdown();
  rumClientSound::Shutdown();
  rumFont::Shutdown();
  rumBroadcast::Shutdown();
  rumInput::Shutdown();
  rumClientGraphic::Shutdown();
  rumGameObject::Shutdown();

  rumAsset::Shutdown();

  rumScript::Shutdown();
  rumDatabase::Shutdown();

  rumNetwork::Shutdown();
}


void Shutdown_VM()
{
  rumClientPlayer::RequestLogout();
  rumClientAccount::AccountLogout();

  std::this_thread::sleep_for( 1s );

  g_cConfigStruct.m_bShutdown = true;
}


void StartPatch_VM()
{
  rumClientPatcher* pcPatcher{ rumClientPatcher::GetInstance() };
  if( pcPatcher )
  {
    pcPatcher->StartPatch();
  }
}
