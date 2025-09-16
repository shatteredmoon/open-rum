#include <s_inventory.h>

#include <s_map.h>
#include <s_pawn.h>
#include <s_player.h>

#include <network/u_packet.h>
#include <u_assert.h>
#include <u_db.h>
#include <u_inventory_asset.h>
#include <u_property_asset.h>


// override
void rumServerInventory::AllocateGameID( rumUniqueID i_uiGameID )
{
  // Newly created server-side inventory should not already have a UID
  rumAssertMsg( INVALID_GAME_ID == i_uiGameID, "Creating server-side inventory with a non-zero GameID" );

  static rumUniqueID s_uiAssetType{ ( rumUniqueID( rumInventoryAsset::GetClassRegistryID() ) ) << 60 };
  static rumUniqueID s_uiGameID{ ( rumUniqueID( Server_ObjectCreationType ) ) << 56 };

  i_uiGameID = ++s_uiGameID | s_uiAssetType;
  SetGameID( i_uiGameID );
}


bool rumServerInventory::Commit( rumUniqueID i_uiPlayerID )
{
  rumAssert( i_uiPlayerID != INVALID_GAME_ID );
  if( INVALID_GAME_ID == i_uiPlayerID )
  {
    return false;
  }

  // Attempt database insert only if the database key is empty
  if( INVALID_GAME_ID == m_uiPersistentID )
  {
    // Get the next available database key
    SetPersistentID( rumDatabase::GetNextIDFromIDStore( rumDatabase::IDStoreTableType::Inventory_IDStoreTableType ) );

    std::string strQuery{ "INSERT INTO inventory (inventory_id,inventory_asset_fk,player_id_fk) VALUES (" };
    strQuery += rumStringUtils::ToHexString64( m_uiPersistentID );
    strQuery += ",";
    strQuery += rumStringUtils::ToHexString( GetAssetID() );
    strQuery += ",";
    strQuery += rumStringUtils::ToHexString64( i_uiPlayerID );
    strQuery += ");";

    rumAsyncDatabase::QueryAsync( rumDatabase::DatabaseID::Player_DatabaseID, strQuery );

    for( const auto& iter : GetInstanceProperties() )
    {
      const rumAssetID ePropertyID{ iter.first };
      Sqrat::Object sqValue{ iter.second };

      CommitProperty( i_uiPlayerID, ePropertyID, sqValue, true /* property added */);
    }

    return true;
  }
  /*else
  {
    // New owner
    std::string strQuery{ "UPDATE inventory SET player_id_fk=" };
    strQuery += ToHexString64( i_uiPlayerID );
    strQuery += " WHERE inventory_id=";
    strQuery += ToHexString64( i_pcInventory->GetGameID() );

    rumAsyncDatabase::QueryAsync( rumDatabase::Player_DatabaseID, strQuery );
  }*/

  return false;
}


bool rumServerInventory::CommitProperty( rumUniqueID i_uiPlayerID, rumAssetID i_ePropertyID, Sqrat::Object i_sqValue,
                                         bool i_bAdded )
{
  if( INVALID_GAME_ID == m_uiPersistentID )
  {
    return false;
  }

  const rumPropertyAsset* pcProperty{ rumPropertyAsset::Fetch( i_ePropertyID ) };
  if( !pcProperty || !pcProperty->IsPersistent() )
  {
    return false;
  }

  RUM_COUT_IFDEF( PROPERTY_DEBUG,
                  "Persisting inventory property " << rumStringUtils::ToHexString( i_ePropertyID ) <<
                  " for player [" << rumStringUtils::ToHexString64( i_uiPlayerID ) << "]\n" );

  std::string strQuery;
  const std::string strValue{ rumDatabaseQuery::FormatProperty( pcProperty, i_sqValue ) };

  if( i_bAdded )
  {
    strQuery = "INSERT INTO inventory_properties (inventory_id_fk,property_asset_fk,value) VALUES (";
    strQuery += rumStringUtils::ToHexString64( m_uiPersistentID );
    strQuery += ',';
    strQuery += rumStringUtils::ToHexString( i_ePropertyID );
    strQuery += ',';
    strQuery += strValue;
    strQuery += ')';
  }
  else
  {
    strQuery = "UPDATE inventory_properties SET value=";
    strQuery += strValue;
    strQuery += " WHERE inventory_id_fk=";
    strQuery += rumStringUtils::ToHexString64( m_uiPersistentID );
    strQuery += " AND property_asset_fk=";
    strQuery += rumStringUtils::ToHexString( i_ePropertyID );
  }

  rumAsyncDatabase::QueryAsync( rumDatabase::DatabaseID::Player_DatabaseID, strQuery );

  return true;
}


