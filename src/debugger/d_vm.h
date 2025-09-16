#pragma once

#include <d_breakpoint.h>
#include <d_context.h>
#include <d_file.h>
#include <d_variable.h>

#include <squirrel.h>

#include <condition_variable>
#include <map>
#include <mutex>

// The VM Manager for the Squirrel ImGui Debugger. This interface tracks attached VM Contexts, breakpoints, open
// files, and various variable groups. Code for this class is executed on the main thread and handles all Squirrel data
// access via requests from the Squirrel ImGui Interface, such as variables and callstack info.

namespace rumDebugVM
{
  SQInteger AttachVM( HSQUIRRELVM i_pcVM );
  SQInteger DetachVM( HSQUIRRELVM i_pcVM );

  void BreakpointAdd( rumDebugBreakpoint i_cBreakpoint );
  void BreakpointRemove( const rumDebugBreakpoint& i_rcBreakpoint );
  void BreakpointToggle( const rumDebugBreakpoint& i_rcBreakpoint );

  void EnableDebugInfo( HSQUIRRELVM i_pcVM, bool i_bEnable = true );

  void FileOpen( const std::filesystem::path& i_fsFilePath, uint32_t i_uiLine );
  void FileClose( const std::filesystem::path& i_fsFilePath );

  const std::vector<rumDebugBreakpoint> GetBreakpointsCopy();
  const std::vector<rumDebugBreakpoint>& GetBreakpointsRef();

  const rumDebugContext* GetCurrentDebugContext();
  const std::vector<rumDebugContext>& GetDebugContexts();

  const std::vector<rumDebugVariable>& GetLocalVariablesRef();

  const std::map<std::string, rumDebugFile> GetOpenedFilesCopy();
  const std::map<std::string, rumDebugFile>& GetOpenedFilesRef();

  const std::vector<rumDebugVariable>& GetRequestedVariablesRef();

  const std::vector<rumDebugVariable> GetWatchedVariablesCopy();
  const std::vector<rumDebugVariable>& GetWatchedVariablesRef();

  SQInteger IsDebuggerAttached( HSQUIRRELVM i_pcVM );

  void RegisterVM( HSQUIRRELVM i_pcVM, const std::string& i_strName );

  void RequestAttachVM( const std::string& i_strName );
  void RequestDetachVM( const std::string& i_strName );

  void RequestChangeStackLevel( uint32_t i_uiStackLevel );

  void RequestResume();
  void RequestStepInto();
  void RequestStepOut();
  void RequestStepOver();

  void RequestVariable( const rumDebugVariable& i_rcVariable );
  void RequestVariableUpdates();

  void Update();

  bool WatchVariableAdd( const std::string& i_strName );
  bool WatchVariableEdit( const rumDebugVariable& i_rcVariable, const std::string& i_strEdit );
  void WatchVariableRemove( const rumDebugVariable& i_rcVariable );
} // namespace rumDebugVM
