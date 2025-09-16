#include <u_broadcast.h>

#include <u_log.h>
#include <u_utility.h>

// Static initializers
Sqrat::Object rumBroadcast::s_sqClass;
rumBroadcast::BroadcastHash rumBroadcast::s_hashBroadcasts;


rumBroadcast::~rumBroadcast()
{
  Unmanage();
}


// override
void rumBroadcast::AllocateGameID( rumUniqueID i_uiGameID )
{
  rumAssert( INVALID_GAME_ID == i_uiGameID );

  static uint64_t s_uiAssetType{ ( rumUniqueID( rumBroadcastAsset::GetClassRegistryID() ) ) << 60 };
  static uint64_t s_uiGameID{ ( rumUniqueID( Neutral_ObjectCreationType ) ) << 56 };
  SetGameID( ++s_uiGameID | s_uiAssetType );
}


// override
void rumBroadcast::Manage()
{
  const rumUniqueID uiGameID{ GetGameID() };
  rumAssert( uiGameID != INVALID_GAME_ID );
  if( uiGameID != INVALID_GAME_ID )
  {
    s_hashBroadcasts.insert( { uiGameID, this } );
  }
}


// override
void rumBroadcast::Unmanage()
{
  s_hashBroadcasts.erase( GetGameID() );
}


// static
void rumBroadcast::ScriptBind()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::DerivedClass<rumBroadcast, rumGameObject> cBroadcast( pcVM, SCRIPT_BROADCAST_NATIVE_CLASS );
  Sqrat::RootTable( pcVM ).Bind( SCRIPT_BROADCAST_NATIVE_CLASS, cBroadcast );
}


// static
void rumBroadcast::Shutdown()
{
  rumAssert( s_hashBroadcasts.empty() );
  if( !s_hashBroadcasts.empty() )
  {
    // Make a copy of the hash map for iteration and deletion. We can't delete directly from the original hash because
    // the iteration will be destroyed by calls to each object's Free method.
    BroadcastHash cHash{ s_hashBroadcasts };
    s_hashBroadcasts.clear();

    for( const auto& iter : cHash )
    {
      RUM_COUT_IFDEF( MEMORY_DEBUG, "Freeing Broadcast: " << iter.second->GetName() << '\n' );

      rumBroadcast* pcObject{ iter.second };
      if( pcObject )
      {
        pcObject->Free();
      }
    }

    cHash.clear();
  }
}
