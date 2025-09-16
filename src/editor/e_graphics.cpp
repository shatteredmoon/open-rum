#include <e_graphics.h>

#define GRAPHICS_FOLDER_NAME "graphics"


// override
void EditorGraphicBase::OnCreated()
{
  super::OnCreated();
  rumGraphicAsset::LoadData( GetAssetID() );
}


// static
void EditorGraphicBase::ScriptBind()
{
  super::ScriptBind();

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::DerivedClass<EditorGraphicBase, rumGraphic, Sqrat::NoConstructor<EditorGraphicBase>>
    cEditorGraphicBase( pcVM, "EditorGraphicBase" );
  Sqrat::RootTable( pcVM ).Bind( "rumEditorGraphicBase", cEditorGraphicBase );
}
