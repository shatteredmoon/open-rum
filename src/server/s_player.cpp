#include <s_player.h>

#include <s_account.h>
#include <s_inventory.h>
#include <s_map.h>
#include <s_pawn.h>

#include <network/u_packet.h>
#include <u_db.h>
#include <u_enum.h>
#include <u_inventory_asset.h>
#include <u_log.h>
#include <u_map_asset.h>
#include <u_property_asset.h>

// Static initializers
rumServerPlayer::PlayerPacketContainer rumServerPlayer::s_hashPlayerPackets;
rumServerPlayer::PendingPlayerContainer rumServerPlayer::s_hashPendingPlayers;
rumServerPlayer::PendingLogoutContainer rumServerPlayer::s_queuePendingPlayerLogouts;
rumServerPlayer::SocketContainer rumServerPlayer::s_hashSockets;


// static
void rumServerPlayer::CheckIdlePlayers()
{
  for( auto& rcPlayer : GetPlayerHash() )
  {
    auto* pcPlayer{ rcPlayer.second };
    if( pcPlayer )
    {
      pcPlayer->CheckIdleTime();
    }
  }
}


// override
void rumServerPlayer::CheckIdleTime()
{
  const auto elapsedTime{ m_cLastPacketTime.GetElapsedSeconds() };
  if( elapsedTime > 180.0 )
  {
  rumScript::ExecOptionalFunc( GetPlayerPawn()->GetScriptInstance(), "OnIdleLogout" );
  }
  else if( elapsedTime > 120.0 && !m_bIdleWarningSent )
  {
    rumScript::ExecOptionalFunc( GetPlayerPawn()->GetScriptInstance(), "OnIdleWarn" );
    m_bIdleWarningSent = true;
  }
}


// static
void rumServerPlayer::CheckPlayerCreations()
{
  std::string strQuery{ "SELECT player_id FROM player WHERE name='" };
  static const size_t pos{ strQuery.length() };

  PendingPlayerContainer::iterator iter( s_hashPendingPlayers.begin() );
  while( iter != s_hashPendingPlayers.end() )
  {
    const SOCKET iSocket{ iter->first };
    PlayerCreationInfo& rcPlayerInfo{ iter->second };

    // Determine if the player has been created
    strQuery += rcPlayerInfo.strPlayerName;
    strQuery += "' LIMIT 1";

    QueryPtr pcQuery{ rumDatabase::Query( rumDatabase::DatabaseID::Player_DatabaseID, strQuery ) };
    if( pcQuery && !pcQuery->IsError() && pcQuery->GetNumRows() > 0 )
    {
      // Notify the client of success
      auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
      rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_PLAYER_CREATE_RESULT )
        .Write( rumByte( PLAYER_CREATE_SUCCESS ) )
        .Write( rcPlayerInfo.strPlayerName )
        .Send( iSocket );

      // TODO? Evoke a script callback for player creation
      //Sqrat::Object sqFunc = sqInstance.GetValue("onPlayerCreated");

      iter = s_hashPendingPlayers.erase( iter );
    }
    else
    {
      if( rcPlayerInfo.timeSubmitted.GetElapsedSeconds() > 10.0 )
      {
        // Notify the client of failure
        auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
        rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_PLAYER_CREATE_RESULT )
          .Write( rumByte( PLAYER_CREATE_FAIL_TIMEOUT ) )
          .Write( rcPlayerInfo.strPlayerName )
          .Send( iSocket );

        // The creation has likely failed
        iter = s_hashPendingPlayers.erase( iter );
      }
      else
      {
        ++iter;
      }
    }
  }
}


void rumServerPlayer::CheckPlayerLogouts()
{
  while( !s_queuePendingPlayerLogouts.empty() )
  {
    SOCKET iSocket{ s_queuePendingPlayerLogouts.front() };
    s_queuePendingPlayerLogouts.pop();

    // See if there is a player associated with the socket
    rumPlayer* pcPlayer{ FetchBySocket( iSocket ) };
    if( !pcPlayer )
    {
      continue;
    }

    const auto uiPlayerID{ pcPlayer->GetPlayerID() };

    Logger::SetOutputColor( COLOR_WARNING );

    std::string strEntry{ "Player logout: " };
    strEntry += pcPlayer->GetName();
    strEntry += " [";
    strEntry += rumStringUtils::ToHexString64( uiPlayerID );
    strEntry += "] socket ";
    strEntry += " [";
    strEntry += rumStringUtils::ToHexString64( iSocket );
    strEntry += "]";
    Logger::LogPlayer( strEntry );

    Logger::SetOutputColor( COLOR_STANDARD );

    // Remove the player from the game
    Remove( uiPlayerID );

    // Notify the client that the player is now logged out
    auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
    rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_PLAYER_LOGOUT_RESULT )
      .Write( rumQWord( uiPlayerID ) )
      .Send( iSocket );
  }
}


