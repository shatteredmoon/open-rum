/*

R/U/M Construction Kit Portal

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

#include <p_rum.h>

#include <u_db.h>
#include <u_log.h>
#include <u_rum.h>
#include <u_utility.h>

#include <filesystem>

// Qt
#include <QApplication>
#include <ui/mainwindow.h>

#include <iniparser.h>

#include <fstream>
#include <iostream>

uint64_t g_uiFrameIndex{ 0UL };

// A generic buffer for global reuse
const int32_t g_uiBufferSize = 1024 * 1024;
char g_cBuffer[g_uiBufferSize];

int32_t configInit( const std::string&, rumConfig &, bool = true );
int32_t configUpdate( const rumConfig & );

int32_t LogInit( const std::string& = Logger::DEFAULT_LOG );
int32_t parseArgs( int32_t argc, char* argv[], rumConfig & );

void SetWindowTitle( MainWindow &rMainWindow, const char *desc = "" );
void ShowUsage();


int32_t main( int32_t argc, char *argv[], char *envp[] )
{
  rumConfig cConfig;

  QApplication app( argc, argv );

  // This is required to exist for Qt plugins
  QApplication::addLibraryPath( QApplication::applicationDirPath() + "/plugins" );

  // -----------------------------------------------------------------------
  // NOTE: Any logging at this point will go to the default log file
  // -----------------------------------------------------------------------

  if( configInit( DEFAULT_CONFIG DEFAULT_CONFIG_EXT, cConfig ) != RESULT_SUCCESS )
  {
    return RESULT_FAILED;
  }

  // Handle command line early for usage requests
  if( argc > 1 )
  {
    // Parse command line arguments, store arguments in config struct
    if( parseArgs( argc, argv, cConfig ) != RESULT_SUCCESS )
    {
      return RESULT_FAILED;
    }
  }

  // -----------------------------------------------------------------------
  // NOTE: Any logging before this point will go to the default log file
  // -----------------------------------------------------------------------
  // This should be done as early in the program as possible. Here, the log initialization is delayed until we at least
  // load possible log settings from the ini file or command line.
  if( LogInit( cConfig.m_strPortalLog ) != RESULT_SUCCESS )
  {
    return RESULT_FAILED;
  }

  MainWindow cMainWindow( cConfig );
  cMainWindow.Init( argc, argv );
  SetWindowTitle( cMainWindow );
  cMainWindow.show();

  // Hand off to Qt
  return app.exec();
}


double GetElapsedTime()
{
  // Unused by RUM Portal
  return 0.0;
}


uint64_t GetFrameIndex()
{
  return g_uiFrameIndex;
}


int32_t parseArgs( int32_t argc, char* argv[], rumConfig& i_rcConfig )
{
  // The config struct should already contain values loaded from ini file.
  // Arguments passed via the command line should override the stored value.

  bool bShowUsage = false;
  int32_t result = RESULT_SUCCESS;

  for( int32_t i = 1; bShowUsage == false && i < argc; ++i )
  {
    if( strcasecmp( "-download", argv[i] ) == 0 )
    {
      i_rcConfig.m_eDownloadType = Download_Full;
    }
    else if( strcasecmp( "-log", argv[i] ) == 0 )
    {
      i_rcConfig.m_strPortalLog = argv[++i];
    }
    else if( strcasecmp( "-patch", argv[i] ) == 0 )
    {
      i_rcConfig.m_eDownloadType = Download_Patch;
    }
    else if( strcasecmp( "-repair", argv[i] ) == 0 )
    {
      i_rcConfig.m_eDownloadType = Download_Repair;
    }
    else if( strcasecmp( "-start", argv[i] ) == 0 )
    {
      i_rcConfig.m_bAutoStart = true;
    }
    else if( strcasecmp( "-uuid", argv[i] ) == 0 )
    {
      i_rcConfig.m_strUuid = argv[++i];
    }
    else if( strcmp( "-?", argv[i] ) == 0 ||
             strcasecmp( "-h", argv[i] ) == 0 ||
             strcasecmp( "-help", argv[i] ) == 0 )
    {
      bShowUsage = true;
    }
    else
    {
      std::cout << "Unknown parameter: " << argv[i] << std::endl;
      bShowUsage = true;
    }
  }

  if( bShowUsage )
  {
    ShowUsage();
    result = RESULT_FAILED;
  }

  return result;
}


void ShowUsage()
{
  std::cout << std::endl;
  std::cout << "Launches " << PROGRAM_LONG_DESC << std::endl;
  std::cout << std::endl;

#ifdef WIN32
  std::cout << "RUM_PORTAL [-?|-h|-help] | [-uuid <uuid>] | [-download] | [-patch] | [-repair] | [-log <file>] |"
    << "           [-start] | [-platform <windows|mac|linux>]" << std::endl;
  std::cout << std::endl;
  std::cout << "-?, -h, -help" << std::endl;
  std::cout << "   Displays program execution requirements and command-line parameter help" << std::endl;
  std::cout << "-download" << std::endl;
  std::cout << "   Starts a full client download specified by -uuid and -platform" << std::endl;
  std::cout << "-patch" << std::endl;
  std::cout << "   Updates game files for the game specified by -uuid and -platform" << std::endl;
  std::cout << "-repair" << std::endl;
  std::cout << "   Attempts to fix a downloaded game by downloading missing files and cleaning scripts" << std::endl;
  std::cout << "-uuid <uuid>" << std::endl;
  std::cout << "   Required by -download, -patch, -repair, and -start to specify the target game" << std::endl;
  std::cout << "-log <file>" << std::endl;
  std::cout << "   Specifies output file the portal will use for logging events and errors" << std::endl;
  std::cout << "   Default is " << Logger::DEFAULT_LOG << std::endl;
  std::cout << "-platform <windows|mac|linux>" << std::endl;
  std::cout << "   Specifies the desired operating system for downloads" << std::endl;
  std::cout << std::endl;
#else
  // Other platforms should display their specific launch process here
#error UNHANDLED PLATFORM
#endif
}


int32_t configInit( const std::string& i_strFile, rumConfig& i_rcConfig, bool i_bUseAlternate )
{
  // Check first the current working directory then the default etc directory

  // Note: Any logging at this point will go to the default log file

  int32_t eResult{ RESULT_SUCCESS };

  // Use boost for platform independent file path
  std::filesystem::path fsFile( i_strFile );

  // Create config dictionary
  dictionary* pcDictionary{ iniparser_new( fsFile.string().c_str() ) };
  if( pcDictionary != nullptr )
  {
    try
    {
      // Save the file used
      i_rcConfig.m_strPortalINI = fsFile.string();
      std::cout << "Parsing " << i_rcConfig.m_strPortalINI << std::endl;

      i_rcConfig.m_strPortalLog =
        iniparser_getstring( pcDictionary, "portal:log", const_cast<char *>( Logger::DEFAULT_LOG.c_str() ) );

      i_rcConfig.m_iWebPort = iniparser_getint( pcDictionary, "web:port", DEFAULT_WEB_PORT );
      i_rcConfig.m_strWebAddress = iniparser_getstring( pcDictionary, "web:url", DEFAULT_WEB_ADDR );

      eResult = RESULT_SUCCESS;
    }
    catch( ... )
    {
      std::cout << "Error encountered while parsing " << i_rcConfig.m_strPortalINI << std::endl;
      eResult = RESULT_FAILED;
    }

    iniparser_free( pcDictionary );
  }
  else if( i_bUseAlternate )
  {
    // Try ETC directory ... (if applicable)
    if( DEFAULT_PATH_ETC != "" )
    {
      eResult = configInit( DEFAULT_PATH_ETC DEFAULT_CONFIG DEFAULT_CONFIG_EXT, i_rcConfig, false );
    }
  }

  return eResult;
}


int32_t configUpdate( const rumConfig& i_rcConfig )
{
  int32_t eResult{ RESULT_FAILED };

  std::cout << "Updating " << i_rcConfig.m_strPortalINI << std::endl;

  // The ini filename should already be in the native format
  dictionary* pcDictionary{ iniparser_new( i_rcConfig.m_strPortalINI.c_str() ) };
  if( pcDictionary != nullptr )
  {
    try
    {
      iniparser_setstr( pcDictionary, "portal:log", const_cast<char *>( i_rcConfig.m_strPortalLog.c_str() ) );

      iniparser_setstr( pcDictionary, "web:url", const_cast<char *>( i_rcConfig.m_strWebAddress.c_str() ) );
      iniparser_setstr( pcDictionary, "web:port",
                        const_cast<char *>( rumStringUtils::ToString( i_rcConfig.m_iWebPort ) ) );

      if( iniparser_dump_ini_file( pcDictionary, i_rcConfig.m_strPortalINI.c_str() ) == 0 )
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
    Logger::LogStandard( "Failed to update " + i_rcConfig.m_strPortalINI );
  }

  return RESULT_SUCCESS;
}


int32_t LogInit( const std::string& i_strStandardLog )
{
  std::filesystem::path fsLog( i_strStandardLog );
  Logger::strStandardLog = fsLog.string();

  std::cout << "Using program log: " << Logger::strStandardLog << std::endl;

  // Test the log location and specify that the mod is starting
  int32_t eResult = Logger::LogStandard( PROGRAM_LONG_DESC " version " QUOTED( PROGRAM_MAJOR_VERSION ) "."
                                         QUOTED( PROGRAM_MINOR_VERSION ) );
  if( eResult == RESULT_FAILED )
  {
    std::cout << "Warning: Log strStandardLog " << Logger::strStandardLog << " is invalid." << std::endl;

    if( i_strStandardLog.compare( Logger::DEFAULT_LOG ) != 0 )
    {
      // Switch back to the default log file
      eResult = LogInit( Logger::DEFAULT_LOG );
    }
    else
    {
      // Disable logging
      std::cout << "Warning: Logging will not be available for this run" << std::endl;
      Logger::EnableLogging( false );
    }
  }

  Logger::LogStandard( LINE_BREAK );

  return eResult;
}


void SetWindowTitle( MainWindow& i_rcMainWindow, const char* i_strDesc )
{
  QString strTitle = QString( PROGRAM_LONG_DESC " v%1.%2" ).arg( PROGRAM_MAJOR_VERSION ).arg( PROGRAM_MINOR_VERSION );

#ifdef _DEBUG
  strTitle += " (DEBUG)";
#endif

  // Append passed in description
  if( i_strDesc != nullptr )
  {
    strTitle += " ";
    strTitle += i_strDesc;
  }

  i_rcMainWindow.setWindowTitle( strTitle );
}


const std::string& GetProjectPath()
{
  //if( nullptr != pcConfig )
  //{
  //    return pcConfig->game_path;
  //}

  return rumStringUtils::NullString();
}
