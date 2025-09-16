#include <s_pawn.h>

#include <s_inventory.h>
#include <s_map.h>
#include <s_player.h>

#include <network/u_packet.h>
#include <u_creature_asset.h>
#include <u_db.h>
#include <u_inventory_asset.h>
#include <u_log.h>
#include <u_portal_asset.h>
#include <u_property_asset.h>
#include <u_resource.h>
#include <u_widget_asset.h>

#include <queue>


rumServerPawn::~rumServerPawn()
{
  SAFE_DELETE( m_pcAstarPath );
}


bool rumServerPawn::FindPath( const rumPosition& i_rcPos, uint32_t i_uiMaxDistance, rumDirectionType i_eDir )
{
  if( !m_pcAstarPath )
  {
    // This pawn has never plotted a path, create the AstarPath object
    m_pcAstarPath = new rumAstarPath();
  }

  if( !m_pcAstarPath->FindNewPath( this, i_rcPos, GetMap(), i_uiMaxDistance, i_uiMaxDistance * 2.f, i_eDir ) )
  {
    ForgetPath();
    return false;
  }

  return true;
}


void rumServerPawn::ForgetPath()
{
  if( m_pcAstarPath )
  {
    m_pcAstarPath->ForgetPath();
    delete m_pcAstarPath;
    m_pcAstarPath = nullptr;
  }
}


rumPosition rumServerPawn::GetPathPosition() const
{
  return ( m_pcAstarPath ? m_pcAstarPath->GetPos() : rumPosition() );
}


rumPosition rumServerPawn::GetPathTargetPosition() const
{
  return ( m_pcAstarPath ? m_pcAstarPath->GetTargetPos() : rumPosition() );
}


SOCKET rumServerPawn::GetSocket() const
{
  if( IsPlayer() )
  {
    rumPlayer* pcPlayer{ rumPlayer::FetchByPlayerID( GetPlayerID() ) };
    if( pcPlayer )
    {
      return pcPlayer->GetSocket();
    }
  }

  return INVALID_SOCKET;
}


bool rumServerPawn::HasPath() const
{
  return ( m_pcAstarPath ? m_pcAstarPath->HasPath() : false );
}


// override
int32_t rumServerPawn::InitFromPlayerDB( QueryPtr i_pcQuery )
{
  std::string strQuery{ "SELECT property_asset_fk,value,b.priority FROM player_properties "
                        "INNER JOIN assets.property b on b.type_id = player_properties.property_asset_fk "
                        "WHERE player_id_fk=" };
  strQuery += rumStringUtils::ToHexString64( GetPlayerID() );
  strQuery += " ORDER BY b.priority DESC";

  QueryPtr pcQuery{ rumDatabase::Query( rumDatabase::DatabaseID::Player_DatabaseID, strQuery ) };
  if( !pcQuery || pcQuery->IsError() )
  {
    return RESULT_FAILED;
  }

  InitPropertiesFromDB( pcQuery );

  return super::InitFromPlayerDB( i_pcQuery );
}


bool rumServerPawn::InitPropertiesFromDB( QueryPtr i_pcQuery )
{
  enum { DB_PROPERTY_ID, DB_PROPERTY_VALUE };

  // Since this is potentially a large fetch, reserve buckets if needed
  AllocatePropertyBuckets( i_pcQuery->GetNumRows() );

  for( int32_t i = 0; i < i_pcQuery->GetNumRows(); ++i )
  {
    const rumAssetID ePropertyID{ FULL_ASSET_ID( Property_AssetType, i_pcQuery->FetchInt( i, DB_PROPERTY_ID ) ) };
    const rumPropertyAsset* pcProperty{ rumPropertyAsset::Fetch( ePropertyID ) };
    rumAssert( pcProperty );
    if( !pcProperty )
    {
      std::string strWarning{ "Failed to fetch property id (" };
      strWarning += rumStringUtils::ToHexString( ePropertyID );
      strWarning += ") for pawn [";
      strWarning += rumStringUtils::ToHexString64( GetGameID() );
      strWarning += "]";
      Logger::LogStandard( strWarning, Logger::LOG_WARNING );

      continue;
    }

    RUM_COUT_IFDEF( PROPERTY_DEBUG,
                    "Loaded property " << pcProperty->GetName() << " [" <<
                    rumStringUtils::ToHexString( ePropertyID ) << "] type " <<
                    rumStringUtils::ToHexString( rumUtility::ToUnderlyingType( pcProperty->GetValueType() ) ) <<
                    '\n' );

    Sqrat::Object sqValue{ i_pcQuery->GetValueFromProperty( i, DB_PROPERTY_VALUE, pcProperty ) };
    SetProperty( ePropertyID, sqValue );
  }

  return true;
}