// static
rumPlayer::PlayerCreateResultType rumServerPlayer::CheckPlayerName( const std::string& i_strPlayerName )
{
  // Is this player name already taken?
  std::string strQuery{ "SELECT COUNT(*) FROM player WHERE name LIKE '" };
  strQuery += i_strPlayerName;
  strQuery += "'";
  QueryPtr pcQuery{ rumDatabase::Query( rumDatabase::DatabaseID::Player_DatabaseID, strQuery ) };
  if( !pcQuery || pcQuery->IsError() )
  {
    return PLAYER_CREATE_ERROR;
  }
  else if( pcQuery->FetchInt( 0, 0 ) > 0 )
  {
    return PLAYER_CREATE_FAIL_NAME_EXISTS;
  }

  // Is this name restricted?
  strQuery = "SELECT COUNT(*) FROM restricted_names WHERE name LIKE '";
  strQuery += i_strPlayerName;
  strQuery += "'";
  pcQuery = rumDatabase::Query( rumDatabase::DatabaseID::Player_DatabaseID, strQuery );
  if( !pcQuery || pcQuery->IsError() )
  {
    return PLAYER_CREATE_ERROR;
  }
  else if( pcQuery->FetchInt( 0, 0 ) > 0 )
  {
    return PLAYER_CREATE_FAIL_NAME_INVALID;
  }

  return PLAYER_CREATE_SUCCESS;
}


// static
rumPlayer* rumServerPlayer::Create( SOCKET i_iSocket )
{
  if( INVALID_SOCKET == i_iSocket )
  {
    rumAssertMsg( false, "Attempt to create player with invalid socket" );
    return nullptr;
  }

#if MEMORY_DEBUG
  std::string strInfo{ "Creating player, socket [" };
  strInfo += ToHexString64( i_iSocket );
  strInfo += "]";
  Logger::LogStandard( strInfo );
#endif // MEMORY_DEBUG

  // Does this player exist already?
  rumPlayer* pcPlayer{ FetchBySocket( i_iSocket ) };
  if( !pcPlayer )
  {
    // Create the player object
    rumServerPlayer* pcServerPlayer{ new rumServerPlayer };
    pcPlayer = pcServerPlayer;
  }

  return pcPlayer;
}


// static
rumPlayer* rumServerPlayer::FetchBySocket( SOCKET i_iSocket )
{
  const auto& iter( s_hashSockets.find( i_iSocket ) );
  if( iter != s_hashSockets.end() )
  {
    rumUniqueID uiPlayerID{ iter->second };
    return FetchByPlayerID( uiPlayerID );
  }

  return nullptr;
}


void rumServerPlayer::InitDone()
{
  SetPersistenceEnabled( true );

  // Learn what other players are in the game, which maps they're on, and what their global properties are
  SyncAllPlayerInfo( GetSocket() );
}


bool rumServerPlayer::InitFromDB()
{
  // Don't update the database while logging in
  SetPersistenceEnabled( false );

  rumPawn* pcPawn{ GetPlayerPawn() };
  rumAssert( pcPawn );
  if( !pcPawn )
  {
    // The player's pawn object must exist
    return false;
  }

  Sqrat::Object sqInstance{ pcPawn->GetScriptInstance() };
  rumAssert( sqInstance.GetType() == OT_INSTANCE );
  if( sqInstance.GetType() != OT_INSTANCE )
  {
    // The player's pawn must have an associated script instance
    return false;
  }

  // Get all subclasses matching the foreign key
  std::string strQuery{ "SELECT map_id_fk,posx,posy FROM player WHERE player_id=" };
  strQuery += rumStringUtils::ToHexString64( GetPlayerID() );
  strQuery += " LIMIT 1";
  QueryPtr pcQuery{ rumDatabase::Query( rumDatabase::DatabaseID::Player_DatabaseID, strQuery ) };
  if( !pcQuery || pcQuery->IsError() || ( pcQuery->GetNumRows() == 0 ) )
  {
    std::string strError{ "Error: Failed to find player " };
    strError += GetName();
    strError += " [";
    strError += rumStringUtils::ToHexString64( GetPlayerID() );
    strError += "] in the database";
    Logger::LogStandard( strError, Logger::LOG_ERROR );

    return false;
  }

  // This single call would init the pawn and pawn properties
  pcPawn->InitFromPlayerDB( pcQuery );

  // Database values - these will be overridden by function parameters
  const rumAssetID eMapID{ (rumAssetID)pcQuery->FetchInt( 0, 0 ) };
  const rumPosition cPos( pcQuery->FetchInt( 0, 1 ), pcQuery->FetchInt( 0, 2 ) );

  rumScript::ExecRequiredFunc( sqInstance, "OnRestored", eMapID, cPos );

  // Fetch player inventory
  QueryInventory( pcPawn );

  return true;
}


void rumServerPlayer::KeepAlive()
{
  m_cLastPacketTime.Restart();
  m_bIdleWarningSent = false;
}


// override
void rumServerPlayer::OnCreated( SOCKET i_iSocket, std::string i_strPlayerName, rumUniqueID i_uiPlayerID,
                                 rumPawn* i_pcPawn )
{
  rumAssert( i_pcPawn );
  if( !i_pcPawn )
  {
    return;
  }

  super::OnCreated( i_iSocket, i_strPlayerName, i_uiPlayerID, i_pcPawn );

  // Note: the player ID must be set!
  Manage( this );

  // Associate the player's socket with their ID
  s_hashSockets.insert( std::make_pair( i_iSocket, i_uiPlayerID ) );
}


