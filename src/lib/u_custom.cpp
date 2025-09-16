#include <u_custom.h>

#include <u_custom_asset.h>

// Static initializers
Sqrat::Object rumCustom::s_sqClass;
rumCustom::CustomHash rumCustom::s_hashCustomObjects;


rumCustom::~rumCustom()
{
  Unmanage();
}


// static
rumCustom* rumCustom::Fetch( rumUniqueID i_uiGameID )
{
  CustomHash::iterator iter{ s_hashCustomObjects.find( i_uiGameID ) };
  return ( iter != s_hashCustomObjects.end() ) ? iter->second : nullptr;
}


// override
void rumCustom::Manage()
{
  const rumUniqueID uiGameID{ GetGameID() };
  rumAssert( uiGameID != INVALID_GAME_ID );
  if( uiGameID != INVALID_GAME_ID )
  {
    s_hashCustomObjects.insert( { uiGameID, this } );
  }
}


// override
void rumCustom::Unmanage()
{
  s_hashCustomObjects.erase( GetGameID() );
}


// static
void rumCustom::ScriptBind()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::DerivedClass<rumCustom, rumGameObject> cCustom( pcVM, SCRIPT_CUSTOM_NATIVE_CLASS );
  Sqrat::RootTable( pcVM ).Bind( SCRIPT_CUSTOM_NATIVE_CLASS, cCustom );

  Sqrat::RootTable( pcVM ).Func( "rumFetchCustomObject", Fetch );
}


// static
void rumCustom::Shutdown()
{
  rumAssert( s_hashCustomObjects.empty() );
  if( !s_hashCustomObjects.empty() )
  {
    // Make a copy of the hash map for iteration and deletion. We can't delete directly from the original hash because
    // the iteration will be destroyed by calls to each object's Free method.
    CustomHash cHash{ s_hashCustomObjects };
    s_hashCustomObjects.clear();

    for( const auto& iter : cHash )
    {
      RUM_COUT_IFDEF( MEMORY_DEBUG, "Freeing Custom Object: " << iter.second->GetName() << '\n' );

      rumCustom* pcObject{ iter.second };
      if( pcObject )
      {
        pcObject->Free();
      }
    }

    cHash.clear();
  }
}
