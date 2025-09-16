#include <u_inventory.h>

#include <u_assert.h>
#include <u_db.h>
#include <u_inventory_asset.h>
#include <u_log.h>
#include <u_script.h>

// Static intializers
Sqrat::Object rumInventory::s_sqClass;
rumInventory::InventoryHash rumInventory::s_hashInventory;


rumInventory::~rumInventory()
{
  FreeInternal();
  Unmanage();
}


// static
rumInventory* rumInventory::Fetch( rumUniqueID i_uiGameID )
{
  const auto& iter{ s_hashInventory.find( i_uiGameID ) };
  return iter != s_hashInventory.end() ? iter->second : nullptr;
}


// static
Sqrat::Object rumInventory::FetchVM( rumUniqueID i_uiGameID )
{
  rumInventory* pcInventory{ Fetch( i_uiGameID ) };
  return pcInventory ? pcInventory->GetScriptInstance() : Sqrat::Object();
}


// override
void rumInventory::Free()
{
  FreeInternal();
  super::Free();
}


// override
void rumInventory::Manage()
{
  const rumUniqueID uiGameID{ GetGameID() };
  rumAssert( uiGameID != INVALID_GAME_ID );
  if( uiGameID != INVALID_GAME_ID )
  {
    s_hashInventory.insert( { uiGameID, this } );
  }
}


// override
void rumInventory::Unmanage()
{
  s_hashInventory.erase( GetGameID() );
}


// override
void rumInventory::OnCreated()
{
  super::OnCreated();

  s_hashInventory.insert( std::make_pair( GetGameID(), this ) );
}


bool rumInventory::OnAddedToPawn( rumPawn* i_pcPawn )
{
  if( !i_pcPawn )
  {
    return false;
  }

  // Set the pawn as the owner of the item
  SetOwner( i_pcPawn->GetGameID() );

  Sqrat::Object sqInstance{ i_pcPawn->GetScriptInstance() };
  if( sqInstance.GetType() != OT_INSTANCE )
  {
    return false;
  }

  // Notify scripts that the inventory change occurred
  rumScript::ExecOptionalFunc( sqInstance, "OnInventoryAdded", GetScriptInstance() );

  return true;
}


bool rumInventory::OnRemovedFromPawn( rumPawn* i_pcPawn )
{
  if( !i_pcPawn )
  {
    return false;
  }

  // Disown the item
  SetOwner( RUM_INVALID_NATIVEHANDLE );

  Sqrat::Object sqInstance{ i_pcPawn->GetScriptInstance() };
  if( sqInstance.GetType() != OT_INSTANCE )
  {
    return false;
  }

  // Notify scripts that the inventory change occurred
  rumScript::ExecOptionalFunc( sqInstance, "OnInventoryRemoved", GetScriptInstance() );

  return true;
}


// static
void rumInventory::ScriptBind()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::RootTable( pcVM )
    .Func( "rumFetchInventory", FetchVM );

  Sqrat::DerivedClass<rumInventory, rumGameObject, Sqrat::NoConstructor<rumInventory>> cInventory( pcVM, "rumInventory" );
  Sqrat::RootTable( pcVM ).Bind( "rumInventoryBase", cInventory );
}


// static
void rumInventory::Shutdown()
{
  rumAssert( s_hashInventory.empty() );
  if( !s_hashInventory.empty() )
  {
    // Make a copy of the hash map for iteration and deletion. We can't delete directly from the original hash because
    // the iteration will be destroyed by calls to each object's Free method.
    InventoryHash cHash{ s_hashInventory };
    s_hashInventory.clear();

    for( const auto& iter : cHash )
    {
      RUM_COUT_IFDEF( MEMORY_DEBUG, "Freeing Inventory: " << iter.second->GetName() << '\n' );

      rumInventory* pcObject{ iter.second };
      if( pcObject )
      {
        pcObject->Free();
      }
    }

    cHash.clear();
  }
}
