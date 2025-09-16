#include <c_player.h>

#include <c_map.h>
#include <c_pawn.h>

#include <network/u_packet.h>
#include <u_account.h>
#include <u_assert.h>
#include <u_inventory.h>
#include <u_log.h>

// Static initializers
SOCKET rumClientPlayer::s_uiClientPlayerID{ INVALID_GAME_ID };
SOCKET rumClientPlayer::s_uiClientPlayerSocket{ INVALID_SOCKET };


// static
rumClientPlayer* rumClientPlayer::Create( rumUniqueID i_uiPlayerID )
{
  rumAssertMsg( i_uiPlayerID != INVALID_GAME_ID, "Attempt to create a player without a valid game id" );

#if MEMORY_DEBUG
  std::string strInfo{ "Creating player [" };
  strInfo += ToHexString64( i_uiPlayerID );
  strInfo += "]";
  Logger::LogStandard( strInfo );
#endif // MEMORY_DEBUG

  // Does this player exist already?
  rumClientPlayer* pcPlayer{ FetchByPlayerID( i_uiPlayerID ) };
  if( !pcPlayer )
  {
    // Create the player
    pcPlayer = new rumClientPlayer;
    if( pcPlayer )
    {
      pcPlayer->SetPlayerID( i_uiPlayerID );
      Manage( pcPlayer );
    }
  }

  return pcPlayer;
}


// static
rumPlayer::PlayerCreateResultType rumClientPlayer::RequestCreatePlayer( const std::string& i_strPlayerName )
{
  PlayerCreateResultType eResult{ PLAYER_CREATE_SUCCESS };

  if( i_strPlayerName.empty() )
  {
    eResult = PLAYER_CREATE_FAIL_NAME_INVALID;
    g_pstrLastErrorString = GetCreateResultTypeString( eResult ).c_str();

    std::string strInfo{ "Player creation failed, reason: " };
    strInfo += rumStringUtils::ToString( eResult );
    strInfo += " ";
    strInfo += g_pstrLastErrorString;
    Logger::LogStandard( strInfo );

    HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

    // Inform scripts that player creation failed
    rumScript::ExecRequiredFunc( Sqrat::RootTable( pcVM ), "OnPlayerCreationFailed", g_pstrLastErrorString,
                                 i_strPlayerName );
  }
  else
  {
    auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
    rcPacket.SetHeader( rumNetwork::PACKET_HEADER_CLIENT_PLAYER_CREATE )
      .Write( i_strPlayerName )
      .Send( rumNetwork::GetNetworkSocket() );
  }

  return eResult;
}


// static
void rumClientPlayer::CreateResult( PlayerCreateResultType i_eResult, const std::string& i_strPlayerName )
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  if( PLAYER_CREATE_SUCCESS == i_eResult )
  {
    // Add this player to the account
    rumAccount& rcAccount{ rumAccount::GetInstance() };
    rcAccount.AddCharacter( i_strPlayerName );

    // Inform scripts that player creation succeeded
    rumScript::ExecRequiredFunc( Sqrat::RootTable( pcVM ), "OnPlayerCreationSuccess", i_strPlayerName.c_str() );
  }
  else
  {
    g_pstrLastErrorString = GetCreateResultTypeString( i_eResult ).c_str();

    std::string strInfo{ "Player creation failed, reason: " };
    strInfo += rumStringUtils::ToString( i_eResult );
    strInfo += " ";
    strInfo += g_pstrLastErrorString;
    Logger::LogStandard( strInfo );

    // Inform scripts that player creation failed
    rumScript::ExecRequiredFunc( Sqrat::RootTable( pcVM ), "OnPlayerCreationFailed", g_pstrLastErrorString,
                                 i_strPlayerName );
  }
}


// static
void rumClientPlayer::LoginResult( rumUniqueID i_uiPlayerID, PlayerLoginResultType i_eResult )
{
  SetClientPlayerID( i_uiPlayerID );

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  if( PLAYER_LOGIN_SUCCESS == i_eResult )
  {
    // Inform scripts that player login has succeeded
    rumAssert( i_uiPlayerID != INVALID_GAME_ID );
    rumScript::ExecRequiredFunc( Sqrat::RootTable( pcVM ), "OnPlayerLoginSuccess" );
  }
  else
  {
    g_pstrLastErrorString = GetLoginResultTypeString( i_eResult ).c_str();

    std::string strInfo{ "Player login failed, reason: " };
    strInfo += rumStringUtils::ToString( i_eResult );
    strInfo += " ";
    strInfo += g_pstrLastErrorString;
    Logger::LogStandard( strInfo );

    // Inform scripts that login has failed
    rumScript::ExecRequiredFunc( Sqrat::RootTable( pcVM ), "OnPlayerLoginFailed", g_pstrLastErrorString );
  }
}


