#include <chrono>
using namespace std::chrono;

#ifndef _U_TIMER_H_
#define _U_TIMER_H_

template <class Duration>
using sys_time_steady = time_point<steady_clock, Duration>;


class rumTimer
{
public:

  rumTimer()
    : m_tTime( time_point_cast<milliseconds>( steady_clock::now() ) )
  {}

  int64_t GetElapsedMilliseconds() const
  {
    return ( steady_clock::now() - m_tTime ).count();
  }

  double GetElapsedSeconds() const
  {
    return duration<double>( steady_clock::now() - m_tTime ).count();
  }

  void Restart()
  {
    m_tTime = time_point_cast<milliseconds>( steady_clock::now() );
  }

  static uint64_t GetSecondsSinceEpoch()
  {
    return duration_cast<seconds>( system_clock::now().time_since_epoch() ).count();
  }

  static void GetTimestamp( char* o_strBuffer )
  {
    constexpr int32_t iMaxSize{ 32 };
    GetTimestampFormatted( o_strBuffer, "%Y-%m-%d %H:%M:%S", iMaxSize );
  }

  static void GetTimestampFormatted( char* o_strBuffer, const char* i_strFormat, int32_t i_iMaxSize )
  {
    const time_t uiRawTime{ time( nullptr ) };
    const struct tm* pcTime{ localtime( &uiRawTime ) };
    strftime( o_strBuffer, sizeof( char ) * i_iMaxSize, i_strFormat, pcTime );
  }

  static void ScriptBind();

private:

  sys_time_steady<milliseconds> m_tTime;
};

#endif // _U_TIMER_H_