// override
void rumServerPawn::OnPropertyRemoved( rumAssetID i_ePropertyID )
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
        auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
        rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_PROPERTY_REMOVED )
          .Write( GetGameID() )
          .Write( i_ePropertyID );
        rumServerPlayer::SendGlobalPacket( rcPacket );

        RUM_COUT_IFDEF( PROPERTY_DEBUG,
                        "Pawn [" << rumStringUtils::ToHexString64( GetGameID() ) <<
                        "] broadcasting global property removal (" << rumStringUtils::ToHexString( i_ePropertyID ) <<
                        " to all players\n" );
      }
    }
    else if( pcProperty->IsRegional() )
    {
      const rumMap* pcMap{ GetMap() };
      if( pcMap && pcMap->GetNumPlayers() > 0 )
      {
        auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
        rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_PROPERTY_REMOVED )
          .Write( GetGameID() )
          .Write( i_ePropertyID );
        rumServerPlayer::SendPacket( rcPacket, pcMap->GetPlayers() );

        RUM_COUT_IFDEF( PROPERTY_DEBUG,
                        "Pawn [" << rumStringUtils::ToHexString64( GetGameID() ) <<
                        "] broadcasting regional property removal (" << rumStringUtils::ToHexString( i_ePropertyID ) <<
                        " to all players on map " << pcMap->GetName() << " [" <<
                        rumStringUtils::ToHexString64( GetMapID() ) << "]\n" );
      }
    }
    else if( pcProperty->IsPrivate() && IsPlayer() )
    {
      if( pcProperty->IsPrivate() )
      {
        RUM_COUT_IFDEF( PROPERTY_DEBUG,
                        "Broadcasting private property removal: " <<
                        pcProperty->GetName() << " (" << rumStringUtils::ToHexString( i_ePropertyID ) <<
                        ") player [" << rumStringUtils::ToHexString64( GetPlayerID() ) << ']\n' );

        auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
        rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_PROPERTY_REMOVED )
          .Write( (rumQWord)GetGameID() )
          .Write( (rumDWord)i_ePropertyID )
          .Send( GetSocket() );
      }
    }
  }
  
  if( pcProperty->IsPersistent() && IsPlayer() )
  {
    const rumPlayer* pcPlayer{ rumPlayer::FetchByPlayerID( GetPlayerID() ) };
    if( pcPlayer && pcPlayer->IsPersistenceEnabled() )
    {
      RUM_COUT_IFDEF( PROPERTY_DEBUG,
                      "Persisting property removal: " << pcProperty->GetName() << " (" <<
                      rumStringUtils::ToHexString( i_ePropertyID ) << ") player [" <<
                      rumStringUtils::ToHexString64( GetPlayerID() ) << "]\n" );

      std::string strQuery{ "DELETE FROM player_properties WHERE property_asset_fk=" };
      strQuery += rumStringUtils::ToHexString( i_ePropertyID );
      strQuery += " AND player_id_fk=";
      strQuery += rumStringUtils::ToHexString64( GetPlayerID() );

      rumAsyncDatabase::QueryAsync( rumDatabase::DatabaseID::Player_DatabaseID, strQuery );
    }
  }

  super::OnPropertyRemoved( i_ePropertyID );
}


