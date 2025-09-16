#include <u_strings.h>

#include <u_db.h>
#include <u_log.h>
#include <u_rum.h>
#include <u_utility.h>

#include <filesystem>
#include <fstream>

std::unordered_map<rumStringTableID, rumStringTable> rumStringTable::s_hashStringTables;
rumStringTableID rumStringTable::s_iNextStringTableID{ 0 };
rumStringTable rumStringTable::s_cInvalidStringTable;

#define STRING_TABLE_NAME   "string_table"
#define STRING_TABLE_PREFIX "strings_"
#define TOKEN_TABLE_PREFIX  "tokens_"

#define EXTRACT_STRING_TABLE_ID( x ) ( (rumStringTableID)( ( x ) >> 24 ) )


// static
bool rumStringTable::AddStringTable( rumStringTableID i_iStringTableID, const std::string& i_strName,
                                     ServiceType i_eServiceType )
{
  if( i_iStringTableID >= s_iNextStringTableID )
  {
    s_iNextStringTableID = i_iStringTableID + 1;
  }

  rumStringTable cStringTable;
  cStringTable.m_iStringTableID = i_iStringTableID;
  cStringTable.m_strName = i_strName;
  cStringTable.m_eServiceType = i_eServiceType;

  // Shift the string ID into the high byte. This means that there can be 256 unique string tables with 16,777,215
  // individual string entries.
  cStringTable.m_iNextTokenID = i_iStringTableID << 24;

  // Add a token hash for each language
  const std::vector<rumLanguageID> vLanguageIDs{ rumLanguage::GetLanguageIDs() };
  for( auto iter : vLanguageIDs )
  {
    rumLanguageID iLanguageID{ iter };
    cStringTable.AddTokenHash( iLanguageID );
  }

  // Add the new table to the string table hash
  const auto cPair{ s_hashStringTables.insert( std::make_pair( i_iStringTableID, cStringTable ) ) };
  return cPair.second;
}


bool rumStringTable::AddToken( rumTokenID i_iTokenID, const std::string& i_strToken )
{
  if( i_iTokenID >= m_iNextTokenID )
  {
    m_iNextTokenID = i_iTokenID + 1;
  }

  const auto cPair{ m_hashTokens.insert( std::pair( i_iTokenID, i_strToken ) ) };
  if( cPair.second )
  {
    // Expose the token to scripts
    HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };
    const std::string strStringID{ i_strToken + "_" + GetName() + "_StringID" };
    Sqrat::ConstTable( pcVM ).Const( strStringID.c_str(), i_iTokenID );
    return true;
  }

  return false;
}


bool rumStringTable::AddTokenHash( rumLanguageID i_iLanguageID )
{
  const auto cPair{ m_hashTranslations.insert( std::make_pair( i_iLanguageID, rumTokenHash() ) ) };
  return cPair.second;
}


bool rumStringTable::AddTranslation( const rumLanguage& i_rcLanguage, rumTokenID i_iTokenID,
                                     const std::string& i_strTranslation )
{
  // Get the token hash for the specified language
  const auto& iter{ m_hashTranslations.find( i_rcLanguage.GetID() ) };
  if( iter != m_hashTranslations.end() )
  {
    // Add the translation to the token hash
    auto& rcTokenHash{ iter->second };
    const auto cPair{ rcTokenHash.insert( std::pair( i_iTokenID, i_strTranslation ) ) };
    return cPair.second;
  }

  return false;
}


bool rumStringTable::AddTranslation( rumLanguageID i_iLanguageID, rumTokenID i_iTokenID,
                                     const std::string& i_strTranslation )
{
  const rumLanguage& rcLanguageInfo{ rumLanguage::GetLanguageInfo( i_iLanguageID ) };
  return AddTranslation( rcLanguageInfo, i_iTokenID, i_strTranslation );
}


