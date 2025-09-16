/*

R/U/M Construction Kit Editor

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

#define PROGRAM_SHORT_DESC  "RUM Editor"
#define PROGRAM_LONG_DESC   "R/U/M Editor"

#define DEFAULT_CONFIG      "rum_editor"

#include <filesystem>

// Qt
#include <QApplication>
#include <QDir>
#include <QMessageBox>

#include <fstream>
#include <map>

#include <iniparser.h>

#include <u_asset.h>
#include <u_assert.h>
#include <u_broadcast.h>
#include <u_custom.h>
#include <u_datatable.h>
#include <u_graphic.h>
#include <u_language.h>
#include <u_log.h>
#include <u_resource.h>
#include <u_scheduler.h>
#include <u_strings.h>
#include <u_structs.h>
#include <u_timer.h>
#include <u_zlib.h>

#include <e_control.h>
#include <e_graphics_opengl.h>
#include <e_inventory.h>
#include <e_map.h>
#include <e_pawn.h>
#include <e_rum.h>
#include <e_sound.h>

#include <ui/mainwindow.h>
#include <ui/mapeditor.h>

// For testing CSV changes:
/*#include <u_creature_asset.h>
#include <u_inventory_asset.h>
#include <u_map_asset.h>
#include <u_portal_asset.h>
#include <u_property_asset.h>
#include <u_tile_asset.h>
#include <u_widget_asset.h>

// For testing CSV changes:
rumAsset::ExportCSVFiles<rumBroadcastAsset>(g_cConfigStruct.m_strProjectPath);
rumAsset::ExportCSVFiles<rumCreatureAsset>( g_cConfigStruct.m_strProjectPath );
rumAsset::ExportCSVFiles<rumCustomAsset>( g_cConfigStruct.m_strProjectPath );
rumAsset::ExportCSVFiles<rumGraphicAsset>( g_cConfigStruct.m_strProjectPath );
rumAsset::ExportCSVFiles<rumInventoryAsset>( g_cConfigStruct.m_strProjectPath );
rumAsset::ExportCSVFiles<rumMapAsset>( g_cConfigStruct.m_strProjectPath );
rumAsset::ExportCSVFiles<rumPortalAsset>( g_cConfigStruct.m_strProjectPath );
rumAsset::ExportCSVFiles<rumPropertyAsset>( g_cConfigStruct.m_strProjectPath );
rumAsset::ExportCSVFiles<rumSoundAsset>( g_cConfigStruct.m_strProjectPath );
rumAsset::ExportCSVFiles<rumTileAsset>( g_cConfigStruct.m_strProjectPath );
rumAsset::ExportCSVFiles<rumWidgetAsset>( g_cConfigStruct.m_strProjectPath );
*/

void ArchiveCreate();

int32_t ConfigInit( const std::string& i_strFile, bool i_bUseAlternate = true );
int32_t ConfigUpdate();

void Log( const SQChar* i_sqStr );
int32_t LogInit( const std::string& i_strLog = Logger::DEFAULT_LOG );

int32_t ParseArgs( int32_t i_iArgc, char* i_pcArgv[] );

uint64_t g_uiFrameIndex{ 0UL };

rumTimer g_cMainTimer;

rumConfig g_cConfigStruct;

typedef std::map<std::string, rumDatabase::DatabaseID> DatabaseMap;
DatabaseMap g_mapDatabases;


int32_t main( int32_t i_iArgc, char* i_pcArgv[], char* i_pcEnvp[] )
{
  qRegisterMetaType<int8_t>( "int8_t" );
  qRegisterMetaType<uint8_t>( "uint8_t" );
  qRegisterMetaType<int16_t>( "int16_t" );
  qRegisterMetaType<uint16_t>( "uint16_t" );
  qRegisterMetaType<int32_t>( "int32_t" );
  qRegisterMetaType<uint32_t>( "uint32_t" );
  qRegisterMetaType<int64_t>( "int64_t" );
  qRegisterMetaType<uint64_t>( "uint64_t" );
  qRegisterMetaType<rumAssetID>( "rumAssetID" );
  qRegisterMetaType<rumUniqueID>( "rumUniqueID" );

  //QGuiApplication::setHighDpiScaleFactorRoundingPolicy( Qt::HighDpiScaleFactorRoundingPolicy::PassThrough );
  QCoreApplication::setAttribute( Qt::AA_ShareOpenGLContexts );
  QApplication cApp( i_iArgc, i_pcArgv );

  // This is required to exist for Qt plugins
  QApplication::addLibraryPath( QApplication::applicationDirPath() + "/plugins" );

  MainWindow cMainWindow( g_cConfigStruct );
  QString strProgramDesc{ QString( PROGRAM_LONG_DESC " v%1.%2" )
    .arg( PROGRAM_MAJOR_VERSION ).arg( PROGRAM_MINOR_VERSION ) };
  cMainWindow.setWindowTitle( strProgramDesc );
  cMainWindow.Init( i_iArgc, i_pcArgv );
  cMainWindow.show();

  // -----------------------------------------------------------------------
  // NOTE: Any logging at this point will go to the default log file
  // -----------------------------------------------------------------------

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
  // This should be done as early in the program as possible. Here, the
  // log initialization is delayed until we at least load possible log
  // settings from the ini file or command line.
  if( LogInit() != RESULT_SUCCESS )
  {
    return RESULT_FAILED;
  }

  // Save config ini settings
  if( ConfigUpdate() != RESULT_SUCCESS )
  {
    Logger::LogStandard( "Warning: Failed to update initialization file" );
  }

  // Hand off to Qt
  return cApp.exec();
}