// override
void rumServerPawn::OnPropertyUpdated( rumAssetID i_ePropertyID, Sqrat::Object i_sqValue, bool i_bAdded )
{
  const rumPropertyAsset* pcProperty{ rumPropertyAsset::Fetch( i_ePropertyID ) };
  if( !pcProperty )
  {
    return;
  }

  //RUM_COUT( "***** Sending Pawn property " << rumStringUtils::ToHexString64( i_ePropertyID ) << " *****\n" );

  if( pcProperty->GetServiceType() == Shared_ServiceType )
  {
    if( pcProperty->IsGlobal() )
    {
      if( rumServerPlayer::GetNumPlayers() > 0 )
      {
        auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
        rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_PROPERTY_UPDATE )
          .Write( GetGameID() )
          .Write( 1 )
          .Write( i_ePropertyID )
          .Write( i_sqValue );
        rumServerPlayer::SendGlobalPacket( rcPacket );

        RUM_COUT_IFDEF( PROPERTY_DEBUG,
                        "Pawn [" << rumStringUtils::ToHexString64( GetGameID() ) << "] broadcasting global property (" <<
                        rumStringUtils::ToHexString( i_ePropertyID ) << " to all players\n" );
      }
    }
    else if( pcProperty->IsRegional() )
    {
      const rumMap* pcMap{ GetMap() };
      if( pcMap && pcMap->GetNumPlayers() > 0 )
      {
        auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
        rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_PROPERTY_UPDATE )
          .Write( GetGameID() )
          .Write( 1 )
          .Write( i_ePropertyID )
          .Write( i_sqValue );
        rumServerPlayer::SendPacket( rcPacket, pcMap->GetPlayers() );

        RUM_COUT_IFDEF( PROPERTY_DEBUG,
                        "Pawn [" << rumStringUtils::ToHexString64( GetGameID() ) <<
                        "] broadcasting regional property (" << rumStringUtils::ToHexString( i_ePropertyID ) <<
                        " to all players on map " << pcMap->GetName() << " [" <<
                        rumStringUtils::ToHexString64( GetMapID() ) << "]\n" );
      }
    }
    else if( pcProperty->IsPrivate() && IsPlayer() )
    {
      RUM_COUT_IFDEF( PROPERTY_DEBUG,
                      "Broadcasting private property " << i_ePropertyID << " for player [" <<
                      rumStringUtils::ToHexString64( GetPlayerID() ) << "]\n" );

      auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
      rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_PROPERTY_UPDATE )
        .Write( GetGameID() )
        .Write( 1 )
        .Write( i_ePropertyID )
        .Write( i_sqValue )
        .Send( GetSocket() );
    }
  }

  if( pcProperty->IsPersistent() && IsPlayer() )
  {
    const rumPlayer* pcPlayer{ rumPlayer::FetchByPlayerID( GetPlayerID() ) };
    if( pcPlayer && pcPlayer->IsPersistenceEnabled() )
    {
      RUM_COUT_IFDEF( PROPERTY_DEBUG,
                      "Persisting property " << i_ePropertyID << " for player [" <<
                      rumStringUtils::ToHexString64( GetPlayerID() ) << "]\n" );

      std::string strQuery;
      const std::string strValue{ rumDatabaseQuery::FormatProperty( pcProperty, i_sqValue ) };

      if( i_bAdded )
      {
        strQuery = "INSERT INTO player_properties (player_id_fk,property_asset_fk,value) VALUES (";
        strQuery += rumStringUtils::ToHexString64( GetPlayerID() );
        strQuery += ',';
        strQuery += rumStringUtils::ToHexString( i_ePropertyID );
        strQuery += ',';
        strQuery += strValue;
        strQuery += ')';
      }
      else
      {
        strQuery = "UPDATE player_properties SET value=";
        strQuery += strValue;
        strQuery += " WHERE player_id_fk=";
        strQuery += rumStringUtils::ToHexString64( GetPlayerID() );
        strQuery += " AND property_asset_fk=";
        strQuery += rumStringUtils::ToHexString( i_ePropertyID );
      }

      rumAsyncDatabase::QueryAsync( rumDatabase::DatabaseID::Player_DatabaseID, strQuery );
    }
  }

  super::OnPropertyUpdated( i_ePropertyID, i_sqValue, i_bAdded );
}