// static
void rumStringTable::ExportCSV( const std::string& i_strPath )
{
  if( s_hashStringTables.empty() )
  {
    return;
  }

  const std::filesystem::path fsFilepath( std::filesystem::path( i_strPath ) / CSV_FOLDER_NAME / STRING_TABLE_NAME /
                                          CSV_EXTENSION );

  std::filesystem::create_directories( fsFilepath.parent_path() );

  std::ofstream cOutfile( fsFilepath, std::ios::out | std::ios::trunc );
  if( cOutfile.is_open() )
  {
    // Sort the hash
    using SortedHash = std::map< rumStringTableID, rumStringTable >;
    const SortedHash hashStringTables( s_hashStringTables.begin(), s_hashStringTables.end() );

    for( const auto& iter : hashStringTables )
    {
      const rumStringTableID iStringTableID{ iter.first };
      const auto& rcStringTable{ iter.second };
      cOutfile << iStringTableID << ',' << rcStringTable.GetName() << ',' << rcStringTable.GetServiceType() << '\n';
    }

    cOutfile.close();
  }

  // Remove the file stub if nothing was written
  if( std::filesystem::file_size( fsFilepath ) == 0U )
  {
    std::filesystem::remove( fsFilepath );
  }
}


void rumStringTable::ExportCSVFiles( const std::string& i_strPath ) const
{
  if( !m_hashTokens.empty() )
  {
    std::filesystem::path fsTokenFilepath( std::filesystem::path( i_strPath ) / CSV_FOLDER_NAME / TOKEN_TABLE_PREFIX );
    fsTokenFilepath += m_strName;
    fsTokenFilepath += CSV_EXTENSION;

    std::filesystem::create_directories( fsTokenFilepath.parent_path() );

    std::ofstream cOutfile( fsTokenFilepath, std::ios::out | std::ios::trunc );
    if( cOutfile.is_open() )
    {
      ExportCSVTokenFile( cOutfile );
      cOutfile.close();
    }

    // Remove the file stub if nothing was written
    if( std::filesystem::file_size( fsTokenFilepath ) == 0U )
    {
      std::filesystem::remove( fsTokenFilepath );
    }
  }

  // Export the string table for each language
  for( const auto& iter : m_hashTranslations )
  {
    const rumLanguageID iLanguageID{ iter.first };
    const rumTokenHash& rcTokenHash{ iter.second };
    if( !rcTokenHash.empty() )
    {
      const rumLanguage& rcLanguage{ rumLanguage::GetLanguageInfo( iLanguageID ) };
      if( rcLanguage.IsValid() )
      {
        std::filesystem::path fsStringTableFilepath( std::filesystem::path( i_strPath ) / CSV_FOLDER_NAME /
		                                             STRING_TABLE_PREFIX );
        fsStringTableFilepath += m_strName;
        fsStringTableFilepath += "_";
        fsStringTableFilepath += rcLanguage.GetCode();
        fsStringTableFilepath += CSV_EXTENSION;

        std::ofstream cOutfile( fsStringTableFilepath, std::ios::out | std::ios::trunc );
        if( cOutfile.is_open() )
        {
          ExportCSVStringTableFile( rcTokenHash, cOutfile );
          cOutfile.close();
        }
      }
    }
  }
}


void rumStringTable::ExportCSVTokenFile( std::ofstream& o_rcOutfile ) const
{
  // Sort the token hash
  using SortedHash = std::map< rumTokenID, std::string >;
  const SortedHash hashTokens( m_hashTokens.begin(), m_hashTokens.end() );

  for( const auto& iter : hashTokens )
  {
    const rumTokenID iTokenID{ iter.first };
    const std::string& strToken{ iter.second };
    o_rcOutfile << strToken << ',' << rumStringUtils::ToHexString( iTokenID ) << '\n';
  }
}


void rumStringTable::ExportCSVStringTableFile( const rumTokenHash& i_rcTokenHash, std::ofstream& o_rcOutfile ) const
{
  // Sort the token hash
  using SortedHash = std::map< rumTokenID, std::string >;
  const SortedHash hashTokens( i_rcTokenHash.begin(), i_rcTokenHash.end() );

  for( const auto& iter : hashTokens )
  {
    const rumTokenID iTokenID{ iter.first };
    std::string strToken{ iter.second };
    if( strToken.find( '"' ) != std::string::npos ||
        strToken.find( ',' ) != std::string::npos ||
        strToken.find( '\n' ) != std::string::npos )
    {
      rumStringUtils::Replace( strToken, "\"", "\"\"" );
      strToken = "\"" + strToken + "\"";
    }

    o_rcOutfile << strToken << ',' << rumStringUtils::ToHexString( iTokenID ) << '\n';
  }
}


