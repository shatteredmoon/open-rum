#include <u_script.h>

#include <u_asset.h>
#include <u_log.h>
#include <u_object.h>
#include <u_utility.h>
#include <u_registry.h>
#include <u_rum.h>
#include <u_structs.h>
#include <u_strings.h>

#include <iniparser.h>

#include <filesystem>

#include <sqrat/sqratVM.h>
#include <sqvm.h>
#include <sqstring.h>

#include <fstream>
#include <map>
#include <vector>

#define SCRIPT_FOLDER_NAME "scripts"
#define SERVER_FOLDER_NAME "server"
#define CLIENT_FOLDER_NAME "client"
#define EDITOR_FOLDER_NAME "editor"
#define DEFAULT_INIT_FILE  "game.nut"


namespace rumScript
{
  enum ProcessType
  {
    PROCESS_LOAD,
    PROCESS_LOAD_AND_RUN,
    PROCESS_COMPILE,
    PROCESS_COMPILE_AND_RUN
  };

  // The container holding all filenames of script files that have already loaded
  typedef std::vector<std::string> FileSet;

  // The container map holding all classes defined script-side
  typedef std::unordered_map<AssetType, rumClassRegistry> ScriptRegistry;

  struct rumVirtualMachine
  {
    VMType m_eVM{ VM_INVALID };
    HSQUIRRELVM m_pcVM{ nullptr };
    Sqrat::SqratVM* m_pcSqratVM{ nullptr };

    ScriptRegistry m_cScriptRegistryHash;

    FileSet cLoadedScriptSet;

    std::filesystem::path m_fsScriptPath;

    bool m_bStartupExecuted{ false };

    static VMType s_eCurrentVM;
  };

  VMType rumVirtualMachine::s_eCurrentVM{ VM_INVALID };

  // Maps vm id to a VMStruct
  typedef std::map<VMType, rumVirtualMachine> VMContainer;
  VMContainer g_cVMMap;

  std::string g_strLastCompilerError;
  std::string g_strLastError;

  ///////////////
  // Prototypes
  ///////////////

  void CompilerErrorFunc( HSQUIRRELVM i_pcVM, const SQChar* i_strDesc, const SQChar* i_strSource, SQInteger i_iLine,
                          SQInteger i_iColumn );

  // The main script loader
  int32_t LoadInternal( const std::string& i_strFilePath, ProcessType i_eProcessType, bool i_bReload );

  // Load Helpers
  int32_t LoadFile( const std::filesystem::path& i_fsFilename, ProcessType i_eProcessType, bool i_bReload );
  int32_t LoadFolder( const std::filesystem::path& i_fsFolder, ProcessType i_eProcessType, bool i_bReload );

  void PrintFunc( HSQUIRRELVM i_pcVM, const SQChar* i_strDesc, ... );