// override
int32_t rumServerInventory::InitFromDB( QueryPtr i_pcQuery, uint32_t i_uiRow )
{
  rumAssert( i_pcQuery );
  if( !i_pcQuery )
  {
    return RESULT_FAILED;
  }

  enum { DB_ID, DB_INVENTORY_FK };

  const rumUniqueID iPersistentID{ (rumUniqueID)i_pcQuery->FetchInt64( i_uiRow, DB_ID ) };
  SetPersistentID( iPersistentID );

  rumAssertMsg( m_uiPersistentID != INVALID_GAME_ID, "Persistent ID is invalid" );
  if( INVALID_GAME_ID == m_uiPersistentID )
  {
    return RESULT_FAILED;
  }

  std::string strQuery{ "SELECT property_asset_fk,value FROM inventory_properties WHERE inventory_id_fk=" };
  strQuery += rumStringUtils::ToHexString64( m_uiPersistentID );

  const QueryPtr pcQuery{ rumDatabase::Query( rumDatabase::DatabaseID::Player_DatabaseID, strQuery ) };
  if( !pcQuery || pcQuery->IsError() )
  {
    return RESULT_FAILED;
  }

  InitPropertiesFromDB( pcQuery );

  return super::InitFromDB( i_pcQuery, i_uiRow );
}


bool rumServerInventory::InitPropertiesFromDB( QueryPtr i_pcQuery )
{
  enum { DB_PROPERTY_ID, DB_PROPERTY_VALUE };

  // Since this is potentially a large fetch, reserve buckets if needed
  AllocatePropertyBuckets( i_pcQuery->GetNumRows() );

  for( int32_t i = 0; i < i_pcQuery->GetNumRows(); ++i )
  {
    const rumAssetID ePropertyID{ (rumAssetID)i_pcQuery->FetchInt( i, DB_PROPERTY_ID ) };
    const rumPropertyAsset* pcProperty{ rumPropertyAsset::Fetch( ePropertyID ) };
    rumAssert( pcProperty );
    if( !pcProperty )
    {
      std::string strWarning{ "Failed to fetch property id (" };
      strWarning += rumStringUtils::ToHexString( ePropertyID );
      strWarning += ") for inventory [";
      strWarning += rumStringUtils::ToHexString64( GetGameID() );
      strWarning += "]";
      Logger::LogStandard( strWarning, Logger::LOG_WARNING );

      continue;
    }

    RUM_COUT_IFDEF( PROPERTY_DEBUG,
                    "Loaded property " << pcProperty->GetName() <<
                    " [" << rumStringUtils::ToHexString( ePropertyID ) << "] type " <<
                    rumStringUtils::ToHexString( rumUtility::ToUnderlyingType( pcProperty->GetValueType() ) ) <<
                    '\n' );

    Sqrat::Object sqValue{ i_pcQuery->GetValueFromProperty( i, DB_PROPERTY_VALUE, pcProperty ) };
    SetProperty( ePropertyID, sqValue );
  }

  return RESULT_SUCCESS;
}


// override
bool rumServerInventory::OnAddedToPawn( rumPawn* i_pcPawn )
{
  rumAssert( i_pcPawn );
  if( !i_pcPawn )
  {
    return false;
  }

  if( !super::OnAddedToPawn( i_pcPawn ) )
  {
    return false;
  }

  const rumInventoryAsset* pcAsset{ rumInventoryAsset::Fetch( GetAssetID() ) };
  if( !pcAsset )
  {
    return false;
  }

  if( pcAsset->IsGlobal() )
  {
    if( rumServerPlayer::GetNumPlayers() > 0 )
    {
      // Send to all players
      auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
      rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_INVENTORY_UPDATE )
        .Write( i_pcPawn->GetPlayerID() )
        .Write( GetGameID() )
        .Write( GetAssetID() )
        .Write( true );
      rumServerPlayer::SendGlobalPacket( rcPacket );
    }
  }
  else if( pcAsset->IsRegional() )
  {
    // Send to any players on the current map only
    const rumMap* pcMap{ i_pcPawn->GetMap() };
    if( pcMap && pcMap->GetNumPlayers() > 0 )
    {
      auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
      rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_INVENTORY_UPDATE )
        .Write( i_pcPawn->GetPlayerID() )
        .Write( GetGameID() )
        .Write( GetAssetID() )
        .Write( true );
      rumServerPlayer::SendPacket( rcPacket, pcMap->GetPlayers() );
    }
  }
  else if( pcAsset->IsPrivate() && i_pcPawn->IsPlayer() )
  {
    // Send to the owner
    auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
    rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_INVENTORY_UPDATE )
      .Write( i_pcPawn->GetPlayerID() )
      .Write( GetGameID() )
      .Write( GetAssetID() )
      .Write( true );
    rumServerPlayer::SendPacket( rcPacket, i_pcPawn->GetPlayerID() );
  }

  if( pcAsset->IsPersistent() && i_pcPawn->IsPlayer() )
  {
    const rumPlayer* pcPlayer{ rumPlayer::FetchByPlayerID( i_pcPawn->GetPlayerID() ) };
    if( pcPlayer && pcPlayer->IsPersistenceEnabled() )
    {
      // Store to db
      Commit( i_pcPawn->GetPlayerID() );
    }
  }

  return true;
}