// override
void rumServerPlayer::OnMapUpdated( rumUniqueID i_eMapID, const rumPosition& i_rcPos )
{
  const rumMap* pcMap{ rumMap::Fetch( i_eMapID ) };
  if( pcMap && m_bPersistenceEnabled )
  {
    // Update the database
    std::string strQuery{ "UPDATE player SET map_id_fk=" };
    strQuery += rumStringUtils::ToHexString( pcMap->GetAssetID() );
    strQuery += ",posx=";
    strQuery += rumStringUtils::ToString( i_rcPos.m_iX );
    strQuery += ",posy=";
    strQuery += rumStringUtils::ToString( i_rcPos.m_iY );
    strQuery += " WHERE player_id=";
    strQuery += rumStringUtils::ToHexString64( GetPlayerID() );
    rumAsyncDatabase::QueryAsync( rumDatabase::DatabaseID::Player_DatabaseID, strQuery );

    m_uiMovesSinceSave = 0;
    m_uiMovesOnCurrentMap = 0;
  }
}


// static
std::pair<rumPlayer::PlayerLoginResultType, Sqrat::Object>
rumServerPlayer::OnPlayerLogin( SOCKET i_iSocket, const std::string& i_strPlayerName, rumUniqueID i_uiPlayerID,
                                bool i_bRestoreDB )
{
  // Create the player's pawn
  Sqrat::Object sqInstance{ rumGameObject::Create( GetPlayerClass() ) };
  if( sqInstance.GetType() != OT_INSTANCE )
  {
    std::string strError{ "Error: Failed to create a pawn for player " };
    strError += i_strPlayerName;
    Logger::LogStandard( strError );
    return { PLAYER_LOGIN_ERROR, Sqrat::Object() };
  }

  rumPawn* pcPawn{ sqInstance.Cast<rumPawn*>() };
  if( !pcPawn )
  {
    std::string strError{ "Error: Failed to create a pawn for player " };
    strError += i_strPlayerName;
    Logger::LogStandard( strError );
    return { PLAYER_LOGIN_ERROR, Sqrat::Object() };
  }

  // Create the player
  rumPlayer* pcPlayer{ Create( i_iSocket ) };
  if( !pcPlayer )
  {
    std::string strError{ "Error: Failed to create player object for player " };
    strError += i_strPlayerName;
    Logger::LogStandard( strError );
    pcPawn->Free();
    return { PLAYER_LOGIN_ERROR, Sqrat::Object() };
  }

  // Note: the client must have created the client (via PACKET_HEADER_SERVER_NEW_PLAYER_INFO) before calling this!
  pcPlayer->OnCreated( i_iSocket, i_strPlayerName, i_uiPlayerID, pcPawn );
  rumAssert( pcPlayer->GetPlayerPawn() != nullptr );

  // Replicate both this new player to other players, and all existing player info to the player logging in
  pcPlayer->Replicate();

  rumScript::ExecRequiredFunc( sqInstance, "OnLogin", i_strPlayerName.c_str(), i_uiPlayerID );

  if( i_bRestoreDB )
  {
    if( !pcPlayer->InitFromDB() )
    {
      std::string strError{ "Error: Failed to restore database for player " };
      strError += i_strPlayerName;
      Logger::LogStandard( strError );
      pcPawn->Free();
      delete pcPlayer;
      return { PLAYER_LOGIN_ERROR, Sqrat::Object() };
    }
  }

  rumPlayer* pcServerPlayer{ FetchByPlayerID( i_uiPlayerID ) };
  if( pcServerPlayer )
  {
    pcServerPlayer->InitDone();
  }

  return { PLAYER_LOGIN_SUCCESS, sqInstance };
}


void rumServerPlayer::OnPositionUpdated( const rumPosition& i_rcPos, uint32_t i_uiDistance )
{
  ++m_uiMovesSinceSave;

  // If the player moved a considerable distance on their current map (e.g. teleported) go ahead and save
  if( m_bPersistenceEnabled && ( m_uiMovesSinceSave >= 10 || ( m_uiMovesOnCurrentMap > 0 && i_uiDistance > 1 ) ) )
  {
    PawnPositionSave( i_rcPos );
    m_uiMovesSinceSave = 0;
  }

  ++m_uiMovesOnCurrentMap;
}


// override
void rumServerPlayer::OnRemoved()
{
  RemovePackets();

  std::string strInfo{ "Disconnecting player: " };
  strInfo += GetName();
  strInfo += " [";
  strInfo += rumStringUtils::ToHexString64( GetPlayerID() );
  strInfo += "] socket ";
  strInfo += " [";
  strInfo += rumStringUtils::ToHexString64( GetSocket() );
  strInfo += "]";
  Logger::LogStandard( strInfo, Logger::LOG_WARNING );

  // Save pawn info
  rumPawn* pcPawn{ GetPlayerPawn() };
  if( pcPawn )
  {
    PawnPositionSave( pcPawn->GetPosX(), pcPawn->GetPosY() );

    // Remove the pawn from the map and free it from memory
    rumMap* pcMap{ pcPawn->GetMap() };
    if( pcMap && pcMap->RemovePawn( pcPawn ) )
    {
      rumGameObject::UnmanageScriptObject( pcPawn->GetScriptInstance() );
    }
  }

  std::string strQuery{ "UPDATE player SET datetime=DATETIME('NOW'),status=0 WHERE player_id=" };
  strQuery += rumStringUtils::ToHexString64( GetPlayerID() );
  rumAsyncDatabase::QueryAsync( rumDatabase::DatabaseID::Player_DatabaseID, strQuery );

  RUM_COUT( GetPlayerHash().size() << " player(s) online\n" );

  SetPersistenceEnabled( false );
}