// static
bool rumServerPawn::PackageAttributes( const rumPawn* i_pcPawn, rumNetwork::rumOutboundPacket& o_rcPacket )
{
  rumAssert( i_pcPawn );
  if( !i_pcPawn || i_pcPawn->IsServerOnly() )
  {
    return false;
  }

  o_rcPacket.Write( (rumQWord)i_pcPawn->GetGameID() )
    .Write( (rumByte)i_pcPawn->GetPawnType() )
    .Write( (rumDWord)i_pcPawn->GetAssetID() )
    .Write( (rumDWord)i_pcPawn->GetPosX() )
    .Write( (rumDWord)i_pcPawn->GetPosY() )
    .Write( (rumByte)( i_pcPawn->IsVisible() ? 1 : 0 ) )
    .Write( (rumDWord)i_pcPawn->GetCollisionFlags() )
    .Write( (rumDWord)i_pcPawn->GetMoveType() )
    .Write( (rumDWord)i_pcPawn->GetLightRange() )
    .Write( (rumDWord)i_pcPawn->GetState() );

  return true;
}


// static
bool rumServerPawn::PackageGlobalProperties( const rumPawn* i_pcPawn, rumNetwork::rumOutboundPacket& o_rcPacket )
{
  rumAssert( i_pcPawn );
  if( !i_pcPawn || i_pcPawn->IsServerOnly() )
  {
    return false;
  }

  std::priority_queue< PacketInfo, std::deque<PacketInfo>, std::less<PacketInfo>> pqPacketInfos;

#pragma message("TODO - combine iteration of both instance and asset properties")
  //auto final_rng{ std::ranges::views::concat( rng1, rng2, rng3 ) };

  // When a player is added to the game, it must receive current global property values from all players
  for( const auto& iter : i_pcPawn->GetInstanceProperties() )
  {
    const rumPropertyAsset* pcProperty{ rumPropertyAsset::Fetch( iter.first ) };
    if( pcProperty && pcProperty->IsGlobal() )
    {
      RUM_COUT_IFDEF( PROPERTY_DEBUG,
                      "Pawn [" << rumStringUtils::ToHexString64( i_pcPawn->GetGameID() ) <<
                      "] broadcasting global property (" << rumStringUtils::ToHexString( iter.first ) << ")\n" );

      PacketInfo cPacketInfo;
      cPacketInfo.m_ePropertyID = iter.first;
      cPacketInfo.m_sqObject = iter.second;
      cPacketInfo.m_iPriority = pcProperty->GetPriority();

      pqPacketInfos.push( cPacketInfo );
    }
  }

  if( i_pcPawn->GetAsset() )
  {
    for( const auto& iter : i_pcPawn->GetAsset()->GetProperties() )
    {
      const rumPropertyAsset* pcProperty{ rumPropertyAsset::Fetch( iter.first ) };
      if( pcProperty && pcProperty->IsGlobal() )
      {
        RUM_COUT_IFDEF( PROPERTY_DEBUG,
                        "Pawn [" << rumStringUtils::ToHexString64( i_pcPawn->GetGameID() ) <<
                        "] broadcasting global property (" << rumStringUtils::ToHexString( iter.first ) << ")\n" );

        PacketInfo cPacketInfo;
        cPacketInfo.m_ePropertyID = iter.first;
        cPacketInfo.m_sqObject = iter.second;
        cPacketInfo.m_iPriority = pcProperty->GetPriority();

        pqPacketInfos.push( cPacketInfo );
      }
    }
  }

  if( !pqPacketInfos.empty() )
  {
    o_rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_PROPERTY_UPDATE );
    o_rcPacket.Write( rumQWord( i_pcPawn->GetGameID() ) )
      .Write( rumDWord( pqPacketInfos.size() ) );

    do
    {
      const PacketInfo& cPacketInfo{ pqPacketInfos.top() };
      o_rcPacket.Write( cPacketInfo.m_ePropertyID )
        .Write( cPacketInfo.m_sqObject );

      pqPacketInfos.pop();
    } while( !pqPacketInfos.empty() );

    return true;
  }

  return false;
}


