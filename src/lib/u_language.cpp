#include <u_language.h>

#include <u_db.h>
#include <u_log.h>
#include <u_rum.h>

#include <filesystem>
#include <fstream>

#define LANGUAGE_TABLE_NAME "language"

rumLanguageID rumLanguage::s_iActiveLanguageID{ DEFAULT_LANGUAGE_ID };
rumLanguageID rumLanguage::s_iNextLanguageID{ 0 };
std::unordered_map<rumLanguageID, rumLanguage> rumLanguage::s_hashLanguages;


// static
bool rumLanguage::AddLanguage( rumLanguageID i_iID, const std::string& i_strName, const std::string& i_strCode )
{
  rumLanguage cLanguage;
  cLanguage.m_iID = i_iID;
  cLanguage.m_strName = i_strName;
  cLanguage.m_strCode = i_strCode;

  // Expose the language name to scripts
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };
  const std::string strLanguageID{ i_strName + "_LanguageID" };
  Sqrat::ConstTable( pcVM ).Const( strLanguageID.c_str(), i_iID );

  if( i_iID >= s_iNextLanguageID )
  {
    s_iNextLanguageID = i_iID + 1;
  }

  const auto cPair{ s_hashLanguages.insert( std::make_pair( i_iID, cLanguage ) ) };
  return cPair.second;
}


// static
void rumLanguage::ExportCSVFile( const std::string& i_strPath )
{
  if( !s_hashLanguages.empty() )
  {
    const std::filesystem::path fsFilepath( std::filesystem::path( i_strPath ) / CSV_FOLDER_NAME /
                                            LANGUAGE_TABLE_NAME / CSV_EXTENSION );

    std::filesystem::create_directories( fsFilepath.parent_path() );

    std::ofstream cOutfile( fsFilepath, std::ios::out | std::ios::trunc );
    if( cOutfile.is_open() )
    {
      // Sort the hash
      using SortedHash = std::map< rumLanguageID, rumLanguage >;
      const SortedHash hashLanguages( s_hashLanguages.begin(), s_hashLanguages.end() );

      for( const auto iter : hashLanguages )
      {
        const rumLanguageID iID{ iter.first };
        const auto& rcLanguage{ iter.second };
        cOutfile << iID << ',' << rcLanguage.GetName() << ',' << rcLanguage.GetCode() << '\n';
      }

      cOutfile.close();
    }

    // Remove the file stub if nothing was written
    if( std::filesystem::file_size( fsFilepath ) == 0U )
    {
      std::filesystem::remove( fsFilepath );
    }
  }
}


// static
void rumLanguage::ExportDBTable()
{
  rumDatabase::Query( rumDatabase::Strings_DatabaseID, "DROP TABLE language" );
  rumDatabase::Query( rumDatabase::Strings_DatabaseID, "CREATE TABLE[language]("
                                                       "[name] TEXT NOT NULL UNIQUE,"
                                                       "[id] INTEGER NOT NULL UNIQUE,"
                                                       "[code] TEXT NOT NULL UNIQUE);" );

  rumDatabase::Query( rumDatabase::Strings_DatabaseID, "BEGIN TRANSACTION" );

  // Sort the hash
  using SortedHash = std::map< rumLanguageID, rumLanguage >;
  const SortedHash hashLanguages( s_hashLanguages.begin(), s_hashLanguages.end() );

  for( const auto iter : hashLanguages )
  {
    const rumLanguageID iID{ iter.first };
    const auto& rcLanguage{ iter.second };
    std::string strQuery{ "INSERT INTO " LANGUAGE_TABLE_NAME " (name,id,code) VALUES('" };
    strQuery += rcLanguage.GetName();
    strQuery += "',";
    strQuery += rumStringUtils::ToString( iID );
    strQuery += ",'";
    strQuery += rcLanguage.GetCode();
    strQuery += "')";
    const QueryPtr pcQuery{ rumDatabase::Query( rumDatabase::Strings_DatabaseID, strQuery ) };
    rumAssert( !pcQuery->IsError() );
  }

  rumDatabase::Query( rumDatabase::Strings_DatabaseID, "COMMIT" );
}


// static
rumLanguageID rumLanguage::GetLanguageIDFromCode( const std::string& i_strCode )
{
  for( const auto& iter : s_hashLanguages )
  {
    const rumLanguageID eID{ iter.first };
    const rumLanguage& rcLanguage{ iter.second };
    if( strcasecmp( rcLanguage.GetCode().c_str(), i_strCode.c_str() ) != 0 )
    {
      return eID;
    }
  }

  return INVALID_LANGUAGE_ID;
}


// static
rumLanguageID rumLanguage::GetLanguageIDFromName( const std::string& i_strName )
{
  for( const auto& iter : s_hashLanguages )
  {
    const rumLanguageID eID{ iter.first };
    const rumLanguage& rcLanguage{ iter.second };
    if( strcasecmp( rcLanguage.GetName().c_str(), i_strName.c_str() ) == 0 )
    {
      return eID;
    }
  }

  return INVALID_LANGUAGE_ID;
}


// static
rumLanguage& rumLanguage::GetLanguageInfo( rumLanguageID i_iID )
{
  static rumLanguage sLanguage;

  const auto iter{ s_hashLanguages.find( i_iID ) };
  if( iter != s_hashLanguages.end() )
  {
    return iter->second;
  }

  return sLanguage;
}


// static
std::vector<rumLanguageID> rumLanguage::GetLanguageIDs()
{
  std::vector<rumLanguageID> vLanguageIDs;
  for( const auto iter : s_hashLanguages )
  {
    vLanguageIDs.push_back( iter.first );
  }

  return vLanguageIDs;
}


