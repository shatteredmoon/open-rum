#include <controls/c_region.h>


// override
void rumClientRegion::Clear()
{
  // Do nothing
}


// override
void rumClientRegion::Draw( const rumPoint& i_rcPos )
{
  // Do nothing
}


void rumClientRegion::ScriptBind()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::DerivedClass<rumClientRegion, rumClientControl> cClientRegion( pcVM, "rumClientRegion" );
  Sqrat::RootTable( pcVM ).Bind( "rumRegion", cClientRegion );

  rumScript::CreateClassScript( "Region", "rumRegion" );
}


// override
bool rumClientRegion::Validate()
{
  return true;
}
