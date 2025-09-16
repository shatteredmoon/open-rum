#include <u_player.h>

#include <u_assert.h>
#include <u_log.h>
#include <u_pawn.h>
#include <u_map.h>

// Static member initializations
Sqrat::Object rumPlayer::s_sqClass;
rumPlayer::PlayerContainer rumPlayer::s_hashPlayers;

// It is sent in a single byte back to the client on creation failure
const std::string rumPlayer::s_aPlayerCreateResultTypeStrings[] =
{
  "",
  "rum_player_create_error",
  "rum_player_create_name_exists",
  "rum_player_create_name_invalid",
  "rum_player_create_name_restricted",
  "rum_player_create_timeout"
};

// It is sent in a single byte back to the client on login failure
const std::string rumPlayer::s_aPlayerLoginResultTypeStrings[] =
{
  "",
  "rum_player_login_error",
  "rum_player_login_name_invalid",
  "rum_player_login_not_found",
  "rum_player_login_already_active",
  "rum_player_login_cycling"
};


// static
bool rumPlayer::Manage( rumPlayer* i_pcPlayer )
{
  if( !i_pcPlayer )
  {
    return false;
  }

  if( i_pcPlayer->GetPlayerID() == INVALID_GAME_ID )
  {
    rumAssertMsg( false, "Invalid player game ID" );
    return false;
  }

  // Add the player to the player container - this will fail if the ID already exists as a key in the container
  const auto cPair{ s_hashPlayers.insert( std::make_pair( i_pcPlayer->GetPlayerID(), i_pcPlayer ) ) };
  if( !cPair.second )
  {
    std::string strError{ "Failed to manage player " };
    strError += i_pcPlayer->GetName();
    strError += " [";
    strError += rumStringUtils::ToHexString64( i_pcPlayer->GetPlayerID() );
    strError += "]";
    Logger::LogStandard( strError, Logger::LOG_ERROR );

    return false;
  }

  return true;
}


// static
rumPlayer* rumPlayer::FetchByPlayerID( rumUniqueID i_uiPlayerID )
{
  const auto& iter( s_hashPlayers.find( i_uiPlayerID ) );
  if( iter != s_hashPlayers.end() )
  {
    return iter->second;
  }

  return nullptr;
}


rumPawn* rumPlayer::GetPlayerPawn() const
{
  return rumPawn::Fetch( m_uiPawnID );
}


// static
Sqrat::Object rumPlayer::GetScriptPlayer( rumUniqueID i_uiPlayerID )
{
  const rumPlayer* pcPlayer{ FetchByPlayerID( i_uiPlayerID ) };
  if( pcPlayer )
  {
    rumPawn* pcPawn{ pcPlayer->GetPlayerPawn() };
    if( pcPawn )
    {
      return pcPawn->GetScriptInstance();
    }
  }

  return Sqrat::Object();
}


// static
int32_t rumPlayer::Init()
{
  s_sqClass = Sqrat::RootTable().GetSlot( SCRIPT_PLAYER_SCRIPT_CLASS );
  if( s_sqClass.GetType() != OT_CLASS )
  {
    // Game scripts did not provide an script, so create a default
    rumScript::CreateClassScript( SCRIPT_PLAYER_SCRIPT_CLASS, "rumCreature" );

    s_sqClass = Sqrat::RootTable().GetSlot( SCRIPT_PLAYER_SCRIPT_CLASS );
    if( s_sqClass.GetType() != OT_CLASS )
    {
      std::string strError = "Could not find a script defined Player class that extends Creature.";
      Logger::LogStandard( strError, Logger::LOG_ERROR );
      return RESULT_FAILED;
    }
  }

  return ( s_sqClass.GetType() == OT_CLASS ? RESULT_SUCCESS : RESULT_FAILED );
}


// virtual
void rumPlayer::OnCreated( SOCKET i_iSocket, std::string i_strPlayerName, rumUniqueID i_uiPlayerID, rumPawn* i_pcPawn )
{
  rumAssert( i_pcPawn );
  if( !i_pcPawn )
  {
    return;
  }

  SetSocket( i_iSocket );
  SetName( i_strPlayerName );
  SetPlayerID( i_uiPlayerID );
  SetPawnID( i_pcPawn->GetGameID() );

  i_pcPawn->SetPlayerID( i_uiPlayerID );
}