void rumStringTable::ExportDBTable() const
{
  const std::string strTokenTableName{ "token_" + GetName() };
  rumDatabase::Query( rumDatabase::Strings_DatabaseID, "DROP TABLE " + strTokenTableName );
  rumDatabase::Query( rumDatabase::Strings_DatabaseID, "CREATE TABLE[" + strTokenTableName + "]( "
                                                       "[id] INTEGER NOT NULL UNIQUE,"
                                                       "[name] TEXT NOT NULL UNIQUE);" );

  const std::string strStringTableName{ "string_" + GetName() };
  rumDatabase::Query( rumDatabase::Strings_DatabaseID, "DROP TABLE " + strStringTableName );
  rumDatabase::Query( rumDatabase::Strings_DatabaseID, "CREATE TABLE[" + strStringTableName + "]( "
                                                       "[lang_id_fk] INTEGER NOT NULL,"
                                                       "[token_id_fk] INTEGER NOT NULL,"
                                                       "[string] TEXT);" );

  // Sort the token hash
  using SortedHash = std::map< rumTokenID, std::string >;
  const SortedHash hashTokens( m_hashTokens.begin(), m_hashTokens.end() );

  rumDatabase::Query( rumDatabase::Strings_DatabaseID, "BEGIN TRANSACTION" );

  // Export the token hash
  for( const auto& tokenIter : hashTokens )
  {
    const rumTokenID iTokenID{ tokenIter.first };

    // The exported name is the string plus the table name appended
    const std::string& strTokenName{ tokenIter.second };

    std::string strQuery{ "INSERT INTO " + strTokenTableName + " ( id,name ) VALUES(" };
    strQuery += rumStringUtils::ToString( iTokenID );
    strQuery += ",'";
    strQuery += strTokenName;
    strQuery += "');";
    const QueryPtr pcQuery{ rumDatabase::Query( rumDatabase::Strings_DatabaseID, strQuery ) };
    rumAssert( !pcQuery->IsError() );
  }

  // Export the string table for each language
  for( const auto& translationIter : m_hashTranslations )
  {
    const rumLanguageID iLanguageID{ translationIter.first };
    const rumTokenHash& rcTokenHash{ translationIter.second };
    if( !rcTokenHash.empty() )
    {
      const rumLanguage& rcLanguage{ rumLanguage::GetLanguageInfo( iLanguageID ) };
      if( rcLanguage.IsValid() )
      {
        // Sort the token hash
        using SortedHash = std::map< rumTokenID, std::string >;
        const SortedHash hashStrings( rcTokenHash.begin(), rcTokenHash.end() );

        for( const auto& stringIter : hashStrings )
        {
          const rumTokenID iTokenID{ stringIter.first };
          std::string strTranslation{ stringIter.second };

          // Escape the string for the sql query
          rumStringUtils::Replace( strTranslation, "'", "''" );

          std::string strQuery{ "INSERT INTO " + strStringTableName + " (lang_id_fk,token_id_fk,string) VALUES(" };
          strQuery += rumStringUtils::ToString( iLanguageID );
          strQuery += ",";
          strQuery += rumStringUtils::ToString( iTokenID );
          strQuery += ",'";
          strQuery += strTranslation;
          strQuery += "');";
          const QueryPtr pcQuery{ rumDatabase::Query( rumDatabase::Strings_DatabaseID, strQuery ) };
          rumAssert( !pcQuery->IsError() );
        }
      }
    }
  }

  rumDatabase::Query( rumDatabase::Strings_DatabaseID, "COMMIT" );
}