// static
bool rumLanguage::Init( const std::string& i_strPath, const std::string& i_strLanguage )
{
  rumLanguageID iLanguageID{ rumLanguage::DEFAULT_LANGUAGE_ID };

  s_hashLanguages.clear();

  if( Load( i_strPath ) == RESULT_FAILED )
  {
    Logger::LogStandard( "Failed to load languages" );
    return false;
  }

  // Determine language id from provided string
  if( !i_strLanguage.empty() )
  {
    if( isdigit( i_strLanguage[0] ) )
    {
      iLanguageID = rumStringUtils::ToInt( i_strLanguage );
    }
    else
    {
      iLanguageID = GetLanguageIDFromName( i_strLanguage );
    }

    if( !SetActiveLanguage( iLanguageID ) )
    {
      Logger::LogStandard( "The language '" + i_strLanguage + "' is not supported. Using default language." );
      iLanguageID = rumLanguage::DEFAULT_LANGUAGE_ID;
    }
  }

  return true;
}


// static
bool rumLanguage::IsLanguageSupported( rumLanguageID i_iID )
{
  const rumLanguage& rcLanguage{ GetLanguageInfo( i_iID ) };
  return rcLanguage.IsValid();
}


// static
int rumLanguage::Load( const std::string& i_strPath )
{
  // Try CSV first
  std::filesystem::path fsFilepath{ std::filesystem::path( i_strPath ) / CSV_FOLDER_NAME / LANGUAGE_TABLE_NAME };
  fsFilepath += CSV_EXTENSION;

  std::ifstream cFile( fsFilepath, std::ios::in );
  if( cFile.is_open() )
  {
    std::string strRow;

    while( !cFile.eof() )
    {
      std::getline( cFile, strRow );
      if( !strRow.empty() )
      {
        const auto vFields{ rumStringUtils::ParseCSVRow( strRow ) };
        ParseFields( vFields );
      }
    }

    cFile.close();
  }
  else
  {
    const std::string strQuery{ "SELECT id,name,code FROM language ORDER BY id" };
    const QueryPtr pcQuery{ rumDatabase::Query( rumDatabase::Strings_DatabaseID, strQuery ) };
    if( pcQuery && !pcQuery->IsError() )
    {
      for( int32_t i = 0; i < pcQuery->GetNumRows(); ++i )
      {
        std::vector<std::string> vFields;
        vFields.resize( pcQuery->GetNumCols() );

        for( int32_t j = 0; j < pcQuery->GetNumCols(); ++j )
        {
          vFields[j] = pcQuery->FetchString( i, j );
        }

        ParseFields( vFields );
      }
    }
    else
    {
      // Create the default language
      const rumLanguageID iLanguageID{ 0 };
      const std::string& strName{ "English" };
      const std::string& strCode{ "en" };
      if( !AddLanguage( iLanguageID, strName, strCode ) )
      {
        std::string strError{ "Error: Failed to create entry for language " };
        strError += strName;
        Logger::LogStandard( strError, Logger::LOG_ERROR );
      }
    }
  }

  return( s_hashLanguages.size() > 0 ? RESULT_SUCCESS : RESULT_FAILED );
}


// static
void rumLanguage::ParseFields( const std::vector<std::string>& i_rvFields )
{
  enum { COL_ID, COL_NAME, COL_CODE };

  const rumLanguageID iLanguageID{ (rumLanguageID)rumStringUtils::ToUInt( i_rvFields.at( COL_ID ) ) };
  const std::string& strName{ i_rvFields.at( COL_NAME ) };
  const std::string& strCode{ i_rvFields.at( COL_CODE ) };
  if( !AddLanguage( iLanguageID, strName, strCode ) )
  {
    std::string strError{ "Error: Failed to create entry for language " };
    strError += strName;
    Logger::LogStandard( strError, Logger::LOG_ERROR );
  }
}


// static
bool rumLanguage::RemoveLanguage( rumLanguageID i_iID )
{
  return s_hashLanguages.erase( i_iID ) >= 1;
}


// static
void rumLanguage::ScriptBind()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::Class<rumLanguage> cLanguage( pcVM, "rumLanguage" );
  cLanguage
    .Func( "GetCode", &GetCode )
    .Func( "GetID", &GetID )
    .Func( "GetName", &GetName )
    .Func( "IsValid", &IsValid );
  Sqrat::RootTable( pcVM ).Bind( "rumLanguage", cLanguage );

  Sqrat::RootTable( pcVM )
    .Func( "rumGetActiveLanguageID", GetActiveLanguage )
    .Func( "rumSetActiveLanguageID", SetActiveLanguage )
    .Func( "rumGetLanguage", GetLanguageInfo )
    .Func( "rumGetLanguageIDFromCode", GetLanguageIDFromCode )
    .Func( "rumGetLanguageIDFromName", GetLanguageIDFromName )
    .Func( "rumGetLanguageIDs", GetLanguageIDs )
    .Func( "rumIsLanguageSupported", IsLanguageSupported );

  // Expose all language name to scripts
  for( const auto& iterLanguages : s_hashLanguages )
  {
    const auto& rumLanguage{ iterLanguages.second };
    const std::string strLanguageID{ rumLanguage.GetName() + "_LanguageID" };
    Sqrat::ConstTable( pcVM ).Const( strLanguageID.c_str(), iterLanguages.first );
  }
}


// static
bool rumLanguage::SetActiveLanguage( rumLanguageID i_iID )
{
  if( IsLanguageSupported( i_iID ) )
  {
    s_iActiveLanguageID = i_iID;
    return true;
  }

  return false;
}


// static
void rumLanguage::Shutdown()
{
  // Nothing to do
}
