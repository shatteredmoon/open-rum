#pragma once

// Represents a Squirrel variable interpreted as string data for display in the Squirrel ImGui Interface

struct rumDebugVariable
{
  std::string m_strName;
  std::string m_strType;
  std::string m_strValue;

  bool operator==( const rumDebugVariable& i_rcVariable ) const
  {
    return( m_strName.compare( i_rcVariable.m_strName ) == 0 );
  }

  bool operator==( const std::string& i_strName ) const
  {
    return( m_strName.compare( i_strName ) == 0 );
  }
};
