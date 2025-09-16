#include <u_pawn.h>

#include <u_assert.h>
#include <u_db.h>
#include <u_enum.h>
#include <u_inventory.h>
#include <u_inventory_asset.h>
#include <u_inventory_iterator.h>
#include <u_log.h>
#include <u_map.h>
#include <u_pawn_asset.h>
#include <u_player.h>
#include <u_resource.h>

// Static Initializers
std::unordered_map< rumUniqueID, rumPawn* > rumPawn::s_hashPawns;


rumPawn::~rumPawn()
{
  FreeInternal();
}


// virtual
bool rumPawn::AddInventory( rumInventory* i_pcInventory )
{
  if( !i_pcInventory )
  {
    return false;
  }

  // Add the inventory if it doesn't already exist
  const auto& iter( m_hashInventory.find( i_pcInventory->GetGameID() ) );
  if( iter == m_hashInventory.end() )
  {
    // Add the item
    m_hashInventory.insert( i_pcInventory->GetGameID() );

    // Add ref
    ManageScriptObject( i_pcInventory->GetScriptInstance() );

    return i_pcInventory->OnAddedToPawn( this );
  }

  return false;
}


bool rumPawn::AddInventoryAsset( rumAssetID i_eInventoryID )
{
  if( RAW_ASSET_TYPE( i_eInventoryID ) == AssetType::Inventory_AssetType )
  {
    Sqrat::Object sqObject{ rumGameObject::Create( i_eInventoryID ) };
    if( sqObject.GetType() == OT_INSTANCE )
    {
      rumInventory* pcInventory{ sqObject.Cast<rumInventory*>() };
      return AddInventory( pcInventory );
    }
  }

  return false;
}


// virtual
bool rumPawn::DeleteInventory( rumInventory* i_pcInventory )
{
  if( !i_pcInventory )
  {
    return false;
  }

  // The object will be cleaned up by garbage collection if nothing else holds a reference to it
  Sqrat::Object sqInstance{ i_pcInventory->GetScriptInstance() };
  const auto numDeleted{ m_hashInventory.erase( i_pcInventory->GetGameID() ) };
  i_pcInventory->OnRemovedFromPawn( this );

  UnmanageScriptObject( sqInstance );

  return numDeleted > 0;
}


bool rumPawn::DeleteInventoryID( rumUniqueID i_uiInventoryID )
{
  // Delete the inventory item if it exists
  return DeleteInventory( rumInventory::Fetch( i_uiInventoryID ) );
}


// static
rumPawn* rumPawn::Fetch( rumUniqueID i_uiGameID )
{
  const auto& iter{ s_hashPawns.find( i_uiGameID ) };
  return iter != s_hashPawns.end() ? iter->second : nullptr;
}


rumInventory* rumPawn::FetchInventory( rumUniqueID i_uiInventoryID )
{
  const auto iter( m_hashInventory.find( i_uiInventoryID ) );
  return ( iter != m_hashInventory.end() ? rumInventory::Fetch( i_uiInventoryID ) : nullptr );
}


rumInventoryIterator rumPawn::FetchInventoryIter()
{
  return rumInventoryIterator( &m_hashInventory );
}


Sqrat::Object rumPawn::FetchInventoryVM( rumUniqueID i_uiInventoryID )
{
  rumInventory* pcInventory{ FetchInventory( i_uiInventoryID ) };
  return pcInventory ? pcInventory->GetScriptInstance() : Sqrat::Object();
}


// static
Sqrat::Object rumPawn::FetchVM( rumUniqueID i_uiGameID )
{
  rumPawn* pcPawn{ Fetch( i_uiGameID ) };
  return pcPawn ? pcPawn->GetScriptInstance() : Sqrat::Object();
}


// override
void rumPawn::Free()
{
  FreeInternal();
  super::Free();
}


void rumPawn::FreeInternal()
{
#pragma message("TODO: calling free on something that still exists on a map is catastrophic")
  // Remove all inventory
  for( const auto iter : m_hashInventory )
  {
    // Free the inventory object from the game
    const rumUniqueID uiInventoryID{ iter };
    rumGameObject* pcObject{ super::Fetch( uiInventoryID ) };
    if( pcObject )
    {
      pcObject->Free();
    }
  }

  m_hashInventory.clear();

  Unmanage();
}


rumPlayer* rumPawn::GetNearestPlayer()
{
  rumMap* pcMap{ GetMap() };
  return ( pcMap ? pcMap->GetNearestPlayer( m_cPos ) : nullptr );
}


rumPlayer* rumPawn::GetNearestPlayer( uint32_t i_uiMaxDistance )
{
  rumMap* pcMap{ GetMap() };
  return( pcMap ? pcMap->GetNearestPlayer( m_cPos, i_uiMaxDistance ) : nullptr );
}


rumPlayer* rumPawn::GetNearestPlayer( uint32_t i_uiMaxDistance, rumDirectionType i_eDir )
{
  rumMap* pcMap{ GetMap() };
  return ( pcMap ? pcMap->GetNearestPlayer( m_cPos, i_uiMaxDistance, i_eDir ) : nullptr );
}


