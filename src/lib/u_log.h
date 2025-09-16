#pragma once

#include <platform.h>

#include <sqconfig.h>

#include <sstream>
#include <string>

#define THREADSAFE_LOGGING      1

#if THREADSAFE_LOGGING
#include <mutex>
#endif

// Switch category to 1 for debug output
#define CONTROL_DEBUG           0
#define DATABASE_DEBUG          0
#define GRAPHIC_DEBUG           0
#define INPUT_DEBUG             0
#define MAP_DRAW_DEBUG          0
#define MAP_LOAD_DEBUG          0
#define MEMORY_DEBUG            0
#define NETWORK_DEBUG           0
#define PAWN_SERIALIZE_DEBUG    0
#define PATHING_DEBUG           0
#define PROPERTY_DEBUG          0
#define SCHEDULER_DEBUG         0
#define SCRIPT_DEBUG            0
#define SOUND_DEBUG             0

// Colored output
#ifdef WIN32
  #define COLOR_STANDARD ( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE )
  #define COLOR_SERVER   ( FOREGROUND_GREEN )
  #define COLOR_WARNING  ( FOREGROUND_RED | FOREGROUND_GREEN )
  #define COLOR_ERROR    ( FOREGROUND_RED )
  #define COLOR_CLIENT   ( FOREGROUND_BLUE | FOREGROUND_INTENSITY )
#else
  #define COLOR_STANDARD  0 // WHITE
  #define COLOR_SERVER   32 // GREEN
  #define COLOR_WARNING  33 // YELLOW
  #define COLOR_ERROR    31 // RED
  #define COLOR_CLIENT   34 // BLUE
#endif


class Logger
{
public:
  enum LogType { LOG_INFO, LOG_WARNING, LOG_ERROR };

  static void Archive( const std::string& i_strFilePath, uint32_t i_uiThreshold = s_uiArchiveSizeThreshold );

  static void EnableEcho( bool i_bEcho )
  {
    s_bEcho = i_bEcho;
  }

  static void EnableLogging( bool i_bEnable )
  {
    s_bEnabled = i_bEnable;
  }

  static bool IsEchoEnabled()
  {
    return s_bEcho;
  }

  static int32_t LogStandard( const std::string& i_strEntry, LogType i_eLogType = LOG_INFO,
                              bool i_bForceEcho = false );
  static void LogStandard_VM( const SQChar* i_strOutput )
  {
    LogStandard( std::string( i_strOutput ) );
  }

  static int32_t LogChat( const std::string& i_strOutput );
  static void LogChat_VM( const SQChar* i_strOutput )
  {
    LogChat( std::string( i_strOutput ) );
  }

  static int32_t LogPlayer( const std::string& i_strOutput );
  static void LogPlayer_VM( const SQChar* i_strOutput )
  {
    LogPlayer( std::string( i_strOutput ) );
  }

  void LogChat( const SQChar* i_strOutput )
  {
    LogChat( std::string( i_strOutput ) );
  }


  static void ScriptBind();
  static void SetOutputColor( uint16_t i_uiColor );

  struct ConsoleOutput
  {
    ConsoleOutput& operator<< ( const std::string& i_strOutput );
    ConsoleOutput& operator<< ( char const* i_strOutput );

    template<typename T>
    ConsoleOutput& operator<< ( T const& i )
    {
#if THREADSAFE_LOGGING
      std::lock_guard<std::mutex> cLockGuard( s_cMutex );
#endif
      std::ostringstream cOutStringStream;
      cOutStringStream << i;
      *this << cOutStringStream.str();
      return *this;
    }
  };

  // The default log filename
  static const std::string DEFAULT_LOG;

  static std::string strStandardLog;
  static std::string strChatLog;
  static std::string strPlayerLog;

protected:

  // Also prints to a console window when true
  static bool s_bEcho;

  // Set to false to disable logging
  static bool s_bEnabled;

private:

  // Size at which we'll copy the log off to a backup file
  static constexpr uint32_t s_uiArchiveSizeThreshold{ 1024 * 1024 * 10 }; // 10MB;

#if THREADSAFE_LOGGING
  static std::mutex s_cMutex;
#endif
};

#define RUM_COUT(X) Logger::ConsoleOutput() << X
#define RUM_COUT_IFDEF(X, Y) if(X) { Logger::ConsoleOutput() << Y; }

#ifdef _DEBUG
  #define RUM_COUT_DBG(X) Logger::ConsoleOutput() << X
  #define RUM_COUT_IFDEF_DBG(X, Y) if(X) { Logger::ConsoleOutput() << Y; }
#else
  #define RUM_COUT_DBG(X)
  #define RUM_COUT_IFDEF_DBG(X, Y)
#endif // _DEBUG
