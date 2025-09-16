#include <u_timer.h>

#include <u_script.h>


void rumTimer::ScriptBind()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };
  Sqrat::RootTable( pcVM ).Func( "rumGetSecondsSinceEpoch", GetSecondsSinceEpoch );
}