/*int archiveVerify()
{
    int32_t result = RESULT_SUCCESS;

    ThreadedDB &handle = GameDB::getHandle();
    DBresult *dbRes = handle.query("SELECT path,name FROM archive WHERE 1");
    if (dbRes != NULL && dbRes->IsError() == false)
    {
        enum { ARCHIVE_PATH, ARCHIVE_NAME };

        int32_t rows = dbRes->numRows();
        for (int32_t i = 0; i < rows; i++)
        {
            std::filesystem::path path(dbRes->fetch(i, ARCHIVE_PATH));
            string archive_filename = dbRes->fetch(i, ARCHIVE_NAME);
            std::filesystem::path archive(path / std::filesystem::path(archive_filename));

            cout << "Verifying archive " << archive.string() << endl;

            // Make sure the target location exists
            if (std::filesystem::exists(path) && std::filesystem::is_directory(path))
            {
                // Zlib archive exists, check it
                if (std::filesystem::exists(archive) && std::filesystem::is_regular_file(archive))
                {
                    // if the timestamp on any file is newer than the compressed file itself, it should be updated
                    // or possibly just do a crc check
                    // also, what about added or deleted files?
                    zlib::verifyArchive(path, archive);
                }
                // Zlib archive does not exist, create it
                else
                {
                    // Create the archive file
                    zlib::createArchive(path, archive);
                }
            }
        }
    }

    return result;
}*/


double GetElapsedTime()
{
  return g_cMainTimer.GetElapsedSeconds();
}


uint64_t GetFrameIndex()
{
  return g_uiFrameIndex;
}


