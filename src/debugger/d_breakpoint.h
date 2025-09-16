#pragma once

#include <filesystem>

// Represents a script breakpoint set by the user

struct rumDebugBreakpoint
{
  rumDebugBreakpoint()
    : m_uiLine( 0 )
    , m_bEnabled( false )
  {}

  rumDebugBreakpoint( const std::filesystem::path& i_fsFilepath, uint32_t i_uiLine, bool i_bEnabled = true )
    : m_fsFilepath( i_fsFilepath )
    , m_uiLine( i_uiLine )
    , m_bEnabled( i_bEnabled )
  {}

  bool operator==( const rumDebugBreakpoint& i_rcBreakpoint ) const
  {
    return( i_rcBreakpoint.m_uiLine == m_uiLine && i_rcBreakpoint.m_fsFilepath == m_fsFilepath );
  }

  std::filesystem::path m_fsFilepath;
  uint32_t m_uiLine{ 0 };
  bool m_bEnabled{ true };
};
