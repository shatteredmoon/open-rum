#include <e_inventory.h>


void EditorInventory::ScriptBind()
{
  rumInventory::ScriptBind();

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::DerivedClass<EditorInventory, rumInventory> cEditorInventory( pcVM, "EditorInventory" );
  Sqrat::RootTable( pcVM ).Bind( "rumInventory", cEditorInventory );
}