  Sqrat::Object AttachToStackObject( int32_t i_iIndex )
  {
    HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };
    return Sqrat::Var<Sqrat::Object>( pcVM, i_iIndex ).value;
  }


  void Bind()
  {
    HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

   Sqrat::ConstTable( pcVM )
     .Const( "rumProgramMajorVersion", PROGRAM_MAJOR_VERSION )
     .Const( "rumProgramMinorVersion", PROGRAM_MINOR_VERSION );

    Sqrat::RootTable( pcVM )
      .Func( "rumGetConstTableEntry", GetConstTableEntry )
      .Func( "rumGetRefCount", GetObjectRefCount )
      .Overload<Sqrat::Object(*)( rumAssetID i_eAssetID )>( "GetClass", &GetClass )
      .Overload<Sqrat::Object(*)( const Sqrat::Object& i_sqObject )>( "GetClass", &GetClass )
      .SquirrelFunc( "rumPrintCallstack", PrintCallstack );
  }


  bool CallInitFunction( const std::string& i_strInitFunc )
  {
    // The script system must be initialized before using this function
    rumAssert( GetCurrentVMType() != VM_INVALID );

    // If an init function was provided, call it
    if( !i_strInitFunc.empty() )
    {
      HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };
      const auto cPair{ EvalRequiredFunc( Sqrat::RootTable( pcVM ), i_strInitFunc.c_str(), false,
                                          GetElapsedTime() ) };
      if( cPair.first )
      {
        return cPair.second;
      }
      else
      {
        Logger::LogStandard( "Missing required script function: " + i_strInitFunc + '\n');
      }
    }

    return false;
  }


  void CallShutdownFunction()
  {
    // The script system must be initialized before using this function
    rumAssert( GetCurrentVMType() != VM_INVALID );

    HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };
    ExecOptionalFunc( Sqrat::RootTable( pcVM ), "OnGameShutdown" );

    sq_collectgarbage( pcVM );
  }


  void Compile( VMType i_eVM, std::string_view i_strPath )
  {
    SetCurrentVMType( i_eVM );

    LoadInternal( DEFAULT_INIT_FILE, PROCESS_LOAD_AND_RUN, false );

    std::string strScript;
    strScript.reserve( 1024 * 1024 * 10 ); // 10 megabytes

    const auto& rcVM{ g_cVMMap[i_eVM] };

    // Visit each script and export a monolithic file
    for( const auto& iter : rcVM.cLoadedScriptSet )
    {
      std::ifstream cInfile;
      cInfile.open( iter.c_str() );
      if( cInfile.is_open() )
      {
        strScript += "\n// " + iter + "\n\n";

        std::string strLine;
        while( std::getline( cInfile, strLine ) )
        {
          strScript += strLine + '\n';
        }

        cInfile.close();
      }
    }

    Sqrat::Script sqScript;
    sqScript.CompileString( strScript );

    const std::filesystem::path fsPath( i_strPath );

#if SCRIPT_DEBUG
    // Write the monolithic script
    std::ofstream cOutfile;
    const std::filesystem::path fsMonoScript( fsPath / "game.mnut");
    std::filesystem::create_directories( fsMonoScript.parent_path() );
    cOutfile.open( fsMonoScript.c_str() );
    if( cOutfile.is_open() )
    {
      cOutfile << strScript;
      cOutfile.close();
    }
#endif // SCRIPT_DEBUG

    // Write the compiled script
    const std::filesystem::path fsCompiledScript( fsPath / "game.cnut" );
    sqScript.WriteCompiledFile( fsCompiledScript.string().c_str() );
  }


  void CompilerErrorFunc( HSQUIRRELVM i_pcVM, const SQChar* i_strDesc, const SQChar* i_strSource, SQInteger i_iLine,
                          SQInteger i_iColumn )
  {
    const std::filesystem::path fsPath( i_strSource );

    g_strLastCompilerError = "Error \"";
    g_strLastCompilerError += i_strDesc;
    g_strLastCompilerError += "\" occurred in\n";
    g_strLastCompilerError += fsPath.string();
    g_strLastCompilerError += " line ";
    g_strLastCompilerError += rumStringUtils::ToString( (int32_t)i_iLine );
    g_strLastCompilerError += " column ";
    g_strLastCompilerError += rumStringUtils::ToString( (int32_t)i_iColumn );

    Logger::LogStandard( g_strLastCompilerError );
  }


  Sqrat::Object CreateClassScript( const std::string& i_strName, const std::string& i_strExtends )
  {
    const auto& rcVM{ g_cVMMap[rumVirtualMachine::s_eCurrentVM] };

    // Create the class description
    std::string strScript{ "class " };
    strScript += i_strName;

    if( !i_strExtends.empty() )
    {
      strScript += " extends ";
      strScript += i_strExtends;
    }

#if SCRIPT_DEBUG
    std::string strInfo{ "Creating script: " };
    strInfo += strScript;
    Logger::LogStandard( strInfo, Logger::LOG_INFO );
#endif // SCRIPT_DEBUG

    strScript += " { constructor() { base.constructor(); } }";

    // Create the script
    Sqrat::Script sqScript;
    sqScript.CompileString( strScript );
    sqScript.Run();

    Sqrat::Object sqClass{ Sqrat::RootTable().GetSlot( i_strName.c_str() ) };
    if( sqClass.GetType() != OT_CLASS )
    {
      std::string strError{ "Failed to create script: " };
      strError += strScript;
      Logger::LogStandard( strError, Logger::LOG_ERROR );
    }

    return sqClass;
  }


  Sqrat::Object CreateInstance( const Sqrat::Object& i_sqClass, Sqrat::Table i_sqParams )
  {
    HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

#ifdef _DEBUG
    const SQInteger iTop{ pcVM->_top };
#endif

    // Push the class we want to create
    sq_pushobject( pcVM, i_sqClass.GetObjectA() );

    // Push literally anything else
    sq_pushroottable( pcVM );

    SQInteger uiNumOptionalParams{ 0 };
    if( i_sqParams.GetType() == OT_TABLE )
    {
      uiNumOptionalParams = i_sqParams.GetSize();
    }

    // Push optional parameters
    if( uiNumOptionalParams > 0 )
    {
      Sqrat::Object::iterator iter;
      while( i_sqParams.Next( iter ) )
      {
        Sqrat::Object sqValue{ iter.getValue() };
        sq_pushobject( pcVM, sqValue.GetObjectA() );
      }
    }

    // Do the creation - prefer sq_call to sq_createinstance because of the automatic constructor call
    Sqrat::Object sqInstance;
    if( SQ_SUCCEEDED( sq_call( pcVM, uiNumOptionalParams + 1, SQTrue /* retval */, SQFalse /* raise error */ ) ) )
    {
      // Fetch the newly created script instance which is now on top of the stack
      sqInstance = AttachToStackObject( -1 );
    }
    else
    {
      // Since constructors weren't evoked, the optional params were never popped
      sq_pop( pcVM, uiNumOptionalParams );
    }

    // Restore the original stack size
    sq_pop( pcVM, 2 );

#ifdef _DEBUG
    // Are we leaving the stack the same way we found it?
    rumAssert( pcVM->_top == iTop );
#endif

    rumAssert( sqInstance.GetType() == OT_INSTANCE );

    return sqInstance;
  }


  std::string EnumToString( const std::string& i_strEnum, int32_t i_eValue, bool i_bBitfield )
  {
    std::string strResult;

    Sqrat::Object sqEnumObj{ Sqrat::ConstTable().GetSlot( i_strEnum.c_str() ) };
    if( sqEnumObj.GetType() != OT_TABLE )
    {
      return strResult;
    }

    bool bDone{ false };

    Sqrat::Object::iterator iter;
    while( sqEnumObj.Next( iter ) && !bDone )
    {
      Sqrat::Object sqKey{ iter.getKey() };
      Sqrat::Object sqValue{ iter.getValue() };
      const int32_t eValue{ sqValue.Cast<int32_t>() };

      if( i_bBitfield )
      {
        if( ( i_eValue & eValue ) == eValue )
        {
          if( !strResult.empty() )
          {
            strResult += "\n";
          }

          strResult += sqKey.Cast<std::string>().c_str();
        }
      }
      else
      {
        if( i_eValue == eValue )
        {
          if( !strResult.empty() )
          {
            strResult += "\n";
          }

          strResult += sqKey.Cast<std::string>().c_str();
          bDone = true;
        }
      }
    }

    return strResult;
  }


  SQInteger ErrorFunc( HSQUIRRELVM i_pcVM )
  {
    SQPRINTFUNCTION sqPrintFunc{ sq_getprintfunc( i_pcVM ) };
    if( sqPrintFunc )
    {
      if( sq_gettop( i_pcVM ) >= 1 )
      {
        sqPrintFunc( i_pcVM, _SC( "\nSCRIPT ERROR\n" ) );

        const SQChar* strError{ nullptr };
        if( SQ_SUCCEEDED( sq_getstring( i_pcVM, 2, &strError ) ) )
        {
          Logger::LogStandard( strError, Logger::LOG_ERROR );
          g_strLastError = strError;
        }

        std::string strCallstack{ GetCallstack( i_pcVM ) };
        Logger::LogStandard( strCallstack, Logger::LOG_ERROR );
      }
    }

    return 0;
  }


  int32_t ExecuteStartupScript( const VMType i_eVM )
  {
    //-------------------------------------------------------------------------
    // Run the startup script
    //-------------------------------------------------------------------------
    RUM_COUT( "Processing initialization script...\n" );

    if( !SetCurrentVMType( i_eVM ) )
    {
      return RESULT_FAILED;
    }

    auto& rcVM{ g_cVMMap.at( i_eVM ) };

    int32_t eResult{ Load( DEFAULT_INIT_FILE ) };
    if( Sqrat::Error::Occurred( rcVM.m_pcVM ) )
    {
      std::string strError{ "An error was encountered while loading startup script " DEFAULT_INIT_FILE ":" };
      strError += Sqrat::Error::Message( rcVM.m_pcVM );
      Logger::LogStandard( strError );

      eResult = RESULT_FAILED;
    }

#if SCRIPT_DEBUG
    RUM_COUT( "Loaded scripts\n" );
    for( auto& iter : rcVM.cLoadedScriptSet )
    {
      RUM_COUT( "  " << iter << '\n' );
    }
#endif // SCRIPT_DEBUG

    // Once the startup script has executed, it is no longer okay to call CreateClass
    rcVM.m_bStartupExecuted = true;

    return eResult;
  }


  Sqrat::Object GetBaseClass( Sqrat::Object& i_sqClass )
  {
    Sqrat::Object sqBaseClass;

    if( i_sqClass.GetType() == OT_CLASS )
    {
      HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

      sq_pushobject( pcVM, i_sqClass.GetObjectA() );
      sq_getbase( pcVM, -1 );
      sqBaseClass = AttachToStackObject( -1 );
      sq_pop( pcVM, 2 );
    }

    // Return by copy
    return sqBaseClass;
  }


  std::string GetCallstack( HSQUIRRELVM i_pcVM )
  {
    SQStackInfos sqStackInfos;
    SQInteger iLevel{ 1 }; // 1 is to skip this function that is level 0
    std::string strDesc;

    while( SQ_SUCCEEDED( sq_stackinfos( i_pcVM, iLevel, &sqStackInfos ) ) )
    {
      const SQChar* strFunction{ _SC( "unknown" ) };
      const SQChar* strSource{ _SC( "unknown" ) };

      if( sqStackInfos.funcname )
      {
        strFunction = sqStackInfos.funcname;
      }

      if( sqStackInfos.source )
      {
        strSource = sqStackInfos.source;
      }

      strDesc += "*FUNCTION [";
      strDesc += strFunction;
      strDesc += "()] ";
      strDesc += strSource;
      strDesc += " line [";
      strDesc += rumStringUtils::ToString( (int32_t)sqStackInfos.line );
      strDesc += "]\n";

      ++iLevel;
    }

    return strDesc;
  }


  Sqrat::Object GetClass( rumAssetID i_eAssetID )
  {
    const auto& rcVM{ g_cVMMap[rumVirtualMachine::s_eCurrentVM] };
    const auto eAssetType{ RAW_ASSET_TYPE( i_eAssetID ) };
    const auto* pcClassRegistry{ GetOrCreateClassRegistry( eAssetType ) };
    return pcClassRegistry != nullptr ? pcClassRegistry->GetScriptClass( i_eAssetID ) : Sqrat::Object();
  }


  rumClassRegistry* GetOrCreateClassRegistry( AssetType i_eAssetType )
  {
    auto& rcVM{ g_cVMMap[rumVirtualMachine::s_eCurrentVM] };

    // Make sure this class does not already exist
    ScriptRegistry::iterator iter( rcVM.m_cScriptRegistryHash.find( i_eAssetType ) );
    if( iter == rcVM.m_cScriptRegistryHash.end() )
    {
      const auto cPair{ rcVM.m_cScriptRegistryHash.insert( std::make_pair( i_eAssetType, rumClassRegistry() ) ) };
      return ( cPair.second ? &( cPair.first->second ) : nullptr );
    }

    return &( iter->second );
  }


  Sqrat::Object GetClass( const Sqrat::Object& i_sqObject )
  {
    HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

    Sqrat::Object sqClass;

    if( i_sqObject.GetType() == OT_INSTANCE )
    {
#ifdef _DEBUG
      const SQInteger iTop{ pcVM->_top };
#endif

      sq_pushobject( pcVM, i_sqObject.GetObjectA() );
      sq_getclass( pcVM, -1 );
      sqClass = AttachToStackObject( -1 );
      sq_pop( pcVM, 2 );

#ifdef _DEBUG
      rumAssert( iTop == pcVM->_top );
#endif
    }
    else if( i_sqObject.GetType() == OT_STRING )
    {
      Sqrat::Object sqRoot{ Sqrat::RootTable( pcVM ) };
      sqClass = sqRoot.GetSlot( i_sqObject.Cast<std::string>().c_str() );
    }

    return sqClass;
  }


  Sqrat::Object GetConstTableEntry( const std::string& i_strName )
  {
    return Sqrat::ConstTable().GetSlot( i_strName.c_str() );
  }


  const std::string& GetFolderName()
  {
    static std::string strFolderName( SCRIPT_FOLDER_NAME );
    return strFolderName;
  }


