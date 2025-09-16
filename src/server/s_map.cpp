#include <s_map.h>

#include <s_pawn.h>
#include <s_player.h>

#include <network/u_packet.h>
#include <u_map_asset.h>


// override
void rumServerMap::AllocateGameID( rumUniqueID i_uiGameID )
{
  // Newly created server-side maps should not already have a UID
  rumAssertMsg( INVALID_GAME_ID == i_uiGameID, "Creating server-side map with a non-zero GameID" );

  static rumUniqueID s_uiAssetType{ ( rumUniqueID( rumMapAsset::GetClassRegistryID() ) ) << 60 };
  static rumUniqueID s_uiGameID{ ( rumUniqueID( Server_ObjectCreationType ) ) << 56 };

  i_uiGameID = ++s_uiGameID | s_uiAssetType;
  SetGameID( i_uiGameID );
}


// The pawn is leaving the map edge, transfer the pawn to the default exit map
int32_t rumServerMap::Exit( Sqrat::Object i_sqInstance, rumMap* i_pcMap )
{
  if( i_sqInstance.GetType() == OT_INSTANCE )
  {
    rumPawn* pcPawn{ i_sqInstance.Cast<rumPawn*>() };
    if( pcPawn )
    {
      return TransferPawn( pcPawn, i_pcMap, m_cExitPos );
    }
  }

  return RESULT_FAILED;
}


// static
void rumServerMap::ScriptBind()
{
  super::ScriptBind();

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::DerivedClass<rumServerMap, rumMap> cServerMap( pcVM, "rumServerMap" );
  cServerMap
    .Func( "Exit", &Exit )
    .Func( "SendRadial", &ScriptBroadcastRadial )
    .Func( "SendRegional", &ScriptBroadcastRegional );
  Sqrat::RootTable( pcVM ).Bind( SCRIPT_MAP_NATIVE_CLASS, cServerMap );
}


int32_t rumServerMap::ScriptBroadcastRadial( Sqrat::Object i_sqInstance, rumPosition i_rcPosition,
                                             uint32_t i_uiRadius ) const
{
  if( i_sqInstance.GetType() != OT_INSTANCE )
  {
    return RESULT_FAILED;
  }

  // Visit all players
  for( const auto iter : m_hashPlayers )
  {
    const rumUniqueID uiPlayerID{ iter };

    const rumPlayer* pcPlayer{ rumPlayer::FetchByPlayerID( uiPlayerID ) };
    if( pcPlayer )
    {
      const rumPawn* pcPawn{ pcPlayer->GetPlayerPawn() };
      if( pcPawn && IsWithinTileDistance( i_rcPosition, pcPawn->GetPos(), i_uiRadius, Intercardinal_DirectionType ) )
      {
        auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
        rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SCRIPT_DEFINED )
          .Write( i_sqInstance );
        rumServerPlayer::SendPacket( rcPacket, uiPlayerID );
      }
    }
  }

  return RESULT_SUCCESS;
}


int32_t rumServerMap::ScriptBroadcastRegional( Sqrat::Object i_sqInstance ) const
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
    rumServerPlayer::SendPacket( rcPacket, m_hashPlayers );
  }

  return RESULT_SUCCESS;
}


