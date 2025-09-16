#include <s_account.h>

#include <network/u_packet.h>
#include <u_db.h>
#include <u_log.h>


// Static initializers
rumServerAccount::AccountIDContainer rumServerAccount::s_hashAccountIDs;
rumServerAccount::PendingAccountContainer rumServerAccount::s_hashPendingAccounts;


// static
void rumServerAccount::AccountLogin( const std::string& i_strAccountName, const std::string& i_strPassword,
                                     SOCKET i_iSocket )
{
  // Hopefully, these were caught by the client, but we still have to check them
  if( i_strAccountName.empty() )
  {
    // Account name cannot be empty
    AccountLoginFailed( AccountLoginResultType::ACCOUNT_LOGIN_FAIL_NAME_INVALID, i_iSocket );
    return;
  }
#ifndef _DEBUG
  else if( i_strPassword.empty() )
  {
    // User must supply a password
    AccountLoginFailed( ACCOUNT_LOGIN_FAIL_PASSWORD_INVALID, i_iSocket );
    return;
  }
#endif

  // Get all subclasses matching the foreign key
  std::string strQuery{ "SELECT account_id,password FROM account WHERE name='" };
  strQuery += i_strAccountName;
  strQuery += "' LIMIT 1";
  QueryPtr pcQuery{ rumDatabase::Query( rumDatabase::DatabaseID::Player_DatabaseID, strQuery ) };
  if( !pcQuery || pcQuery->IsError() )
  {
    AccountLoginFailed( AccountLoginResultType::ACCOUNT_LOGIN_ERROR, i_iSocket );
  }
  else if( 0 == pcQuery->GetNumRows() )
  {
    AccountLoginFailed( AccountLoginResultType::ACCOUNT_LOGIN_FAIL_ACCOUNT_NOT_FOUND, i_iSocket );
  }
  else
  {
    const rumUniqueID uiAccountID{ (rumUniqueID)pcQuery->FetchInt64( 0, 0 ) };
    const std::string strAccountPassword{ pcQuery->FetchString( 0, 1 ) };

    Logger::SetOutputColor( COLOR_WARNING );

    std::string strInfo{ "Account login " };
    strInfo += i_strAccountName;
    strInfo += " [";
    strInfo += rumStringUtils::ToHexString64( uiAccountID );
    strInfo += "] socket [";
    strInfo += rumStringUtils::ToHexString64( i_iSocket );
    strInfo += "]";
    Logger::LogStandard( strInfo );

    Logger::SetOutputColor( COLOR_STANDARD );

#ifndef _DEBUG
    if( strAccountPassword != i_strPassword )
    {
      AccountLoginFailed( ACCOUNT_LOGIN_FAIL_AUTHENTICATION_FAILED, i_iSocket );
    }
    else
#endif
    {
      // Associated the account key with the socket - we'll need this when a
      // player is logged in later to make sure that other accounts aren't able
      // to log into someone elses' players
      const auto cPair{ s_hashAccountIDs.insert( std::make_pair( i_iSocket, uiAccountID ) ) };
      if( !cPair.second )
      {
        AccountLoginFailed( AccountLoginResultType::ACCOUNT_LOGIN_FAIL_ACCOUNT_ACTIVE, i_iSocket );
      }
      else
      {
        strQuery = "SELECT name FROM player WHERE account_id_fk=";
        strQuery += rumStringUtils::ToString64( uiAccountID );
        pcQuery = rumDatabase::Query( rumDatabase::DatabaseID::Player_DatabaseID, strQuery );
        if( pcQuery && !pcQuery->IsError() )
        {
          const int32_t iPlayers{ pcQuery->GetNumRows() };

          // Send all player names to the account logging in
          auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
          rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_ACCOUNT_LOGIN_RESULT )
            << rumByte( AccountLoginResultType::ACCOUNT_LOGIN_SUCCESS )
            << rumQWord( i_iSocket )
            << rumByte( iPlayers );

          // If the account has players, send them along as well
          if( iPlayers > 0 )
          {
            // Add player information to the account login packet
            for( int32_t i = 0; i < iPlayers; ++i )
            {
              rcPacket.Write( pcQuery->Fetch( i, 0 ) );
            }
          }

          rcPacket.Send( i_iSocket );

          // Notify scripts of account login
          HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };
          rumScript::ExecOptionalFunc( Sqrat::RootTable( pcVM ), "OnAccountLogin", i_iSocket, i_strAccountName.c_str(),
                                       uiAccountID );
        }
      }
    }
  }
}


// static
void rumServerAccount::AccountLoginFailed( AccountLoginResultType i_eReason, SOCKET i_iSocket )
{
  auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
  rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_ACCOUNT_LOGIN_RESULT )
    .Write( rumByte( i_eReason ) )
    .Send( i_iSocket );
}


void rumServerAccount::AccountLogout( SOCKET i_iSocket )
{
  const auto iter{ s_hashAccountIDs.find( i_iSocket ) };
  if( iter == s_hashAccountIDs.end() )
  {
    return;
  }

  Logger::SetOutputColor( COLOR_WARNING );

  std::string strInfo{ "Account logout [" };
  strInfo += rumStringUtils::ToHexString64( iter->second );
  strInfo += "] socket [";
  strInfo += rumStringUtils::ToHexString64( i_iSocket );
  strInfo += "]";
  Logger::LogStandard( strInfo );

  Logger::SetOutputColor( COLOR_STANDARD );

  // Disassociate the account key with the socket
  s_hashAccountIDs.erase( i_iSocket );
}


