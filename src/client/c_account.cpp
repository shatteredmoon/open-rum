#include <c_account.h>

#include <c_player.h>

#include <network/u_packet.h>
#include <u_log.h>
#include <u_rum.h>


// static
rumAccount::AccountLoginResultType rumClientAccount::AccountLogin( const std::string& i_strAccountName,
                                                                   const std::string& i_strPassword )
{
  AccountLoginResultType eResult{ AccountLoginResultType::ACCOUNT_LOGIN_SUCCESS };

  if( i_strAccountName.empty() )
  {
    eResult = AccountLoginResultType::ACCOUNT_LOGIN_FAIL_NAME_INVALID;
    g_pstrLastErrorString = s_aAccountLoginResultTypeStrings[(int32_t)eResult];
  }
#ifndef _DEBUG
  else if( i_strPassword.empty() )
  {
    eResult = ACCOUNT_LOGIN_FAIL_PASSWORD_INVALID;
    g_pstrLastErrorString = s_aAccountLoginResultTypeStrings[eResult];
  }
#endif
  else
  {
    auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
    rcPacket.SetHeader( rumNetwork::PACKET_HEADER_CLIENT_ACCOUNT_LOGIN )
      .Write( i_strAccountName )
      .Write( i_strPassword )
      .Send( rumNetwork::GetNetworkSocket() );
  }

  return eResult;
}


// static
int32_t rumClientAccount::AccountLogin_VM( Sqrat::Object i_sqUserName, Sqrat::Object i_sqPassword )
{
  if( ( i_sqUserName.GetType() == OT_STRING ) && ( i_sqPassword.GetType() == OT_STRING ) )
  {
    return (int32_t)AccountLogin( i_sqUserName.Cast<std::string>(), i_sqPassword.Cast<std::string>() );
  }

  g_pstrLastErrorString = s_aAccountLoginResultTypeStrings[(int32_t)AccountLoginResultType::ACCOUNT_LOGIN_ERROR];
  return (int32_t)AccountLoginResultType::ACCOUNT_LOGIN_ERROR;
}


// static
void rumClientAccount::AccountLogout()
{
  rumAccount& rcAccount{ GetInstance() };
  if( rcAccount.IsSignedIn() )
  {
    auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
    rcPacket.SetHeader( rumNetwork::PACKET_HEADER_CLIENT_ACCOUNT_LOGOUT )
      .Send( rumNetwork::GetNetworkSocket() );

    rcAccount.Clear();
  }

  rumClientPlayer::SetClientPlayerID( INVALID_GAME_ID );
}


//static
rumAccount::AccountCreateResultType rumClientAccount::CreateAccount( const std::string& i_strAccountName,
                                                                     const std::string& i_strEmail,
                                                                     const std::string& i_strPassword )
{
  AccountCreateResultType eResult{ AccountCreateResultType::ACCOUNT_CREATE_SUCCESS };

  if( i_strAccountName.empty() )
  {
    eResult = AccountCreateResultType::ACCOUNT_CREATE_FAIL_NAME_INVALID;
    g_pstrLastErrorString = s_aAccountCreateResultTypeStrings[(int32_t)eResult];
  }
  else if( i_strPassword.empty() )
  {
    eResult = AccountCreateResultType::ACCOUNT_CREATE_FAIL_PASSWORD_INVALID;
    g_pstrLastErrorString = s_aAccountCreateResultTypeStrings[(int32_t)eResult];
  }
  else
  {
    auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
    rcPacket.SetHeader( rumNetwork::PACKET_HEADER_CLIENT_ACCOUNT_CREATE )
      .Write( i_strAccountName )
      .Write( i_strEmail )
      .Write( i_strPassword )
      .Send( rumNetwork::GetNetworkSocket() );
  }

  return eResult;
}


// static
int32_t rumClientAccount::CreateAccount_VM( Sqrat::Object i_sqAccountName, Sqrat::Object i_sqEmail,
                                            Sqrat::Object i_sqPassword )
{
  if( ( i_sqAccountName.GetType() == OT_STRING ) && ( i_sqEmail.GetType() == OT_STRING ) &&
      ( i_sqPassword.GetType() == OT_STRING ) )
  {
    return (int32_t)CreateAccount( i_sqAccountName.Cast<std::string>(), i_sqEmail.Cast<std::string>(),
                                   i_sqPassword.Cast<std::string>() );
  }

  g_pstrLastErrorString = s_aAccountCreateResultTypeStrings[(int32_t)AccountCreateResultType::ACCOUNT_CREATE_ERROR];
  return (int32_t)AccountCreateResultType::ACCOUNT_CREATE_ERROR;
}


// static
void rumClientAccount::CreateResult( AccountCreateResultType i_eResult )
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  if( AccountCreateResultType::ACCOUNT_CREATE_SUCCESS == i_eResult )
  {
    // Inform scripts that account creation succeeded
    rumScript::ExecRequiredFunc( Sqrat::RootTable( pcVM ), "OnAccountCreationSuccess" );
  }
  else
  {
    std::string strInfo{ "Account creation failed, reason: " };
    strInfo += rumStringUtils::ToString( (int32_t)i_eResult );
    strInfo += " ";
    strInfo += s_aAccountCreateResultTypeStrings[(int32_t)i_eResult];
    Logger::LogStandard( strInfo );

    // Inform scripts that account creation failed
    rumScript::ExecRequiredFunc( Sqrat::RootTable( pcVM ), "OnAccountCreationFailed",
                                 s_aAccountCreateResultTypeStrings[(int32_t)i_eResult] );
  }
}


// static
void rumClientAccount::LoginResult( AccountLoginResultType i_eResult, SOCKET i_iSocket,
                                    CharacterList& i_listCharacterNames )
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  if( AccountLoginResultType::ACCOUNT_LOGIN_SUCCESS == i_eResult )
  {
    rumAccount& rcAccount{ GetInstance() };
    rcAccount.ResetCharacterList();

    uint32_t uiIndex{ 1 };
    for( const auto& iter : i_listCharacterNames )
    {
      const std::string& strName{ iter };
      RUM_COUT( uiIndex++ << ": " << strName << '\n' );
      rcAccount.AddCharacter( strName );
    }

    // Server-side player IDs match the server-side socket, and so we must remember that any player updates
    // matching this socket from here on out reflects a change to this client and player
    rumClientPlayer::SetClientPlayerSocket( i_iSocket );

    // Inform scripts that account login has succeeded
    rumScript::ExecRequiredFunc( Sqrat::RootTable( pcVM ), "OnAccountLoginSuccess" );

    rcAccount.SetSignedIn( true );
  }
  else
  {
    std::string strInfo{ "Login failed, reason: " };
    strInfo += rumStringUtils::ToString( (int32_t)i_eResult );
    strInfo += " ";
    strInfo += s_aAccountLoginResultTypeStrings[(int32_t)i_eResult];
    Logger::LogStandard( strInfo );

    // Inform scripts that login has failed
    rumScript::ExecRequiredFunc( Sqrat::RootTable( pcVM ), "OnAccountLoginFailed",
                                 s_aAccountLoginResultTypeStrings[(int32_t)i_eResult] );
  }
}


// static
void rumClientAccount::ScriptBind()
{
  rumAccount::ScriptBind();

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::RootTable( pcVM )
    .Func( "rumAccountLogin", AccountLogin_VM )
    .Func( "rumAccountLogout", AccountLogout )
    .Func( "rumCreateAccount", CreateAccount_VM );
}