rumPlayer* rumPawn::GetNearestPlayer( uint32_t i_uiMaxDistance, rumDirectionType i_eDir, bool i_bCheckLOS )
{
  rumMap* pcMap{ GetMap() };
  return ( pcMap ? pcMap->GetNearestPlayer( m_cPos, i_uiMaxDistance, i_eDir, i_bCheckLOS ) : nullptr );
}


Sqrat::Object rumPawn::GetMap_VM() const
{
  rumMap* pcMap{ GetMap() };
  if( pcMap )
  {
    return pcMap->GetScriptInstance();
  }

  return Sqrat::Object();
}


const std::string& rumPawn::GetPlayerName() const
{
  if( IsPlayer() )
  {
    const rumPlayer* pcPlayer{ rumPlayer::FetchByPlayerID( GetPlayerID() ) };
    return ( pcPlayer ? pcPlayer->GetName() : rumStringUtils::NullString() );
  }

  return rumStringUtils::NullString();
}


// virtual
int32_t rumPawn::InitFromMapDB( QueryPtr i_pcQuery, uint32_t i_uiRow )
{
  rumAssert( i_pcQuery );
  if( !i_pcQuery )
  {
    return RESULT_FAILED;
  }

  enum { DB_PAWN_ID, DB_PAWN_ASSET_ID, DB_POS_X, DB_POS_Y };

  m_cPos.m_iX = i_pcQuery->FetchInt( i_uiRow, DB_POS_X );
  m_cPos.m_iY = i_pcQuery->FetchInt( i_uiRow, DB_POS_Y );

  return RESULT_SUCCESS;
}


// override
void rumPawn::Manage()
{
  const rumUniqueID uiGameID{ GetGameID() };
  rumAssert( uiGameID != INVALID_GAME_ID );
  if( uiGameID != INVALID_GAME_ID )
  {
    s_hashPawns.insert( { uiGameID, this } );
  }
}


// override
void rumPawn::OnCreated()
{
  const rumPawnAsset* pcAsset{ rumPawnAsset::Fetch( GetAssetID() ) };
  if( pcAsset )
  {
    SetCollisionFlags( pcAsset->GetCollisionFlags() );
    SetMoveType( pcAsset->GetMoveType() );
    SetBlocksLOS( pcAsset->GetBlocksLOS() );
    SetLightRange( pcAsset->GetLightRange() );
  }

  super::OnCreated();
}


// static
void rumPawn::ScriptBind()
{
  rumInventoryIterator::ScriptBind();

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::ConstTable( pcVM )
    .Const( "rumCreaturePawnType", Creature_PawnType )
    .Const( "rumPortalPawnType", Portal_PawnType )
    .Const( "rumWidgetPawnType", Widget_PawnType );

  Sqrat::RootTable( pcVM )
    .Func( "rumFetchPawn", FetchVM );

  Sqrat::DerivedClass<rumPawn, rumGameObject, Sqrat::NoConstructor<rumPawn>> cPawn( pcVM, "rumPawn" );
  cPawn
    .Func( "SetBlocksLOS", &SetBlocksLOS )
    .Func( "GetBlocksLOS", &GetBlocksLOS )
    .Func( "SetCollisionFlags", &SetCollisionFlags )
    .Func( "GetCollisionFlags", &GetCollisionFlags )
    .Func( "SetMoveType", &SetMoveType )
    .Func( "GetMoveType", &GetMoveType )
    .Func( "GetPosition", &GetPos )
    .Func( "GetMap", &GetMap_VM )
    .Func( "GetMapID", &GetMapID )
    .Func( "IsVisible", &IsVisible )
    .Func( "GetState", &GetState )
    .Func( "SetState", &SetState )
    .Func( "SetVisibility", &SetVisibility )
    .Func( "GetNumInventoryItems", &GetNumInventoryItems )
    .Func( "GetLightRange", &GetLightRange )
    .Func( "SetLightRange", &SetLightRange )
    .Func( "GetPlayerID", &GetPlayerID )
    .Func( "GetPlayerName", &GetPlayerName )
    .Func( "AddItem", &AddInventory )
    .Func( "AddItemAsset", &AddInventoryAsset )
    .Func( "DeleteInventory", &DeleteInventory )
    .Func( "DeleteInventoryID", &DeleteInventoryID )
    .Func( "TransferInventory", &TransferInventory )
    .Func( "TransferInventoryID", &TransferInventoryID )
    .Overload<Sqrat::Object( rumPawn::* )( rumUniqueID )>( "GetInventory", &FetchInventoryVM )
    .Overload<rumInventoryIterator( rumPawn::* )( void )>( "GetInventory", &FetchInventoryIter )
    .Overload<rumPlayer* ( rumPawn::* )( void )>( "GetNearestPlayer", &GetNearestPlayer )
    .Overload<rumPlayer* ( rumPawn::* )( uint32_t )>( "GetNearestPlayer", &GetNearestPlayer )
    .Overload<rumPlayer* ( rumPawn::* )( uint32_t, rumDirectionType )>( "GetNearestPlayer", &GetNearestPlayer )
    .Overload<rumPlayer* ( rumPawn::* )( uint32_t, rumDirectionType, bool )>( "GetNearestPlayer", &GetNearestPlayer );
  Sqrat::RootTable( pcVM ).Bind( "rumPawnBase", cPawn );
}