// static
void rumServerAccount::CreateAccount( const std::string& i_strAccountName, const std::string& i_strEmail,
                                      const std::string& i_strPassword, SOCKET i_iSocket )
{
  // Hopefully, these were caught by the client, but we still have to check them
  if( i_strAccountName.empty() )
  {
    // Account name cannot be empty
    auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
    rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_ACCOUNT_CREATE_RESULT )
      .Write( rumByte( AccountCreateResultType::ACCOUNT_CREATE_FAIL_NAME_INVALID ) )
      .Send( i_iSocket );
    return;
  }
  else if( i_strPassword.empty() )
  {
    // User must supply a password
    auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
    rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_ACCOUNT_CREATE_RESULT )
      .Write( rumByte( AccountCreateResultType::ACCOUNT_CREATE_FAIL_PASSWORD_INVALID ) )
      .Send( i_iSocket );
    return;
  }

  // See if this account already exists
  std::string strQuery{ "SELECT account_id FROM account WHERE name='" };
  strQuery += i_strAccountName;
  strQuery += "'";
  const QueryPtr pcQuery{ rumDatabase::Query( rumDatabase::DatabaseID::Player_DatabaseID, strQuery ) };
  if( pcQuery && !pcQuery->IsError() && pcQuery->GetNumRows() > 0 )
  {
    // Inform the client that this account name already exists
    auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
    rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_ACCOUNT_CREATE_RESULT )
      .Write( rumByte( AccountCreateResultType::ACCOUNT_CREATE_FAIL_NAME_EXISTS ) )
      .Send( i_iSocket );
    return;
  }

  const rumUniqueID uiAccountID
  {
    rumDatabase::GetNextIDFromIDStore( rumDatabase::IDStoreTableType::Account_IDStoreTableType )
  };

  strQuery = "INSERT INTO account (account_id,name,email,password) VALUES (";
  strQuery += rumStringUtils::ToString64( uiAccountID );
  strQuery += ",'";
  strQuery += i_strAccountName;
  strQuery += "','";
  strQuery += i_strEmail;
  strQuery += "','";
  strQuery += i_strPassword;
  strQuery += "')";

  rumAsyncDatabase::QueryAsync( rumDatabase::DatabaseID::Player_DatabaseID, strQuery );

  rumAccountCreationInfo cCreationInfo;
  cCreationInfo.m_uiAccountID = uiAccountID;
  cCreationInfo.m_strAccountName = i_strAccountName;
  cCreationInfo.m_strPassword = i_strPassword;
  cCreationInfo.m_tSubmitted.Restart();

  s_hashPendingAccounts.insert( std::make_pair( i_iSocket, cCreationInfo ) );
}


// static
rumUniqueID rumServerAccount::GetAccountID( SOCKET i_iSocket )
{
  rumUniqueID uiAccountID{ 0 };

  // Get the account key associated with this socket
  auto iter( s_hashAccountIDs.find( i_iSocket ) );
  if( iter != s_hashAccountIDs.end() )
  {
    uiAccountID = iter->second;
  }

  return uiAccountID;
}


// static
void rumServerAccount::ProcessPendingAccounts()
{
  std::string strQuery{ "SELECT account_id FROM account WHERE name='" };
  static const size_t iLength{ strQuery.length() };

  PendingAccountContainer::iterator iter( s_hashPendingAccounts.begin() );
  const PendingAccountContainer::iterator end( s_hashPendingAccounts.end() );
  while( iter != end )
  {
    const SOCKET iSocket{ iter->first };
    const rumAccountCreationInfo& rcAccountInfo{ iter->second };

    strQuery = strQuery.substr( 0, iLength );

    // Determine if the player has been created
    strQuery += rcAccountInfo.m_strAccountName;
    strQuery += "' LIMIT 1";

    const QueryPtr pcQuery{ rumDatabase::Query( rumDatabase::DatabaseID::Player_DatabaseID, strQuery ) };
    if( pcQuery && !pcQuery->IsError() )
    {
      if( pcQuery->GetNumRows() > 0 )
      {
        // The creation is no longer pending
        iter = s_hashPendingAccounts.erase( iter );

        // Notify the client of success
        auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
        rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_ACCOUNT_CREATE_RESULT )
          .Write( rumByte( AccountCreateResultType::ACCOUNT_CREATE_SUCCESS ) )
          .Send( iSocket );
      }
      else if( rcAccountInfo.m_tSubmitted.GetElapsedSeconds() > 10.0 )
      {
        // The creation has likely failed
        iter = s_hashPendingAccounts.erase( iter );

        // Notify the client of failure
        auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
        rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_ACCOUNT_CREATE_RESULT )
          .Write( rumByte( AccountCreateResultType::ACCOUNT_CREATE_FAIL_TIMEOUT ) )
          .Send( iSocket );
      }
      else
      {
        ++iter;
      }
    }
    else
    {
      ++iter;
    }
  }
}


// static
void rumServerAccount::ScriptBind()
{
  rumAccount::ScriptBind();
}
