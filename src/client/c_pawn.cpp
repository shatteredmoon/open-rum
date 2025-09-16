#include <c_pawn.h>

#include <c_map.h>

#include <u_assert.h>
#include <u_creature_asset.h>
#include <u_portal_asset.h>
#include <u_resource.h>
#include <u_widget_asset.h>


void rumClientPawn::OnCreated()
{
  const rumPawnAsset* pcAsset{ rumPawnAsset::Fetch( GetAssetID() ) };
  if( pcAsset )
  {
    SetGraphicID( pcAsset->GetGraphicID() );
    SetDrawOrder( pcAsset->GetDrawOrder() );
  }

  super::OnCreated();
}


// static
void rumClientPawn::ScriptBind()
{
  super::ScriptBind();

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::DerivedClass<rumClientPawn, rumAnimation, Sqrat::NoConstructor<rumClientPawn>>
    cClientPawn( pcVM, "rumClientPawn" );
  Sqrat::RootTable( pcVM ).Bind( "rumPawn", cClientPawn );

  Sqrat::DerivedClass<rumClientCreature, rumClientPawn> cClientCreature( pcVM, "rumClientCreature" );
  Sqrat::RootTable( pcVM ).Bind( SCRIPT_CREATURE_NATIVE_CLASS, cClientCreature );

  Sqrat::DerivedClass<rumClientPortal, rumClientPawn> cClientPortal( pcVM, "rumClientPortal" );
  Sqrat::RootTable( pcVM ).Bind( SCRIPT_PORTAL_NATIVE_CLASS, cClientPortal );

  Sqrat::DerivedClass<rumClientWidget, rumClientPawn> cClientWidget( pcVM, "rumClientWidget" );
  Sqrat::RootTable( pcVM ).Bind( SCRIPT_WIDGET_NATIVE_CLASS, cClientWidget );
}


// final
int32_t rumClientPawn::Serialize( rumResource& io_rcResource )
{
  // The client should never save or load pawns
  rumAssert( false );
  return RESULT_FAILED;
}


// virtual
void rumClientCreature::AllocateGameID( rumUniqueID i_uiGameID )
{
  if( INVALID_GAME_ID == i_uiGameID )
  {
    // This is a client-only object. The high 32-bits contain object meta data, while the low 32-bits represent its
    // unique ID.
    static rumUniqueID s_uiAssetType{ ( rumUniqueID( rumCreatureAsset::GetClassRegistryID() ) ) << 60 };
    static rumUniqueID s_uiGameID{ ( rumUniqueID( Client_ObjectCreationType ) ) << 56 };
    i_uiGameID = ++s_uiGameID | s_uiAssetType;
  }
  else
  {
#pragma message ("TODO - how to handle player UIDs?")
    // This object originated from the server, so use that UID
    //rumAssertMsg( OBJECT_CREATION_TYPE( i_uiGameID ) == ObjectCreationType_Server,
    //              "Creating a creature with a non-zero UID that doesn't originate from the server" );
  }

  SetGameID( i_uiGameID );
}


// virtual
void rumClientPortal::AllocateGameID( rumUniqueID i_uiGameID )
{
  if( INVALID_GAME_ID == i_uiGameID )
  {
    // This is a client-only object. The high 32-bits contain object meta data, while the low 32-bits represent its
    // unique ID.
    static rumUniqueID s_uiAssetType = ( rumUniqueID( rumPortalAsset::GetClassRegistryID() ) ) << 60;
    static rumUniqueID s_uiGameID = ( rumUniqueID( Client_ObjectCreationType ) ) << 56;
    i_uiGameID = ++s_uiGameID | s_uiAssetType;
  }
  else
  {
    // This object originated from the server, so use that UID
    rumAssertMsg( OBJECT_CREATION_TYPE( i_uiGameID ) == Server_ObjectCreationType,
                  "Creating a portal with a non-zero UID that doesn't originate from the server" );
  }

  SetGameID( i_uiGameID );
}


// virtual
void rumClientWidget::AllocateGameID( rumUniqueID i_uiGameID )
{
  if( INVALID_GAME_ID == i_uiGameID )
  {
    // This is a client-only object. The high 32-bits contain object meta data, while the low 32-bits represent its
    // unique ID.
    static rumUniqueID s_uiAssetType = ( rumUniqueID( rumWidgetAsset::GetClassRegistryID() ) ) << 60;
    static rumUniqueID s_uiGameID = ( rumUniqueID( Client_ObjectCreationType ) ) << 56;
    i_uiGameID = ++s_uiGameID | s_uiAssetType;
  }
  else
  {
    // This object originated from the server, so use that UID
    rumAssertMsg( OBJECT_CREATION_TYPE( i_uiGameID ) == Server_ObjectCreationType,
                  "Creating a widget with a non-zero UID that doesn't originate from the server" );
  }

  SetGameID( i_uiGameID );
}