// virtual
Sqrat::Object rumPawn::ScriptInstanceRelease()
{
  Sqrat::Object sqInstance{ GetScriptInstance() };
  if( sqInstance.GetType() == OT_INSTANCE )
  {
    rumScript::ExecOptionalFunc( sqInstance, "OnObjectReleased" );
  }

  return super::ScriptInstanceRelease();
}


int32_t rumPawn::Serialize( rumResource& io_rcResource )
{
  int32_t eResult{ RESULT_FAILED };

  io_rcResource << (rumDWord&)m_cPos.m_iX;
  io_rcResource << (rumDWord&)m_cPos.m_iY;

  return super::Serialize( io_rcResource );
}


void rumPawn::SetLightRange( uint32_t i_uiRange )
{
  if( m_uiLightRange != i_uiRange )
  {
    m_uiLightRange = i_uiRange;

    Sqrat::Object sqInstance{ GetScriptInstance() };
    if( sqInstance.GetType() == OT_INSTANCE )
    {
      rumScript::ExecOptionalFunc( sqInstance, "OnLightRangeUpdated", i_uiRange );
    }
  }
}


int32_t rumPawn::SetPos( const rumPosition& i_rcPos )
{
  if( m_cPos != i_rcPos )
  {
    const rumPosition i_cOldPos( m_cPos );
    m_cPos = i_rcPos;

    Sqrat::Object sqInstance{ GetScriptInstance() };
    if( sqInstance.GetType() == OT_INSTANCE )
    {
      rumScript::ExecOptionalFunc( sqInstance, "OnPositionUpdated", i_rcPos, i_cOldPos );
    }
  }

  return RESULT_SUCCESS;
}


void rumPawn::SetState( int32_t i_iState )
{
  if( m_iState != i_iState )
  {
    m_iState = i_iState;

    Sqrat::Object sqInstance{ GetScriptInstance() };
    if( sqInstance.GetType() == OT_INSTANCE )
    {
      rumScript::ExecOptionalFunc( sqInstance, "OnStateUpdated", m_iState );
    }
  }
}


// virtual
void rumPawn::SetVisibility( bool i_bVisible )
{
  if( IsVisible() != i_bVisible )
  {
    m_bVisible = i_bVisible;

    Sqrat::Object sqInstance{ GetScriptInstance() };
    if( sqInstance.GetType() == OT_INSTANCE )
    {
      rumScript::ExecOptionalFunc( sqInstance, "OnVisibilityUpdated", IsVisible() );
    }
  }
}


// static
void rumPawn::Shutdown()
{
  rumAssert( s_hashPawns.empty() );
  if( !s_hashPawns.empty() )
  {
    // Make a copy of the hash map for iteration and deletion. We can't delete directly from the original hash because
    // the iteration will be destroyed by calls to each object's Free method.
    PawnHash hashTemp{ s_hashPawns };
    s_hashPawns.clear();

    for( const auto& iter : hashTemp )
    {
      RUM_COUT_IFDEF( MEMORY_DEBUG, "Freeing Pawn: " << iter.second->GetName() << '\n' );

      rumPawn* pcObject{ iter.second };
      if( pcObject )
      {
        pcObject->Free();
      }
    }

    hashTemp.clear();
  }
}


bool rumPawn::TransferInventory( rumInventory* i_pcInventory, rumPawn* i_pcTargetPawn )
{
  if( !i_pcInventory || !i_pcTargetPawn )
  {
    return false;
  }

  // Temporarily hold a ref to this item
  Sqrat::Object sqInstance{ i_pcInventory->GetScriptInstance() };
  m_hashInventory.erase( i_pcInventory->GetGameID() );
  i_pcInventory->OnRemovedFromPawn( this );

  UnmanageScriptObject( sqInstance );

  return i_pcTargetPawn->AddInventory( i_pcInventory );
}


bool rumPawn::TransferInventoryID( rumUniqueID i_uiInventoryID, rumPawn* i_pcTargetPawn )
{
  return TransferInventory( rumInventory::Fetch( i_uiInventoryID ), i_pcTargetPawn );
}


// override
void rumPawn::Unmanage()
{
  s_hashPawns.erase( GetGameID() );
}


// static
void rumPawn::Update()
{
  for( const auto& iter : s_hashPawns )
  {
    rumPawn* pcPawn{ iter.second };
    if( pcPawn )
    {
      pcPawn->Tick();
    }
  }
}