// static
const std::string& rumStringTable::Fetch( rumTokenID i_eTokenID, rumLanguageID i_eLanguageID ) const
{
  // Get the token hash for the specified language
  const auto& translationIter{ m_hashTranslations.find( i_eLanguageID ) };
  if( translationIter != m_hashTranslations.end() )
  {
    // Get the translation from the token hash
    const rumTokenHash& rcTokenHash{ translationIter->second };
    const auto& tokenIter{ rcTokenHash.find( i_eTokenID ) };
    if( tokenIter != rcTokenHash.end() )
    {
      return tokenIter->second;
    }
  }

  return rumStringUtils::NullString();
}


// static
std::string rumStringTable::GetString( rumTokenID i_eTokenID, rumLanguageID i_eLanguageID )
{
  const rumStringTableID uiStringTable{ GetStringTableIDFromToken( i_eTokenID ) };

  const auto& iter{ s_hashStringTables.find( uiStringTable ) };
  if( iter != s_hashStringTables.end() )
  {
    const rumStringTable& rcStringTable{ iter->second };
    const std::string& str{ rcStringTable.Fetch( i_eTokenID, i_eLanguageID ) };
    return str;
  }

  return rumStringUtils::NullString();
}


// static
std::string rumStringTable::GetString( rumTokenID i_eTokenID )
{
  return GetString( i_eTokenID, rumLanguage::GetActiveLanguage() );
}


// static
std::string rumStringTable::GetStringFromConstTable( const std::string& i_strTokenID )
{
  return GetStringFromConstTable( i_strTokenID, rumLanguage::GetActiveLanguage() );
}


// static
std::string rumStringTable::GetStringFromConstTable( const std::string& i_strTokenID, rumLanguageID i_iLanguageID )
{
  Sqrat::Object sqObject{ Sqrat::ConstTable().GetSlot( i_strTokenID.c_str() ) };
  if( sqObject.GetType() == OT_INTEGER )
  {
    return GetString( sqObject.Cast<rumTokenID>(), i_iLanguageID );
  }

  return rumStringUtils::NullString();
}


// static
rumStringTable& rumStringTable::GetStringTable( rumStringTableID i_iStringTableID )
{
  const auto& iter{ s_hashStringTables.find( i_iStringTableID ) };
  if( iter != s_hashStringTables.end() )
  {
    return iter->second;
  }

  return s_cInvalidStringTable;
}


// static
rumStringTable* rumStringTable::GetStringTable( std::string_view i_strName )
{
  for( auto& iter : s_hashStringTables )
  {
    if( iter.second.GetName().compare( i_strName ) == 0 )
    {
      return &iter.second;
    }
  }

  return nullptr;
}


// static
rumStringTableID rumStringTable::GetStringTableIDFromToken( rumTokenID i_eTokenID )
{
  return EXTRACT_STRING_TABLE_ID( i_eTokenID );
}


// static
std::vector<rumStringTableID> rumStringTable::GetStringTableIDs()
{
  std::vector<rumStringTableID> vStringTableIDs;
  for( const auto& iter : s_hashStringTables )
  {
    vStringTableIDs.push_back( iter.first );
  }

  return vStringTableIDs;
}


// static
std::string rumStringTable::GetTokenName( rumTokenID i_eTokenID )
{
  const rumStringTableID uiStringTable{ GetStringTableIDFromToken( i_eTokenID ) };

  const auto& iter{ s_hashStringTables.find( uiStringTable ) };
  if( iter != s_hashStringTables.end() )
  {
    const rumStringTable& rcStringTable{ iter->second };
    const auto& rcTokenHash{ rcStringTable.GetTokenHash() };
    const auto& iter{ rcTokenHash.find( i_eTokenID ) };
    if( iter != rcTokenHash.end() )
    {
      return iter->second;
    }
  }

  return rumStringUtils::NullString();
}


// static
int32_t rumStringTable::Init( const std::string& i_strPath, const std::string& i_strLanguage )
{
  if( !rumLanguage::Init( i_strPath, i_strLanguage ) )
  {
    return RESULT_FAILED;
  }

  s_hashStringTables.clear();

  if( LoadStringTableHash( i_strPath ) != RESULT_SUCCESS )
  {
    return RESULT_FAILED;
  }

  // Load default language strings for each string table
  for( const auto& iter : s_hashStringTables )
  {
    if( LoadStringTable( iter.first, rumLanguage::GetActiveLanguage(), i_strPath ) == RESULT_FAILED )
    {
      return RESULT_FAILED;
    }
  }

  return RESULT_SUCCESS;
}


