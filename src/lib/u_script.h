#ifndef _U_SCRIPT_H_
#define _U_SCRIPT_H_

#include <u_enum.h>
#include <u_log.h>

#include <sqrat.h>

#include <string>

class rumClassRegistry;


namespace rumScript
{
  enum VMType { VM_INVALID = -1, VM_SERVER, VM_CLIENT, VM_EDITOR, NUM_VM };

  Sqrat::Object AttachToStackObject( int32_t i_iIndex );

  // Signal to game scripts that the game is ready to start
  bool CallInitFunction( const std::string& i_strInitFunc = "" );

  // Signal to game scripts that the game is shutting down
  void CallShutdownFunction();

  // Generates a monolithic byte-code script
  void Compile( VMType i_eVM, std::string_view i_strPath );

  // Creates a deafult class description with the given name and base class
  Sqrat::Object CreateClassScript( const std::string& i_strName, const std::string& i_strExtends );

  // Creates an instance of a class with given params
  Sqrat::Object CreateInstance( const Sqrat::Object& i_sqClass, Sqrat::Table i_sqParams );

  // Converts a script-defined enum to string. Set i_bBitfield if the results if the individual values in the enum are
  // used as bit values.
  std::string EnumToString( const std::string& i_strEnum, int32_t i_eValue, bool i_bBitfield );

  SQInteger ErrorFunc( HSQUIRRELVM i_pcVM );

  // Calls an optional function and returns a value for a given type
  template<typename T, typename... TArgs>
  std::pair<bool, T> EvalOptionalFunc( Sqrat::Object i_sqObject, const std::string& i_strFunc, T i_vDefault,
                                       TArgs... i_TArgs );

  // Calls a required function and returns a value for a given type
  template<typename T, typename... TArgs>
  std::pair<bool, T> EvalRequiredFunc( Sqrat::Object i_sqObject, const std::string& i_strFunc, T i_vDefault,
                                       TArgs... i_TArgs );

  // Calls an optional function and returns true if the function was successfully executed
  template<typename... TArgs>
  bool ExecOptionalFunc( Sqrat::Object i_sqObject, const std::string& i_strFunc, TArgs... i_TArgs );

  // Calls a required function and returns true if the function was successfully executed
  template<typename... TArgs>
  bool ExecRequiredFunc( Sqrat::Object i_sqObject, const std::string& i_strFunc, TArgs... i_TArgs );

  int32_t ExecuteStartupScript( const VMType i_eVM );

  Sqrat::Object GetBaseClass( Sqrat::Object& i_sqClass );

  std::string GetCallstack( HSQUIRRELVM i_pcVM );
  SQInteger PrintCallstack( HSQUIRRELVM i_pcVM );

  Sqrat::Object GetClass( rumAssetID i_eAssetID );
  Sqrat::Object GetClass( const Sqrat::Object& i_sqObject );

  rumClassRegistry* GetOrCreateClassRegistry( AssetType i_uiClassTag );

  Sqrat::Object GetConstTableEntry( const std::string& i_strName );

  // Returns the name of the folder script files are stored in
  const std::string& GetFolderName();

#ifdef _DEBUG
  // Find the name of an instance
  std::string GetInstanceName( HSQUIRRELVM i_pcVM, const Sqrat::Object& i_sqInstance );
#endif // _DEBUG

  const std::string& GetLastCompilerError();
  const std::string& GetLastError();

#ifdef _DEBUG
  // Find the name of a class, array, or table
  std::string GetNameFromRootTable( HSQUIRRELVM i_pcVM, const Sqrat::Object& i_sqObject );

  // The best all-purpose look-up method when the type is unknown. This method first looks up the type of the object
  // and then calls GetInstanceName() or GetNameFromRootTable() as needed.
  std::string GetObjectName( HSQUIRRELVM i_pcVM, const Sqrat::Object& i_sqObject );
#endif // _DEBUG

  uint32_t GetObjectRefCount( Sqrat::Object& i_sqObject );

  std::string GetObjectTypeName( const Sqrat::Object& i_sqObject );

  uint32_t GetNumVMs();

  const std::string GetScriptPath();

  Sqrat::Object GetStackObject( SQInteger i_iIndex, HSQUIRRELVM i_pcVM = Sqrat::DefaultVM::Get() );

  VMType GetCurrentVMType();
  bool SetCurrentVMType( VMType i_eVM );

  Sqrat::Object GetWeakReference( const Sqrat::Object& i_sqObject );
  Sqrat::Object GetWeakReferenceValue( const Sqrat::Object& i_sqWeakRef );

  // Initializes the script system
  int32_t Init( const VMType i_eVM, const std::string& strPath );

  bool IsInstanceOf( const Sqrat::Object& i_sqInstance, const Sqrat::Object& i_sqClass );

  // Loads or reloads a script if it has already been loaded
  int32_t Load( const std::string& i_strFilePath );

  // Loads a script only if it has never been loaded
  int32_t Require( const std::string& i_strFilePath );

  Sqrat::Object ReadConfig( const std::string& i_strFilePath );
  void WriteConfig( const std::string& i_strFilePath, const Sqrat::Object& i_sqTable );

  bool SetSlot( Sqrat::Object& i_sqObject, Sqrat::Object& i_sqKey, Sqrat::Object& i_sqValue );

  template<typename T>
  Sqrat::Object SetValue( Sqrat::Object& o_sqObject, const T& i_TVal );

  void Shutdown();
  void ShutdownVM( VMType i_eVM );

  bool StartupScriptExecuted();

  // Most suited for human-readable values
  std::string ToDisplayString( PropertyValueType i_eValueType, const Sqrat::Object& i_sqObject );

  Sqrat::Object ToValue( PropertyValueType i_eValueType, const std::string& i_strValue );

  std::string ToSerializationString( PropertyValueType i_eValueType, const Sqrat::Object& i_sqObject );
  std::string ToSerializationString( const Sqrat::Object& i_sqObject );
  std::string ToString( const Sqrat::Object& i_sqObject );
}

#include <u_script.inl>

#endif //_U_SCRIPT_H_