// static
bool rumServerPawn::PackageRegionalProperties( const rumPawn* i_pcPawn, rumNetwork::rumOutboundPacket& o_rcPacket )
{
  rumAssert( i_pcPawn );
  if( !i_pcPawn || i_pcPawn->IsServerOnly() )
  {
    return false;
  }

  std::priority_queue< PacketInfo, std::deque<PacketInfo>, std::less<PacketInfo>> pqPacketInfos;

#pragma message("TODO - combine iteration of both instance and asset properties")
  //auto final_rng{ std::ranges::views::concat( rng1, rng2, rng3 ) };

  // When a pawn is added to a map, it must receive current regional property values from all pawns
  for( const auto& iter : i_pcPawn->GetInstanceProperties() )
  {
    const rumPropertyAsset* pcProperty{ rumPropertyAsset::Fetch( iter.first ) };
    if( pcProperty && pcProperty->IsRegional() )
    {
      RUM_COUT_IFDEF( PROPERTY_DEBUG,
                      "Pawn [" << rumStringUtils::ToHexString64( i_pcPawn->GetGameID() ) <<
                      "] broadcasting regional property (" << rumStringUtils::ToHexString( iter.first ) << ")\n" );

      PacketInfo cPacketInfo;
      cPacketInfo.m_ePropertyID = iter.first;
      cPacketInfo.m_sqObject = iter.second;
      cPacketInfo.m_iPriority = pcProperty->GetPriority();

      pqPacketInfos.push( cPacketInfo );
    }
  }

  if( i_pcPawn->GetAsset() )
  {
    for( const auto& iter : i_pcPawn->GetAsset()->GetProperties() )
    {
      const rumPropertyAsset* pcProperty{ rumPropertyAsset::Fetch( iter.first ) };
      if( pcProperty && pcProperty->IsRegional() )
      {
        RUM_COUT_IFDEF( PROPERTY_DEBUG,
                        "Pawn [" << rumStringUtils::ToHexString64( i_pcPawn->GetGameID() ) <<
                        "] broadcasting regional property (" << rumStringUtils::ToHexString( iter.first ) << ")\n" );

        PacketInfo cPacketInfo;
        cPacketInfo.m_ePropertyID = iter.first;
        cPacketInfo.m_sqObject = iter.second;
        cPacketInfo.m_iPriority = pcProperty->GetPriority();

        pqPacketInfos.push( cPacketInfo );
      }
    }
  }

  if( !pqPacketInfos.empty() )
  {
    o_rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_PROPERTY_UPDATE );
    o_rcPacket.Write( i_pcPawn->GetGameID() )
      .Write( rumDWord( pqPacketInfos.size() ) );

    do
    {
      const PacketInfo& cPacketInfo{ pqPacketInfos.top() };
      o_rcPacket.Write( cPacketInfo.m_ePropertyID )
        .Write( cPacketInfo.m_sqObject );
      pqPacketInfos.pop();
    } while( !pqPacketInfos.empty() );

    return true;
  }

  return false;
}


bool rumServerPawn::PopPacket()
{
  if( IsPlayer() )
  {
    rumPlayer* pcPlayer{ rumPlayer::FetchByPlayerID( GetPlayerID() ) };
    if( pcPlayer )
    {
      return pcPlayer->PopPacket();
    }
  }

  return false;
}


rumPosition rumServerPawn::PopPathPosition()
{
  const rumPosition cPos{ GetPathPosition() };

  if( m_pcAstarPath )
  {
    m_pcAstarPath->PathNextNode();
    if( !m_pcAstarPath->HasPath() )
    {
      // The goal has been reached, so no need to keep the path around
      ForgetPath();
    }
  }

  return cPos;
}