void rumServerPlayer::PawnPositionSave( const rumPosition& i_rcPos ) const
{
  std::string strQuery{ "UPDATE player SET posx=" };
  strQuery += rumStringUtils::ToString( i_rcPos.m_iX );
  strQuery += ",posy=";
  strQuery += rumStringUtils::ToString( i_rcPos.m_iY );
  strQuery += " WHERE player_id=";
  strQuery += rumStringUtils::ToHexString64( GetPlayerID() );
  rumAsyncDatabase::QueryAsync( rumDatabase::DatabaseID::Player_DatabaseID, strQuery );
}


bool rumServerPlayer::PopPacket()
{
  m_bPacketRequested = true;

  bool bSuccess{ false };
  const SOCKET iSocket{ GetSocket() };

  const auto& iter( s_hashPlayerPackets.find( iSocket ) );
  if( iter != s_hashPlayerPackets.end() )
  {
    std::queue<Sqrat::Object>& rQueue{ iter->second };

    if( !rQueue.empty() )
    {
      Sqrat::Object sqInstance{ rQueue.front() };
      bSuccess = ProcessBroadcastPacket( sqInstance );
      rQueue.pop();
    }
  }

  return bSuccess;
}


// static
void rumServerPlayer::ProcessBroadcast( SOCKET i_iSocket, Sqrat::Object i_sqInstance )
{
  rumPlayer* pcPlayer{ FetchBySocket( i_iSocket ) };
  if( pcPlayer )
  {
    pcPlayer->ProcessBroadcastPacket( i_sqInstance );
  }
  else
  {
    // A packet was received that's not directly tied to a player. This is expected for player creation requests, etc.
    if( i_sqInstance.GetType() == OT_INSTANCE )
    {
      rumScript::ExecRequiredFunc( i_sqInstance, "OnRecv", i_iSocket, Sqrat::Object() );
    }
  }
}


bool rumServerPlayer::ProcessBroadcastPacket( Sqrat::Object i_sqInstance )
{
  // Is this a valid player?
  const auto iter{ s_hashPlayerPackets.find( GetSocket() ) };
  if( iter == s_hashPlayerPackets.end() )
  {
    // Should the packet be handled right away?
    if( IsWaitingForBroadcastPacket() )
    {
      RUM_COUT_IFDEF( NETWORK_DEBUG, "Processing broadcast packet immediately (no queue)\n" );

      bool bExecuted{ rumScript::ExecRequiredFunc( i_sqInstance, "OnRecv", GetSocket(),
                                                   GetScriptPlayer( GetPlayerID() ) ) };
      if( !bExecuted )
      {
        // The script call failed for whatever reason, so mark the player as waiting for another packet in case the
        // script didn't get a chance to do so before it crashed
        m_bPacketRequested = true;
      }
    }
    else
    {
      RUM_COUT_IFDEF( NETWORK_DEBUG, "Creating packet queue for new player\n" );

      // Create a packet queue for the player
      std::queue<Sqrat::Object> queuePlayerPackets;
      queuePlayerPackets.push( i_sqInstance );
      s_hashPlayerPackets.insert( std::make_pair( GetSocket(), queuePlayerPackets ) );
    }
  }
  else
  {
    std::queue<Sqrat::Object>& rQueue{ iter->second };
    if( rQueue.empty() && IsWaitingForBroadcastPacket() )
    {
      RUM_COUT_IFDEF( NETWORK_DEBUG, "Processing broadcast packet immediately (no queue)\n" );

      // Process the packet right away
      bool bExecuted{ rumScript::ExecRequiredFunc( i_sqInstance, "OnRecv", GetSocket(),
                                                   GetScriptPlayer( GetPlayerID() ) ) };
      if( !bExecuted )
      {
        // The script call failed for whatever reason, so mark the player as waiting for another packet in case the
        // script didn't get a chance to do so before it crashed
        m_bPacketRequested = true;
      }
    }
    else if( rQueue.size() < s_uiMaxQueuedPackets )
    {
      RUM_COUT_IFDEF( NETWORK_DEBUG, "Adding new packet to queue\n" );

      // Add the new packet to the queue
      rQueue.push( i_sqInstance );
    }
    else
    {
      std::string strInfo{ "Player [" };
      strInfo += rumStringUtils::ToHexString64( GetPlayerID() );
      strInfo += "] socket [";
      strInfo += rumStringUtils::ToHexString64( GetSocket() );
      strInfo += "] has queued too many incoming packets, dropping new arrival";
      Logger::LogStandard( strInfo );
    }
  }

  return true;
}