int32_t ConfigInit( const std::string& i_strFile, bool i_bUseAlternate )
{
  // Check first the current working directory then the default etc directory

  // Note: Any logging at this point will go to the default log file

  int32_t eResult{ RESULT_SUCCESS };

  // Use boost for platform independent file path
  std::filesystem::path fsFilePath( i_strFile );

  // Create config dictionary
  dictionary* pcDictionary{ iniparser_new( fsFilePath.string().c_str() ) };
  if( pcDictionary )
  {
    try
    {
      // Save the file used
      g_cConfigStruct.m_strEditorIni = fsFilePath.string();
      RUM_COUT( "Parsing " << g_cConfigStruct.m_strEditorIni << '\n' );

      g_cConfigStruct.m_strEditorLog = iniparser_getstring( pcDictionary, "editor:log",
                                                            const_cast<char*>( Logger::DEFAULT_LOG.c_str() ) );
      g_cConfigStruct.m_strProjectPath = iniparser_getstring( pcDictionary, "editor:project", DEFAULT_GAME_PATH );
    }
    catch( ... )
    {
      RUM_COUT( "Error encountered while parsing " << g_cConfigStruct.m_strEditorIni << '\n' );
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

  RUM_COUT( "Updating " << g_cConfigStruct.m_strEditorIni << '\n' );

  // The ini filename should already be in the native format
  dictionary* pcDictionary{ iniparser_new( g_cConfigStruct.m_strEditorIni.c_str() ) };
  if( pcDictionary )
  {
    try
    {
      iniparser_setstr( pcDictionary, "editor:log", const_cast<char *>( g_cConfigStruct.m_strEditorLog.c_str() ) );
      iniparser_setstr( pcDictionary, "editor:project",
                        const_cast<char *>( g_cConfigStruct.m_strProjectPath.c_str() ) );

      if( iniparser_dump_ini_file( pcDictionary, g_cConfigStruct.m_strEditorIni.c_str() ) == 0 )
      {
        eResult = RESULT_SUCCESS;
      }
    }
    catch( ... )
    {
    }

    iniparser_free( pcDictionary );
  }

  if( RESULT_FAILED == eResult )
  {
    Logger::LogStandard( "Failed to update " + g_cConfigStruct.m_strEditorIni );
  }

  return RESULT_SUCCESS;
}


const std::string& GetProjectPath()
{
  return g_cConfigStruct.m_strProjectPath;
}


void SetProjectPath( const std::string& i_strPath )
{
  std::filesystem::path fsPath( i_strPath );
  if( std::filesystem::is_regular_file( fsPath ) )
  {
    fsPath = fsPath.parent_path();
  }

  g_cConfigStruct.m_strProjectPath = fsPath.generic_string();
}


bool InEditor()
{
  return true;
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


int32_t ParseArgs( int32_t i_iArgc, char* i_pcArgv[] )
{
  // The config struct should already contain values loaded from ini file.
  // Arguments passed via the command line should override the stored value

  bool bShowUsage{ false };
  int32_t eResult{ RESULT_SUCCESS };

  for( int32_t i = 1; !bShowUsage && i < i_iArgc; ++i )
  {
    if( strcasecmp( "-project", i_pcArgv[i] ) == 0 )
    {
      g_cConfigStruct.m_strProjectPath = i_pcArgv[++i];
    }
    else if( strcasecmp( "-log", i_pcArgv[i] ) == 0 )
    {
      g_cConfigStruct.m_strEditorLog = i_pcArgv[++i];
    }
    else if( strcmp( "-?", i_pcArgv[i] ) == 0 ||
             strcasecmp( "-h", i_pcArgv[i] ) == 0 ||
             strcasecmp( "-help", i_pcArgv[i] ) == 0 )
    {
      bShowUsage = true;
    }
    else
    {
      RUM_COUT( "Unknown parameter: " << i_pcArgv[i] << '\n' );
      bShowUsage = true;
    }
  }

  if( bShowUsage )
  {
    MainWindow::ShowUsage();
    eResult = RESULT_FAILED;
  }

  return eResult;
}


void Shutdown()
{
  // Nothing to do
}


rumDatabase::DatabaseID GetDatabaseIDFromFilename( const std::string& i_strFile )
{
  // Move this into the db module
  rumAssert( false );
  const std::filesystem::path fsPath( i_strFile );

  const DatabaseMap::iterator& iter{ g_mapDatabases.find( fsPath.filename().string() ) };
  if( iter != g_mapDatabases.end() )
  {
    return iter->second;
  }

  return rumDatabase::DatabaseID::Invalid_DatabaseID;
}


int32_t InitDatabase()
{
  if( rumDatabase::Init() != RESULT_SUCCESS )
  {
    return RESULT_FAILED;
  }

  const std::filesystem::path fsPath( GetProjectPath() );

  using DatabaseID = rumDatabase::DatabaseID;

  struct EditorDatabases
  {
    std::string m_strFilename;
    DatabaseID m_eDatabaseID{ DatabaseID::Invalid_DatabaseID };
  };

  const std::vector<EditorDatabases> vDatabases
  {
    { "game.rum", DatabaseID::Game_DatabaseID }
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


int32_t ScriptLoad( const std::string& i_strFilePath )
{
  return rumScript::Load( i_strFilePath );
}


int32_t ScriptRequire( const std::string& i_strFilePath )
{
  return rumScript::Require( i_strFilePath );
}


Sqrat::Object ScriptModalDialog()
{
  Sqrat::Array sqArray;
  return MainWindow::ScriptModalDialog( sqArray );
}


void BindScripts()
{
  int32_t i{ 0 };

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::Table sqTable( pcVM );
  Sqrat::RootTable( pcVM ).Bind( "EDITOR", sqTable );

  Sqrat::RootTable( pcVM )
    .Func( "rumInEditor", InEditor )
    .Func( "rumLoadScript", ScriptLoad )
    .Func( "rumLoadFolder", ScriptLoad )
    .Func( "rumModalDialog", ScriptModalDialog )
    .Func( "rumRequireScript", ScriptRequire )
    .Func( "rumRequireFolder", ScriptRequire );

  Logger::ScriptBind();
  rumLanguage::ScriptBind();
  rumStringTable::ScriptBind();
  rumDataTable::ScriptBind();
  rumUtility::ScriptBind();
  rumEnum::ScriptBind();
  rumStructs::ScriptBind();
  rumScheduler::ScriptBind();
  rumPropertyContainer::ScriptBind();
  rumGameObject::ScriptBind();
  rumCustom::ScriptBind();
  EditorMap::ScriptBind();
  EditorPawn::ScriptBind();
  EditorInventory::ScriptBind();
  rumResource::ScriptBind();
  rumBroadcast::ScriptBind();
  zlib::ScriptBind();

  if( rumScript::GetCurrentVMType() != rumScript::VM_SERVER )
  {
    rumEditorControl::ScriptBind();
    EditorGraphic::ScriptBind();
    EditorSound::ScriptBind();
  }

  rumAsset::ScriptBind();
}