void rumServerPawn::ScriptBind()
{
  super::ScriptBind();

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::DerivedClass<rumServerPawn, rumPawn, Sqrat::NoConstructor<rumServerPawn>> cServerPawn( pcVM, "rumServerPawn" );
  cServerPawn
    .Func( "ForgetPath", &ForgetPath )
    .Func( "GetPathPosition", &GetPathPosition )
    .Func( "GetPathTargetPosition", &GetPathTargetPosition )
    .Func( "GetSocket", &GetSocket )
    .Func( "HasPath", &HasPath )
    .Func( "PopPacket", &PopPacket )
    .Func( "PopPathPosition", &PopPathPosition )
    .Func( "SendBroadcast", &ScriptSendPacket )
    .Overload<bool( rumServerPawn::* )( const rumPosition&, uint32_t, rumDirectionType )>( "FindPath", &FindPath )
    .Overload<bool( rumServerPawn::* )( const rumPosition&, uint32_t )>( "FindPath", &FindPath );
  Sqrat::RootTable( pcVM ).Bind( "rumPawn", cServerPawn );

  Sqrat::DerivedClass<rumServerCreature, rumServerPawn> cServerCreature( pcVM, "rumServerCreature" );
  Sqrat::RootTable( pcVM ).Bind( SCRIPT_CREATURE_NATIVE_CLASS, cServerCreature );

  Sqrat::DerivedClass<rumServerPortal, rumServerPawn> cServerPortal( pcVM, "rumServerPortal" );
  Sqrat::RootTable( pcVM ).Bind( SCRIPT_PORTAL_NATIVE_CLASS, cServerPortal );

  Sqrat::DerivedClass<rumServerWidget, rumServerPawn> cServerWidget( pcVM, "rumServerWidget" );
  Sqrat::RootTable( pcVM ).Bind( SCRIPT_WIDGET_NATIVE_CLASS, cServerWidget );
}


int32_t rumServerPawn::ScriptSendPacket( Sqrat::Object i_sqInstance )
{
  if( i_sqInstance.GetType() != OT_INSTANCE )
  {
    return RESULT_FAILED;
  }

  if( IsPlayer() )
  {
    const rumPlayer* pcPlayer{ rumPlayer::FetchByPlayerID( GetPlayerID() ) };
    if( pcPlayer )
    {
      rumServerPlayer::ScriptBroadcastPrivate( pcPlayer->GetSocket(), i_sqInstance );
    }
  }

  return RESULT_FAILED;
}


// final
int32_t rumServerPawn::Serialize( rumResource& io_rcResource )
{
  // The server should never save pawns
  rumAssert( io_rcResource.IsLoading() );
  return io_rcResource.IsLoading() ? super::Serialize( io_rcResource ) : RESULT_FAILED;
}


void rumServerPawn::SetCollisionFlags( uint32_t i_uiCollisionFlags )
{
  if( GetCollisionFlags() != i_uiCollisionFlags )
  {
    super::SetCollisionFlags( i_uiCollisionFlags );

    const rumMap* pcMap{ GetMap() };
    if( pcMap && pcMap->GetNumPlayers() > 0 )
    {
      auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
      rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_PAWN_COLLISIONFLAGS_UPDATE )
        << (rumQWord)GetGameID()
        << (rumDWord)( GetCollisionFlags() );
      rumServerPlayer::SendPacket( rcPacket, pcMap->GetPlayers() );
    }
  }
}


// override
void rumServerPawn::SetLightRange( uint32_t i_uiRange )
{
  if( GetLightRange() != i_uiRange )
  {
    super::SetLightRange( i_uiRange );

    const rumMap* pcMap{ GetMap() };
    if( pcMap && pcMap->GetNumPlayers() > 0 )
    {
      auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
      rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_PAWN_LIGHT_UPDATE )
        << (rumQWord)GetGameID()
        << (rumDWord)i_uiRange;
      rumServerPlayer::SendPacket( rcPacket, pcMap->GetPlayers() );
    }
  }
}


void rumServerPawn::SetMapID( rumUniqueID i_eMapID, const rumPosition& i_rcStartingPos )
{
  super::SetMapID( i_eMapID, i_rcStartingPos );

  // Perform database and client syncronization
  if( IsPlayer() )
  {
    rumPlayer* pcPlayer{ rumPlayer::FetchByPlayerID( GetPlayerID() ) };
    if( pcPlayer )
    {
      pcPlayer->OnMapUpdated( i_eMapID, i_rcStartingPos );
    }
  }
}


// override
void rumServerPawn::SetMoveType( uint32_t i_uiMoveType )
{
  if( GetMoveType() != i_uiMoveType )
  {
    super::SetMoveType( i_uiMoveType );

    const rumMap* pcMap{ GetMap() };
    if( pcMap && pcMap->GetNumPlayers() > 0 )
    {
      auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
      rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_PAWN_MOVETYPE_UPDATE )
        << (rumQWord)GetGameID()
        << (rumDWord)GetMoveType();
      rumServerPlayer::SendPacket( rcPacket, pcMap->GetPlayers() );
    }
  }
}


