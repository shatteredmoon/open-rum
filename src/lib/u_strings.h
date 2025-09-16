#ifndef _U_STRINGS_H_
#define _U_STRINGS_H_

#include <u_db.h>
#include <u_enum.h>
#include <u_language.h>

#include <unordered_map>

using rumStringTableID = uint32_t;
using rumTokenID = uint32_t;

using rumTokenHash = std::unordered_map< rumTokenID, std::string >;
using rumTranslationHash = std::unordered_map< rumLanguageID, rumTokenHash >;

class rumStringTable
{
public:

  bool AddToken( rumTokenID i_iTokenID, const std::string& strToken );
  bool AddTokenHash( rumLanguageID i_iLanguageID );

  bool AddTranslation( const rumLanguage& i_rcLanguage, rumTokenID i_iTokenID, const std::string& i_strTranslation );
  bool AddTranslation( rumLanguageID i_iLanguageID, rumTokenID i_iTokenID, const std::string& i_strTranslation );

  void RemoveTranslation( rumLanguageID i_iLanguageID, rumTokenID i_iTokenID );
  void RemoveTranslation( rumTokenID i_iTokenID );

  void ExportCSVFiles( const std::string& i_strPath ) const;

  const std::string& Fetch( rumTokenID i_eTokenID, rumLanguageID i_eLanguageID ) const;

  const rumStringTableID GetID() const
  {
    return m_iStringTableID;
  }

  const std::string& GetName() const
  {
    return m_strName;
  }

  void SetName( const std::string& i_strName )
  {
    m_strName = i_strName;
  }

  rumTokenID GetNextTokenID() const
  {
    return m_iNextTokenID;
  }

  uint32_t GetNumTokens() const
  {
    return (uint32_t)m_hashTokens.size();
  }

  ServiceType GetServiceType() const
  {
    return m_eServiceType;
  }

  void SetServiceType( ServiceType i_eServiceType )
  {
    m_eServiceType = i_eServiceType;
  }

  const rumTokenHash& GetTokenHash() const
  {
    return m_hashTokens;
  }

  const rumTranslationHash& GetTranslationHash() const
  {
    return m_hashTranslations;
  }

  bool RemoveStringTable( rumStringTableID i_iStringTableID );
  void RemoveToken( rumTokenID i_iTokenID );
  void RemoveTokenHash( rumLanguageID i_iLanguageID );

  void SetTokenName( rumTokenID i_uiTokenID, const std::string& i_strName );
  void SetTranslation( rumLanguageID i_iLanguageID, rumTokenID i_uiTokenID, const std::string& i_strTranslation );

  static bool AddStringTable( rumStringTableID i_iStringTableID, const std::string& i_strName,
                              ServiceType i_eServiceType );

  static void ExportDB( const std::string& i_strPath, std::vector<ServiceType>&& i_vServiceTypes );

  static rumStringTableID GetNextStringTableID()
  {
    return s_iNextStringTableID;
  }

  static rumStringTable& GetStringTable( rumStringTableID i_uiStringTableID );
  static rumStringTable* GetStringTable( std::string_view i_strName );
  static std::vector<rumStringTableID> GetStringTableIDs();

  static rumStringTableID GetStringTableIDFromToken( rumTokenID i_eTokenID );

  static int32_t Init( const std::string& i_strPath, const std::string& i_strLanguage );

  // Intended for the editor
  static int32_t LoadStringTable( rumStringTableID i_uiTableID, rumLanguageID i_iLanguageID,
                                  const std::string& i_strPath );

  static void ScriptBind();

  static std::string GetString( rumTokenID i_eTokenID );
  static std::string GetString( rumTokenID i_eTokenID, rumLanguageID i_iLanguageID );

  static std::string GetStringFromConstTable( const std::string& i_strTokenID );
  static std::string GetStringFromConstTable( const std::string& i_strTokenID, rumLanguageID i_iLanguageID );

  static std::string GetTokenName( rumTokenID i_eTokenID );

  static void Shutdown();

  static constexpr rumStringTableID DEFAULT_STRINGTABLE_ID{ 0 };
  static constexpr rumStringTableID INVALID_STRINGTABLE_ID{ 0xFFFFFFFF };
  static constexpr rumTokenID INVALID_TOKEN_ID{ 0xFFFFFFFF };

protected:

  void ExportCSVTokenFile( std::ofstream& o_rcOutfile ) const;
  void ExportCSVStringTableFile( const rumTokenHash& i_rcTokenHash, std::ofstream& o_rcOutfile ) const;

private:

  static void ExportCSV( const std::string& i_strPath );

  static int32_t LoadStringTableHash( const std::string& i_strPath );

  void ExportDBTable() const;

  int32_t LoadFromFile( const std::string& i_strFilePath, rumLanguageID i_iLanguageID );
  int32_t LoadFromDatabase( rumLanguageID i_iLanguageID );

  static void ParseFields( const std::vector<std::string>& i_rvFields );

  std::string m_strName;

  rumStringTableID m_iStringTableID{ INVALID_STRINGTABLE_ID };

  // Maps token ids to string tokens
  rumTokenHash m_hashTokens;

  // Maps token ids to translation strings for each language
  rumTranslationHash m_hashTranslations;

  // The next available token ID
  rumTokenID m_iNextTokenID{ 0 };

  ServiceType m_eServiceType{ Shared_ServiceType };

  //static void ScriptBind( rumAssetID i_eAssetID, const std::string& i_strName );

  static std::unordered_map<rumStringTableID, rumStringTable> s_hashStringTables;

  // The next availabe string table ID
  static rumStringTableID s_iNextStringTableID;

  static rumStringTable s_cInvalidStringTable;
};

#endif // _U_STRINGS_H_
