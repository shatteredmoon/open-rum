/*

Squirrel ImGui Debugger VM Manager

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

#include <d_vm.h>

#include <d_interface.h>
#include <d_settings.h>
#include <d_utility.h>

#include <fstream>
#include <regex>
#include <sstream>

#if DEBUG_OUTPUT == 0
#define assert(x)
#endif // DEBUG_OUTPUT

#include <sqconfig.h>
#include <sqobject.h>
#include <sqstring.h>

#if DEBUG_OUTPUT == 0
#undef assert
#endif // DEBUG_OUTPUT

#define SQ_LINEEXECUTION  'l'
#define SQ_FUNCTIONCALL   'c'
#define SQ_FUNCTIONRETURN 'r'


namespace rumDebugVM
{
  // Holds the mutex lock state for cross-thread communication
  std::condition_variable s_cvDebugLock;

  // All of the currently attached VMs
  std::vector<rumDebugContext> g_cDebugContexts;

  // VMs that are awaiting attach/detach state changes
  std::string g_strAttachRequest;
  std::string g_strDetachRequest;

  rumDebugContext* g_pcCurrentDebugContext{ nullptr };

  // Currently set breakpoints
  std::vector<rumDebugBreakpoint> g_cBreakpoints;

  // Currently opened files
  std::map<std::string, rumDebugFile> g_cOpenedFiles;

  // The current stack level used to parse local variables
  uint32_t g_uiLocalVariableStackLevel{ 0 };

  // Local variables
  std::vector<rumDebugVariable> g_cLocalVariables;

  // Requested variables - added to watch or hovered over during a pause
  std::vector<rumDebugVariable> g_cRequestedVariables;

  // Watched variables
  std::vector<rumDebugVariable> g_cWatchVariables;

  // The lock used when updating shared information
  std::mutex g_mtxAccessLock;

  // The lock used to pause execution of the main thread
  std::mutex g_mtxDebugLock;


  ///////////////
  // Prototypes
  ///////////////

  // The VM must be registered by name in order to use these
  void AttachVM( const std::string& i_strName );
  void AttachVM( HSQUIRRELVM i_pcVM, const std::string& i_strName );
  void DetachVM( const std::string& i_strName );

  void BuildLocalVariables( HSQUIRRELVM i_pcVM, int32_t i_StackLevel );
  void BuildVariables( HSQUIRRELVM i_pcVM, std::vector<rumDebugVariable>& io_vVariables );

  rumDebugContext* GetVMByName( const std::string& i_strName );

  void NativeDebugHook( HSQUIRRELVM const i_pcVM, const SQInteger i_eHookType, const SQChar* i_strFileName,
                        const SQInteger i_iLine, const SQChar* const i_strFunctionName );

  void SuspendVM( HSQUIRRELVM i_pcVM, rumDebugContext& i_rcContext, uint32_t i_uiLine,
                  const std::filesystem::path& i_fsFilePath );


  void AttachVM( const std::string& i_strName )
  {
    rumDebugContext* pcVM{ GetVMByName( i_strName ) };
    if( pcVM )
    {
      AttachVM( pcVM->m_pcVM, i_strName );
    }
  }

  SQInteger AttachVM( HSQUIRRELVM i_pcVM )
  {
    std::string strName;

    // Is this vm already registered?
    const auto& iter{ std::find( g_cDebugContexts.begin(), g_cDebugContexts.end(), i_pcVM ) };
    if( iter == g_cDebugContexts.end() )
    {
      // Create a name
      std::ostringstream strAddress;
      strAddress << i_pcVM;
      strName = strAddress.str();
    }
    else
    {
      // Use the existing name
      strName = iter->m_strName;
    }

    AttachVM( i_pcVM, strName );

    return SQ_OK;
  }


  void AttachVM( HSQUIRRELVM i_pcVM, const std::string& i_strName )
  {
    const auto& iter{ std::find( g_cDebugContexts.begin(), g_cDebugContexts.end(), i_pcVM ) };
    if( iter == g_cDebugContexts.end() )
    {
      // Create a new context and attach to it
      rumDebugContext cContext( i_pcVM );
      cContext.m_strName = i_strName;
      cContext.m_bAttached = true;

      if( nullptr == g_pcCurrentDebugContext )
      {
        g_pcCurrentDebugContext = &cContext;
      }

      g_cDebugContexts.emplace_back( std::move( cContext ) );
    }
    else
    {
      // Attach to the existing context
      iter->m_bAttached = true;

      if( nullptr == g_pcCurrentDebugContext )
      {
        g_pcCurrentDebugContext = &*iter;
      }
    }

    sq_setnativedebughook( i_pcVM, &NativeDebugHook );

  }


  void BreakpointAdd( rumDebugBreakpoint i_cBreakpoint )
  {
    std::lock_guard<std::mutex> cLockGuard( g_mtxAccessLock );

    g_cBreakpoints.emplace_back( std::move( i_cBreakpoint ) );

    rumDebugInterface::RequestSettingsUpdate();
  }


  void BreakpointRemove( const rumDebugBreakpoint& i_rcBreakpoint )
  {
    std::lock_guard<std::mutex> cLockGuard( g_mtxAccessLock );

    auto iter{ std::find( g_cBreakpoints.begin(), g_cBreakpoints.end(), i_rcBreakpoint ) };
    if( iter != g_cBreakpoints.end() )
    {
      // Remove the existing breakpoint
      g_cBreakpoints.erase( iter );
      rumDebugInterface::RequestSettingsUpdate();
    }
  }


  void BreakpointToggle( const rumDebugBreakpoint& i_rcBreakpoint )
  {
    std::lock_guard<std::mutex> cLockGuard( g_mtxAccessLock );

    auto iter{ std::find( g_cBreakpoints.begin(), g_cBreakpoints.end(), i_rcBreakpoint ) };
    if( g_cBreakpoints.end() == iter )
    {
      // Breakpoint wasn't found, so add it
      g_cBreakpoints.push_back( i_rcBreakpoint );
    }
    else
    {
      // Toggle
      iter->m_bEnabled = !iter->m_bEnabled;
    }

    rumDebugInterface::RequestSettingsUpdate();
  }


  void BuildLocalVariables( HSQUIRRELVM i_pcVM, int32_t i_StackLevel )
  {
#if DEBUG_OUTPUT
    SQInteger iTopBegin{ sq_gettop( i_pcVM ) };
#endif

    g_cLocalVariables.clear();

    int32_t iIndex{ 0 };
    const SQChar* strName{ sq_getlocal( i_pcVM, i_StackLevel, iIndex++ ) };
    while( strName )
    {
      const SQObjectType eType{ sq_gettype( i_pcVM, -1 ) };

      rumDebugVariable cLocalEntry;
      cLocalEntry.m_strName = strName;
      cLocalEntry.m_strType = rumDebugUtility::GetTypeName( eType );
      cLocalEntry.m_strValue = rumDebugUtility::FormatVariable( i_pcVM, -1, rumDebugInterface::WantsValuesAsHex() );

      g_cLocalVariables.emplace_back( std::move( cLocalEntry ) );

      sq_poptop( i_pcVM );

      strName = sq_getlocal( i_pcVM, i_StackLevel, iIndex++ );
    }

#if DEBUG_OUTPUT
    SQInteger iTopEnd{ sq_gettop( i_pcVM ) };
    assert( iTopBegin == iTopEnd );
#endif
  }


  void BuildVariables( HSQUIRRELVM i_pcVM, std::vector<rumDebugVariable>& io_vVariables )
  {
    for( auto& watchIter : io_vVariables )
    {
      // Check locals first
      const auto& localIter{ std::find( g_cLocalVariables.begin(), g_cLocalVariables.end(), watchIter ) };
      if( localIter != g_cLocalVariables.end() )
      {
        // This watch variable is a local variable, so just re-use that info
        watchIter.m_strType = localIter->m_strType;
        watchIter.m_strValue = localIter->m_strValue;
      }
      else
      {
        SQObject sqObject{ rumDebugUtility::FindSymbol( i_pcVM, watchIter.m_strName, g_uiLocalVariableStackLevel ) };
        if( rumDebugUtility::IsUnknownType( sqObject._type ) )
        {
          // Try one more time with a "this." prefix
          std::string strVariable( "this." + watchIter.m_strName );
          sqObject = rumDebugUtility::FindSymbol( i_pcVM, strVariable, g_uiLocalVariableStackLevel );
        }

        watchIter.m_strType = rumDebugUtility::GetTypeName( sqObject._type );
        watchIter.m_strValue = rumDebugUtility::FormatVariable( i_pcVM, sqObject, rumDebugInterface::WantsValuesAsHex() );
      }
    }
  }


  SQInteger DetachVM( HSQUIRRELVM i_pcVM )
  {
    const auto& iter{ std::find( g_cDebugContexts.begin(), g_cDebugContexts.end(), i_pcVM ) };
    if( iter != g_cDebugContexts.end() )
    {
      bool bPaused{ iter->m_bPaused };

      iter->m_eStepDirective = rumDebugContext::StepDirective::Resume;
      iter->m_bAttached = false;

      sq_setnativedebughook( i_pcVM, NULL );

      if( bPaused )
      {
        RequestResume();
      }

      return SQ_OK;
    }

    return SQ_ERROR;
  }


  void DetachVM( const std::string& i_strName )
  {
    rumDebugContext* pcVM{ GetVMByName( i_strName ) };
    if( pcVM )
    {
      DetachVM( pcVM->m_pcVM );
    }
  }


  void EnableDebugInfo( HSQUIRRELVM i_pcVM, bool i_bEnable )
  {
    sq_enabledebuginfo( i_pcVM, i_bEnable ? SQTrue : SQFalse );
  }


  void FileClose( const std::filesystem::path& i_fsFilePath )
  {
    std::string strFilePath{ i_fsFilePath.generic_string() };

    std::lock_guard<std::mutex> cLockGuard( g_mtxAccessLock );

    const auto& iter{ g_cOpenedFiles.find( strFilePath ) };
    if( iter != g_cOpenedFiles.end() )
    {
      g_cOpenedFiles.erase( iter );
    }

    rumDebugInterface::RequestSettingsUpdate();
  }


  void FileOpen( const std::filesystem::path& i_fsFilePath, uint32_t i_uiLine )
  {
    std::string strFilePath{ i_fsFilePath.generic_string() };

    std::lock_guard<std::mutex> cLockGuard( g_mtxAccessLock );

    // Determine if the file is already opened
    const auto& iter{ g_cOpenedFiles.find( strFilePath ) };
    if( iter == g_cOpenedFiles.end() )
    {
      // Open and cache the file contents
      rumDebugFile cFile;
      cFile.m_fsFilePath = i_fsFilePath;
      cFile.m_strFilename = i_fsFilePath.filename().generic_string();

      std::ifstream cInfile;
      cInfile.open( cFile.m_fsFilePath.native().c_str() );

      std::ostringstream cBuffer;
      cBuffer << cInfile.rdbuf();
      cFile.m_strData = cBuffer.str();

      cFile.m_vStringOffsets.resize( 10000 );

      size_t szLastOffset{ 0 };
      size_t szCurrentOffset{ 0 };
      size_t uiLongestLine{ rumDebugFile::s_uiMinimumColumns };

      bool bInMultilineComment{ false };
      uint32_t iMultilineCommentStart{ 0 };

      uint32_t index{ 0 };

      while( ( szCurrentOffset = cFile.m_strData.find( '\n', szLastOffset ) ) != std::string::npos )
      {
        cFile.m_vStringOffsets[++index] = ++szCurrentOffset;

        std::string_view strLine{ std::string_view( cFile.m_strData ).substr( szLastOffset, szCurrentOffset - szLastOffset ) };
        const size_t uiLineLength{ strLine.size() };
        uiLongestLine = { std::max( uiLineLength, uiLongestLine ) };

        // Parse multiline comment ranges
        size_t iMultilineCommentStartSymbol{ strLine.find( "/*" ) };
        if( !bInMultilineComment && iMultilineCommentStartSymbol != std::string::npos )
        {
          size_t iMultilineCommentEndSymbol{ strLine.find( "*/" ) };
          while( iMultilineCommentStartSymbol != std::string::npos && iMultilineCommentEndSymbol != std::string::npos )
          {
            while( iMultilineCommentEndSymbol != std::string::npos && iMultilineCommentEndSymbol < iMultilineCommentStartSymbol )
            {
              // end before start, let's check if we have real closure in this line
              iMultilineCommentEndSymbol = strLine.find( "*/", iMultilineCommentEndSymbol + 2 );
            }
            if( iMultilineCommentEndSymbol != std::string::npos )
            {
              // found a closure for multiline comment in same line, let's check that if we try to start a new multi line comment after this block
              iMultilineCommentStartSymbol = strLine.find( "/*", iMultilineCommentEndSymbol + 2 );
            }
          }
          if( iMultilineCommentStartSymbol != std::string::npos )
          {
            bInMultilineComment = true;
            iMultilineCommentStart = index;
          }
        }
        else if( bInMultilineComment && strLine.find( "*/" ) != std::string::npos )
        {
          rumDebugFile::MultilineComment cMultilineComment;
          cMultilineComment.m_iStartLine = iMultilineCommentStart;
          cMultilineComment.m_iEndLine = index;

          cFile.m_vMultilineComments.emplace_back( std::move( cMultilineComment ) );

          bInMultilineComment = false;
        }

        szLastOffset = szCurrentOffset;
      }

      // Add the last offset if there isn't a final '\n'
      if( szLastOffset != cFile.m_strData.length() )
      {
        cFile.m_vStringOffsets[index++] = cFile.m_strData.length();
      }

      // Handle files that do not have a closing multiline comment
      if( bInMultilineComment )
      {
        rumDebugFile::MultilineComment cMultilineComment;
        cMultilineComment.m_iStartLine = iMultilineCommentStart;
        cMultilineComment.m_iEndLine = index;

        cFile.m_vMultilineComments.emplace_back( std::move( cMultilineComment ) );
      }

      cFile.m_vStringOffsets.resize( index + 1 );
      cFile.m_uiLongestLine = uiLongestLine;

      g_cOpenedFiles.insert( std::make_pair( strFilePath, std::move( cFile ) ) );
      rumDebugInterface::RequestSettingsUpdate();
    }

    // Request a switch to the target file and line
    rumDebugInterface::SetFileFocus( i_fsFilePath, i_uiLine );
  }


  const std::vector<rumDebugBreakpoint> GetBreakpointsCopy()
  {
    return g_cBreakpoints;
  }


  const std::vector<rumDebugBreakpoint>& GetBreakpointsRef()
  {
    return g_cBreakpoints;
  }


  const rumDebugContext* GetCurrentDebugContext()
  {
    return g_pcCurrentDebugContext;
  }


  const std::vector<rumDebugContext>& GetDebugContexts()
  {
    return g_cDebugContexts;
  }


  const std::vector<rumDebugVariable>& GetLocalVariablesRef()
  {
    return g_cLocalVariables;
  }


  const std::map<std::string, rumDebugFile> GetOpenedFilesCopy()
  {
    return g_cOpenedFiles;
  }


  const std::map<std::string, rumDebugFile>& GetOpenedFilesRef()
  {
    return g_cOpenedFiles;
  }


  const std::vector<rumDebugVariable>& GetRequestedVariablesRef()
  {
    return g_cRequestedVariables;
  }


  rumDebugContext* GetVMByName( const std::string& i_strName )
  {
    const auto& iterVM{ std::find_if( g_cDebugContexts.begin(), g_cDebugContexts.end(),
                                      [&]( const auto& iter )
                                      {
                                        return iter.m_strName.compare( i_strName ) == 0;
                                      } ) };
    if( iterVM != g_cDebugContexts.end() )
    {
      return &*iterVM;
    }

    return nullptr;
  }


  const std::vector<rumDebugVariable> GetWatchedVariablesCopy()
  {
    return g_cWatchVariables;
  }


  const std::vector<rumDebugVariable>& GetWatchedVariablesRef()
  {
    return g_cWatchVariables;
  }


  SQInteger IsDebuggerAttached( HSQUIRRELVM i_pcVM )
  {
    const auto& iter{ std::find( g_cDebugContexts.begin(), g_cDebugContexts.end(), i_pcVM ) };
    return iter != g_cDebugContexts.end() ? SQTrue : SQFalse;
  }


  void NativeDebugHook( HSQUIRRELVM const i_pcVM, const SQInteger i_eHookType, const SQChar* i_strFileName,
                        const SQInteger i_iLine, [[maybe_unused]] const SQChar* const i_strFunctionName )
  {
    if( SQ_LINEEXECUTION != i_eHookType )
    {
      return;
    }

    auto iterContext{ std::find( g_cDebugContexts.begin(), g_cDebugContexts.end(), i_pcVM ) };
    if( iterContext == g_cDebugContexts.end() )
    {
      // The VM is not attached for debugging
      return;
    }

    g_pcCurrentDebugContext = &*iterContext;

    SQStackInfos cStackInfos;
    SQInteger iStackLevel{ 0 };
    while( SQ_SUCCEEDED( sq_stackinfos( i_pcVM, iStackLevel, &cStackInfos ) ) )
    {
      ++iStackLevel;
    }

    std::filesystem::path fsFilePath( i_strFileName );
    uint32_t uiLine{ static_cast<uint32_t>( i_iLine ) };

    // Check for breakpoints first, even if there is a step directive because breakpoints override step directives
    rumDebugBreakpoint cBreakpoint( fsFilePath, uiLine );
    const auto& iterBP{ std::find_if( g_cBreakpoints.begin(), g_cBreakpoints.end(),
                                      [&]( const auto& i_rcBreakpoint )
      {
        return i_rcBreakpoint.m_bEnabled && ( i_rcBreakpoint == cBreakpoint );
      } ) };

    if( g_cBreakpoints.end() != iterBP )
    {
#if DEBUG_OUTPUT
      std::cout << "Breakpoint hit (type: " << static_cast<int32_t>( i_eHookType );
      std::cout << ") source: " << i_strFileName;
      std::cout << ") line: " << uiLine;
      std::cout << ") function: " << i_strFunctionName << '\n';
#endif // DEBUG_OUTPUT

      if( iterContext != g_cDebugContexts.end() )
      {
        SuspendVM( i_pcVM, *iterContext, uiLine, fsFilePath );
      }
    }

    switch( iterContext->m_eStepDirective )
    {
      case rumDebugContext::StepDirective::StepOver:
      {
        if( iStackLevel <= (SQInteger)iterContext->m_uiStepDirectiveStackLevel )
        {
          SuspendVM( i_pcVM, *iterContext, uiLine, fsFilePath );
        }
        break;
      }

      case rumDebugContext::StepDirective::StepInto:
      {
        if( iStackLevel >= (SQInteger)iterContext->m_uiStepDirectiveStackLevel )
        {
          SuspendVM( i_pcVM, *iterContext, uiLine, fsFilePath );
        }
        break;
      }

      case rumDebugContext::StepDirective::StepOut:
      {
        if( iStackLevel < (SQInteger)iterContext->m_uiStepDirectiveStackLevel )
        {
          SuspendVM( i_pcVM, *iterContext, uiLine, fsFilePath );
        }
        break;
      }
    }
  }


  void RegisterVM( HSQUIRRELVM i_pcVM, const std::string& i_strName )
  {
    AttachVM( i_pcVM, i_strName );
  }


  void RequestAttachVM( const std::string& i_strName )
  {
    std::lock_guard<std::mutex> cLockGuard( g_mtxAccessLock );
    g_strAttachRequest = i_strName;

    s_cvDebugLock.notify_all();
  }


  void RequestDetachVM( const std::string& i_strName )
  {
    std::lock_guard<std::mutex> cLockGuard( g_mtxAccessLock );
    g_strDetachRequest = i_strName;

    if( g_pcCurrentDebugContext )
    {
      g_pcCurrentDebugContext->m_bUpdateVariables = false;
    }

    s_cvDebugLock.notify_all();
  }


  void RequestChangeStackLevel( uint32_t i_uiStackLevel )
  {
    std::lock_guard<std::mutex> cLockGuard( g_mtxAccessLock );

    g_uiLocalVariableStackLevel = i_uiStackLevel;

    if( g_pcCurrentDebugContext )
    {
      g_pcCurrentDebugContext->m_bUpdateVariables = true;
    }

    s_cvDebugLock.notify_all();
  }


  void RequestResume()
  {
    std::lock_guard<std::mutex> cLockGuard( g_mtxAccessLock );

    if( g_pcCurrentDebugContext )
    {
      g_pcCurrentDebugContext->m_eStepDirective = rumDebugContext::StepDirective::Resume;
    }

    s_cvDebugLock.notify_all();
  }


  void RequestStepInto()
  {
    std::lock_guard<std::mutex> cLockGuard( g_mtxAccessLock );

    if( g_pcCurrentDebugContext )
    {
      g_pcCurrentDebugContext->m_eStepDirective = rumDebugContext::StepDirective::StepInto;
      g_pcCurrentDebugContext->m_uiStepDirectiveStackLevel =
        static_cast<uint32_t>( g_pcCurrentDebugContext->m_vCallstack.size() );
    }

    s_cvDebugLock.notify_all();
  }


  void RequestStepOut()
  {
    std::lock_guard<std::mutex> cLockGuard( g_mtxAccessLock );

    if( g_pcCurrentDebugContext )
    {
      g_pcCurrentDebugContext->m_eStepDirective = rumDebugContext::StepDirective::StepOut;
      g_pcCurrentDebugContext->m_uiStepDirectiveStackLevel =
        static_cast<uint32_t>( g_pcCurrentDebugContext->m_vCallstack.size() );
    }

    s_cvDebugLock.notify_all();
  }


  void RequestStepOver()
  {
    std::lock_guard<std::mutex> cLockGuard( g_mtxAccessLock );

    if( g_pcCurrentDebugContext )
    {
      g_pcCurrentDebugContext->m_eStepDirective = rumDebugContext::StepDirective::StepOver;
      g_pcCurrentDebugContext->m_uiStepDirectiveStackLevel =
        static_cast<uint32_t>( g_pcCurrentDebugContext->m_vCallstack.size() );
    }

    s_cvDebugLock.notify_all();
  }


  void RequestVariable( const rumDebugVariable& i_cVariable )
  {
    std::lock_guard<std::mutex> cLockGuard( g_mtxAccessLock );

    g_cRequestedVariables.push_back( i_cVariable );

    if( g_pcCurrentDebugContext )
    {
      g_pcCurrentDebugContext->m_bUpdateVariables = true;
    }

    s_cvDebugLock.notify_all();
  }


  void RequestVariableUpdates()
  {
    std::lock_guard<std::mutex> cLockGuard( g_mtxAccessLock );

    if( g_pcCurrentDebugContext )
    {
      g_pcCurrentDebugContext->m_bUpdateVariables = true;
    }

    s_cvDebugLock.notify_all();
  }


  void SuspendVM( HSQUIRRELVM i_pcVM, rumDebugContext& i_rcContext, uint32_t i_uiLine,
                  const std::filesystem::path& i_fsFilePath )
  {
    i_rcContext.m_vCallstack.clear();

    i_rcContext.m_bFocusOnCurrentInstruction = true;

    i_rcContext.m_uiPausedLine = i_uiLine;
    i_rcContext.m_fsPausedFile = i_fsFilePath;

    FileOpen( i_fsFilePath, i_uiLine );

    SQStackInfos cStackInfos;
    SQInteger iStackLevel{ 0 };
    while( SQ_SUCCEEDED( sq_stackinfos( i_pcVM, iStackLevel++, &cStackInfos ) ) )
    {
      i_rcContext.m_vCallstack.push_back( { static_cast<int32_t>( cStackInfos.line ),
                                            std::string( cStackInfos.source ),
                                            std::string( cStackInfos.funcname ) } );
    }

    auto iter{ std::find( g_cDebugContexts.begin(), g_cDebugContexts.end(), i_pcVM ) };
    if( iter != g_cDebugContexts.end() )
    {
      g_pcCurrentDebugContext = &*iter;
    }

    g_uiLocalVariableStackLevel = 0;

    do
    {
      {
#if DEBUG_OUTPUT
        std::cout << "Parsing variables\n";
#endif

        std::lock_guard<std::mutex> cLockGuard( g_mtxAccessLock );

        BuildLocalVariables( i_pcVM, g_uiLocalVariableStackLevel );
        BuildVariables( i_pcVM, g_cWatchVariables );
        BuildVariables( i_pcVM, g_cRequestedVariables );

        i_rcContext.m_bUpdateVariables = false;
      }

      i_rcContext.m_bPaused = true;

      std::unique_lock<std::mutex> ulock( g_mtxDebugLock );
      s_cvDebugLock.wait( ulock );

      i_rcContext.m_bPaused = false;
    } while( i_rcContext.m_bUpdateVariables );

    g_cRequestedVariables.clear();

    Update();
  }


  void Update()
  {
    if( !g_strAttachRequest.empty() )
    {
      std::lock_guard<std::mutex> cLockGuard( g_mtxAccessLock );

      AttachVM( g_strAttachRequest );
      g_strAttachRequest.clear();
    }

    if( !g_strDetachRequest.empty() )
    {
      std::lock_guard<std::mutex> cLockGuard( g_mtxAccessLock );

      DetachVM( g_strDetachRequest );
      g_strDetachRequest.clear();
    }
  }


  bool WatchVariableAdd( const std::string& i_strName )
  {
    std::lock_guard<std::mutex> cLockGuard( g_mtxAccessLock );

    auto iter{ std::find( g_cWatchVariables.begin(), g_cWatchVariables.end(), i_strName ) };
    if( g_cWatchVariables.end() == iter )
    {
      rumDebugVariable cVariable;
      cVariable.m_strName = i_strName;
      g_cWatchVariables.emplace_back( std::move( cVariable ) );

      if( g_pcCurrentDebugContext )
      {
        g_pcCurrentDebugContext->m_bUpdateVariables = true;
      }

      rumDebugInterface::RequestSettingsUpdate();

      s_cvDebugLock.notify_all();

      return true;
    }

    return false;
  }


  bool WatchVariableEdit( const rumDebugVariable& i_rcVariable, const std::string& i_strName )
  {
    std::lock_guard<std::mutex> cLockGuard( g_mtxAccessLock );

    auto iter{ std::find( g_cWatchVariables.begin(), g_cWatchVariables.end(), i_rcVariable ) };
    if( iter != g_cWatchVariables.end() )
    {
      iter->m_strName = i_strName;
      iter->m_strValue.clear();
      iter->m_strType.clear();

      if( g_pcCurrentDebugContext )
      {
        g_pcCurrentDebugContext->m_bUpdateVariables = true;
      }

      rumDebugInterface::RequestSettingsUpdate();

      s_cvDebugLock.notify_all();

      return true;
    }

    return false;
  }


  void WatchVariableRemove( const rumDebugVariable& i_rcVariable )
  {
    std::lock_guard<std::mutex> cLockGuard( g_mtxAccessLock );

    auto iter{ std::find( g_cWatchVariables.begin(), g_cWatchVariables.end(), i_rcVariable ) };
    if( iter != g_cWatchVariables.end() )
    {
      // Remove the variable
      g_cWatchVariables.erase( iter );
      rumDebugInterface::RequestSettingsUpdate();
    }
  }
} // namespace rumDebugVM
