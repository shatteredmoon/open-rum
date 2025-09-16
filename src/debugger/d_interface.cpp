/*

Squirrel ImGui Debugger Interface

MIT License

Copyright 2022 Jonathon Blake Wood-Brooks

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

#include <d_interface.h>

#include <d_breakpoint.h>
#include <d_settings.h>
#include <d_utility.h>
#include <d_variable.h>
#include <d_vm.h>

#include <mutex>
#include <regex>

#include <imgui_internal.h>
#include <NetImgui_Api.h>
#include <private/NetImgui_CmdPackets.h>

// Make the GImGui pointer thread-local so that multiple contexts can run (see imconfig.h)
thread_local ImGuiContext* g_pcImGuiTLSContext{ nullptr };


namespace rumDebugInterface
{
  // The user-set path to debuggable scripts
  std::string g_strScriptPath;

  // The file that should have tab focus
  std::filesystem::path g_fsFocusFile;

  // The line that should be shown
  int32_t g_iFocusLine{ -1 };

  // Lock guard
  std::mutex g_mtxLockGuard;

  // Breakpoint colors
  using ImU32 = unsigned int;
  ImU32 g_uiEnabledBreakpointColor{ 0 };
  ImU32 g_uiDisabledBreakpointColor{ 0 };

  // Should settings be saved to .ini?
  bool g_bUpdateSettings{ false };

  // Show integer values as hex
  bool g_bShowHex{ false };


  ///////////////
  // Prototypes
  ///////////////

  void DisplayCode( const std::string& i_strSource, uint32_t i_uiLine,
                    bool i_bInMultilineComment );
  void DisplayVariable( const rumDebugVariable& i_rcVariable );

  void DoVariableExpansion( const rumDebugVariable& i_rcVariable );

  size_t FindNthOccurrence( const std::string_view i_strSource, const std::string_view i_strFind,
                            size_t i_szOccurence, size_t i_szOffset = 0 );

  rumDebugVariable GetVariable( const std::string& i_strName );

  void Settings_ReadLine( ImGuiContext* i_pcContext, ImGuiSettingsHandler* i_pcSettingsHandler, void* i_pcEntry,
                          const char* i_strLine );
  void* Settings_ReadOpen( ImGuiContext* i_pcContext, ImGuiSettingsHandler* i_pcSettingsHandler,
                           const char* i_strName );
  void Settings_WriteAll( ImGuiContext* i_pcContext, ImGuiSettingsHandler* i_pcSettingsHandler,
                          ImGuiTextBuffer* io_pcBuffer );

  void UpdateBreakpointTab();
  void UpdateDisplayFolder( const std::string& i_strFolder,
                            std::filesystem::recursive_directory_iterator& i_rcIter,
                            std::filesystem::recursive_directory_iterator& i_rcIterEnd,
                            const std::string& i_strFilter );
  void UpdateFileExplorer();
  void UpdateLocalsTab();
  void UpdateKeyDirectives();
  void UpdatePrimaryRow( float i_fHeight );
  void UpdateSecondaryRow();
  void UpdateSettings();
  void UpdateSkipChildren( const std::string& i_strFolder,
                           std::filesystem::recursive_directory_iterator& i_rcIter,
                           std::filesystem::recursive_directory_iterator& i_rcIterEnd );
  void UpdateSourceCode();
  void UpdateStackBreakpointWindow();
  void UpdateStackTab();
  void UpdateVMsTab();
  void UpdateWatchLocalWindow();
  void UpdateWatchTab();


  void DisplayVariable( const rumDebugVariable& i_rcVariable )
  {
    ImGui::TableNextRow();

    // The local variable name
    ImGui::TableNextColumn();
    ImGui::TextUnformatted( i_rcVariable.m_strName.c_str() );

    // The local variable type
    ImGui::TableNextColumn();
    ImGui::TextUnformatted( i_rcVariable.m_strType.c_str() );

    // The local variable value
    ImGui::TableNextColumn();

    // Preview the results if there are more than 3 lines
    const auto szOffset{ FindNthOccurrence( i_rcVariable.m_strValue, "\n", NUM_VARIABLE_PREVIEW_LINES ) };
    if( szOffset == std::string::npos )
    {
      ImGui::TextUnformatted( i_rcVariable.m_strValue.c_str() );
    }
    else
    {
      ImGui::TextUnformatted( i_rcVariable.m_strValue.substr( 0, szOffset ).c_str() );
      DoVariableExpansion( i_rcVariable );
    }
  }


  void DoVariableExpansion( const rumDebugVariable& i_rcDebugVariable )
  {
    std::string strButtonID{ "...##" + i_rcDebugVariable.m_strName };
    if( ImGui::SmallButton( strButtonID.c_str() ) )
    {
      ImGui::OpenPopup( i_rcDebugVariable.m_strName.c_str() );
    }

    ImGui::SetNextWindowSize( { 200.0f, 300.0f }, ImGuiCond_FirstUseEver );
    if( ImGui::BeginPopupModal( i_rcDebugVariable.m_strName.c_str(), nullptr,
                                ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_HorizontalScrollbar ) )
    {
      if( ImGui::SmallButton( "Copy" ) )
      {
        ImGui::SetClipboardText( i_rcDebugVariable.m_strValue.c_str() );
      }

      ImGui::SameLine();
      if( ImGui::SmallButton( "Close" ) )
      {
        ImGui::CloseCurrentPopup();
      }

      ImGui::Text( i_rcDebugVariable.m_strType.c_str() );
      ImGui::Separator();
      ImGui::TextUnformatted( i_rcDebugVariable.m_strValue.c_str() );

      ImGui::EndPopup();
    }
  }


  size_t FindNthOccurrence( const std::string_view i_strSource, const std::string_view i_strFind,
                            size_t i_szOccurence, size_t i_szOffset )
  {
    size_t szPos = i_strSource.find( i_strFind, i_szOffset );
    if( ( 0 == i_szOccurence ) || ( std::string::npos == szPos ) )
    {
      return szPos;
    }

    return FindNthOccurrence( i_strSource, i_strFind, i_szOccurence - 1, szPos + 1 );
  }


  void DisplayCode( const std::string& i_strSource, uint32_t i_uiLine, bool i_bInMultilineComment )
  {
    constexpr ImVec4 uiCommentColor{ 0.34f, 0.65f, 0.29f, 1.0f };
    constexpr ImVec4 uiOperatorColor{ 0.8f, 0.8f, 0.0f, 1.0f };
    constexpr ImVec4 uiReservedWordColor{ 0.34f, 0.61f, 0.76f, 1.0f };
    constexpr ImVec4 uiStringColor{ 0.84f, 0.62f, 0.46f, 1.0f };

    bool bInComment{ false };
    bool bInDoubleQuotes{ false };
    bool bInSingleQuotes{ false };

    std::regex rxSeparators( R"(/\*|\*/|\\'|\\"|//|[():;=+-, !^&*\[\]\|\\'\"<>?~])" );
    std::sregex_token_iterator iter( i_strSource.begin(), i_strSource.end(), rxSeparators, { -1, 0 } );
    std::vector<std::string> vTokens;
    std::remove_copy_if( iter, std::sregex_token_iterator(),
                         std::back_inserter( vTokens ),
                         []( std::string const& i_strToken )
    {
      return i_strToken.empty();
    } );

    auto pcContext{ rumDebugVM::GetCurrentDebugContext() };

    int32_t iIndex{ 0 };
    for( const auto& token : vTokens )
    {
      if( iIndex > 0 )
      {
        ImGui::SameLine( 0.0f, 0.0f );
      }

      if( i_bInMultilineComment || ( token.compare( "/*" ) == 0 ) || ( token.compare( "*/" ) == 0 ) )
      {
        if( token.compare( "/*" ) == 0 )
        {
          i_bInMultilineComment = true;
        }
        else if( token.compare( "*/" ) == 0 )
        {
          i_bInMultilineComment = false;
        }

        ImGui::PushStyleColor( ImGuiCol_Text, uiCommentColor );
        ImGui::TextUnformatted( token.c_str() );
        ImGui::PopStyleColor();
      }
      else if( bInComment || ( token.compare( "//" ) == 0 ) )
      {
        bInComment = true;
        ImGui::PushStyleColor( ImGuiCol_Text, uiCommentColor );
        ImGui::TextUnformatted( token.c_str() );
        ImGui::PopStyleColor();
      }
      else if( bInDoubleQuotes || ( token.compare( "\"" ) == 0 ) )
      {
        if( token.compare( "\"" ) == 0 )
        {
          bInDoubleQuotes = !bInDoubleQuotes;
        }

        ImGui::PushStyleColor( ImGuiCol_Text, uiStringColor );
        ImGui::TextUnformatted( token.c_str() );
        ImGui::PopStyleColor();
      }
      else if( bInSingleQuotes || ( token.compare( "'" ) == 0 ) )
      {
        if( token.compare( "'" ) == 0 )
        {
          bInSingleQuotes = !bInSingleQuotes;
        }

        ImGui::PushStyleColor( ImGuiCol_Text, uiStringColor );
        ImGui::TextUnformatted( token.c_str() );
        ImGui::PopStyleColor();
      }
      else if( rumDebugUtility::IsReservedWord( token ) )
      {
        ImGui::PushStyleColor( ImGuiCol_Text, uiReservedWordColor );
        ImGui::TextUnformatted( token.c_str() );
        ImGui::PopStyleColor();
      }
      else if( rumDebugUtility::IsOperator( token ) )
      {
        ImGui::PushStyleColor( ImGuiCol_Text, uiOperatorColor );
        ImGui::TextUnformatted( token.c_str() );
        ImGui::PopStyleColor();
      }
      else
      {
        ImGui::BeginGroup();
        ImGui::TextUnformatted( token.c_str() );

        if( pcContext && pcContext->m_bPaused )
        {
          if( ImGui::IsItemHovered() )
          {
            // Only show tooltip info for potential variable names
            std::regex rxToken( R"([a-zA-Z_]+)" );
            if( std::regex_search( token, rxToken ) )
            {
              rumDebugVariable cVariable{ GetVariable( token ) };
              ImGui::BeginTooltip();
              constexpr ImGuiTableFlags eTableFlags{ ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Borders |
                                                     ImGuiTableFlags_NoSavedSettings };
              constexpr int32_t iNumColumns{ 3 };
              if( ImGui::BeginTable( "LocalsTable", iNumColumns, eTableFlags ) )
              {
                DisplayVariable( cVariable );
                ImGui::EndTable();
              }
              ImGui::EndTooltip();
            }
          }

          // Build a unique ID for the token base on the line and index
          std::string strID( token + "_" );
          strID += std::to_string( i_uiLine );
          strID += "_";
          strID += std::to_string( iIndex );

          if( ImGui::BeginPopupContextItem( strID.c_str() ) )
          {
            if( ImGui::SmallButton( "Copy Name" ) )
            {
              ImGui::SetClipboardText( token.c_str() );
              ImGui::CloseCurrentPopup();
            }
            else if( ImGui::SmallButton( "Copy Value" ) )
            {
              rumDebugVariable cVariable{ GetVariable( token ) };
              ImGui::SetClipboardText( cVariable.m_strValue.c_str() );
              ImGui::CloseCurrentPopup();
            }
            else if( ImGui::SmallButton( "Watch" ) )
            {
              rumDebugVM::WatchVariableAdd( token );
              ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
          }
        }

        ImGui::EndGroup();
      }

      ++iIndex;
    }
  }


  rumDebugVariable GetVariable( const std::string& i_strVariableName )
  {
    rumDebugVariable cVariable;

    // Check local variables
    const auto& rcvLocalVariables{ rumDebugVM::GetLocalVariablesRef() };
    const auto& localIter{ std::find( rcvLocalVariables.begin(), rcvLocalVariables.end(), i_strVariableName ) };
    if( localIter != rcvLocalVariables.end() )
    {
      cVariable = *localIter;
    }
    else
    {
      // Check watched variables
      const auto& rcvWatchedVariables{ rumDebugVM::GetWatchedVariablesRef() };
      const auto& watchIter{ std::find( rcvWatchedVariables.begin(), rcvWatchedVariables.end(), i_strVariableName ) };
      if( watchIter != rcvWatchedVariables.end() )
      {
        cVariable = *watchIter;
      }
      else
      {
        // Check the recently requested variables
        const auto& rcvRequestedVariables{ rumDebugVM::GetRequestedVariablesRef() };
        const auto& requestedIter{ std::find( rcvRequestedVariables.begin(), rcvRequestedVariables.end(), i_strVariableName ) };
        if( requestedIter != rcvRequestedVariables.end() )
        {
          cVariable = *requestedIter;
        }
        else
        {
          cVariable.m_strName = i_strVariableName;
          rumDebugVM::RequestVariable( cVariable );
        }
      }
    }

    return cVariable;
  }


  void Init( const std::string& i_strName, uint32_t i_iPort, const std::string& i_strScriptPath )
  {
    using namespace NetImgui::Internal;

    g_pcImGuiTLSContext = ImGui::CreateContext();

    ImGuiIO& rImGuiIO{ ImGui::GetIO() };

    ImGuiSettingsHandler ini_handler;
    ini_handler.TypeName = "UserData";
    ini_handler.TypeHash = ImHashStr( "UserData" );
    ini_handler.ReadOpenFn = Settings_ReadOpen; // Called when entering into a new ini entry e.g. "[Window][Name]"
    ini_handler.ReadLineFn = Settings_ReadLine; // Called for every line of text within an ini entry
    ini_handler.WriteAllFn = Settings_WriteAll; // Output every entries into 'out_buf'

    g_pcImGuiTLSContext->SettingsHandlers.push_back( ini_handler );

    // Add support for function keys
    rImGuiIO.KeyMap[ImGuiKey_G] = static_cast<int32_t>( CmdInput::eVirtualKeys::vkKeyboardA ) + ( ImGuiKey_G - ImGuiKey_A );
    rImGuiIO.KeyMap[ImGuiKey_F] = static_cast<int32_t>( CmdInput::eVirtualKeys::vkKeyboardA ) + ( ImGuiKey_F - ImGuiKey_A );
    rImGuiIO.KeyMap[ImGuiKey_F5] = static_cast<int32_t>( CmdInput::eVirtualKeys::vkKeyboardSuperF1 ) + ( ImGuiKey_F5 - ImGuiKey_F1 );
    rImGuiIO.KeyMap[ImGuiKey_F9] = static_cast<int32_t>( CmdInput::eVirtualKeys::vkKeyboardSuperF1 ) + ( ImGuiKey_F9 - ImGuiKey_F1 );
    rImGuiIO.KeyMap[ImGuiKey_F10] = static_cast<int32_t>( CmdInput::eVirtualKeys::vkKeyboardSuperF1 ) + ( ImGuiKey_F10 - ImGuiKey_F1 );
    rImGuiIO.KeyMap[ImGuiKey_F11] = static_cast<int32_t>( CmdInput::eVirtualKeys::vkKeyboardSuperF1 ) + ( ImGuiKey_F11 - ImGuiKey_F1 );

    rImGuiIO.DisplaySize.x = static_cast<float>( DEBUGGER_DISPLAY_WIDTH );
    rImGuiIO.DisplaySize.y = static_cast<float>( DEBUGGER_DISPLAY_HEIGHT );

    int32_t iWidth, iHeight;
    unsigned char* pcPixels{ nullptr };
    rImGuiIO.Fonts->GetTexDataAsRGBA32( &pcPixels, &iWidth, &iHeight );

    NetImgui::Startup();
    NetImgui::ConnectFromApp( i_strName.c_str(), i_iPort );

    g_strScriptPath = i_strScriptPath;

    g_uiEnabledBreakpointColor = ImGui::GetColorU32( { 0.4f, 0.0f, 0.0f, 1.0f } );
    g_uiDisabledBreakpointColor = ImGui::GetColorU32( { 0.4f, 0.4f, 0.0f, 1.0f } );
  }


  void RequestSettingsUpdate()
  {
    std::lock_guard<std::mutex> cLockGuard( g_mtxLockGuard );
    g_bUpdateSettings = true;
  }


  void SetFileFocus( const std::filesystem::path& i_fsFocusFile, int32_t i_iFocusLine )
  {
    std::lock_guard<std::mutex> cLockGuard( g_mtxLockGuard );
    g_fsFocusFile = i_fsFocusFile;
    g_iFocusLine = i_iFocusLine;
  }


  void Settings_ReadLine( [[maybe_unused]] ImGuiContext* i_pcContext,
                          [[maybe_unused]] ImGuiSettingsHandler* i_pcSettingsHandler,
                          [[maybe_unused]] void* i_pcEntry,
                          const char* i_strLine )
  {
    std::string strLine( i_strLine );
    if( strLine.find_first_of( "Breakpoint" ) == 0 )
    {
      size_t szLineNumberStart{ strLine.find_first_of( '=' ) + 1 };
      size_t szBreakpointEnabled{ strLine.find_first_of( ',' ) + 1 };
      size_t szFilePathStart{ strLine.find( ',', szBreakpointEnabled ) + 1 };

      if( szLineNumberStart != std::string::npos && szFilePathStart != std::string::npos )
      {
        std::string strLineNumber{ strLine.substr( szLineNumberStart, szBreakpointEnabled - szLineNumberStart - 1 ) };
        std::string strBreakpointEnabled{ strLine.substr( szBreakpointEnabled, 1 ) };
        std::filesystem::path fsFilePath{ strLine.substr( szFilePathStart ) };

        rumDebugBreakpoint cBreakpoint( fsFilePath, std::stoi( strLineNumber ),
                                        strBreakpointEnabled.compare( "1" ) == 0 ? true : false );
        rumDebugVM::BreakpointAdd( cBreakpoint );
      }
    }
    else if( strLine.find_first_of( "File" ) == 0 )
    {
      constexpr int32_t iFocusLineNumber{ 0 };
      std::filesystem::path fsFilePath{ strLine.substr( strLine.find_first_of( "=" ) + 1 ) };
      rumDebugVM::FileOpen( fsFilePath, iFocusLineNumber );
    }
    else if( strLine.find_first_of( "WatchVariable" ) == 0 )
    {
      rumDebugVM::WatchVariableAdd( strLine.substr( strLine.find_first_of( "=" ) + 1 ) );
    }
  }


  void* Settings_ReadOpen( [[maybe_unused]] ImGuiContext* i_pcContext,
                           [[maybe_unused]] ImGuiSettingsHandler* i_pcSettingsHandler,
                           const char* i_strName )
  {
    ImGuiWindowSettings* pcSettings{ ImGui::FindOrCreateWindowSettings( i_strName ) };
    ImGuiID id{ pcSettings->ID };

    // Clear existing if recycling previous entry
    *pcSettings = ImGuiWindowSettings();
    pcSettings->ID = id;
    pcSettings->WantApply = true;

    return (void*)pcSettings;
  }


  void Settings_WriteAll( [[maybe_unused]] ImGuiContext* i_pcContext,
                          [[maybe_unused]] ImGuiSettingsHandler* i_pcSettingsHandler,
                          ImGuiTextBuffer* io_pcBuffer )
  {
    // Ballpark reserve
    io_pcBuffer->reserve( io_pcBuffer->size() + 1000 );
    io_pcBuffer->appendf( "[UserData][Script Debugger]\n" );

    uint32_t uiBreakpointIndex{ 1 };
    for( const auto& iter : rumDebugVM::GetBreakpointsRef() )
    {
      io_pcBuffer->appendf( "Breakpoint%d=%d,%d,%s\n",
                            uiBreakpointIndex++, iter.m_uiLine, iter.m_bEnabled ? 1 : 0,
                            iter.m_fsFilepath.generic_string().c_str() );
#if DEBUG_OUTPUT
      std::cout << "Saving Breakpoint: " << iter.m_uiLine << '\n';
#endif
    }

    uint32_t uiFileIndex{ 1 };
    for( const auto& iter : rumDebugVM::GetOpenedFilesRef() )
    {
      io_pcBuffer->appendf( "File%d=%s\n", uiFileIndex++, iter.second.m_fsFilePath.generic_string().c_str() );
#if DEBUG_OUTPUT
      std::cout << "Saving File: " << iter.second.m_fsFilePath.generic_string() << '\n';
#endif
    }

    uint32_t uiWatchVariableIndex{ 1 };
    for( const auto& iter : rumDebugVM::GetWatchedVariablesRef() )
    {
      io_pcBuffer->appendf( "WatchVariable%d=%s\n", uiWatchVariableIndex++, iter.m_strName.c_str() );
#if DEBUG_OUTPUT
      std::cout << "Saving WatchVariable: " << iter.m_strName << '\n';
#endif
    }
  }


  void Shutdown()
  {
    NetImgui::Shutdown();

    if( g_pcImGuiTLSContext )
    {
      ImGui::DestroyContext( g_pcImGuiTLSContext );
      g_pcImGuiTLSContext = nullptr;
    }
  }


  void Update()
  {
    ImGuiIO& rcIO{ ImGui::GetIO() };

    // TODO - pass in the actual amount
    rcIO.DeltaTime = 1.0f / 60.0f; // set the time elapsed since the previous frame (in seconds)

#if DEBUG_OUTPUT
    for( ImGuiKey key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_COUNT; key++ )
    {
      if( ImGui::IsKeyReleased( key ) )
      {
        std::cout << "Key down: " << ImGui::GetKeyName( key ) << '\n';
      }
    }
#endif // DEBUG_OUTPUT

    UpdateKeyDirectives();

    NetImgui::NewFrame();

    constexpr float fMaxWindowWidth{ 3840.0f };
    constexpr float fMaxWindowHeight{ 2160.0f };
    constexpr float fMinWindowWidth{ 800.0f };
    constexpr float fMinWindowHeight{ 600.0f };

    ImGui::SetNextWindowSizeConstraints( { fMinWindowWidth, fMinWindowHeight }, { fMaxWindowWidth, fMaxWindowHeight } );

    static bool bOpen{ true };
    if( ImGui::Begin( "Script Debugger", &bOpen, ImGuiWindowFlags_NoScrollbar ) )
    {
      // A table split by variable inspection, breakpoints, and callstack
      constexpr int32_t numColumns{ 1 };
      if( ImGui::BeginTable( "MainTable", numColumns, ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp ) )
      {
        ImGui::TableNextRow();

        const auto cRegion{ ImGui::GetWindowContentRegionMax() };

        ImGui::TableNextColumn();
        UpdatePrimaryRow( cRegion.y * 0.7f ); // 70% of available height

        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        UpdateSecondaryRow();

        // MainTable
        ImGui::EndTable();
      }
    }

    // Script Debugger window
    ImGui::End();

    //ImGui::ShowDemoWindow();

    NetImgui::EndFrame();

    if( g_bUpdateSettings )
    {
      UpdateSettings();
    }
  }


  void UpdateBreakpointTab()
  {
    if( ImGui::BeginTabItem( "Breakpoints##TabItem" ) )
    {
      const ImVec2 vRegion{ ImGui::GetContentRegionAvail() };
      const ImVec2 cSize{ 0.0f, std::max( 0.0f, vRegion.y - 2.0f ) };
      ImGui::BeginChild( "BreakpointsTabChild", cSize, false, ImGuiWindowFlags_HorizontalScrollbar );

      // Fetch by copy here because there is potential to modify the list during iteration
      const auto cvBreakpoints{ rumDebugVM::GetBreakpointsCopy() };
      if( cvBreakpoints.empty() )
      {
        ImGui::TextUnformatted( "No breakpoints set" );
      }
      else
      {
        constexpr int32_t iNumColumns{ 3 };
        constexpr ImGuiTableFlags eTableFlags{ ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp |
                                               ImGuiTableFlags_Borders | ImGuiTableFlags_NoSavedSettings };
        if( ImGui::BeginTable( "BreakpointsTable", iNumColumns, eTableFlags ) )
        {
          rumDebugBreakpoint cRemovedBreakpoint;
          bool bBreakpointRemoved{ false };

          for( const auto& iter : cvBreakpoints )
          {
            ImGui::TableNextRow();

            // The breakpoint enabled/disabled status
            ImGui::TableNextColumn();
            ImGui::TableSetBgColor( ImGuiTableBgTarget_CellBg,
                                    iter.m_bEnabled ? g_uiEnabledBreakpointColor : g_uiDisabledBreakpointColor );
            ImGui::TextUnformatted( " * " );
            if( ImGui::IsItemHovered() )
            {
              if( ImGui::IsKeyPressed( ImGuiKey_F9 ) || ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) )
              {
                rumDebugVM::BreakpointToggle( iter );
              }
              else if( ImGui::IsKeyPressed( ImGuiKey_Delete ) )
              {
                // Schedule for removal since we're mid-iteration
                cRemovedBreakpoint = iter;
                bBreakpointRemoved = true;
              }
            }

            // The breakpoint line number
            ImGui::TableNextColumn();
            ImGui::Text( "Line %d", iter.m_uiLine );
            if( ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) )
            {
              rumDebugVM::FileOpen( iter.m_fsFilepath, iter.m_uiLine );
            }

            // The breakpoint source
            ImGui::TableNextColumn();
            ImGui::TextUnformatted( iter.m_fsFilepath.filename().generic_string().c_str() );
            if( ImGui::IsItemHovered() )
            {
              ImGui::SetTooltip( iter.m_fsFilepath.generic_string().c_str() );

              if( ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) )
              {
                rumDebugVM::FileOpen( iter.m_fsFilepath, iter.m_uiLine );
              }
            }
          }

          if( bBreakpointRemoved )
          {
            rumDebugVM::BreakpointRemove( cRemovedBreakpoint );
          }

          // BreakpointsTable
          ImGui::EndTable();
        }
      }

      // BreakpointsTabChild
      ImGui::EndChild();

      // BreakpointsTabItem
      ImGui::EndTabItem();
    }
  }


  void UpdateDisplayFolder( const std::string& i_strFolder,
                            std::filesystem::recursive_directory_iterator& i_rcIter,
                            std::filesystem::recursive_directory_iterator& i_rcIterEnd,
                            const std::string& i_strFilter )
  {
    while( i_rcIter != i_rcIterEnd )
    {
      auto strPath{ i_rcIter->path().generic_string() };

      auto strChildPath{ strPath };
      if( strChildPath.find( i_strFolder ) == std::string::npos )
      {
        break;
      }

      if( std::filesystem::is_directory( i_rcIter->path() ) )
      {

        if( ImGui::TreeNode( i_rcIter->path().stem().generic_string().c_str() ) )
        {
          // The tree node is expanded, so show its contents
          UpdateDisplayFolder( strPath, ++i_rcIter, i_rcIterEnd, i_strFilter );
          ImGui::TreePop();
        }
        else
        {
          // The tree node is collapsed, so skip its contents
          UpdateSkipChildren( strPath, i_rcIter, i_rcIterEnd );
        }
      }
      else
      {
        std::string strFilename{ i_rcIter->path().filename().generic_string() };

        if( i_strFilter.empty() || strFilename.find( i_strFilter ) != std::string::npos )
        {
          if( ImGui::Selectable( strFilename.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick ) )
          {
            rumDebugVM::FileOpen( i_rcIter->path(), 0 );
          }
        }

        ++i_rcIter;
      }
    }
  }


  void UpdateFileExplorer()
  {
    static char strCFilter[MAX_FILENAME_LENGTH];

    // Adds a text input field for filtering file and folder names
    ImGui::InputText( "##FileFilter", strCFilter, IM_ARRAYSIZE( strCFilter ) );

    // Adds a clear button for clearing the file filter
    ImGui::SameLine();
    if( ImGui::Button( "Clear" ) )
    {
      memset( strCFilter, '\0', sizeof( char ) * MAX_FILENAME_LENGTH );
    }

    const ImVec2 vRegion{ ImGui::GetContentRegionAvail() };
    const ImVec2 cSize{ 0.0f, std::max( 0.0f, vRegion.y - 2.0f ) };
    ImGui::BeginChild( "FileExplorerTabChild", cSize, false, ImGuiWindowFlags_HorizontalScrollbar );

    std::string strFilter( strCFilter );

    std::filesystem::path fsPath( g_strScriptPath );
    std::filesystem::recursive_directory_iterator cEnd, cDir( fsPath );
    while( cDir != cEnd )
    {
      UpdateDisplayFolder( cDir->path().generic_string(), cDir, cEnd, strFilter );
    }

    // FileExplorerTabChild
    ImGui::EndChild();
  }


  void UpdateLocalsTab()
  {
    auto pcContext{ rumDebugVM::GetCurrentDebugContext() };
    if( !pcContext )
    {
      return;
    }

    if( ImGui::BeginTabItem( "Locals##TabItem" ) )
    {
      const ImVec2 vRegion{ ImGui::GetContentRegionAvail() };
      const ImVec2 cSize{ 0.0f, std::max( 0.0f, vRegion.y - 2.0f ) };
      ImGui::BeginChild( "LocalsTabChild", cSize, false, ImGuiWindowFlags_HorizontalScrollbar );

      if( pcContext->m_bPaused )
      {
        constexpr int32_t iNumColumns{ 3 };
        constexpr ImGuiTableFlags eTableFlags{ ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp |
                                               ImGuiTableFlags_Borders | ImGuiTableFlags_NoSavedSettings };
        if( ImGui::BeginTable( "LocalsTable", iNumColumns, eTableFlags ) )
        {
          const auto& rcvLocalVariables{ rumDebugVM::GetLocalVariablesRef() };
          for( const auto& iter : rcvLocalVariables )
          {
            DisplayVariable( iter );
          }

          // LocalsTable
          ImGui::EndTable();
        }
      }
      else
      {
        ImGui::TextUnformatted( "Running" );
      }

      // LocalsTabChild
      ImGui::EndChild();

      // LocalsTabItem
      ImGui::EndTabItem();
    }
  }


  void UpdateKeyDirectives()
  {
    auto pcContext{ rumDebugVM::GetCurrentDebugContext() };
    if( !pcContext || !pcContext->m_bPaused )
    {
      return;
    }

    ImGuiIO& rcIO{ ImGui::GetIO() };

    if( ImGui::IsKeyPressed( ImGuiKey_F5 ) )
    {
#if DEBUG_OUTPUT
      std::cout << "Resuming\n";
#endif
      rumDebugVM::RequestResume();
    }
    else if( ImGui::IsKeyPressed( ImGuiKey_F10 ) )
    {
#if DEBUG_OUTPUT
      std::cout << "Step over\n";
#endif
      rumDebugVM::RequestStepOver();
    }
    else if( ImGui::IsKeyPressed( ImGuiKey_F11 ) )
    {
      if( rcIO.KeyShift )
      {
#if DEBUG_OUTPUT
        std::cout << "Step out\n";
#endif
        rumDebugVM::RequestStepOut();
      }
      else
      {
#if DEBUG_OUTPUT
        std::cout << "Step into\n";
#endif
        rumDebugVM::RequestStepInto();
      }
    }
  }


  void UpdatePrimaryRow( float i_fHeight )
  {
    // A table split by a file explorer on the left and source code on the right
    constexpr ImGuiTableFlags eTableFlags{ ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders |
                                           ImGuiTableFlags_ScrollY };
    constexpr int32_t iNumColumns{ 2 };
    if( ImGui::BeginTable( "PrimaryRow", iNumColumns, eTableFlags, { 0.0f, i_fHeight } ) )
    {
      ImGui::TableNextRow();

      ImGui::TableNextColumn();
      UpdateFileExplorer();

      ImGui::TableNextColumn();
      UpdateSourceCode();

      // PrimaryRow
      ImGui::EndTable();
    }
  }


  void UpdateSecondaryRow()
  {
    constexpr ImGuiTableFlags eTableFlags{ ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders |
                                           ImGuiTableFlags_ScrollY };
    constexpr int32_t iNumColumns{ 2 };
    if( ImGui::BeginTable( "SecondaryRow", iNumColumns, eTableFlags ) )
    {
      ImGui::TableNextRow();

      ImGui::TableNextColumn();
      UpdateWatchLocalWindow();

      ImGui::TableNextColumn();
      UpdateStackBreakpointWindow();

      // SecondaryRow
      ImGui::EndTable();
    }
  }


  void UpdateSettings()
  {
    ImGuiContext& rcContext{ *GImGui };
    if( rcContext.SettingsLoaded )
    {
      ImGui::SaveIniSettingsToDisk( rcContext.IO.IniFilename );

      std::lock_guard<std::mutex> cLockGuard( g_mtxLockGuard );
      g_bUpdateSettings = false;
    }
  }


  void UpdateSkipChildren( const std::string& i_strFolder,
                           std::filesystem::recursive_directory_iterator& i_rcIter,
                           std::filesystem::recursive_directory_iterator& i_rcIterEnd )
  {
    while( ++i_rcIter != i_rcIterEnd )
    {
      auto strChildPath{ i_rcIter->path().generic_string() };
      if( strChildPath.find( i_strFolder ) == std::string::npos )
      {
        break;
      }
    }
  }


  void UpdateSourceCode()
  {
    static ImU32 uiCurrentLineColor{ ImGui::GetColorU32( { 0.2f, 0.4f, 0.7f, 0.5f } ) };
    static const ImU32 uiFindTextLineColor{ ImGui::GetColorU32( { 1.0f, 1.0f, 0.0f, 0.5f } ) };

    static char strFindText[MAX_FILENAME_LENGTH];
    static int32_t iFindFileOffset{ 0 };
    static int32_t iFindLineOffset{ 0 };

    const ImGuiIO& rcIO{ ImGui::GetIO() };

    const auto pcDebugContext{ rumDebugVM::GetCurrentDebugContext() };

    if( pcDebugContext )
    {
      if( pcDebugContext->m_bPaused )
      {
        const bool bHasFocus{ ImGui::IsWindowFocused( ImGuiFocusedFlags_ChildWindows ) };

        // Provide suspended context control buttons so that the user can resume or step as needed
        if( ( bHasFocus && ImGui::IsKeyPressed( ImGuiKey_F5 ) ) || ImGui::Button( "Resume" ) )
        {
          rumDebugVM::RequestResume();
        }

        if( ImGui::IsItemHovered() )
        {
          ImGui::SetTooltip( "F5" );
        }

        ImGui::SameLine();

        if( ( bHasFocus && ImGui::IsKeyPressed( ImGuiKey_F10 ) ) || ImGui::Button( "Step Over" ) )
        {
          rumDebugVM::RequestStepOver();
        }

        if( ImGui::IsItemHovered() )
        {
          ImGui::SetTooltip( "F10" );
        }

        ImGui::SameLine();

        if( ( bHasFocus && ImGui::IsKeyPressed( ImGuiKey_F11 ) ) || ImGui::Button( "Step Into" ) )
        {
          rumDebugVM::RequestStepInto();
        }

        if( ImGui::IsItemHovered() )
        {
          ImGui::SetTooltip( "F11" );
        }

        ImGui::SameLine();

        if( ( bHasFocus && rcIO.KeyShift && ImGui::IsKeyPressed( ImGuiKey_F11 ) ) || ImGui::Button( "Step Out" ) )
        {
          rumDebugVM::RequestStepOut();
        }

        if( ImGui::IsItemHovered( ImGuiHoveredFlags_AllowWhenDisabled ) )
        {
          ImGui::SetTooltip( "Shift+F11" );
        }
      }
    }

    if( ImGui::BeginTabBar( "OpenedFiles", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_FittingPolicyScroll ) )
    {
      // Fetch by copy here because there is potential to modify the list during iteration
      // #TODO - There is a lot of data being copied - work out a threadsafe share
      const auto cvOpenedFiles{ rumDebugVM::GetOpenedFilesCopy() };
      for( const auto& fileIter : cvOpenedFiles )
      {
        const rumDebugFile& rcFile{ fileIter.second };

        bool bSetFocus{ false };
        if( !g_fsFocusFile.empty() && ( rcFile.m_fsFilePath.compare( g_fsFocusFile ) == 0 ) )
        {
          bSetFocus = true;
        }

        bool bTabOpened{ true };
        if( ImGui::BeginTabItem( rcFile.m_strFilename.c_str(), &bTabOpened,
                                 bSetFocus ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None ) )
        {
          const ImVec2 vRegion{ ImGui::GetContentRegionAvail() };
          const ImVec2 cSize{ 0.0f, std::max( 0.0f, vRegion.y - 2.0f ) };
          ImGui::BeginChild( "SourceCodeTabChild", cSize, false, ImGuiWindowFlags_HorizontalScrollbar );

          if( bSetFocus )
          {
            g_fsFocusFile.clear();
          }

          std::map<uint32_t, bool> vLinesWithBreakpoints;

          // Parse the breakpoints for matching lines
          const auto& rcvBreakpoints{ rumDebugVM::GetBreakpointsRef() };
          for( const auto& breakpointIter : rcvBreakpoints )
          {
            if( breakpointIter.m_fsFilepath == rcFile.m_fsFilePath )
            {
              vLinesWithBreakpoints.insert( std::pair( breakpointIter.m_uiLine, breakpointIter.m_bEnabled ) );
            }
          }

          constexpr ImGuiTableFlags eTableFlags{ ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable |
                                                 ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY |
                                                 ImGuiTableFlags_NoSavedSettings };
          constexpr int32_t iNumColumns{ 2 };
          if( ImGui::BeginTable( "SourceCode", iNumColumns, eTableFlags ) )
          {
            ImGui::TableSetupColumn( "Line", ImGuiTableColumnFlags_WidthFixed, ImGui::GetFontSize() * 3.0f );
            ImGui::TableSetupColumn( "Source", ImGuiTableColumnFlags_WidthFixed, ImGui::GetFontSize() * rcFile.m_uiLongestLine );
            ImGui::TableSetupScrollFreeze( 1, 0 );

            const int32_t iNumLines{ static_cast<int32_t>( rcFile.m_vStringOffsets.size() ) };
            const bool bHasFocus{ ImGui::IsWindowFocused( ImGuiFocusedFlags_ChildWindows ) };

            if( bHasFocus && ImGui::IsKeyPressed( ImGuiKey_Escape ) )
            {
              // Clear the existing find text results
              strFindText[0] = '\0';
              iFindFileOffset = 0;
              iFindLineOffset = 0;
            }

            // #TODOJBW - Implement "Find Next" with F3
            if( bHasFocus && rcIO.KeyCtrl && ImGui::IsKeyPressed( ImGuiKey_F ) )
            {
              ImGui::OpenPopup( "Find In File" );
            }

            constexpr ImVec2 cButtonSize{ 120, 0 };

            if( ImGui::BeginPopupModal( "Find In File", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
            {
              ImGui::TextUnformatted( "Find:" );

              if( !ImGui::IsAnyItemFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked( 0 ) )
              {
                ImGui::SetKeyboardFocusHere( 0 );
              }

              ImGui::InputText( "##FindText", strFindText, IM_ARRAYSIZE( strFindText ) );

              if( ImGui::Button( "OK", cButtonSize ) ||
                  ( bHasFocus && ( ImGui::IsKeyPressed( ImGuiKey_Enter ) ||
                                   ImGui::IsKeyPressed( ImGuiKey_KeyPadEnter ) ) ) )
              {
                // #TODOJBW - provide a more advanced implementation
                if( iFindFileOffset >= static_cast<int32_t>( rcFile.m_strData.size() ) )
                {
                  iFindFileOffset = 0;
                }

                // #TODOJBW - implement case insensitive search
                const auto findOffset{ rcFile.m_strData.find( strFindText, iFindFileOffset ) };
                if( findOffset != std::string::npos )
                {
                  // Determine the line offset by counting the occurrences of \n up to the find index
                  const std::string_view sourceCodeView{ rcFile.m_strData };
                  const std::string_view sourceCodeSubstring{ sourceCodeView.substr( 0, findOffset ) };
                  iFindLineOffset = static_cast<int32_t>( std::count( sourceCodeSubstring.begin(),
                                                                      sourceCodeSubstring.end(),
                                                                      '\n' ) + 1 );

                  // Save the existing offset, but nudge it forward so that a consecutive find will
                  // find the next occurrence of the string, not the one we're currently sitting at
                  iFindFileOffset = static_cast<int32_t>( findOffset + 1 );
                }
                else
                {
                  // EOF
                  iFindFileOffset = static_cast<int32_t>( rcFile.m_strData.size() );
                  iFindLineOffset = iNumLines - 1;
                }

                g_fsFocusFile = rcFile.m_fsFilePath;
                g_iFocusLine = iFindLineOffset;

                ImGui::CloseCurrentPopup();
              }

              ImGui::SameLine();

              if( ImGui::Button( "Cancel", cButtonSize ) )
              {
                ImGui::CloseCurrentPopup();
              }

              // Find In File
              ImGui::EndPopup();
            }

            if( bHasFocus && rcIO.KeyCtrl && ImGui::IsKeyPressed( ImGuiKey_G ) )
            {
              ImGui::OpenPopup( "Go To Line" );
            }

            if( ImGui::BeginPopupModal( "Go To Line", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
            {
              static int32_t iGotoLine{ 1 };

              ImGui::Text( "Line number (1 - %d)", static_cast<int32_t>( iNumLines ) );

              if( !ImGui::IsAnyItemFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked( 0 ) )
              {
                ImGui::SetKeyboardFocusHere( 0 );
              }

              ImGui::InputInt( "##GotoLine", &iGotoLine );

              if( ImGui::Button( "OK", cButtonSize ) ||
                  ( bHasFocus && ( ImGui::IsKeyPressed( ImGuiKey_Enter ) ||
                                   ImGui::IsKeyPressed( ImGuiKey_KeyPadEnter ) ) ) )
              {
                // Clamp the input values to the valid range
                if( iGotoLine < 1 )
                {
                  iGotoLine = 1;
                }
                else if( iGotoLine > static_cast<int32_t>( iNumLines ) )
                {
                  iGotoLine = static_cast<int32_t>( iNumLines );
                }

                SetFileFocus( rcFile.m_fsFilePath, iGotoLine );
                ImGui::CloseCurrentPopup();
              }

              ImGui::SameLine();

              if( ImGui::Button( "Cancel", cButtonSize ) )
              {
                ImGui::CloseCurrentPopup();
              }

              // Go To Line
              ImGui::EndPopup();
            }

            // Show the table using a clipper since file sizes can be very large
            ImGuiListClipper cClipper;
            cClipper.Begin( (int32_t)iNumLines );

            if( bSetFocus && g_iFocusLine > 0 )
            {
              cClipper.ForceDisplayRangeByIndices( g_iFocusLine - 1, g_iFocusLine );
            }

            // Skip the header row
            while( cClipper.Step() )
            {
              for( int32_t iRow{ cClipper.DisplayStart }; iRow < cClipper.DisplayEnd; ++iRow )
              {
                bool bInMultilineComment{ false };
                int32_t iLine{ iRow + 1 };

                // Determine if this line is inside of a multiline comment
                for( const auto& commentIter : rcFile.m_vMultilineComments )
                {
                  if( iLine > commentIter.m_iStartLine && iLine < commentIter.m_iEndLine )
                  {
                    bInMultilineComment = true;
                    break;
                  }
                }

                ImGui::TableNextRow();

                // The line number column
                ImGui::TableNextColumn();

                const auto& iter{ vLinesWithBreakpoints.find( iLine ) };
                if( iter != vLinesWithBreakpoints.end() )
                {
                  ImGui::TableSetBgColor( ImGuiTableBgTarget_CellBg,
                                          iter->second ? g_uiEnabledBreakpointColor : g_uiDisabledBreakpointColor );
                }

                ImGui::Text( "%d", iLine );

                // Calculate the column bounds that can take mouse-clicks
                auto vItemRectMin{ ImGui::GetItemRectMin() };
                auto vItemRectMax{ ImGui::GetItemRectMax() };
                vItemRectMax.x += ImGui::GetColumnWidth();

                if( ImGui::IsMouseHoveringRect( vItemRectMin, vItemRectMax ) )
                {
                  constexpr bool bEnabled{ true };

                  if( ( bHasFocus && ImGui::IsKeyPressed( ImGuiKey_F9 ) ) ||
                      ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) )
                  {
                    rumDebugBreakpoint cBreakpoint( rcFile.m_fsFilePath, iLine, bEnabled );
                    rumDebugVM::BreakpointToggle( cBreakpoint );
                  }
                  else if( ImGui::IsKeyPressed( ImGuiKey_Delete ) )
                  {
                    rumDebugBreakpoint cBreakpoint( rcFile.m_fsFilePath, iLine, bEnabled );
                    rumDebugVM::BreakpointRemove( cBreakpoint );
                  }
                }

                // The source code column
                ImGui::TableNextColumn();

                if( ( iRow + 1 ) < static_cast<int32_t>( rcFile.m_vStringOffsets.size() ) )
                {
                  size_t iBegin{ rcFile.m_vStringOffsets[iRow] };
                  size_t iEnd{ rcFile.m_vStringOffsets[iRow + 1] - 1 };
                  std::string strLine{ rcFile.m_strData.substr( iBegin, iEnd - iBegin ) };

                  const auto pcContext{ rumDebugVM::GetCurrentDebugContext() };
                  if( pcContext && pcContext->m_bPaused &&
                      ( pcContext->m_uiPausedLine == static_cast<uint32_t>( iLine ) ) &&
                      ( pcContext->m_fsPausedFile.compare( rcFile.m_fsFilePath ) == 0 ) )
                  {
                    ImGui::TableSetBgColor( ImGuiTableBgTarget_CellBg, uiCurrentLineColor );
                  }

                  if( strFindText[0] != '\0' && ( iLine == iFindLineOffset ) )
                  {
                    // Highlight the line containing the string typed in the find dialog
                    ImGui::TableSetBgColor( ImGuiTableBgTarget_CellBg, uiFindTextLineColor );
                  }

                  ImGui::BeginGroup();
                  DisplayCode( strLine, iLine, bInMultilineComment );
                  ImGui::EndGroup();

                  // Calculate the column bounds that can take mouse-clicks
                  vItemRectMin = ImGui::GetItemRectMin();
                  vItemRectMax = ImGui::GetItemRectMax();
                  vItemRectMax.x += ImGui::GetColumnWidth();

                  if( ImGui::IsMouseHoveringRect( vItemRectMin, vItemRectMax ) )
                  {
                    constexpr bool bEnabled{ true };

                    if( bHasFocus && ImGui::IsKeyPressed( ImGuiKey_F9 ) )
                    {
                      rumDebugBreakpoint cBreakpoint{ rcFile.m_fsFilePath, static_cast<uint32_t>( iLine ), bEnabled };
                      rumDebugVM::BreakpointToggle( cBreakpoint );
                    }
                    else if( bHasFocus && ImGui::IsKeyPressed( ImGuiKey_Delete ) )
                    {
                      rumDebugBreakpoint cBreakpoint{ rcFile.m_fsFilePath, static_cast<uint32_t>( iLine ), bEnabled };
                      rumDebugVM::BreakpointRemove( cBreakpoint );
                    }
                  }

                  if( bSetFocus && g_iFocusLine == iLine )
                  {
                    g_iFocusLine = -1;
                    ImGui::SetScrollHereY();
                  }
                }
              }
            }

            cClipper.End();

            // SourceCode
            ImGui::EndTable();
          }

          // BreakpointsTabChild
          ImGui::EndChild();

          // Filename
          ImGui::EndTabItem();
        }

        if( !bTabOpened )
        {
          rumDebugVM::FileClose( rcFile.m_fsFilePath );
        }
      }

      // OpenedFiles
      ImGui::EndTabBar();
    }
  }


  void UpdateStackBreakpointWindow()
  {
    const ImVec2 vRegion{ ImGui::GetContentRegionAvail() };
    const ImVec2 cSize{ 0.0f, std::max( 0.0f, vRegion.y - 2.0f ) };
    ImGui::BeginChild( "StackAndBreakpointsChild", cSize, false, ImGuiWindowFlags_HorizontalScrollbar );

    if( ImGui::BeginTabBar( "StackAndBreakpointsChildTabBar", ImGuiTabBarFlags_None ) )
    {
      UpdateStackTab();
      UpdateBreakpointTab();
      UpdateVMsTab();

      // StackAndBreakpointsTabBar
      ImGui::EndTabBar();
    }

    // StackAndBreakpointsChild
    ImGui::EndChild();
  }


  void UpdateStackTab()
  {
    if( ImGui::BeginTabItem( "Callstack##TabItem" ) )
    {
      const ImVec2 vRegion{ ImGui::GetContentRegionAvail() };
      const ImVec2 cSize{ 0.0f, std::max( 0.0f, vRegion.y - 2.0f ) };

      ImGui::BeginChild( "CallstackTabChild", cSize, false, ImGuiWindowFlags_HorizontalScrollbar );

      auto pcContext{ rumDebugVM::GetCurrentDebugContext() };
      if( pcContext )
      {
        if( !pcContext->m_bPaused )
        {
          ImGui::TextUnformatted( "Running" );
        }
        else
        {
          constexpr ImGuiTableFlags eTableFlags{ ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp |
                                                 ImGuiTableFlags_Borders | ImGuiTableFlags_NoSavedSettings };
          constexpr int32_t iNumColumns{ 3 };
          if( ImGui::BeginTable( "CallstackTable", iNumColumns, eTableFlags ) )
          {
            uint32_t uiStackIndex{ 0 };
            for( const auto& iterCallstack : pcContext->m_vCallstack )
            {
              ImGui::TableNextRow();

              // The stack line number
              ImGui::TableNextColumn();
              ImGui::Text( "Line %d", iterCallstack.m_iLine );
              if( ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) )
              {
                rumDebugVM::FileOpen( iterCallstack.m_strFilename, iterCallstack.m_iLine );
                rumDebugVM::RequestChangeStackLevel( uiStackIndex );
              }

              // The stack source file
              ImGui::TableNextColumn();
              ImGui::TextUnformatted( iterCallstack.m_strFilename.c_str() );
              if( ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) )
              {
                rumDebugVM::FileOpen( iterCallstack.m_strFilename, iterCallstack.m_iLine );
                rumDebugVM::RequestChangeStackLevel( uiStackIndex );
              }

              // The function
              ImGui::TableNextColumn();
              ImGui::TextUnformatted( iterCallstack.m_strFunction.c_str() );
              if( ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) )
              {
                rumDebugVM::FileOpen( iterCallstack.m_strFilename, iterCallstack.m_iLine );
                rumDebugVM::RequestChangeStackLevel( uiStackIndex );
              }

              ++uiStackIndex;
            }

            // CallstackTable
            ImGui::EndTable();
          }
        }
      }

      // CallstackTabChild
      ImGui::EndChild();

      // Callstack##TabItem
      ImGui::EndTabItem();
    }
  }


  void UpdateWatchLocalWindow()
  {
    const ImVec2 vRegion{ ImGui::GetContentRegionAvail() };
    const ImVec2 cSize{ 0.0f, std::max( 0.0f, vRegion.y - 2.0f ) };
    ImGui::BeginChild( "WatchAndLocalsChild", cSize, false, ImGuiWindowFlags_HorizontalScrollbar );

    //ImGui::BeginGroup();
    if( ImGui::BeginTabBar( "WatchAndLocalsChildTabBar", ImGuiTabBarFlags_None ) )
    {
      UpdateWatchTab();
      UpdateLocalsTab();

      // WatchAndLocalsChildTabBar
      ImGui::EndTabBar();
    }
    //ImGui::EndGroup();

    ImGui::SameLine();
    if( ImGui::Checkbox( "Show Hex", &g_bShowHex ) )
    {
      rumDebugVM::RequestVariableUpdates();
    }

    // WatchAndLocalsChild
    ImGui::EndChild();
  }


  void UpdateVMsTab()
  {
    if( ImGui::BeginTabItem( "VMs##TabItem" ) )
    {
      const ImVec2 vRegion{ ImGui::GetContentRegionAvail() };
      const ImVec2 cSize{ 0.0f, std::max( 0.0f, vRegion.y - 2.0f ) };
      ImGui::BeginChild( "VMsTabChild", cSize, false, ImGuiWindowFlags_HorizontalScrollbar );

      constexpr ImGuiTableFlags eTableFlags{ ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders |
                                             ImGuiTableFlags_ScrollY | ImGuiTableFlags_ContextMenuInBody |
                                             ImGuiTableFlags_NoSavedSettings };
      constexpr int32_t iNumColumns{ 2 };
      if( ImGui::BeginTable( "VMsTable", iNumColumns, eTableFlags ) )
      {
        const auto& cDebugContexts{ rumDebugVM::GetDebugContexts() };
        for( const auto& iter : cDebugContexts )
        {
          ImGui::TableNextRow();

          // VM Name
          ImGui::TableNextColumn();

          ImGui::TextUnformatted( iter.m_strName.c_str() );

          // VM State
          ImGui::TableNextColumn();
          if( iter.m_bAttached )
          {
            if( ImGui::SmallButton( "Detach" ) )
            {
              rumDebugVM::RequestDetachVM( iter.m_strName );
            }
          }
          else
          {
            if( ImGui::SmallButton( "Attach" ) )
            {
              rumDebugVM::RequestAttachVM( iter.m_strName );
            }
          }
        }

        // VMsTable
        ImGui::EndTable();
      }

      // VMsTabChild
      ImGui::EndChild();

      // VMsTabItem
      ImGui::EndTabItem();
    }
  }


  void UpdateWatchTab()
  {
    auto pcContext{ rumDebugVM::GetCurrentDebugContext() };
    if( !pcContext )
    {
      return;
    }

    if( ImGui::BeginTabItem( "Watched##TabItem" ) )
    {
      const ImVec2 vRegion{ ImGui::GetContentRegionAvail() };
      const ImVec2 cSize{ 0.0f, std::max( 0.0f, vRegion.y - 2.0f ) };
      ImGui::BeginChild( "WatchedTabChild", cSize, false, ImGuiWindowFlags_HorizontalScrollbar );

      if( pcContext->m_bPaused )
      {
        constexpr ImGuiTableFlags eTableFlags{ ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders |
                                               ImGuiTableFlags_ScrollY | ImGuiTableFlags_ContextMenuInBody |
                                               ImGuiTableFlags_NoSavedSettings };
        constexpr int32_t iNumColumns{ 3 };
        if( ImGui::BeginTable( "WatchTable", iNumColumns, eTableFlags ) )
        {
          static char strCWatchVariable[MAX_FILENAME_LENGTH];

          // Fetch by copy here because there is potential to modify the list during iteration
          // #TODO - There is a lot of data being copied - work out a threadsafe share
          const auto cvWatchedVariables{ rumDebugVM::GetWatchedVariablesCopy() };
          for( const auto& iter : cvWatchedVariables )
          {
            ImGui::TableNextRow();

            // Watch variable name
            ImGui::TableNextColumn();

            ImGui::PushItemWidth( ImGui::GetColumnWidth() );

            strncpy_s( strCWatchVariable, iter.m_strName.c_str(), iter.m_strName.length() + 1 );
            std::string strContentID{ "##" + iter.m_strName };
            if( ImGui::InputText( strContentID.c_str(), strCWatchVariable, IM_ARRAYSIZE( strCWatchVariable ),
                                  ImGuiInputTextFlags_EnterReturnsTrue ) )
            {
              rumDebugVM::WatchVariableEdit( iter, strCWatchVariable );
            }

            ImGui::PopItemWidth();

            ImGui::PushID( strContentID.c_str() );
            if( ImGui::BeginPopupContextItem() )
            {
              if( ImGui::SmallButton( "Copy Name" ) )
              {
                ImGui::SetClipboardText( iter.m_strName.c_str() );
                ImGui::CloseCurrentPopup();
              }
              else if( ImGui::SmallButton( "Copy Value" ) )
              {
                ImGui::SetClipboardText( iter.m_strValue.c_str() );
                ImGui::CloseCurrentPopup();
              }
              else if( ImGui::Button( "Delete" ) )
              {
                rumDebugVM::WatchVariableRemove( iter );
                ImGui::CloseCurrentPopup();
              }

              ImGui::EndPopup();
            }
            ImGui::PopID();

            // Watch variable type
            ImGui::TableNextColumn();
            ImGui::TextUnformatted( iter.m_strType.c_str() );

            // Watch variable value
            ImGui::TableNextColumn();

            // Preview the results if there are more than 3 lines
            const auto szOffset{ FindNthOccurrence( iter.m_strValue, "\n", NUM_VARIABLE_PREVIEW_LINES ) };
            if( szOffset == std::string::npos )
            {
              ImGui::TextUnformatted( iter.m_strValue.c_str() );
            }
            else
            {
              ImGui::TextUnformatted( iter.m_strValue.substr( 0, szOffset ).c_str() );
              DoVariableExpansion( iter );
            }
          }

          ImGui::TableNextRow();

          static char strCNewWatchVariable[MAX_FILENAME_LENGTH];

          ImGui::TableNextColumn();
          ImGui::TextUnformatted( "+" );
          ImGui::SameLine();

          ImGui::PushItemWidth( ImGui::GetColumnWidth() );

          if( ImGui::InputText( "##NewWatchVariable", strCNewWatchVariable, IM_ARRAYSIZE( strCNewWatchVariable ),
                                ImGuiInputTextFlags_EnterReturnsTrue ) )
          {
            rumDebugVM::WatchVariableAdd( strCNewWatchVariable );

            memset( strCNewWatchVariable, '\0', sizeof( char ) * MAX_FILENAME_LENGTH );
          }

          ImGui::PopItemWidth();

          // WatchTable
          ImGui::EndTable();
        }
      }
      else
      {
        ImGui::TextUnformatted( "Running" );
      }

      // WatchedTabChild
      ImGui::EndChild();

      // WatchedTabItem
      ImGui::EndTabItem();
    }
  }


  bool WantsValuesAsHex()
  {
    return g_bShowHex;
  }
} // namespace rumDebugInterface
