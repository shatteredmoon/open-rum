#pragma once

#include <filesystem>
#include <string>


// The interactable ImGui interface by which users debug attached Squirrel VMs. Through this interface, users can
// control which files are open, view file contents, query program variables and stack info, set breakpoints, and step
// through code when execution is paused.

namespace rumDebugInterface
{
  void Init( const std::string& i_strName, uint32_t i_iPort, const std::string& i_strScriptPath );

  void RequestSettingsUpdate();

  void SetFileFocus( const std::filesystem::path& i_fsFocusFile, int32_t i_iFocusLine );

  void Shutdown();

  void Update();

  bool WantsValuesAsHex();
} //  namespace rumDebugInterface