void rumServerPlayer::ProcessPlayerCreateRequest( SOCKET i_iSocket, const std::string& i_strPlayerName )
{
  // Hopefully, this was caught by the client, but we still have to check
  if( i_strPlayerName.empty() )
  {
    // Player name cannot be empty
    auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
    rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_PLAYER_CREATE_RESULT )
      .Write( rumByte( PLAYER_CREATE_FAIL_NAME_INVALID ) )
      .Write( i_strPlayerName )
      .Send( i_iSocket );
    return;
  }

  const PlayerCreateResultType eResult{ CheckPlayerName( i_strPlayerName ) };
  if( PLAYER_CREATE_SUCCESS == eResult )
  {
    // Gather info from the player DB
    const uint64_t uiAccountID{ rumServerAccount::GetAccountID( i_iSocket ) };
    const uint64_t uiPlayerID
    {
      rumDatabase::GetNextIDFromIDStore(rumDatabase::IDStoreTableType::Player_IDStoreTableType )
    };

    std::string strQuery{ "INSERT INTO player (player_id,account_id_fk,name,status,datetime,map_id_fk,posx,posy) "
                          "VALUES (" };
    strQuery += rumStringUtils::ToHexString64( uiPlayerID );
    strQuery += ",";
    strQuery += rumStringUtils::ToHexString64( uiAccountID );
    strQuery += ",'";
    strQuery += i_strPlayerName;
    strQuery += "',0,0,-1,0,0)";

    rumAsyncDatabase::QueryAsync( rumDatabase::DatabaseID::Player_DatabaseID, strQuery );

    PlayerCreationInfo pci;
    pci.strPlayerName = i_strPlayerName;
    pci.timeSubmitted.Restart();

    s_hashPendingPlayers.insert( std::make_pair( i_iSocket, pci ) );

    std::string strEntry{ "[ACCT " };
    strEntry += rumStringUtils::ToHexString64( uiAccountID );
    strEntry += "] Created player ";
    strEntry += i_strPlayerName;
    Logger::LogPlayer( strEntry );
  }
  else
  {
    // Notify the client of failure
    auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
    rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_PLAYER_CREATE_RESULT )
      .Write( rumByte( eResult ) )
      .Write( i_strPlayerName )
      .Send( i_iSocket );
  }
}


void rumServerPlayer::ProcessPlayerDeleteRequest( SOCKET i_iSocket, const std::string& i_strPlayerName )
{
  // Hopefully, this was caught by the client, but we still have to check
  if( i_strPlayerName.empty() )
  {
    return;
  }

  const uint64_t uiAccountID{ rumServerAccount::GetAccountID( i_iSocket ) };

  std::string strQuery{ "SELECT player_id,status FROM player WHERE account_id_fk=" };
  strQuery += rumStringUtils::ToHexString64( uiAccountID );
  strQuery += " AND name='";
  strQuery += i_strPlayerName;
  strQuery += "' LIMIT 1";
  QueryPtr pcQuery{ rumDatabase::Query( rumDatabase::DatabaseID::Player_DatabaseID, strQuery ) };
  if( pcQuery && !pcQuery->IsError() && pcQuery->GetNumRows() > 0 )
  {
    const rumUniqueID uiPlayerID{ (rumUniqueID)pcQuery->FetchInt64( 0, 0 ) };
    const std::string strPlayerID{ rumStringUtils::ToHexString64( uiPlayerID ) };
    const int32_t iStatus{ pcQuery->FetchInt( 0, 1 ) };

    // TODO - use status to make sure player is not logged in

    std::string strDeleteQuery{ "BEGIN;" };

    strQuery = "SELECT inventory_id FROM inventory WHERE player_id_fk=";
    strQuery += strPlayerID;
    pcQuery = rumDatabase::Query( rumDatabase::DatabaseID::Player_DatabaseID, strQuery );
    if( pcQuery && !pcQuery->IsError() && pcQuery->GetNumRows() > 0 )
    {
      for( int32_t i = 0; i < pcQuery->GetNumRows(); ++i )
      {
        const rumUniqueID uiInventoryID{ (rumUniqueID)pcQuery->FetchInt64( i, 0 ) };
        strDeleteQuery += "DELETE FROM inventory_properties WHERE inventory_id_fk=";
        strDeleteQuery += rumStringUtils::ToHexString64( uiInventoryID );
        strDeleteQuery += ";";
      }

      strDeleteQuery += "DELETE FROM inventory WHERE player_id_fk=";
      strDeleteQuery += strPlayerID;
      strDeleteQuery += ";";
    }

    strDeleteQuery += "DELETE FROM player_properties WHERE player_id_fk=";
    strDeleteQuery += strPlayerID;
    strDeleteQuery += ";";

    strDeleteQuery += "DELETE FROM player WHERE player_id=";
    strDeleteQuery += strPlayerID;
    strDeleteQuery += ";";

    strDeleteQuery += "COMMIT;";

    rumAsyncDatabase::QueryAsync( rumDatabase::DatabaseID::Player_DatabaseID, strDeleteQuery );

    std::string strEntry{ "[ACCT " };
    strEntry += rumStringUtils::ToHexString64( uiAccountID );
    strEntry += "] Deleted player ";
    strEntry += i_strPlayerName;
    strEntry += " [PID ";
    strEntry += rumStringUtils::ToHexString64( uiPlayerID );
    strEntry += "]";
    Logger::LogPlayer( strEntry );
  }
}


