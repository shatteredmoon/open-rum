#ifndef _S_ACCOUNT_H_
#define _S_ACCOUNT_H_

#include <u_account.h>
#include <u_script.h>
#include <u_timer.h>

#include <unordered_map>


// A singleton class that holds server-side account and character information

class rumServerAccount : public rumAccount
{
public:

  struct rumAccountCreationInfo
  {
    rumUniqueID m_uiAccountID{ INVALID_GAME_ID };
    std::string m_strAccountName;
    std::string m_strPassword;
    rumTimer m_tSubmitted;
  };

  static void AccountLogin( const std::string& i_strAccountName, const std::string& i_strPassword, SOCKET i_iSocket );

  static void AccountLoginFailed( AccountLoginResultType i_eReason, SOCKET i_iSocket );

  static void AccountLogout( SOCKET i_iSocket );

  static void CreateAccount( const std::string& i_strAccountName, const std::string& i_strEmail,
                             const std::string& i_strPassword, SOCKET i_iSocket );

  static rumUniqueID GetAccountID( SOCKET i_iSocket );

  static void ProcessPendingAccounts();

  static void ScriptBind();

private:

  typedef std::unordered_map<SOCKET, rumUniqueID> AccountIDContainer;
  static AccountIDContainer s_hashAccountIDs;

  typedef std::unordered_map<SOCKET, rumAccountCreationInfo> PendingAccountContainer;
  static PendingAccountContainer s_hashPendingAccounts;
};

#endif // _S_ACCOUNT_H_