#ifdef _DEBUG

  std::string GetInstanceName( HSQUIRRELVM i_pcVM, const Sqrat::Object& i_sqInstance )
  {
    const SQInteger iTopBegin{ sq_gettop( i_pcVM ) };

    if( !sq_isinstance( i_sqInstance.GetObjectA() ) )
    {
      const SQInteger iTopEnd{ sq_gettop( i_pcVM ) };
      rumAssert( iTopBegin == iTopEnd );
      return "<non-instanced object>";
    }

    // Push the instance on the stack
    sq_pushobject( i_pcVM, i_sqInstance.GetObjectA() );

    // Get the class of the instance
    sq_getclass( i_pcVM, -1 );

    HSQOBJECT sqClass;
    sq_getstackobj( i_pcVM, -1, &sqClass );

    const std::string strName{ GetNameFromRootTable( i_pcVM, sqClass ) };

    // Pop the instance and the class
    sq_pop( i_pcVM, 2 );

    const SQInteger iTopEnd{ sq_gettop( i_pcVM ) };
    assert( iTopBegin == iTopEnd );

    return strName;
  }

#endif // _DEBUG

  const std::string& GetLastCompilerError()
  {
    return g_strLastCompilerError;
  }


  const std::string& GetLastError()
  {
    return g_strLastError;
  }