// override
bool rumServerInventory::OnRemovedFromPawn( rumPawn* i_pcPawn )
{
  rumAssert( i_pcPawn );
  if( !i_pcPawn )
  {
    return false;
  }

  if( !super::OnRemovedFromPawn( i_pcPawn ) )
  {
    return false;
  }

  const rumInventoryAsset* pcAsset{ rumInventoryAsset::Fetch( GetAssetID() ) };
  if( !pcAsset )
  {
    return false;
  }

  if( pcAsset->IsGlobal() )
  {
    if( rumServerPlayer::GetNumPlayers() > 0 )
    {
      // Send to all players
      auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
      rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_INVENTORY_UPDATE )
        .Write( i_pcPawn->GetPlayerID() )
        .Write( GetGameID() )
        .Write( GetAssetID() )
        .Write( false );
      rumServerPlayer::SendGlobalPacket( rcPacket );
    }
  }
  else if( pcAsset->IsRegional() )
  {
    // Send to any players on the current map only
    const rumMap* pcMap{ i_pcPawn->GetMap() };
    if( pcMap && pcMap->GetNumPlayers() > 0 )
    {
      auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
      rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_INVENTORY_UPDATE )
        .Write( i_pcPawn->GetPlayerID() )
        .Write( GetGameID() )
        .Write( GetAssetID() )
        .Write( false );
      rumServerPlayer::SendPacket( rcPacket, pcMap->GetPlayers() );
    }
  }
  else if( pcAsset->IsPrivate() && i_pcPawn->IsPlayer() )
  {
    // Send to the owner
    auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
    rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_INVENTORY_UPDATE )
      .Write( i_pcPawn->GetPlayerID() )
      .Write( GetGameID() )
      .Write( GetAssetID() )
      .Write( false );
    rumServerPlayer::SendPacket( rcPacket, i_pcPawn->GetPlayerID() );
  }

  if( pcAsset->IsPersistent() && i_pcPawn->IsPlayer() )
  {
    const rumPlayer* pcPlayer{ rumPlayer::FetchByPlayerID( i_pcPawn->GetPlayerID() ) };
    if( pcPlayer && pcPlayer->IsPersistenceEnabled() )
    {
      if( m_uiPersistentID != INVALID_GAME_ID )
      {
        // Delete this item and all of its properties
        std::string strQuery{ "DELETE FROM inventory_properties WHERE inventory_id_fk=" };
        strQuery += rumStringUtils::ToHexString64( m_uiPersistentID );
        strQuery += ";DELETE FROM inventory WHERE inventory_id=";
        strQuery += rumStringUtils::ToHexString64( m_uiPersistentID );
        strQuery += " AND player_id_fk=";
        strQuery += rumStringUtils::ToHexString64( i_pcPawn->GetPlayerID() );

        rumAsyncDatabase::QueryAsync( rumDatabase::DatabaseID::Player_DatabaseID, strQuery );

        m_uiPersistentID = INVALID_GAME_ID;
      }
      else
      {
        std::string strError{ "Attempt to delete a non-existing inventory item from the database" };
        Logger::LogStandard( strError, Logger::LOG_ERROR );
      }
    }
  }

  return true;
}


