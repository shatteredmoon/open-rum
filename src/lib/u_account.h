#ifndef _U_ACCOUNT_H_
#define _U_ACCOUNT_H_

#include <platform.h>

#include <u_script.h>
#include <u_utility.h>

#include <list>

// A singleton class that holds account and character information

class rumAccount
{
public:

  ~rumAccount()
  {
    Clear();
    SAFE_DELETE( s_pcInstance );
  }

  enum AccountCreateResultType
  {
    ACCOUNT_CREATE_SUCCESS,
    ACCOUNT_CREATE_ERROR,
    ACCOUNT_CREATE_FAIL_NAME_EXISTS,
    ACCOUNT_CREATE_FAIL_NAME_INVALID,
    ACCOUNT_CREATE_FAIL_PASSWORD_INVALID,
    ACCOUNT_CREATE_FAIL_PASSWORD_MISMATCH,
    ACCOUNT_CREATE_FAIL_TIMEOUT
  };

  enum AccountLoginResultType
  {
    ACCOUNT_LOGIN_SUCCESS,
    ACCOUNT_LOGIN_ERROR,
    ACCOUNT_LOGIN_FAIL_NAME_INVALID,
    ACCOUNT_LOGIN_FAIL_PASSWORD_INVALID,
    ACCOUNT_LOGIN_FAIL_ACCOUNT_NOT_FOUND,
    ACCOUNT_LOGIN_FAIL_AUTHENTICATION_FAILED,
    ACCOUNT_LOGIN_FAIL_ACCOUNT_ACTIVE
    //ACCOUNT_LOGIN_FAIL_ACCOUNT_BANNED
  };

  const std::string &GetAccountName() const
  {
    return m_strName;
  }

  void SetName( const std::string& i_strName )
  {
    m_strName = i_strName;
  }

  void AddCharacter( const std::string& i_strCharName )
  {
    m_listCharacters.push_front( i_strCharName );
  }

  void RemoveCharacter( const std::string& i_strCharName )
  {
    m_listCharacters.remove( i_strCharName );
  }

  void ResetCharacterList()
  {
    m_listCharacters.clear();
  }

  static Sqrat::Object GetCharacters_VM();

  void Clear()
  {
    m_bSignedIn = false;
    m_strName.clear();
    ResetCharacterList();
  }

  bool IsSignedIn() const
  {
    return m_bSignedIn;
  }

  static bool IsSignedIn_VM();

  void SetSignedIn( bool i_bSignedIn )
  {
    m_bSignedIn = i_bSignedIn;
  }

  uint32_t GetNumCharacters() const
  {
    return (uint32_t)m_listCharacters.size();
  }

  static uint32_t GetNumCharacters_VM()
  {
    return GetInstance().GetNumCharacters();
  }

  static rumAccount& GetInstance();

protected:

  static void ScriptBind();

  typedef std::list<std::string> CharacterList;

  static const char* s_aAccountCreateResultTypeStrings[];
  static const char* s_aAccountLoginResultTypeStrings[];

private:

  CharacterList m_listCharacters;

  std::string m_strName;

  bool m_bSignedIn{ false };

  // The singleton instance
  static rumAccount* s_pcInstance;
};

#endif // _U_ACCOUNT_H_