int32_t rumStringTable::LoadFromFile( const std::string& i_strFilePath, rumLanguageID i_iLanguageID )
{
  int32_t eResult{ RESULT_SUCCESS };

  // Load and parse the token file if it hasn't already been created
  if( m_hashTokens.empty() )
  {
    std::ifstream cFile( i_strFilePath, std::ios::in );
    if( cFile.is_open() )
    {
      enum{ COL_NAME, COL_ID };

      std::string strRow;

      while( !cFile.eof() )
      {
        std::getline( cFile, strRow );
        if( !strRow.empty() )
        {
          const auto vFields{ rumStringUtils::ParseCSVRow( strRow ) };

          const std::string& strID{ vFields.at( COL_ID ) };
          const std::string& strToken{ vFields.at( COL_NAME ) };

          if( !AddToken( rumStringUtils::ToUInt( strID ), strToken ) )
          {
            std::string strWarning{ "Warning: Failed to add token " };
            strWarning += strToken;
            strWarning += " [";
            strWarning += strID;
            strWarning += "] to string table ";
            strWarning += m_strName;
            Logger::LogStandard( strWarning, Logger::LOG_WARNING );
          }
        }
      }

      cFile.close();
    }
  }

  // Load the string table for the provided language
  const rumLanguage& rcLanguage{ rumLanguage::GetLanguageInfo( i_iLanguageID ) };
  if( rcLanguage.IsValid() )
  {
    // Get the token hash for the specified language
    const auto& iter{ m_hashTranslations.find( i_iLanguageID ) };
    if( iter != m_hashTranslations.end() && iter->second.empty() )
    {
      std::filesystem::path fsStringTable( i_strFilePath );
      fsStringTable.remove_filename();
      fsStringTable /= STRING_TABLE_PREFIX;
      fsStringTable += m_strName;
      fsStringTable += "_";
      fsStringTable += rcLanguage.GetCode();
      fsStringTable += CSV_EXTENSION;

      if( std::filesystem::exists( fsStringTable ) )
      {
        std::ifstream cFile( fsStringTable.string(), std::ios::in );
        if( cFile.is_open() )
        {
          enum{ COL_TRANSLATION, COL_ID };

          std::string strRow;

          while( !cFile.eof() )
          {
            std::getline( cFile, strRow );
            if( !strRow.empty() )
            {
              const auto vFields{ rumStringUtils::ParseCSVRow( strRow ) };

              const std::string& strID{ vFields.at( COL_ID ) };
              const std::string& strTranslation{ vFields.at( COL_TRANSLATION ) };

              if( !AddTranslation( rcLanguage, rumStringUtils::ToUInt( strID ), strTranslation ) )
              {
                std::string strWarning{ "Warning: Failed to add translation " };
                strWarning += strTranslation;
                strWarning += ", token [";
                strWarning += strID;
                strWarning += "], language [";
                strWarning += rumStringUtils::ToString( rcLanguage.GetID() );
                strWarning += "] to string table ";
                strWarning += m_strName;
                Logger::LogStandard( strWarning, Logger::LOG_WARNING );
              }
            }
          }
        }
        else
        {
          std::string strError{ "Error: Failed to load string table " };
          strError += m_strName;
          strError += ", file ";
          strError += fsStringTable.string();
          strError += " could not be found or opened";
          Logger::LogStandard( strError, Logger::LOG_WARNING );

          eResult = RESULT_FAILED;
        }
      }
    }
  }
  else
  {
    std::string strError{ "Error: Failed to load string table " };
    strError += m_strName;
    strError += ", language id ";
    strError += i_iLanguageID;
    strError += " could not be mapped to a language entry";
    Logger::LogStandard( strError, Logger::LOG_WARNING );

    eResult = RESULT_FAILED;
  }

  return eResult;
}