#ifdef _DEBUG

  std::string GetNameFromRootTable( HSQUIRRELVM i_pcVM, const Sqrat::Object& i_sqObject )
  {
    if( !( sq_isarray( i_sqObject.GetObjectA() ) ||
           sq_istable( i_sqObject.GetObjectA() ) ||
           sq_isclass( i_sqObject.GetObjectA() ) ) )
    {
      rumAssertMsg( false, "Called GetNameFromRootTable on an unnamed object" );
      return "<unnamed object>";
    }

    const SQInteger iTop{ sq_gettop( i_pcVM ) };

    sq_pushobject( i_pcVM, i_sqObject.GetObjectA() );
    const auto iObjectHash{ sq_gethash( i_pcVM, -1 ) };
    sq_poptop( i_pcVM );

    sq_pushroottable( i_pcVM );
    const auto iRootTableHash{ sq_gethash( i_pcVM, -1 ) };

    // Early out if this is the root table
    if( iRootTableHash == iObjectHash )
    {
      sq_poptop( i_pcVM );

      const SQInteger iTopEnd{ sq_gettop( i_pcVM ) };
      rumAssert( iTop == iTopEnd );

      return "<RootTable>";
    }

    // Push iteration index
    SQInteger iIndex{ 0 };

    std::string strName;
    bool bSuccess{ false };

    do
    {
      sq_pushinteger( i_pcVM, iIndex );

      // Iterate
      if( bSuccess = SQ_SUCCEEDED( sq_next( i_pcVM, -2 ) ) )
      {
        // The object value is at position -1, so get its hash
        const auto iIterHash{ sq_gethash( i_pcVM, -1 ) };
        if( iObjectHash == iIterHash )
        {
          // Get the entry's key
          HSQOBJECT sqKey;
          sq_getstackobj( i_pcVM, -2, &sqKey );
          strName = sq_objtostring( &sqKey );
        }

        // Next iteration index
        sq_getinteger( i_pcVM, -3, &iIndex );

        // Pop the key and value
        sq_pop( i_pcVM, 2 );
      }

      // Pop the iterator index
      sq_poptop( i_pcVM );
    } while( bSuccess && strName.empty() );

    // Pop the root table
    sq_poptop( i_pcVM );

    const SQInteger iTopEnd{ sq_gettop( i_pcVM ) };
    rumAssert( iTop == iTopEnd );

    return strName;
  }


  uint32_t GetNumVMs()
  {
    return (uint32_t)g_cVMMap.size();
  }


  std::string GetObjectName( HSQUIRRELVM i_pcVM, const Sqrat::Object& i_sqObject )
  {
    const SQInteger iTopBegin{ sq_gettop( i_pcVM ) };

    sq_pushobject( i_pcVM, i_sqObject.GetObjectA() );

    std::string strName;

    const auto eType{ sq_gettype( i_pcVM, -1 ) };
    switch( eType )
    {
      case OT_ARRAY:
      case OT_CLASS:
      case OT_TABLE:
        strName = GetNameFromRootTable( i_pcVM, i_sqObject );
        break;

      case OT_INSTANCE:
        strName = GetInstanceName( i_pcVM, i_sqObject );
        break;

      case OT_CLOSURE:
      {
        if( SQ_SUCCEEDED( sq_getclosurename( i_pcVM, -1 ) ) )
        {
          const ::SQChar* strVal{ nullptr };
          if( SQ_SUCCEEDED( sq_getstring( i_pcVM, -1, &strVal ) ) )
          {
            strName = strVal ? strVal : "<anonymous closure>";
            sq_poptop( i_pcVM );
          }
        }
        else
        {
          strName = "<invalid closure>";
        }

        break;
      }

      default:
        strName = "<unnamed object>";
        break;
    }

    // Pop the object
    sq_poptop( i_pcVM );

    const SQInteger iTopEnd{ sq_gettop( i_pcVM ) };
    rumAssert( iTopBegin == iTopEnd );

    return strName;
  }