// override
void rumServerInventory::OnPropertyRemoved( rumAssetID i_ePropertyID )
{
  const rumPropertyAsset* pcProperty{ rumPropertyAsset::Fetch( i_ePropertyID ) };
  if( !pcProperty )
  {
    return;
  }

  // Get the owning pawn
  const rumPawn* pcPawn{ rumPawn::Fetch( GetOwner() ) };
  if( !pcPawn )
  {
    // The object having this property set has no owner, and therefore persistence or replication is unnecessary
    return;
  }

  const bool bPlayer{ pcPawn->IsPlayer() };

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
      }
    }
    else if( pcProperty->IsRegional() )
    {
      const rumMap* pcMap{ pcPawn->GetMap() };
      if( pcMap && pcMap->GetNumPlayers() > 0 )
      {
        auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
        rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_PROPERTY_REMOVED )
          .Write( GetGameID() )
          .Write( i_ePropertyID );
        rumServerPlayer::SendPacket( rcPacket, pcMap->GetPlayers() );
      }
    }
    else if( bPlayer && pcProperty->IsPrivate() )
    {
      auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
      rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_PROPERTY_REMOVED )
        .Write( GetGameID() )
        .Write( i_ePropertyID );
      rumServerPlayer::SendPacket( rcPacket, pcPawn->GetPlayerID() );
    }
  }

  if( bPlayer && pcProperty->IsPersistent() )
  {
    const rumPlayer* pcPlayer{ rumPlayer::FetchByPlayerID( pcPawn->GetPlayerID() ) };
    if( pcPlayer && pcPlayer->IsPersistenceEnabled() )
    {
      RUM_COUT_IFDEF( PROPERTY_DEBUG,
                      "Persisting inventory property removal: " <<
                      pcProperty->GetName() << " (" << rumStringUtils::ToHexString( i_ePropertyID ) << ") player [" <<
                      rumStringUtils::ToHexString64( pcPawn->GetPlayerID() ) << "]\n" );

      std::string strQuery{ "DELETE FROM inventory_properties WHERE inventory_id_fk=" };
      strQuery += rumStringUtils::ToHexString64( m_uiPersistentID );
      strQuery += " AND property_asset_fk=";
      strQuery += rumStringUtils::ToHexString( i_ePropertyID );

      rumAsyncDatabase::QueryAsync( rumDatabase::DatabaseID::Player_DatabaseID, strQuery );
    }
  }

  super::OnPropertyRemoved( i_ePropertyID );
}


// override
void rumServerInventory::OnPropertyUpdated( rumAssetID i_ePropertyID, Sqrat::Object i_sqValue, bool i_bAdded )
{
  const rumPropertyAsset* pcProperty{ rumPropertyAsset::Fetch( i_ePropertyID ) };
  if( !pcProperty )
  {
    return;
  }

  // Get the owning pawn
  const rumPawn* pcPawn{ rumPawn::Fetch( GetOwner() ) };
  if( !pcPawn )
  {
    // The object having this property set has no owner, and therefore
    // persistence or replication is unnecessary
    return;
  }

  const bool bPlayer{ pcPawn->IsPlayer() };

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
      }
    }
    else if( pcProperty->IsRegional() )
    {
      const rumMap* pcMap{ pcPawn->GetMap() };
      if( pcMap && pcMap->GetNumPlayers() > 0 )
      {
        auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
        rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_PROPERTY_UPDATE )
          .Write( GetGameID() )
          .Write( 1 )
          .Write( i_ePropertyID )
          .Write( i_sqValue );
        rumServerPlayer::SendPacket( rcPacket, pcMap->GetPlayers() );
      }
    }
    else if( bPlayer && pcProperty->IsPrivate() )
    {
      auto& rcPacket{ rumNetwork::rumOutboundPacketPool::CheckoutPacket() };
      rcPacket.SetHeader( rumNetwork::PACKET_HEADER_SERVER_PROPERTY_UPDATE )
        .Write( GetGameID() )
        .Write( 1 )
        .Write( i_ePropertyID )
        .Write( i_sqValue );
      rumServerPlayer::SendPacket( rcPacket, pcPawn->GetPlayerID() );
    }
  }

  if( bPlayer && pcProperty->IsPersistent() )
  {
    const rumPlayer* pcPlayer{ rumPlayer::FetchByPlayerID( pcPawn->GetPlayerID() ) };
    if( pcPlayer && pcPlayer->IsPersistenceEnabled() )
    {
      CommitProperty( pcPawn->GetPlayerID(), i_ePropertyID, i_sqValue, i_bAdded );
    }
  }

  super::OnPropertyUpdated( i_ePropertyID, i_sqValue, i_bAdded );
}


void rumServerInventory::ScriptBind()
{
  super::ScriptBind();

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::DerivedClass<rumServerInventory, rumInventory> cServerInventory( pcVM, "rumServerInventory" );
  cServerInventory.Func( "GetStorageID", &GetPersistentID );
  Sqrat::RootTable( pcVM ).Bind( "rumInventory", cServerInventory );
}