int32_t rumStringTable::LoadFromDatabase( rumLanguageID i_iLanguageID )
{
  int32_t eResult{ RESULT_SUCCESS };

  // Load and parse the token file if it hasn't already been created
  if( m_hashTokens.empty() )
  {
    const std::string strQuery( "SELECT id,name FROM token_" + GetName() );

    const QueryPtr pcQuery{ rumDatabase::Query( rumDatabase::Strings_DatabaseID, strQuery ) };
    if( pcQuery && !pcQuery->IsError() )
    {
      enum{ COL_ID, COL_NAME };

      for( int32_t i = 0; i < pcQuery->GetNumRows(); ++i )
      {
        const rumTokenID iTokenID{ (rumTokenID)pcQuery->FetchInt( i, COL_ID) };
        std::string strToken{ pcQuery->FetchString(i, COL_NAME) };
        if( !AddToken( iTokenID, strToken ) )
        {
          std::string strWarning{ "Warning: Failed to add token " };
          strWarning += strToken;
          strWarning += " [";
          strWarning += rumStringUtils::ToHexString( iTokenID );
          strWarning += "] to string table ";
          strWarning += m_strName;
          Logger::LogStandard( strWarning, Logger::LOG_WARNING );
        }
      }
    }
  }

  // Load the string table for the provided language
  const rumLanguage& rcLanguage{ rumLanguage::GetLanguageInfo( i_iLanguageID ) };
  if( rcLanguage.IsValid() )
  {
    // Get the token hash for the specified language
    const auto& iter{ m_hashTranslations.find( i_iLanguageID ) };
    if( iter != m_hashTranslations.end() && iter->second.empty() )
    {
      std::string strQuery{ "SELECT string,token_id_fk FROM string_" + GetName() + " WHERE lang_id_fk=" };
      strQuery += rumStringUtils::ToString( i_iLanguageID );
      const QueryPtr pcQuery{ rumDatabase::Query( rumDatabase::Strings_DatabaseID, strQuery ) };
      if( pcQuery && !pcQuery->IsError() )
      {
        enum{ COL_STRING, COL_ID };

        for( int32_t i = 0; i < pcQuery->GetNumRows(); ++i )
        {
          const rumTokenID iTokenID{ (rumTokenID)pcQuery->FetchInt( i, COL_ID ) };
          const std::string strTranslation{ pcQuery->FetchString( i, COL_STRING ) };
          if( !AddTranslation( rcLanguage, iTokenID, strTranslation ) )
          {
            std::string strWarning{ "Warning: Failed to add translation " };
            strWarning += strTranslation;
            strWarning += ", token [";
            strWarning += rumStringUtils::ToHexString( iTokenID );
            strWarning += "], language [";
            strWarning += rumStringUtils::ToString( rcLanguage.GetID() );
            strWarning += "] to string table ";
            strWarning += m_strName;
            Logger::LogStandard( strWarning, Logger::LOG_WARNING );
          }
        }
      }
      else
      {
        std::string strError{ "Error: Failed to load string table " };
        strError += m_strName;
        strError += ", ";
        strError += pcQuery->GetErrorMsg();
        Logger::LogStandard( strError, Logger::LOG_WARNING );

        eResult = RESULT_FAILED;
      }
    }
  }
  else
  {
    std::string strError{ "Error: Failed to load string table " };
    strError += m_strName;
    strError += ", language id ";
    strError += i_iLanguageID;
    strError += " could not be mapped to a language entry";
    Logger::LogStandard( strError, Logger::LOG_WARNING );

    eResult = RESULT_FAILED;
  }

  return eResult;
}