rumServerPlayer::LoginResult rumServerPlayer::ProcessPlayerLoginRequest( SOCKET i_iSocket,
                                                                         const std::string& i_strPlayerName,
                                                                         bool i_bRestoreDB )
{
  // Hopefully, these were caught by the client, but we still have to check them
  if( i_strPlayerName.empty() )
  {
    // Account name cannot be empty
    return { PLAYER_LOGIN_FAIL_NAME_INVALID, Sqrat::Object() };
  }

  const rumUniqueID uiAccountID{ rumServerAccount::GetAccountID( i_iSocket ) };

  // Get all subclasses matching the foreign key (casting datetime column and 'now' result to integer)
  std::string strQuery{ "SELECT player_id,CAST(strftime('%s', datetime) AS INT), CAST(strftime('%s', DATETIME('NOW')) AS INT),status FROM player WHERE name='" };
  strQuery += i_strPlayerName;
  strQuery += "' AND account_id_fk=";
  strQuery += rumStringUtils::ToHexString64( uiAccountID );
  strQuery += " LIMIT 1";
  QueryPtr pcQuery{ rumDatabase::Query( rumDatabase::DatabaseID::Player_DatabaseID, strQuery ) };
  if( !pcQuery || pcQuery->IsError() )
  {
    std::string strError{ "Error: Failed to query player database for player " };
    strError += i_strPlayerName;
    if( pcQuery )
    {
      strError += ", ";
      strError += pcQuery->GetErrorMsg();
    }
    Logger::LogStandard( strError, Logger::LOG_ERROR );
    return { PLAYER_LOGIN_ERROR, Sqrat::Object() };
  }
  else if( 0 == pcQuery->GetNumRows() )
  {
    std::string strError{ "Error: Failed to query player database for player " };
    strError += i_strPlayerName;
    return { PLAYER_LOGIN_FAIL_PLAYER_NOT_FOUND, Sqrat::Object() };
  }

  enum{ PLAYER_ID, UPDATETIME, CURRENTTIME, STATUS };

  const rumUniqueID uiPlayerID{ (rumUniqueID)pcQuery->FetchInt64( 0, PLAYER_ID ) };
  const int32_t iUpdateTime{ pcQuery->FetchInt( 0, UPDATETIME ) };
  const int32_t iCurrentTime{ pcQuery->FetchInt( 0, CURRENTTIME ) };
  const int32_t iStatus{ pcQuery->FetchInt( 0, STATUS ) };

#ifdef _DEBUG
  constexpr int32_t iNumSecondsForPlayerReset{ 5 };
#else
  constexpr int32_t iNumSecondsForPlayerReset{ 30 };
#endif // _DEBUG

  if( iStatus != 0 )
  {
    SendPlayerLoginFailed( PLAYER_LOGIN_FAIL_PLAYER_ACTIVE, i_iSocket );
    return { PLAYER_LOGIN_FAIL_PLAYER_ACTIVE, Sqrat::Object() };
  }
  else if( iCurrentTime - iUpdateTime < iNumSecondsForPlayerReset )
  {
    SendPlayerLoginFailed( PLAYER_LOGIN_FAIL_PLAYER_CYCLING, i_iSocket );
    return { PLAYER_LOGIN_FAIL_PLAYER_CYCLING, Sqrat::Object() };
  }

  Logger::SetOutputColor( COLOR_WARNING );
  std::string strEntry{ "Logging in player: " };
  strEntry += i_strPlayerName;
  strEntry += " [";
  strEntry += rumStringUtils::ToHexString64( uiPlayerID );
  strEntry += "] account [";
  strEntry += rumStringUtils::ToHexString64( uiAccountID );
  strEntry += "] socket [";
  strEntry += rumStringUtils::ToHexString64( i_iSocket );
  strEntry += "]";
  RUM_COUT( strEntry << '\n' );
  Logger::LogPlayer( strEntry );

  Logger::SetOutputColor( COLOR_STANDARD );

  strQuery = "UPDATE player SET datetime=DATETIME('NOW'),status=1 WHERE player_id=";
  strQuery += rumStringUtils::ToHexString64( uiPlayerID );
  rumAsyncDatabase::QueryAsync( rumDatabase::DatabaseID::Player_DatabaseID, strQuery );
  if( !pcQuery || pcQuery->IsError() )
  {
    std::string strError{ "Error: Failed to update player database for player " };
    strError += i_strPlayerName;
    if( pcQuery )
    {
      strError += ", ";
      strError += pcQuery->GetErrorMsg();
    }
    Logger::LogStandard( strError, Logger::LOG_ERROR );
    return { PLAYER_LOGIN_ERROR, Sqrat::Object() };
  }

  auto cPair{ OnPlayerLogin( i_iSocket, i_strPlayerName, uiPlayerID, i_bRestoreDB ) };
  if( cPair.first != PLAYER_LOGIN_SUCCESS )
  {
    SendPlayerLoginFailed( PLAYER_LOGIN_ERROR, i_iSocket );
  }

  return cPair;
}


