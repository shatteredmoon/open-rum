#include <e_control.h>


// static
void rumEditorControl::ScriptBind()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::Class<rumEditorControl, Sqrat::NoConstructor<rumEditorControl>> cEditorControl( pcVM, "rumEditorControl" );
  Sqrat::RootTable( pcVM ).Bind( "rumControl", cEditorControl );

  Sqrat::DerivedClass<rumEditorScrollBuffer, rumEditorControl> cEditorScrollBuffer( pcVM, "rumEditorScrollBuffer" );
  Sqrat::RootTable( pcVM ).Bind( "rumScrollBuffer", cEditorScrollBuffer );

  Sqrat::DerivedClass<rumEditorLabel, rumEditorControl> cEditorLabel( pcVM, "rumEditorLabel" );
  Sqrat::RootTable( pcVM ).Bind( "rumLabel", cEditorLabel );
  rumScript::CreateClassScript( "Label", "rumLabel" );

  Sqrat::DerivedClass<rumEditorListView, rumEditorScrollBuffer> cEditorListView( pcVM, "rumEditorListView" );
  Sqrat::RootTable( pcVM ).Bind( "rumListView", cEditorListView );
  rumScript::CreateClassScript( "ListView", "rumListView" );

  Sqrat::DerivedClass<rumEditorRegion, rumEditorControl> cEditorRegion( pcVM, "rumEditorRegion" );
  Sqrat::RootTable( pcVM ).Bind( "rumRegion", cEditorRegion );
  rumScript::CreateClassScript( "Region", "rumRegion" );

  Sqrat::DerivedClass<rumEditorSlider, rumEditorLabel> cEditorSlider( pcVM, "rumEditorSlider" );
  Sqrat::RootTable( pcVM ).Bind( "rumSlider", cEditorSlider );
  rumScript::CreateClassScript( "Slider", "rumSlider" );

  Sqrat::DerivedClass<rumEditorTextBox, rumEditorControl> cEditorTextBox( pcVM, "rumEditorTextBox" );
  Sqrat::RootTable( pcVM ).Bind( "rumTextBox", cEditorTextBox );
  rumScript::CreateClassScript( "TextBox", "rumTextBox" );

  Sqrat::DerivedClass<rumEditorTextView, rumEditorScrollBuffer> cEditorTextView( pcVM, "rumEditorTextView" );
  Sqrat::RootTable( pcVM ).Bind( "rumTextView", cEditorTextView );
  rumScript::CreateClassScript( "TextView", "rumTextView" );
}