// override
bool rumServerMap::AddPawn( rumPawn* i_pcPawn, const rumPosition& i_rcPos )
{
  rumAssert( i_pcPawn );
  if( !i_pcPawn )
  {
    return false;
  }

  if( !super::AddPawn( i_pcPawn, i_rcPos ) )
  {
    return false;
  }

  if( GetNumPlayers() > 0 )
  {
    // Send all basic pawn info to all players on this map
    auto& rcPacket1{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
    rcPacket1.SetHeader( rumNetwork::PACKET_HEADER_SERVER_PAWN_UPDATES )
      .Write( (rumDWord)1 );
    if( rumServerPawn::PackageAttributes( i_pcPawn, rcPacket1 ) )
    {
      rumServerPlayer::SendPacket( rcPacket1, m_hashPlayers );
    }

    // When a pawn is added to a map, it must send its regional properties to all players in the same region
    auto& rcPacket2{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
    if( rumServerPawn::PackageRegionalProperties( i_pcPawn, rcPacket2 ) )
    {
      rumServerPlayer::SendPacket( rcPacket2, m_hashPlayers, i_pcPawn->GetGameID() );
    }
  }

  if( i_pcPawn->GetScriptInstance().GetType() == OT_INSTANCE )
  {
    // Optional script callback
    rumScript::ExecOptionalFunc( i_pcPawn->GetScriptInstance(), "OnAddedToMap" );
  }

  return true;
}


// override
bool rumServerMap::RemovePawn( rumPawn* i_pcPawn )
{
  rumAssert( i_pcPawn );
  if( !i_pcPawn )
  {
    return false;
  }

  // Send a pawn removal notice to all players on this map. This must be done prior to calling super::RemovePawn
  // because the player will not longer exist on the map after that call! The player being removed will not receive the
  // update that removes the player's pawn.
  auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
  rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_PAWN_REMOVE )
    << i_pcPawn->GetGameID();
  rumServerPlayer::SendPacket( rcPacket, m_hashPlayers );

  return super::RemovePawn( i_pcPawn );
}


// override
void rumServerMap::OnPropertyRemoved( rumAssetID i_ePropertyID )
{
  const rumPropertyAsset* pcProperty{ rumPropertyAsset::Fetch( i_ePropertyID ) };
  if( !pcProperty )
  {
    return;
  }

  if( pcProperty->GetServiceType() == Shared_ServiceType )
  {
    if( pcProperty->IsGlobal() )
    {
      if( rumServerPlayer::GetNumPlayers() > 0 )
      {
        // Broadcast change to all players in the game
        auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
        rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_PROPERTY_REMOVED )
          .Write( GetGameID() ).Write( i_ePropertyID );
        rumServerPlayer::SendGlobalPacket( rcPacket );
      }
    }
    else if( pcProperty->IsRegional() )
    {
      // Send change to only players on this map
      // TODO: Regional replication doesn't exist for maps
      // Get list of maps from scripts, send to all players on all maps
      rumAssert( false );
    }
    else if( pcProperty->IsPrivate() )
    {
      if( GetNumPlayers() > 0 )
      {
        // Send change to only players on this map
        auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
        rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_PROPERTY_REMOVED )
          .Write( GetGameID() ).Write( i_ePropertyID );
        rumServerPlayer::SendPacket( rcPacket, m_hashPlayers );
      }
    }
  }

  if( pcProperty->IsPersistent() )
  {
    // TODO: Implement
    rumAssert( false );
  }

  super::OnPropertyRemoved( i_ePropertyID );
}


// override
void rumServerMap::OnPropertyUpdated( rumAssetID i_ePropertyID, Sqrat::Object i_sqValue, bool i_bAdded )
{
  const rumPropertyAsset* pcProperty{ rumPropertyAsset::Fetch( i_ePropertyID ) };
  if( !pcProperty )
  {
    return;
  }

  if( pcProperty->GetServiceType() == Shared_ServiceType )
  {
    if( pcProperty->IsGlobal() )
    {
      if( rumServerPlayer::GetNumPlayers() > 0 )
      {
        // Broadcast change to all players in the game
        auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
        rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_PROPERTY_UPDATE )
          .Write( GetGameID() )
          .Write( 1 )
          .Write( i_ePropertyID )
          .Write( i_sqValue );
        rumServerPlayer::SendGlobalPacket( rcPacket );
      }
    }
    else if( pcProperty->IsRegional() )
    {
      // Send change to only players on this map
      // TODO: Regional replication doesn't exist for maps
      // Get list of maps from scripts, send to all players on all maps
      rumAssert( false );
    }
    else if( pcProperty->IsPrivate() )
    {
      if( GetNumPlayers() > 0 )
      {
        // Send change to only players on this map
        auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
        rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_PROPERTY_UPDATE )
          .Write( 1 )
          .Write( i_ePropertyID )
          .Write( i_sqValue );
        rumServerPlayer::SendPacket( rcPacket, m_hashPlayers );
      }
    }
  }

  if( pcProperty->IsPersistent() )
  {
    // TODO: Implement
    rumAssert( false );
  }

  super::OnPropertyUpdated( i_ePropertyID, i_sqValue, i_bAdded );
}


int32_t rumServerMap::Load()
{
  //cout << "==============================================================================" << endl;
  //cout << "Asynchronously loading map " << strFile << endl;

  m_eStatus = STATUS_LOADING;
  const int32_t eResult{ super::Load() };
  if( RESULT_SUCCESS == eResult )
  {
    m_eStatus = STATUS_ACTIVE;
  }
  else
  {
    m_eStatus = STATUS_LOAD_FAILED;
  }

  return eResult;
}