// override
int32_t rumServerPawn::SetPos( const rumPosition& i_rcPos )
{
  const rumPosition cOldPos( GetPos() );
  if( cOldPos == i_rcPos )
  {
    return RESULT_SUCCESS;
  }

  const rumMap* pcMap{ GetMap() };
  if( pcMap && pcMap->GetNumPlayers() > 0 )
  {
    auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
    rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_PAWN_POSITION_UPDATE )
      << (rumQWord)GetGameID()
      << (rumDWord)i_rcPos.m_iX
      << (rumDWord)i_rcPos.m_iY;
    rumServerPlayer::SendPacket( rcPacket, pcMap->GetPlayers() );
  }

  const int32_t eResult{ super::SetPos( i_rcPos ) };

  if( IsPlayer() )
  {
    rumPlayer* pcPlayer{ rumPlayer::FetchByPlayerID( GetPlayerID() ) };
    if( pcPlayer )
    {
      const auto uiDistance{ pcMap->GetTileDistance( i_rcPos, cOldPos ) };
      pcPlayer->OnPositionUpdated( i_rcPos, uiDistance );
    }
  }

  return eResult;
}


void rumServerPawn::SetState( int32_t i_iState )
{
  if( GetState() != i_iState )
  {
    super::SetState( i_iState );

    const rumMap* pcMap{ GetMap() };
    if( pcMap && pcMap->GetNumPlayers() > 0 )
    {
      auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
      rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_PAWN_STATE_UPDATE )
        << (rumQWord)GetGameID()
        << (rumDWord)( GetState() );
      rumServerPlayer::SendPacket( rcPacket, pcMap->GetPlayers() );
    }
  }
}


void rumServerPawn::SetVisibility( bool i_bVisible )
{
  if( IsVisible() != i_bVisible )
  {
    super::SetVisibility( i_bVisible );

    const rumMap* pcMap{ GetMap() };
    if( pcMap && pcMap->GetNumPlayers() > 0 )
    {
      auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
      rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_PAWN_VISIBILITY_UPDATE )
        << (rumQWord)GetGameID()
        << (rumByte)( IsVisible() ? 1 : 0 );
      rumServerPlayer::SendPacket( rcPacket, pcMap->GetPlayers() );
    }
  }
}


// override
void rumServerCreature::AllocateGameID( rumUniqueID i_uiGameID )
{
  if( INVALID_GAME_ID == i_uiGameID )
  {
    static rumUniqueID s_uiAssetType{ ( rumUniqueID( rumCreatureAsset::GetClassRegistryID() ) ) << 60 };
    static rumUniqueID s_uiGameID{ ( rumUniqueID( Server_ObjectCreationType ) ) << 56 };

    i_uiGameID = ++s_uiGameID | s_uiAssetType;
  }

  SetGameID( i_uiGameID );
}


// override
void rumServerPortal::AllocateGameID( rumUniqueID i_uiGameID )
{
  if( INVALID_GAME_ID == i_uiGameID )
  {
    static rumUniqueID s_uiAssetType{ ( rumUniqueID( rumPortalAsset::GetClassRegistryID() ) ) << 60 };
    static rumUniqueID s_uiGameID{ ( rumUniqueID( Server_ObjectCreationType ) ) << 56 };

    i_uiGameID = ++s_uiGameID | s_uiAssetType;
  }

  SetGameID( i_uiGameID );
}


// override
void rumServerWidget::AllocateGameID( rumUniqueID i_uiGameID )
{
  if( INVALID_GAME_ID == i_uiGameID )
  {
    static rumUniqueID s_uiAssetType{ ( rumUniqueID( rumWidgetAsset::GetClassRegistryID() ) ) << 60 };
    static rumUniqueID s_uiGameID{ ( rumUniqueID( Server_ObjectCreationType ) ) << 56 };

    i_uiGameID = ++s_uiGameID | s_uiAssetType;
  }

  SetGameID( i_uiGameID );
}
