#pragma once

#include <squirrel.h>

#include <filesystem>
#include <queue>
#include <vector>

using HSQUIRRELCONSTVM = SQVM const*;

// Represents a Squirrel VM Context that has been attached to the VM Manager, and its various state attributes

struct rumDebugContext
{
  rumDebugContext( HSQUIRRELVM i_pcVM ) : m_pcVM( i_pcVM )
  {}

  bool operator==( HSQUIRRELCONSTVM i_pcVM ) const
  {
    return( m_pcVM == i_pcVM );
  }

  enum class StepDirective
  {
    Resume,
    StepOver,
    StepInto,
    StepOut
  };

  struct CallstackEntry
  {
    int32_t m_iLine{ 0 };
    std::string m_strFilename;
    std::string m_strFunction;
  };

  // A pointer to the VM
  HSQUIRRELVM m_pcVM{ nullptr };

  // A friendly name for the context
  std::string m_strName;

  // The last known callstack
  std::vector<CallstackEntry> m_vCallstack;

  // The current file the VM is paused on
  std::filesystem::path m_fsPausedFile;

  // The last issued step directive
  StepDirective m_eStepDirective{ StepDirective::Resume };

  // The current line the VM is paused at
  uint32_t m_uiPausedLine{ 0 };

  // The last known stack level when a step directive was issued
  uint32_t m_uiStepDirectiveStackLevel{ 0 };

  // Whether or not the context is attached or detached
  bool m_bAttached{ false };

  // Whether or not the VM is paused
  bool m_bPaused{ false };

  // Should the update focus on the paused instruction pointer?
  bool m_bFocusOnCurrentInstruction{ false };

  // Has there been a variable request or stack level change?
  bool m_bUpdateVariables{ false };
};