// final
int32_t rumServerMap::Serialize( rumResource& io_rcResource )
{
  // The server should never save maps
  rumAssert( io_rcResource.IsLoading() );
  return io_rcResource.IsLoading() ? super::Serialize( io_rcResource ) : RESULT_FAILED;
}


// final
int32_t rumServerMap::SerializePawns( rumResource& io_rcResource )
{
  // The server should never save map pawns
  rumAssert( io_rcResource.IsLoading() );
  return io_rcResource.IsLoading() ? super::SerializePawns( io_rcResource ) : RESULT_FAILED;
}


// override
int32_t rumServerMap::AddPlayer( rumPlayer* i_pcPlayer )
{
  int32_t eResult{ RESULT_FAILED };

  if( m_eStatus != STATUS_ACTIVE )
  {
    //debug
    //cout << "Map is not active... player pawn is queued for add" << endl;

    // Queue player for later add
    //pendingPlayers.push(pcPlayer);
    eResult = RESULT_SUCCESS;
  }
  else if( super::AddPlayer( i_pcPlayer ) == RESULT_SUCCESS )
  {
    rumPosition cPos;
    const rumPawn* pcPawn{ i_pcPlayer->GetPlayerPawn() };
    if( pcPawn )
    {
      cPos = pcPawn->GetPos();
    }

    if( rumServerPlayer::GetNumPlayers() > 0 )
    {
      // Notify everyone that this player is changing maps
      auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
      rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_PLAYER_MAP_UPDATE )
        << rumQWord( i_pcPlayer->GetPlayerID() )
        << rumDWord( GetAssetID() )
        << rumQWord( GetGameID() )
        << rumDWord( cPos.m_iX )
        << rumDWord( cPos.m_iY );
      rumServerPlayer::SendGlobalPacket( rcPacket );
    }

    // Synchronize all map data with the newly added player
    eResult = SynchronizeClient( i_pcPlayer->GetSocket() );
  }

  return eResult;
}


int32_t rumServerMap::SynchronizeClient( SOCKET i_iSocket ) const
{
  // Only send a pawn update packet if there are pawns to update
  if( m_hashPawns.size() == 0 )
  {
    return RESULT_SUCCESS;
  }

  // TODO - for future
  // Walk changed tile list

#pragma message( "TODO - Needs improvement. Investigate sending properties as part of the PAWN_UPDATES packet.")

  auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
  rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_PAWN_UPDATES )
    .Write( (rumDWord)m_hashPawns.size() );

  // First, send down the pawn updates - the pawn must exist on the client before the properties can be sent
  for( auto iter : m_hashPawns )
  {
    const rumUniqueID uiPawnID{ iter };
    const rumPawn* pcPawn{ rumPawn::Fetch( uiPawnID ) };
    if( pcPawn )
    {
      rumServerPawn::PackageAttributes( pcPawn, rcPacket );
    }
    else
    {
      std::string strWarning{ "Failed to fetch pawn [" };
      strWarning += rumStringUtils::ToHexString64( uiPawnID );
      strWarning += " for synchronization on socket [";
      strWarning += rumStringUtils::ToHexString64( i_iSocket );
      strWarning += "]";
      Logger::LogStandard( strWarning, Logger::LOG_WARNING );
    }
  }

  rcPacket.Send( i_iSocket );

  // Finally, send the property updates
  for( auto iter : m_hashPawns )
  {
    const rumUniqueID uiPawnID{ iter };
    const rumPawn* pcPawn{ rumPawn::Fetch( uiPawnID ) };
    if( pcPawn )
    {
      auto& rcPropertyPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
      if( rumServerPawn::PackageRegionalProperties( pcPawn, rcPropertyPacket ) )
      {
        rcPropertyPacket.Send( i_iSocket );
      }
    }
    else
    {
      std::string strWarning{ "Failed to fetch pawn [" };
      strWarning += rumStringUtils::ToHexString64( uiPawnID );
      strWarning += " for synchronization on socket [";
      strWarning += rumStringUtils::ToHexString64( i_iSocket );
      strWarning += "]";
      Logger::LogStandard( strWarning, Logger::LOG_WARNING );
    }
  }

  return RESULT_SUCCESS;
}
