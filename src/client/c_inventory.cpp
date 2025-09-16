#include <c_inventory.h>

#include <u_inventory_asset.h>


// override
void rumClientInventory::AllocateGameID( rumUniqueID i_uiGameID )
{
  if( INVALID_GAME_ID == i_uiGameID )
  {
    static uint64_t s_uiAssetType{ ( rumUniqueID( rumInventoryAsset::GetClassRegistryID() ) ) << 60 };
    static uint64_t s_uiGameID{ ( rumUniqueID( Client_ObjectCreationType ) ) << 56 };
    i_uiGameID = ++s_uiGameID | s_uiAssetType;
  }
  else
  {
    // This object originated from the server, so use that UID
    rumAssertMsg( OBJECT_CREATION_TYPE( i_uiGameID ) == Server_ObjectCreationType,
                  "Creating inventory with a non-zero UID that doesn't originate from the server" );
  }

  SetGameID( i_uiGameID );
}


// static
void rumClientInventory::ScriptBind()
{
  rumInventory::ScriptBind();

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::DerivedClass<rumClientInventory, rumInventory> cClientInventory( pcVM, "rumClientInventory" );
  Sqrat::RootTable( pcVM ).Bind( "rumInventory", cClientInventory );
}
