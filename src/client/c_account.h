#ifndef _C_ACCOUNT_H_
#define _C_ACCOUNT_H_

#include <u_account.h>
#include <u_script.h>

#include <unordered_map>

namespace network { class inboundPacket; }


// A singleton class that holds client-side account and character information
class rumClientAccount : public rumAccount
{
public:

  static AccountLoginResultType AccountLogin( const std::string& i_strAccountName, const std::string& i_strPassword );
  static int32_t AccountLogin_VM( Sqrat::Object i_sqUserName, Sqrat::Object i_sqPassword );

  static void AccountLogout();
  
  static AccountCreateResultType CreateAccount( const std::string& i_strAccountName, const std::string& i_strEmail,
                                                const std::string& i_strPassword );
  static int32_t CreateAccount_VM( Sqrat::Object i_sqAccountName, Sqrat::Object i_sqEmail,
                                   Sqrat::Object i_sqPassword );

  static void CreateResult( AccountCreateResultType i_eResult );

  static void LoginResult( AccountLoginResultType i_eResult, SOCKET i_iSocket, CharacterList& i_listCharacterNames );

  static void ScriptBind();

private:

  typedef std::unordered_map<SOCKET, uint64_t> AccountIDContainer;
  static AccountIDContainer s_hashAccountIDs;
};

#endif // _C_ACCOUNT_H_
