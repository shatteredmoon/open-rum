#include <e_sound.h>


// static
void EditorSound::ScriptBind()
{
  super::ScriptBind();

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::DerivedClass<EditorSound, rumSound> cEditorSound( pcVM, "EditorSound" );
  Sqrat::RootTable( pcVM ).Bind( "rumSound", cEditorSound );
}