// static
int32_t rumStringTable::LoadStringTable( rumStringTableID i_iTableID, rumLanguageID i_iLanguageID,
                                         const std::string& i_strPath )
{
  const auto& iter{ s_hashStringTables.find( i_iTableID ) };
  if( iter == s_hashStringTables.end() )
  {
    return RESULT_FAILED;
  }

  rumStringTable& rcStringTable{ iter->second };

  // Try CSV files first
  std::filesystem::path fsFilepath( std::filesystem::path( i_strPath ) / CSV_FOLDER_NAME / TOKEN_TABLE_PREFIX );
  fsFilepath += rcStringTable.m_strName;
  fsFilepath += CSV_EXTENSION;

  if( std::filesystem::exists( fsFilepath ) )
  {
    return rcStringTable.LoadFromFile( fsFilepath.string(), i_iLanguageID );
  }

  return rcStringTable.LoadFromDatabase( i_iLanguageID );
}


// static
int32_t rumStringTable::LoadStringTableHash( const std::string& i_strPath )
{
  // Try CSV first
  std::filesystem::path fsFilepath{ std::filesystem::path( i_strPath ) / CSV_FOLDER_NAME / STRING_TABLE_NAME };
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
    const std::string strQuery{ "SELECT id,name FROM string_table" };
    const QueryPtr pcQuery{ rumDatabase::Query( rumDatabase::Strings_DatabaseID, strQuery ) };
    if( pcQuery && !pcQuery->IsError() )
    {
      enum{ COL_ID, COL_NAME };

      for( int32_t i = 0; i < pcQuery->GetNumRows(); ++i )
      {
        const rumStringTableID iStringTableID{ (rumStringTableID)pcQuery->FetchInt( i, COL_ID ) };
        const std::string strName{ pcQuery->FetchString( i, COL_NAME ) };

        if( !AddStringTable( iStringTableID, strName, Shared_ServiceType ) )
        {
          const std::string strError{ "Error: Failed to create entry for string table '" + strName + '"' };
          Logger::LogStandard( strError, Logger::LOG_ERROR );
        }
      }
    }
    else
    {
      const std::string strError{ "Error: Failed to query string_table from strings.db" };
      Logger::LogStandard( strError, Logger::LOG_ERROR );
    }
  }

  return( s_hashStringTables.size() > 0 ? RESULT_SUCCESS : RESULT_FAILED );
}


// static
void rumStringTable::ParseFields( const std::vector<std::string>& i_rvFields )
{
  enum { COL_ID, COL_NAME, COL_SERVICE_TYPE };

  const rumStringTableID iID{ (rumStringTableID)rumStringUtils::ToUInt( i_rvFields.at( COL_ID ) ) };
  const std::string& strName{ i_rvFields.at( COL_NAME ) };
  const ServiceType eServiceType{ (ServiceType)rumStringUtils::ToUInt( i_rvFields.at( COL_SERVICE_TYPE ) ) };
  if( !AddStringTable( iID, strName, eServiceType ) )
  {
    std::string strError{ "Error: Failed to create entry for string table '" };
    strError += strName;
    strError += '\'';
    Logger::LogStandard( strError, Logger::LOG_ERROR );
  }
}


// static
void rumStringTable::ExportDB( const std::string& i_strPath, std::vector<ServiceType>&& i_vServiceTypes )
{
  rumLanguage::ExportDBTable();

  rumDatabase::Query( rumDatabase::Strings_DatabaseID, "DROP TABLE string_table" );
  rumDatabase::Query( rumDatabase::Strings_DatabaseID, "CREATE TABLE[string_table]("
                                                       "[id] INTEGER NOT NULL UNIQUE,"
                                                       "[name] TEXT NOT NULL UNIQUE);" );

  for( const auto& iter : rumStringTable::GetStringTableIDs() )
  {
    const rumStringTableID iStringTableID{ iter };
    const rumStringTable& rcStringTable{ rumStringTable::GetStringTable( iStringTableID ) };
    const ServiceType eServiceType{ rcStringTable.GetServiceType() };
    if( std::find( i_vServiceTypes.begin(), i_vServiceTypes.end(), eServiceType ) != i_vServiceTypes.end() )
    {
      std::string strQuery{ "INSERT INTO string_table (id,name) VALUES (" };
      strQuery += rumStringUtils::ToString( iStringTableID );
      strQuery += ",'";
      strQuery += rcStringTable.GetName();
      strQuery += "')";

      rumDatabase::Query( rumDatabase::Strings_DatabaseID, strQuery );

      // Make sure each table for each language is loaded
      for( const auto iter : rumLanguage::GetLanguageIDs() )
      {
        // Load the table for the selected language
        const rumLanguageID iLanguageID{ iter };
        rcStringTable.LoadStringTable( iStringTableID, iLanguageID, i_strPath );
      }

      rcStringTable.ExportDBTable();
    }
  }

  rumDatabase::Query( rumDatabase::Strings_DatabaseID, "VACUUM" );
}


