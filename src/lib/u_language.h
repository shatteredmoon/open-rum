#ifndef _U_LANGUAGE_H_
#define _U_LANGUAGE_H_

#include <unordered_map>

using rumLanguageID = int32_t;


class rumLanguage
{
public:

  const std::string GetCode() const
  {
    return m_strCode;
  }

  void SetCode( const std::string& i_strCode )
  {
    m_strCode = i_strCode;
  }

  rumLanguageID GetID() const
  {
    return m_iID;
  }

  const std::string GetName() const
  {
    return m_strName;
  }

  void SetName( const std::string& i_strName )
  {
    m_strName = i_strName;
  }

  bool IsValid() const
  {
    return m_iID != INVALID_LANGUAGE_ID;
  }

  static bool AddLanguage( rumLanguageID i_iID, const std::string& i_strName, const std::string& i_strCode );

  static void ExportCSVFile( const std::string& i_strPath );
  static void ExportDBTable();

  static rumLanguageID GetActiveLanguage()
  {
    return s_iActiveLanguageID;
  }

  static bool SetActiveLanguage( rumLanguageID i_iID );

  static rumLanguageID GetLanguageIDFromCode( const std::string& i_strCode );
  static rumLanguageID GetLanguageIDFromName( const std::string& i_strName );

  static rumLanguage& GetLanguageInfo( rumLanguageID i_iID );
  static std::vector<rumLanguageID> GetLanguageIDs();

  static rumLanguageID GetNextAvailableLanguageID()
  {
    return s_iNextLanguageID;
  }

  static bool Init( const std::string& i_strPath, const std::string& i_strLanguage );

  static bool IsLanguageSupported( rumLanguageID i_iID );

  static bool RemoveLanguage( rumLanguageID i_iID );

  static void ScriptBind();

  static void Shutdown();

  static constexpr rumLanguageID DEFAULT_LANGUAGE_ID{ 0 };
  static constexpr rumLanguageID INVALID_LANGUAGE_ID{ -1 };

private:

  static int32_t Load( const std::string& i_strPath );
  static void ParseFields( const std::vector<std::string>& i_rvFields );

  std::string m_strName;
  std::string m_strCode;
  rumLanguageID m_iID{ INVALID_LANGUAGE_ID };

  static rumLanguageID s_iActiveLanguageID;

  static rumLanguageID s_iNextLanguageID;

  static std::unordered_map<rumLanguageID, rumLanguage> s_hashLanguages;
};

#endif // _U_LANGUAGE_H_