// static
Sqrat::Object rumServerPlayer::ProcessPlayerLoginRequest_VM( SOCKET i_iSocket, Sqrat::Object i_sqPlayerName,
                                                             bool i_bRestoreDB )
{
  if( i_sqPlayerName.GetType() == OT_STRING )
  {
    auto cPair{ ProcessPlayerLoginRequest( i_iSocket, i_sqPlayerName.Cast<std::string>(), i_bRestoreDB ) };
    if( PLAYER_LOGIN_SUCCESS == cPair.first )
    {
      // return the player object
      return cPair.second;
    }
  }

  g_pstrLastErrorString = GetLoginResultTypeString( PLAYER_LOGIN_ERROR ).c_str();
  return Sqrat::Object();
}


// static
void rumServerPlayer::ProcessPlayerLogoutRequest( SOCKET i_iSocket )
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  // Notify scripts of player logout
  rumScript::ExecRequiredFunc( Sqrat::RootTable( pcVM ), "OnPlayerLogout", i_iSocket );

  // Queue the logout since this function might've been called during script execution. During script execution, the
  // player's instance may still be holding various references.
  s_queuePendingPlayerLogouts.push( i_iSocket );
}


int32_t rumServerPlayer::QueryInventory( rumPawn* i_pcPawn )
{
  std::string strQuery{ "SELECT inventory_id,inventory_asset_fk FROM inventory WHERE player_id_fk=" };
  strQuery += rumStringUtils::ToHexString64( GetPlayerID() );

  QueryPtr pcQuery{ rumDatabase::Query( rumDatabase::DatabaseID::Player_DatabaseID, strQuery ) };
  if( pcQuery && !pcQuery->IsError() )
  {
    enum{ DB_ID, DB_INVENTORY_FK };

    // Iterate through all results rows
    for( int32_t i = 0; i < pcQuery->GetNumRows(); ++i )
    {
      rumAssetID eInventoryID{ (rumAssetID)pcQuery->FetchInt( i, DB_INVENTORY_FK ) };

      Sqrat::Object sqObject{ rumGameObject::Create( eInventoryID ) };
      if( sqObject.GetType() == OT_INSTANCE )
      {
        rumServerInventory* pcInventory{ sqObject.Cast<rumServerInventory*>() };
        if( i_pcPawn->AddInventory( pcInventory ) && ( i_pcPawn->GetScriptInstance().GetType() == OT_INSTANCE ) )
        {
          if( pcInventory->InitFromDB( pcQuery, i ) == RESULT_SUCCESS )
          {
            // Optional script callback
            rumScript::ExecOptionalFunc( i_pcPawn->GetScriptInstance(), "OnInventoryRestored", sqObject );
          }
        }
      }
    }

    return RESULT_SUCCESS;
  }

  return RESULT_FAILED;
}


// static
bool rumServerPlayer::Remove( rumUniqueID i_uiPlayerID )
{
  auto pcPlayer{ FetchByPlayerID( i_uiPlayerID ) };
  if( pcPlayer )
  {
    s_hashSockets.erase( pcPlayer->GetSocket() );
  }

  return super::Remove( i_uiPlayerID );
}


void rumServerPlayer::RemovePackets()
{
  const auto& iter{ s_hashPlayerPackets.find( GetSocket() ) };
  if( iter != s_hashPlayerPackets.end() )
  {
    std::queue<Sqrat::Object>& rQueue{ iter->second };
    while( !rQueue.empty() )
    {
      rQueue.pop();
    }

    // Remove the packet container completely for this player
    s_hashPlayerPackets.erase( iter );
  }
}


// override
void rumServerPlayer::Replicate()
{
  if( GetNumPlayers() > 0 )
  {
    // Note: The player must be managed before this happens!
    // Broadcast the new player so that all clients can create it. This has to be done prior to property and inventory
    // fetching so that applicable items can replicate to clients.
    auto& rcNewPlayerPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
    rcNewPlayerPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_NEW_PLAYER_INFO )
      << GetName()
      << rumQWord( GetPlayerID() )
      << rumQWord( GetPawnID() );
    SendGlobalPacket( rcNewPlayerPacket );
  }

  // Send all of the existing players to the new player
  if( GetPlayerHash().size() > 1 )
  {
    auto& rcAllPlayersPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
    rcAllPlayersPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_ALL_PLAYER_INFO )
      << rumWord( GetPlayerHash().size() - 1 );

    for( const auto iter : GetPlayerHash() )
    {
      rumPlayer* pcPlayer{ iter.second };
      if( pcPlayer && ( pcPlayer->GetPlayerID() != GetPlayerID() ) )
      {
        rcAllPlayersPacket
          << pcPlayer->GetName()
          << rumQWord( pcPlayer->GetPlayerID() )
          << rumQWord( pcPlayer->GetPawnID() );
      }
    }

    rcAllPlayersPacket.Send( GetSocket() );
  }

  auto& rcLoginPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
  rcLoginPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_PLAYER_LOGIN_RESULT )
    .Write( rumQWord( GetPlayerID() ) )
    .Write( rumByte( PLAYER_LOGIN_SUCCESS ) )
    .Send( GetSocket() );
}