// static
void rumClientPlayer::LogoutResult( rumUniqueID i_uiPlayerID )
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  // Notify scripts of player logout
  rumScript::ExecRequiredFunc( Sqrat::RootTable( pcVM ), "OnPlayerLogout", i_uiPlayerID );

  const rumPlayer* pcPlayer{ FetchByPlayerID( i_uiPlayerID ) };
  if( pcPlayer )
  {
    rumPawn* pcPawn{ pcPlayer->GetPlayerPawn() };
    if( pcPawn )
    {
      rumMap* pcMap{ pcPawn->GetMap() };
      if( pcMap )
      {
        pcMap->RemovePawn( pcPawn );
      }

      // Release the script ref
      rumGameObject::UnmanageScriptObject( pcPawn->GetGameID() );
    }
  }

  Remove( i_uiPlayerID );
}


// static
void rumClientPlayer::ProcessBroadcast( Sqrat::Object i_sqInstance )
{
  rumPlayer* pcPlayer{ GetClientPlayer() };
  rumAssert( pcPlayer );
  if( pcPlayer )
  {
    pcPlayer->ProcessBroadcastPacket( i_sqInstance );
  }
}


// static
void rumClientPlayer::RecvInventoryUpdate( rumUniqueID i_uiPlayerID, rumUniqueID i_uiInventoryID,
                                           rumAssetID i_eInventoryID, bool i_bCreate )
{
  rumClientPlayer* pcPlayer{ FetchByPlayerID( i_uiPlayerID ) };
  rumAssert( pcPlayer );
  if( !pcPlayer )
  {
    return;
  }

  rumPawn* pcPawn{ pcPlayer->GetPlayerPawn() };
  rumAssert( pcPawn );
  if( !pcPawn )
  {
    return;
  }

  // Are we adding or removing the item?
  if( i_bCreate )
  {
    // Create the item and add it to the player
    Sqrat::Object sqObject{ rumGameObject::Create( i_eInventoryID, Sqrat::Table(), i_uiInventoryID ) };
    if( sqObject.GetType() == OT_INSTANCE )
    {
      rumInventory* pcInventory{ sqObject.Cast<rumInventory*>() };
      rumAssert( pcInventory );
      if( pcInventory )
      {
        pcPawn->AddInventory( pcInventory );
      }
    }
  }
  else
  {
    // Delete the item from the player
    rumInventory* pcInventory{ rumInventory::Fetch( i_uiInventoryID ) };
    rumAssert( pcInventory );
    if( pcInventory )
    {
      pcPawn->DeleteInventory( pcInventory );
    }
  }
}


// static
void rumClientPlayer::RecvMapUpdate( rumUniqueID i_uiPlayerID, rumAssetID i_eMapID, rumUniqueID i_uiMapID,
                                     const rumPosition& i_rcPos )
{
  rumClientPlayer* pcPlayer{ FetchByPlayerID( i_uiPlayerID ) };
  rumAssert( pcPlayer );
  if( !pcPlayer )
  {
    return;
  }

  if( rumClientPlayer::GetClientPlayerID() == i_uiPlayerID )
  {
    // The actual client player is getting an update
    rumPawn* pcPawn{ pcPlayer->GetPlayerPawn() };
    if( pcPawn )
    {
      rumAssetID ePrevMapID{ INVALID_ASSET_ID };

      // Point to the existing map (if it exists)
      rumMap* pcMap{ pcPawn->GetMap() };
      if( pcMap )
      {
        ePrevMapID = pcMap->GetAssetID();
      }

      if( ePrevMapID != i_eMapID )
      {
        // Create the new map
        Sqrat::Object sqMap{ rumGameObject::Create( i_eMapID, Sqrat::Table(), i_uiMapID ) };
        pcMap = sqMap.Cast<rumMap*>();
      }
      else if( pcMap )
      {
        // Just update the pawn's position
        rumMoveFlags eMoveFlags{ (rumMoveFlags)( IgnoreTileCollision_MoveFlag | IgnorePawnCollision_MoveFlag |
                                                 IgnoreDistance_MoveFlag ) };
        pcMap->MovePawn( pcPawn, i_rcPos, eMoveFlags, 0 );
      }

      rumAssert( pcMap );
      if( pcMap )
      {
        // Add the pawn to the map if needed
        if( pcPawn->GetMapID() == INVALID_GAME_ID )
        {
          // When the game is first launched, there is no previous map to transfer a player's pawn from. Just add the
          // player's pawn for the first time.
          pcMap->AddPawn( pcPawn, i_rcPos );
        }
      }

      // Let scripts know that the active map has changed
      if( ePrevMapID != i_eMapID )
      {
        HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };
        rumScript::ExecRequiredFunc( Sqrat::RootTable( pcVM ), "OnMapChange", pcMap->GetScriptInstance() );
      }
    }
  }
  else
  {
    // Not the main player
    rumPawn* pcPawn{ pcPlayer->GetPlayerPawn() };
    if( pcPawn )
    {
      // If the pawn exists on the main player's current map, add the pawn to it
      rumMap* pcTargetMap{ rumClientMap::Fetch( i_uiMapID ) };
      if( pcTargetMap )
      {
        pcTargetMap->AddPawn( pcPawn, i_rcPos );
      }
      else
      {
        // Just set the pawn's map id to reflect the map it's supposed to be on?
        pcPawn->SetMapID( i_eMapID, i_rcPos );
      }
    }
  }
}


