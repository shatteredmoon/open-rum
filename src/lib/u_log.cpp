#include <u_log.h>

#include <u_rum.h>
#include <u_script.h>
#include <u_timer.h>
#include <u_utility.h>

#include <filesystem>
#include <fstream>
#include <iostream>


// Static initializers
const std::string Logger::DEFAULT_LOG{ "rum.log" };
std::string Logger::strChatLog{ Logger::DEFAULT_LOG };
std::string Logger::strPlayerLog{ Logger::DEFAULT_LOG };
std::string Logger::strStandardLog{ Logger::DEFAULT_LOG };
bool Logger::s_bEnabled{ true };

#if THREADSAFE_LOGGING
std::mutex Logger::s_cMutex;
#endif

#ifdef _DEBUG
bool Logger::s_bEcho{ true };
#else
bool Logger::s_bEcho{ false };
#endif


void Logger::Archive( const std::string& i_strFilename, uint32_t i_uiThreshold )
{
  const std::filesystem::path fsPath( i_strFilename );

  // Check log file sizes
  if( exists( fsPath ) )
  {
    const uintmax_t uiSize{ std::filesystem::file_size( fsPath ) };
    if( uiSize > i_uiThreshold )
    {
      char strTimestamp[32];
      rumTimer::GetTimestampFormatted( strTimestamp, "%Y-%m-%d__%H-%M-%S", 32 );

      std::string strNewFilename{ strTimestamp };
      strNewFilename += "__";
      strNewFilename += fsPath.string();
      rename( fsPath.string().c_str(), i_strFilename.c_str() );
    }
  }
}


Logger::ConsoleOutput& Logger::ConsoleOutput::operator<< ( const std::string& i_strOutput )
{
  std::cout << i_strOutput;
  return *this;
}


Logger::ConsoleOutput& Logger::ConsoleOutput::operator<< ( char const* i_strOutput )
{
  std::cout << i_strOutput;
  return *this;
}


#pragma message ("TODO: LogChat and LogPlayer should only exist server-side?")
int32_t Logger::LogChat( const std::string& i_strOutput )
{
  std::ofstream cOutfile;
  cOutfile.open( strChatLog.c_str(), std::ios::app );
  if( cOutfile.is_open() )
  {
    char strTimestamp[32];
    rumTimer::GetTimestamp( strTimestamp );
    cOutfile << "[" << strTimestamp << "] " << i_strOutput << std::endl;
    cOutfile.close();
  }

  return RESULT_SUCCESS;
}


int32_t Logger::LogPlayer( const std::string& i_strOutput )
{
  std::ofstream cOutfile;
  cOutfile.open( strPlayerLog.c_str(), std::ios::app );
  if( cOutfile.is_open() )
  {
    char strTimestamp[32];
    rumTimer::GetTimestamp( strTimestamp );
    cOutfile << "[" << strTimestamp << "] " << i_strOutput << std::endl;
    cOutfile.close();
  }

  return RESULT_SUCCESS;
}


int32_t Logger::LogStandard( const std::string& i_strOutput, LogType i_eLogType, bool i_bForceEcho )
{
  if( !s_bEnabled )
  {
    return RESULT_FAILED;
  }

  int32_t eResult{ RESULT_SUCCESS };

  if( s_bEcho || i_bForceEcho )
  {
    // Print error message on server console
    switch( i_eLogType )
    {
      default:
      case LOG_INFO:
        std::cout << i_strOutput << std::endl;
        break;

      case LOG_WARNING:
        SetOutputColor( COLOR_WARNING );
        std::cout << i_strOutput << std::endl;
        SetOutputColor( COLOR_STANDARD );
        break;

      case LOG_ERROR:
        SetOutputColor( COLOR_ERROR );
        std::cout << i_strOutput << std::endl;
        SetOutputColor( COLOR_STANDARD );
        break;
    }
  }

  std::ofstream cOutfile;
  cOutfile.open( strStandardLog.c_str(), std::ios::app );
  if( cOutfile.is_open() )
  {
    char strTimestamp[32];
    rumTimer::GetTimestamp( strTimestamp );
    cOutfile << "[" << strTimestamp << "] " << i_strOutput << std::endl;
    cOutfile.close();
  }
  else
  {
    std::cout << "Failed to log: " << i_strOutput << std::endl;
    eResult = RESULT_FAILED;
  }

  return eResult;
}


void Logger::ScriptBind()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::RootTable( pcVM )
    .Func( "rumLog", LogStandard_VM )
    .Func( "rumLogChat", LogChat_VM )
    .Func( "rumLogPlayer", LogPlayer_VM );
}


void Logger::SetOutputColor( uint16_t i_uiColor )
{
#ifdef WIN32
  SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), (WORD)i_uiColor );
#else
  printf( "\x1B[%hdm", i_rcColor );
#endif
}