// static
void rumServerPlayer::ScriptBind()
{
  super::ScriptBind();

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::RootTable( pcVM )
    .Func( "rumGetPlayerBySocket", &ScriptFetchBySocket )
    .Func( "rumPlayerLogin", ProcessPlayerLoginRequest_VM )
    .Func( "rumPlayerLogout", ProcessPlayerLogoutRequest )
    .Func( "rumSendGlobal", ScriptBroadcastGlobal )
    .Func( "rumSendPrivate", ScriptBroadcastPrivate );
}


// static
int32_t rumServerPlayer::ScriptBroadcastGlobal( Sqrat::Object i_sqInstance )
{
  if( i_sqInstance.GetType() != OT_INSTANCE )
  {
    return RESULT_FAILED;
  }

  if( GetNumPlayers() > 0 )
  {
    auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
    rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SCRIPT_DEFINED )
      .Write( i_sqInstance );
    return SendGlobalPacket( rcPacket );
  }

  return RESULT_SUCCESS;
}


// static
int32_t rumServerPlayer::ScriptBroadcastPrivate( SOCKET i_iSocket, Sqrat::Object i_sqInstance )
{
  if( i_sqInstance.GetType() != OT_INSTANCE )
  {
    return RESULT_FAILED;
  }

  auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
  rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SCRIPT_DEFINED )
    .Write( i_sqInstance )
    .Send( i_iSocket );

  return RESULT_SUCCESS;
}


// static
Sqrat::Object rumServerPlayer::ScriptFetchBySocket( SOCKET i_iSocket )
{
  rumPlayer* pcPlayer{ FetchBySocket( i_iSocket ) };
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
int32_t rumServerPlayer::SendGlobalPacket( rumNetwork::rumOutboundPacket& i_rcPacket, SOCKET i_iIgnoredSocket )
{
  for( const auto& iter : GetPlayerHash() )
  {
    rumPlayer* pcPlayer{ iter.second };
    rumAssert( pcPlayer );
    if( pcPlayer && pcPlayer->GetSocket() != i_iIgnoredSocket )
    {
      i_rcPacket.AddRecipient( pcPlayer->GetSocket() );
    }
  }

  if( i_rcPacket.HasRecipients() )
  {
    i_rcPacket.Send();
  }

  return RESULT_SUCCESS;
}


// static
void rumServerPlayer::SendPacket( rumNetwork::rumOutboundPacket& i_rcPacket, const GameIDHash& i_hashPlayers,
                                  rumUniqueID i_uiIgnoredPlayerID )
{
  for( const auto& iter : i_hashPlayers )
  {
    const rumUniqueID uiPlayerID{ iter };
    if( uiPlayerID != INVALID_GAME_ID && uiPlayerID != i_uiIgnoredPlayerID )
    {
      const rumPlayer* pcPlayer{ FetchByPlayerID( uiPlayerID ) };
      if( pcPlayer )
      {
        i_rcPacket.AddRecipient( pcPlayer->GetSocket() );
      }
    }
  }

  if( i_rcPacket.HasRecipients() )
  {
    i_rcPacket.Send();
  }
}


// static
void rumServerPlayer::SendPacket( rumNetwork::rumOutboundPacket& i_rcPacket, rumUniqueID i_uiPlayerID )
{
  rumAssert( i_uiPlayerID != INVALID_GAME_ID );
  if( INVALID_GAME_ID == i_uiPlayerID )
  {
    return;
  }

  const rumPlayer* pcPlayer{ FetchByPlayerID( i_uiPlayerID ) };
  if( pcPlayer )
  {
    i_rcPacket.Send( pcPlayer->GetSocket() );
  }
}


void rumServerPlayer::SendPlayerLoginFailed( PlayerLoginResultType i_eReason, SOCKET i_iSocket )
{
  g_pstrLastErrorString = GetLoginResultTypeString( i_eReason ).c_str();

  auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
  rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_PLAYER_LOGIN_RESULT )
    .Write( rumQWord( INVALID_GAME_ID ) )
    .Write( rumByte( i_eReason ) )
    .Send( i_iSocket );
}


void rumServerPlayer::Shutdown()
{
  s_hashSockets.clear();
  super::Shutdown();
}


// static
void rumServerPlayer::SyncAllPlayerInfo( SOCKET i_iSocket )
{
  for( auto iter : GetPlayerHash() )
  {
    rumUniqueID uiPlayerID{ iter.first };
    rumPlayer* pcPlayer{ iter.second };
    rumAssert( pcPlayer );
    if( !pcPlayer || ( pcPlayer->GetSocket() == i_iSocket ) )
    {
      // No need to sync our own info
      continue;
    }

    rumPawn* pcPawn{ pcPlayer->GetPlayerPawn() };
    rumAssert( pcPawn );
    if( pcPawn )
    {
      // Send the player's global properties
      auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
      if( rumServerPawn::PackageGlobalProperties( pcPawn, rcPacket ) )
      {
        rcPacket.Send( i_iSocket );
      }
    }
  }
}
