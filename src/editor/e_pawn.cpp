#include <e_pawn.h>

#include <u_assert.h>
#include <u_creature_asset.h>
#include <u_portal_asset.h>
#include <u_widget_asset.h>


namespace pawns
{
  int32_t Init()
  {
    return RESULT_SUCCESS;
  }
}


// override
void EditorCreature::AllocateGameID( rumUniqueID i_uiGameID )
{
  if( INVALID_GAME_ID == i_uiGameID )
  {
    static rumUniqueID s_uiAssetType{ ( rumUniqueID( rumCreatureAsset::GetClassRegistryID() ) ) << 60 };
    static rumUniqueID s_uiGameID{ ( rumUniqueID( Neutral_ObjectCreationType ) ) << 56
  };

    i_uiGameID = ++s_uiGameID | s_uiAssetType;
  }

  SetGameID( i_uiGameID );
}


// override
void EditorPortal::AllocateGameID( rumUniqueID i_uiGameID )
{
  if( INVALID_GAME_ID == i_uiGameID )
  {
    static rumUniqueID s_uiAssetType{ ( rumUniqueID( rumPortalAsset::GetClassRegistryID() ) ) << 60 };
    static rumUniqueID s_uiGameID{ ( rumUniqueID( Neutral_ObjectCreationType ) ) << 56 };

    i_uiGameID = ++s_uiGameID | s_uiAssetType;
  }

  SetGameID( i_uiGameID );
}


// override
void EditorWidget::AllocateGameID( rumUniqueID i_uiGameID )
{
  if( INVALID_GAME_ID == i_uiGameID )
  {
    static rumUniqueID s_uiAssetType{ ( rumUniqueID( rumWidgetAsset::GetClassRegistryID() ) ) << 60 };
    static rumUniqueID s_uiGameID{ ( rumUniqueID( Neutral_ObjectCreationType ) ) << 56 };

    i_uiGameID = ++s_uiGameID | s_uiAssetType;
  }

  SetGameID( i_uiGameID );
}


void EditorPawn::OnCreated()
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
void EditorPawn::ScriptBind()
{
  super::ScriptBind();

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::DerivedClass<EditorPawn, rumAnimation, Sqrat::NoConstructor<EditorPawn>>
    cEditorPawn( pcVM, "rumEditorPawn" );
  Sqrat::RootTable( pcVM ).Bind( "rumPawn", cEditorPawn );

  Sqrat::DerivedClass<EditorCreature, EditorPawn> cEditorCreature( pcVM, "rumEditorCreature" );
  Sqrat::RootTable( pcVM ).Bind( SCRIPT_CREATURE_NATIVE_CLASS, cEditorCreature );

  Sqrat::DerivedClass<EditorPortal, EditorPawn> cEditorPortal( pcVM, "rumEditorPortal" );
  Sqrat::RootTable( pcVM ).Bind( SCRIPT_PORTAL_NATIVE_CLASS, cEditorPortal );

  Sqrat::DerivedClass<EditorWidget, EditorPawn> cEditorWidget( pcVM, "rumEditorWidget" );
  Sqrat::RootTable( pcVM ).Bind( SCRIPT_WIDGET_NATIVE_CLASS, cEditorWidget );
}