bool rumStringTable::RemoveStringTable( rumStringTableID i_iStringTableID )
{
  return s_hashStringTables.erase( i_iStringTableID ) >= 1;
}


void rumStringTable::RemoveToken( rumTokenID i_iTokenID )
{
  // Remove the token
  m_hashTokens.erase( i_iTokenID );

  // Remove any string tied to the token
  RemoveTranslation( i_iTokenID );
}


void rumStringTable::RemoveTokenHash( rumLanguageID i_iLanguageID )
{
  m_hashTranslations.erase( i_iLanguageID );
}


void rumStringTable::RemoveTranslation( rumLanguageID i_iLanguageID, rumTokenID i_iTokenID )
{
  const auto& iter{ m_hashTranslations.find( i_iLanguageID ) };
  if( iter != m_hashTranslations.end() )
  {
    // Remove the string matching the provided token
    rumTokenHash& hashToken{ iter->second };
    hashToken.erase( i_iTokenID );
  }
}


void rumStringTable::RemoveTranslation( rumTokenID i_iTokenID )
{
  // Remove the translation string from each language
  const std::vector<rumLanguageID> vLanguageIDs{ rumLanguage::GetLanguageIDs() };
  for( const auto iter : vLanguageIDs )
  {
    const rumLanguageID iLanguageID{ iter };
    RemoveTranslation( iLanguageID, i_iTokenID );
  }
}


// static
void rumStringTable::ScriptBind()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::ConstTable( pcVM ).Const( "rumInvalidStringToken", INVALID_TOKEN_ID );

  Sqrat::RootTable( pcVM )
    .Overload<std::string( * )( rumTokenID, rumLanguageID )>( "rumGetString", GetString )
    .Overload<std::string( * )( rumTokenID )>( "rumGetString", GetString )
    .Overload<std::string( * )( const std::string&, rumLanguageID )>( "rumGetStringByName", GetStringFromConstTable )
    .Overload<std::string( * )( const std::string& )>( "rumGetStringByName", GetStringFromConstTable );

  // Expose all tokens to script
  for( const auto& iterStringTable : s_hashStringTables )
  {
    const auto& rcStringTable{ iterStringTable.second };
    for( const auto& iterToken : rcStringTable.GetTokenHash() )
    {
      const std::string& strToken{ iterToken.second };
      const std::string strStringID{ strToken + "_" + rcStringTable.GetName() + "_StringID" };
      Sqrat::ConstTable( pcVM ).Const( strStringID.c_str(), iterToken.first );
    }
  }
}


void rumStringTable::SetTokenName( rumTokenID i_iTokenID, const std::string& i_strName )
{
  const auto& iter{ m_hashTokens.find( i_iTokenID ) };
  if( iter != m_hashTokens.end() )
  {
    iter->second = i_strName;
  }
}


void rumStringTable::SetTranslation( rumLanguageID i_iLanguageID, rumTokenID i_iTokenID,
                                     const std::string& i_strTranslation )
{
  // Get the token hash for the specified language
  const auto translation_iter{ m_hashTranslations.find( i_iLanguageID ) };
  if( translation_iter != m_hashTranslations.end() )
  {
    // Update the translation
    auto& rcTokenHash{ translation_iter->second };
    const auto token_iter{ rcTokenHash.find( i_iTokenID ) };
    if( token_iter != rcTokenHash.end() )
    {
      token_iter->second = i_strTranslation;
    }
    else
    {
      rcTokenHash.insert( std::pair( i_iTokenID, i_strTranslation ) );
    }
  }
}


// static
void rumStringTable::Shutdown()
{
  // Nothing to do
}