// virtual
bool rumPlayer::ProcessBroadcastPacket( Sqrat::Object i_sqInstance )
{
#if NETWORK_DEBUG
  const rumGameObject* pcObject{ i_sqInstance.Cast<rumGameObject*>() };
  RUM_COUT( "Received SCRIPT DEFINED packet type: " << pcObject->GetName() << '\n' );
#endif // NETWORK_DEBUG

  rumScript::ExecRequiredFunc( i_sqInstance, "OnRecv" );

  return true;
}


// static
bool rumPlayer::Remove( rumUniqueID i_uiPlayerID )
{
  SOCKET i_iSocket{ INVALID_SOCKET };

  const auto& iter( s_hashPlayers.find( i_uiPlayerID ) );
  if( iter != s_hashPlayers.end() )
  {
    rumPlayer* pcPlayer{ iter->second };
    rumAssert( pcPlayer );
    if( pcPlayer )
    {
      pcPlayer->OnRemoved();
    }
    SAFE_DELETE( pcPlayer );

    s_hashPlayers.erase( iter );

    return true;
  }

  return false;
}


// static
void rumPlayer::ScriptBind()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::RootTable( pcVM )
    .Func( "rumGetPlayer", &ScriptGetPlayer )
    .Func( "rumGetPlayerByName", &ScriptGetPlayerByName )
    .Func( "rumGetAllPlayers", &ScriptGetAllPlayers );
}


// static
Sqrat::Object rumPlayer::ScriptGetAllPlayers()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::Array sqArray( pcVM, (int32_t)s_hashPlayers.size() );

  uint32_t uiIndex{ 0 };

  for( const auto& iter : s_hashPlayers )
  {
    const rumPlayer* pcPlayer{ iter.second };
    if( pcPlayer )
    {
      rumPawn* pcPawn{ pcPlayer->GetPlayerPawn() };
      if( pcPawn )
      {
        sqArray.SetValue( uiIndex++, pcPawn->GetScriptInstance() );
      }
    }
  }

  return sqArray;
}


// static
Sqrat::Object rumPlayer::ScriptGetPlayer( rumUniqueID i_uiGameID )
{
  if( i_uiGameID != INVALID_GAME_ID )
  {
    rumPawn* pcPawn{ rumPawn::Fetch( i_uiGameID ) };
    if( pcPawn )
    {
      return pcPawn->GetScriptInstance();
    }
  }

  return Sqrat::Object();
}


// static
Sqrat::Object rumPlayer::ScriptGetPlayerByName( Sqrat::Object i_sqStrName )
{
  if( i_sqStrName.GetType() == OT_STRING )
  {
    const std::string strName{ i_sqStrName.Cast<std::string>() };

    for( const auto& iter : s_hashPlayers )
    {
      const rumPlayer* pcPlayer{ iter.second };
      if( strcasecmp( strName.c_str(), pcPlayer->GetName().c_str() ) == 0 )
      {
        rumPawn* pcPawn{ pcPlayer->GetPlayerPawn() };
        if( pcPawn )
        {
          return pcPawn->GetScriptInstance();
        }

        return Sqrat::Object();
      }
    }
  }

  return Sqrat::Object();
}


// static
void rumPlayer::Shutdown()
{
  // Deallocate all player pawns and deallocate memory
  for( const auto& iter : s_hashPlayers )
  {
    const rumPlayer* pcPlayer{ iter.second };
    rumAssert( pcPlayer );
    if( pcPlayer )
    {
      rumPawn* pcPawn{ pcPlayer->GetPlayerPawn() };
      if( pcPawn )
      {
        pcPawn->Free();
      }

      delete pcPlayer;
    }
  }

  if( s_sqClass.GetType() != OT_NULL )
  {
    s_sqClass.Release();
  }

  s_hashPlayers.clear();
}