#endif // _DEBUG


  uint32_t GetObjectRefCount( Sqrat::Object& i_sqObject )
  {
    HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };
    return (uint32_t)sq_getrefcount( pcVM, &i_sqObject.GetObjectA() );
  }


  std::string GetObjectTypeName( const Sqrat::Object& i_sqObject )
  {
    // Get the string equivalent of the object value
    return GetTypeName( i_sqObject.GetObjectA() );
  }


  const std::string GetScriptPath()
  {
    const auto& rcVM{ g_cVMMap[rumVirtualMachine::s_eCurrentVM] };
    return rcVM.m_fsScriptPath.generic_string();
  }


  Sqrat::Object GetStackObject( SQInteger i_iIndex, HSQUIRRELVM i_pcVM )
  {
    const SQInteger iTop{ sq_gettop( i_pcVM ) };

    HSQOBJECT sqObj;
    if( i_iIndex > 0 && i_iIndex <= iTop )
    {
      sq_getstackobj( i_pcVM, i_iIndex, &sqObj );
    }

    return sqObj;
  }


  VMType GetCurrentVMType()
  {
    return rumVirtualMachine::s_eCurrentVM;
  }


  Sqrat::Object GetWeakReference( const Sqrat::Object& i_sqObject )
  {
    HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

    Sqrat::Object sqWeakRef;

    if( ISREFCOUNTED( i_sqObject.GetType() ) )
    {
#ifdef _DEBUG
      const SQInteger iTop{ pcVM->_top };
#endif

      sq_pushobject( pcVM, i_sqObject.GetObjectA() );
      sq_weakref( pcVM, -1 );
      sqWeakRef = AttachToStackObject( -1 );
      sq_pop( pcVM, 2 );

#ifdef _DEBUG
      rumAssert( iTop == pcVM->_top );
#endif
    }

    return sqWeakRef;
  }


  Sqrat::Object GetWeakReferenceValue( const Sqrat::Object& i_sqWeakRef )
  {
    HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

    Sqrat::Object sqObject;

    if( i_sqWeakRef.GetType() == OT_WEAKREF )
    {
#ifdef _DEBUG
      const SQInteger iTop{ pcVM->_top };
#endif

      // Get the weak reference's instance
      sq_pushobject( pcVM, i_sqWeakRef.GetObjectA() );
      sq_getweakrefval( pcVM, -1 );
      sqObject = AttachToStackObject( -1 );
      sq_pop( pcVM, 2 );

#ifdef _DEBUG
      rumAssert( iTop == pcVM->_top );
#endif
    }

    return sqObject;
  }


  int32_t Init( const VMType i_eVM, const std::string& strPath )
  {
    //-------------------------------------------------------------------------
    // Initialize Squirrel Virtual machine
    //-------------------------------------------------------------------------
    if( g_cVMMap.empty() )
    {
      RUM_COUT( "Starting Squirrel virtual machine...\n" << SQUIRREL_VERSION << " " << SQUIRREL_COPYRIGHT << '\n' );
    }

    // Check the path
    std::filesystem::path fsPath( strPath );
    if( !std::filesystem::exists( fsPath ) || !std::filesystem::is_directory( fsPath ) )
    {
      std::string strError{ "The script path " };
      strError += fsPath.generic_string();
      strError += " is invalid";
      Logger::LogStandard( strError );

      return RESULT_FAILED;
    }

    Sqrat::SqratVM* pcVM{ new Sqrat::SqratVM( /*4096*/ ) };
    pcVM->SetPrintFunc( PrintFunc, PrintFunc );
    pcVM->SetErrorHandler( ErrorFunc, CompilerErrorFunc );

    // Create a new VMStruct on our list of VMs
    const auto cPair{ g_cVMMap.insert( std::make_pair( i_eVM, rumVirtualMachine() ) ) };
    if( !cPair.second )
    {
      std::string strError{ "Failed to create new VMStruct for vm_id " };
      strError += rumStringUtils::ToString( i_eVM );
      Logger::LogStandard( strError, Logger::LOG_ERROR );
      return RESULT_FAILED;
    }

    // Ref the newly added struct so we can begin to fill it out
    rumVirtualMachine& rcVM{ cPair.first->second };
    rcVM.m_pcSqratVM = pcVM;
    rcVM.m_pcVM = pcVM->GetVM();
    rcVM.m_fsScriptPath = fsPath;

    // By initializing a new VM, we automatically switch to it
    rcVM.s_eCurrentVM = rcVM.m_eVM = i_eVM;
    Sqrat::DefaultVM::Set( rcVM.m_pcVM );

    Bind();

    return RESULT_SUCCESS;
  }


  bool InstanceOf( const Sqrat::Object& i_sqInstance, const Sqrat::Object& i_sqClass )
  {
    HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

#ifdef _DEBUG
    const SQInteger iTop{ pcVM->_top };
#endif

    // Type checks not necessary, sq_instanceof does these checks
    sq_pushobject( pcVM, i_sqClass.GetObjectA() );
    sq_pushobject( pcVM, i_sqInstance.GetObjectA() );
    SQBool bInstance{ sq_instanceof( pcVM ) };
    sq_pop( pcVM, 2 );

#ifdef _DEBUG
    rumAssert( iTop == pcVM->_top );
#endif

    return ( bInstance != 0 ? true : false );
  }


  int32_t Load( const std::string& i_strFilePath )
  {
    return LoadInternal( i_strFilePath, PROCESS_LOAD_AND_RUN, true );
  }


  int32_t LoadInternal( const std::string& i_strFilePath, ProcessType i_eProcessType, bool i_bReload )
  {
    int32_t eResult{ RESULT_SUCCESS };

    const auto& rcVM{ g_cVMMap[rumVirtualMachine::s_eCurrentVM] };

    try
    {
      std::filesystem::path fsPath( rcVM.m_fsScriptPath / i_strFilePath );
      std::filesystem::path fsBinPath( fsPath );

      std::string strExt{ fsPath.extension().string() };
      if( strExt.compare( ".nut" ) == 0 )
      {
        fsBinPath.replace_extension( ".cnut" );
      }

      if( !std::filesystem::exists( fsPath ) && !std::filesystem::exists( fsBinPath ) )
      {
        // Try other paths based on the current VM
        if( GetCurrentVMType() == VM_SERVER )
        {
          fsPath = rcVM.m_fsScriptPath / SERVER_FOLDER_NAME / i_strFilePath;
        }
        else if( GetCurrentVMType() == VM_CLIENT )
        {
          fsPath = rcVM.m_fsScriptPath / CLIENT_FOLDER_NAME / i_strFilePath;
        }
        else if( GetCurrentVMType() == VM_EDITOR )
        {
          fsPath = rcVM.m_fsScriptPath / EDITOR_FOLDER_NAME / i_strFilePath;
        }

        fsBinPath = fsPath;

        strExt = fsPath.extension().string();
        if( strExt.compare( ".nut" ) == 0 )
        {
          fsBinPath.replace_extension( ".cnut" );
        }
      }

      if(std::filesystem::exists( fsPath ) || std::filesystem::exists( fsBinPath ) )
      {
        if( is_regular_file( fsPath ) || is_regular_file( fsBinPath ) )
        {
          // Load file will internally switch to the binary equivalent if necessary
          eResult = LoadFile( fsPath, i_eProcessType, i_bReload );
        }
        else
        {
          // Recursively load all squirrel scripts in the specified path
          eResult = LoadFolder( fsPath, i_eProcessType, i_bReload );
        }
      }
      else
      {
        std::string strError{ "The specified script '" };
        strError += fsPath.generic_string();
        strError += "' (or binary equivalent) does not exist";
        Logger::LogStandard( strError );
        eResult = RESULT_FAILED;
      }
    }
    catch( ... )
    {
      std::string strError{ "An exception occurred while loading script: " };
      strError += i_strFilePath;
      Logger::LogStandard( strError );
      eResult = RESULT_FAILED;
    }

    return eResult;
  }


  int32_t LoadFile( const std::filesystem::path& i_fsPath, ProcessType i_eProcessType, bool i_bReload )
  {
    int32_t eResult{ RESULT_SUCCESS };

    std::filesystem::path fsFilepath( i_fsPath );

    // Get the native name of the file (with relative path)
    std::string strBase{ i_fsPath.filename().string() };
    rumStringUtils::ToLower( strBase );

    std::string strExt{ i_fsPath.extension().string() };
    rumStringUtils::ToLower( strExt );

    // Make sure file extension is a squirrel script
    if( ( strExt.compare( ".nut" ) == 0 ) || ( strExt.compare( ".cnut" ) == 0 ) )
    {
      if( i_eProcessType != PROCESS_COMPILE && !std::filesystem::exists( fsFilepath ) &&
          strExt.compare( ".nut" ) == 0 )
      {
        fsFilepath.replace_extension( ".cnut" );
      }

      // Make heavy use of generic strings since string matching is important
      const std::string strGeneric{ fsFilepath.generic_string() };

      bool bLoad{ true };
      bool bReloaded{ false };
      const auto& rcVM{ g_cVMMap[rumVirtualMachine::s_eCurrentVM] };

      // Has this script already been loaded?
      const auto iter{ std::find( rcVM.cLoadedScriptSet.begin(), rcVM.cLoadedScriptSet.end(), strGeneric ) };
      if( iter != rcVM.cLoadedScriptSet.end() )
      {
        // The script has already been loaded
        if( !i_bReload )
        {
#if SCRIPT_DEBUG
          const std::string strWarn{ "Skipping script " + strGeneric + "\nThe script has been previously loaded" };
          Logger::LogStandard( strWarn, Logger::LOG_WARNING );
#endif // SCRIPT_DEBUG
          bLoad = false;
        }
        else
        {
#if SCRIPT_DEBUG
          const std::string strWarn{ "WARNING: Reloading script " + strGeneric };
          Logger::LogStandard( strWarn, Logger::LOG_WARNING );
#endif // SCRIPT_DEBUG
          bReloaded = true;
        }
      }

      if( bLoad )
      {
#if SCRIPT_DEBUG
        const std::string strInfo{ "Loading script: " + strGeneric };
        Logger::LogStandard( strInfo );
#endif // SCRIPT_DEBUG

        Sqrat::Script sqScript;
        sqScript.CompileFile( strGeneric.c_str() );

        HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

        if( ( PROCESS_COMPILE == i_eProcessType ) || ( PROCESS_COMPILE_AND_RUN == i_eProcessType ) )
        {
          // Determine compiled folder location
          std::filesystem::path compiled_path( i_fsPath );
          compiled_path.replace_extension( ".cnut" );

#ifdef _DEBUG
          const SQInteger iTop{ pcVM->_top };
#endif

          // Write the compiled script
          sqScript.WriteCompiledFile( compiled_path.string().c_str() );

#ifdef _DEBUG
          rumAssert( iTop == pcVM->_top );
#endif
        }

        if( ( PROCESS_LOAD_AND_RUN == i_eProcessType ) || ( PROCESS_COMPILE_AND_RUN == i_eProcessType ) )
        {
          sqScript.Run();
          if( Sqrat::Error::Occurred( pcVM ) )
          {
            std::string strError{ "Error: Failure occurred during script compilation of " };
            strError += fsFilepath.string().c_str();
            strError += ": ";
            strError += Sqrat::Error::Message( pcVM );
            Logger::LogStandard( strError );

            eResult = RESULT_FAILED;
          }
          else if( !bReloaded )
          {
            // Remember that this script has been loaded. Use generic string since exact string matching is important!
            g_cVMMap[rumVirtualMachine::s_eCurrentVM].cLoadedScriptSet.push_back( strGeneric );
          }
        }
      }
    }

    return eResult;
  }


  int32_t LoadFolder( const std::filesystem::path& i_fsFolder, ProcessType i_eProcessType, bool i_bReload )
  {
    int32_t eResult{ RESULT_SUCCESS };

    std::filesystem::directory_iterator iter( i_fsFolder );
    const std::filesystem::directory_iterator end; // default construction yields past-the-end
    while( ( RESULT_SUCCESS == eResult ) && iter != end )
    {
      try
      {
        // If a directory encountered, recurse into the directory
        if(std::filesystem::is_directory( iter->status() ) )
        {
          // Recurse into directory
          eResult = LoadFolder( iter->path(), i_eProcessType, i_bReload );
        }
        else
        {
          eResult = LoadFile( iter->path(), i_eProcessType, i_bReload );
        }
      }
      catch( const std::exception& cException )
      {
        std::string strError{ "Error: " };
        strError += iter->path().filename().string();
        strError += " ";
        strError += cException.what();
        Logger::LogStandard( strError );

        eResult = RESULT_FAILED;
      }

      ++iter;
    }

    return eResult;
  }


  SQInteger PrintCallstack( HSQUIRRELVM i_pcVM )
  {
    sqstd_printcallstack( i_pcVM );
    return 0;
  }


  void PrintFunc( HSQUIRRELVM i_pcVM, const SQChar* i_strDesc, ... )
  {
    va_list strArgList;
    va_start( strArgList, i_strDesc );
    vprintf( i_strDesc, strArgList );
    va_end( strArgList );
  }


  Sqrat::Object ReadConfig( const std::string& i_strFilePath )
  {
    HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

    Sqrat::Table sqTable( pcVM );

    // The ini filename should already be in the native format
    dictionary* pcDictionary{ iniparser_new( i_strFilePath.c_str() ) };
    if( pcDictionary )
    {
      // Read all keys and values from the dictionary
      for( int32_t i = 0; i < pcDictionary->n; ++i )
      {
        if( !pcDictionary->key[i] )
        {
          continue;
        }

        std::string strKey{ pcDictionary->key[i] };
        if( strKey.find( ":" ) != std::string::npos )
        {
          if( strKey[0] == ':' )
          {
            // Sectionless, entry - assign a section
            strKey = "ini" + strKey;

            std::string strError{ "Adding key to default configuration section: " };
            strError += strKey;
            Logger::LogStandard( strError, Logger::LOG_ERROR );

            if( iniparser_getnsec( pcDictionary ) == 0 )
            {
              // Add the default section
              iniparser_setstr( pcDictionary, "ini", nullptr );
              sqTable.SetValue( "ini", '\0' );
            }
          }

          sqTable.SetValue( strKey.c_str(), pcDictionary->val[i] );
        }
      }

      iniparser_free( pcDictionary );
    }

    return sqTable;
  }


  int32_t Require( const std::string& i_strFilePath )
  {
    return LoadInternal( i_strFilePath, PROCESS_LOAD_AND_RUN, false );
  }


  bool SetSlot( Sqrat::Object& i_sqObject, Sqrat::Object& i_sqKey, Sqrat::Object& i_sqValue )
  {
    HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

    bool bResult{ false };
    const SQInteger iTop{ sq_gettop( pcVM ) };
    sq_pushobject( pcVM, i_sqObject.GetObjectA() );
    sq_pushobject( pcVM, i_sqKey.GetObjectA() );
    sq_pushobject( pcVM, i_sqValue.GetObjectA() );
    if( SQ_SUCCEEDED( sq_rawset( pcVM, -3 ) ) )
    {
      bResult = true;
    }

    sq_settop( pcVM, iTop );

    return bResult;
  }


  bool SetCurrentVMType( VMType i_eVM )
  {
    // Early out if the request VM is already set
    if( rumVirtualMachine::s_eCurrentVM == i_eVM )
    {
      return true;
    }

    // Early out if we're requesting a vm that couldn't possibly exist
    if( g_cVMMap.size() <= i_eVM )
    {
      return false;
    }

    Sqrat::DefaultVM::Set( g_cVMMap[i_eVM].m_pcVM );

    // We just switched VMs, track the id
    rumVirtualMachine::s_eCurrentVM = i_eVM;

    return true;
  }


  void Shutdown()
  {
    // Remove VMs in the order they were initialized
    VMContainer::reverse_iterator iter( g_cVMMap.rbegin() );
    VMContainer::reverse_iterator end( g_cVMMap.rend() );
    while( iter != end )
    {
      auto& cVM{ iter->second };

      cVM.cLoadedScriptSet.clear();
      cVM.m_cScriptRegistryHash.clear();

      if( cVM.m_pcVM )
      {
        sq_collectgarbage( cVM.m_pcVM );

        // Release scriptVMs - this is where Squirrel will complain that script objects are still holding onto references
        sq_close( cVM.m_pcVM );
        cVM.m_pcVM = nullptr;
      }

      ++iter;
    }

    g_cVMMap.clear();

    RUM_COUT( "********************************\n" );
    RUM_COUT( "Script VMs shutdown successfully\n" );
    RUM_COUT( "********************************\n" );

    rumVirtualMachine::s_eCurrentVM = VM_INVALID;
  }


  void ShutdownVM( VMType i_eVM )
  {
    auto iter{ g_cVMMap.find( i_eVM ) };
    if( iter != g_cVMMap.end() )
    {
      auto& cVM{ iter->second };

      cVM.cLoadedScriptSet.clear();
      cVM.m_cScriptRegistryHash.clear();

      if( cVM.m_pcVM )
      {
        sq_collectgarbage( cVM.m_pcVM );
        sq_close( cVM.m_pcVM );
        cVM.m_pcVM = nullptr;
      }
    }

    SetCurrentVMType( VM_INVALID );
  }


  bool StartupScriptExecuted()
  {
    auto& rcVM{ g_cVMMap.at( GetCurrentVMType() ) };
    return rcVM.m_bStartupExecuted;
  }


  std::string ToDisplayString( PropertyValueType i_eValueType, const Sqrat::Object& i_sqObject )
  {
    if( PropertyValueType::Bool == i_eValueType )
    {
      return ( i_sqObject.Cast<bool>() ? "true" : "false" );
    }
    else if( i_eValueType >= PropertyValueType::FirstAssetRef && i_eValueType <= PropertyValueType::LastAssetRef )
    {
      const int32_t eValue{ i_sqObject.Cast<int32_t>() };
      if( INVALID_ASSET_ID == eValue )
      {
        return "<Invalid>";
      }

      const auto* pcAsset{ rumAsset::Fetch( eValue ) };
      if( pcAsset != nullptr )
      {
        return pcAsset->GetName();
      }

      return rumStringUtils::ToHexString( eValue );
    }
    else if( PropertyValueType::StringToken == i_eValueType )
    {
      const int32_t eValue{ i_sqObject.Cast<int32_t>() };
      std::string& strName{ rumStringTable::GetTokenName( eValue ) };
      if( !strName.empty() )
      {
        return strName;
      }

      return rumStringUtils::ToHexString( eValue );
    }

    return ToString( i_sqObject );
  }


  std::string ToSerializationString( PropertyValueType i_eValueType, const Sqrat::Object& i_sqObject )
  {
    if( ( i_eValueType >= PropertyValueType::FirstAssetRef && i_eValueType <= PropertyValueType::LastAssetRef ) ||
        ( PropertyValueType::StringToken == i_eValueType ) )
    {
      return rumStringUtils::ToHexString( i_sqObject.Cast<int32_t>() );
    }
    
    return ToSerializationString( i_sqObject );
  }


  std::string ToSerializationString( const Sqrat::Object& i_sqObject )
  {
    switch( i_sqObject.GetType() )
    {
      case OT_INTEGER: return rumStringUtils::ToString( i_sqObject.Cast<int32_t>() );
      case OT_FLOAT:   return rumStringUtils::ToFloatString( i_sqObject.Cast<float>() );
      case OT_BOOL:    return i_sqObject.Cast<bool>() ? "1" : "0";
      case OT_STRING:  return i_sqObject.Cast<std::string>();
      case OT_NULL:    return rumStringUtils::NullString();
      default:
        rumAssertMsg( false, "Unsupported type" );
        break;
    }

    return rumStringUtils::NullString();
  }


  std::string ToString( const Sqrat::Object& i_sqObject )
  {
    HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

    // Get the string equivalent of the object value
    SQObjectPtr sqStr;
    pcVM->ToString( i_sqObject.GetObjectA(), sqStr );
    return _stringval( sqStr );
  }


  Sqrat::Object ToValue( PropertyValueType i_eValueType, const std::string& i_strValue )
  {
    Sqrat::Object sqValue;

    if( ( i_eValueType >= PropertyValueType::FirstAssetRef && i_eValueType <= PropertyValueType::LastAssetRef ) ||
        ( PropertyValueType::StringToken == i_eValueType ) )
    {
      rumScript::SetValue( sqValue, rumStringUtils::ToUInt( i_strValue ) );
    }
    else
    {
      switch( i_eValueType )
      {
        case PropertyValueType::Bitfield:
        case PropertyValueType::Integer:
          rumScript::SetValue( sqValue, rumStringUtils::ToInt( i_strValue ) );
          break;

        case PropertyValueType::Bool:
          rumScript::SetValue( sqValue, rumStringUtils::ToBool( i_strValue ) );
          break;

        case PropertyValueType::Float:
          rumScript::SetValue( sqValue, rumStringUtils::ToFloat( i_strValue ) );
          break;

        case PropertyValueType::String:
          rumScript::SetValue( sqValue, i_strValue );
          break;

        default:
          rumAssertMsg( false, "Unsupported value type" );
          break;
      }
    }

    return sqValue;
  }


  void WriteConfig( const std::string& i_strFilePath, const Sqrat::Object& i_sqTable )
  {
    if( i_sqTable.GetType() != OT_TABLE )
    {
      std::string strError{ "Attempt to write invalid data to configuration file: " };
      strError += i_strFilePath;
      Logger::LogStandard( strError, Logger::LOG_ERROR );
    }

    // If the file doesn't exist, create it
    const std::filesystem::path fsPath( i_strFilePath );
    if( !std::filesystem::exists( fsPath ) )
    {
      std::ofstream cOutfile;
      cOutfile.open( i_strFilePath );
      cOutfile.close();
    }

    dictionary* pcDictionary{ iniparser_new( i_strFilePath.c_str() ) };

    // Visit all entries in the table
    Sqrat::Object sqKey, sqValue;
    Sqrat::Object::iterator iter;

    while( i_sqTable.Next( iter ) )
    {
      sqKey = iter.getKey();
      sqValue = iter.getValue();

      if( sqKey.GetType() == OT_STRING )
      {
        const std::string strKey{ sqKey.Cast<std::string>() };

        std::string strValue;
        if( sqValue.GetType() == OT_BOOL )
        {
          strValue = sqValue.Cast<bool>() ? "Yes" : "No";
        }
        else if( sqValue.GetType() == OT_STRING )
        {
          strValue = sqValue.Cast<std::string>();
        }
        else if( sqValue.GetType() == OT_INTEGER || sqValue.GetType() == OT_FLOAT )
        {
          strValue = ToString( sqValue );
        }
        else
        {
          std::string strError{ "Invalid configuration value specified for key name: " };
          strError += strKey;
          Logger::LogStandard( strError, Logger::LOG_ERROR );
        }

        if( !strValue.empty() )
        {
          iniparser_setstr( pcDictionary, strKey.c_str(), strValue.c_str() );
        }
      }
      else
      {
        std::string strError{ "Invalid configuration key name: " };
        strError += sqKey.Cast<std::string>();
        Logger::LogStandard( strError, Logger::LOG_ERROR );
      }
    }

    // Export to file
    if( iniparser_dump_ini_file( pcDictionary, i_strFilePath.c_str() ) != 0 )
    {
      std::string strError{ "Failed to update configuration file: " };
      strError += i_strFilePath;
      Logger::LogStandard( strError, Logger::LOG_ERROR );
    }

    iniparser_free( pcDictionary );
  }
} // namespace script