// static
void rumClientPlayer::RecvNewPlayerInfo( const std::string& i_strPlayerName, rumUniqueID i_uiPlayerID,
                                         rumUniqueID i_uiPawnID )
{
  // Grab the player if it is available, if not, create it
  rumClientPlayer* pcPlayer{ FetchByPlayerID( i_uiPlayerID ) };
  if( !pcPlayer )
  {
    // Create the new player
    pcPlayer = Create( i_uiPlayerID );
    rumAssert( pcPlayer );
    if( !pcPlayer )
    {
      return;
    }
  }

  // Fetch the pawn if it exists, otherwise, create it. Pawns may already exist if received from a pawn updates packet.
  rumPawn* pcPawn{ rumClientPawn::Fetch( i_uiPawnID ) };
  if( !pcPawn )
  {
    // Create the player's pawn - player pawns are always loaded in memory so that we can query their map and position
    // without talking to the server
    Sqrat::Object sqInstance{ rumGameObject::Create( GetPlayerClass(), Sqrat::Table(), i_uiPawnID ) };
    if( sqInstance.GetType() == OT_INSTANCE )
    {
      pcPawn = sqInstance.Cast<rumPawn*>();
      rumGameObject::ManageScriptObject( sqInstance );
    }
  }

  if( !pcPawn )
  {
    std::string strError{ "Error: Failed to create pawn for player " };
    strError += i_strPlayerName;
    Logger::LogStandard( strError );
  }

  pcPlayer->OnCreated( INVALID_SOCKET, i_strPlayerName, i_uiPlayerID, pcPawn );
}


// static
void rumClientPlayer::RequestDeletePlayer( const std::string& i_strPlayerName )
{
  if( i_strPlayerName.empty() )
  {
    Logger::LogStandard( "Error: Failed to delete character - no character name provided" );
  }
  else
  {
    auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
    rcPacket.SetHeader( rumNetwork::PACKET_HEADER_CLIENT_PLAYER_DELETE )
      .Write( i_strPlayerName )
      .Send( rumNetwork::GetNetworkSocket() );

    rumAccount& rcAccount{ rumAccount::GetInstance() };
    rcAccount.RemoveCharacter( i_strPlayerName );
  }
}


// static
rumPlayer::PlayerLoginResultType rumClientPlayer::RequestLogin( const std::string& i_strPlayerName )
{
  PlayerLoginResultType eResult{ PLAYER_LOGIN_SUCCESS };

  if( i_strPlayerName.empty() )
  {
    eResult = PLAYER_LOGIN_FAIL_NAME_INVALID;
    g_pstrLastErrorString = GetLoginResultTypeString( eResult ).c_str();
  }
  else
  {
    auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
    rcPacket.SetHeader( rumNetwork::PACKET_HEADER_CLIENT_PLAYER_LOGIN )
      .Write( i_strPlayerName )
      .Send( rumNetwork::GetNetworkSocket() );
  }

  return eResult;
}


// static
void rumClientPlayer::RequestLogout()
{
  if( rumNetwork::GetNetworkSocket() != INVALID_SOCKET )
  {
    auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
    rcPacket.SetHeader( rumNetwork::PACKET_HEADER_CLIENT_PLAYER_LOGOUT )
      .Send( rumNetwork::GetNetworkSocket() );
  }
}


// static
void rumClientPlayer::ScriptBind()
{
  super::ScriptBind();

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::RootTable( pcVM )
    .Func( "rumGetMainPlayer", &GetClientPlayerVM )
    .Func( "rumSendBroadcast", &SendPacketVM )
    .Func( "rumCreatePlayer", &RequestCreatePlayer )
    .Func( "rumDeleteCharacter", &RequestDeletePlayer )
    .Func( "rumPlayerLogin", &RequestLogin )
    .Func( "rumPlayerLogout", &RequestLogout );
}


// static
int32_t rumClientPlayer::SendPacketVM( Sqrat::Object i_sqInstance )
{
  auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
  rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SCRIPT_DEFINED )
    .Write( i_sqInstance );
  return rcPacket.Send( rumNetwork::GetNetworkSocket() );
}
