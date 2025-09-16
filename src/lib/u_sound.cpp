#include <u_sound.h>

#include <u_log.h>
#include <u_utility.h>

// Static initializers
bool rumSound::s_bInitialized{ false };
Sqrat::Object rumSound::s_sqClass;
rumSound::SoundHash rumSound::s_cSoundHash;
rumPosition rumSound::s_posListener{ rumPosition( 0, 0 ) };
float rumSound::s_fMaxSoundDistance{ 10.0f };
float rumSound::s_fMinSoundDistance{ 0.0f };


rumSound::~rumSound()
{
  FreeInternal();
  Unmanage();
}


// override
void rumSound::AllocateGameID( rumUniqueID i_uiGameID )
{
  rumAssert( INVALID_GAME_ID == i_uiGameID );

  static uint64_t s_uiAssetType{ ( rumUniqueID( rumSoundAsset::GetClassRegistryID() ) ) << 60 };
  static uint64_t s_uiGameID{ ( rumUniqueID( Client_ObjectCreationType ) ) << 56 };
  SetGameID( ++s_uiGameID | s_uiAssetType );
}


// static
rumSound* rumSound::Fetch( rumUniqueID i_uiGameID )
{
  const SoundHash::iterator iter{ s_cSoundHash.find( i_uiGameID ) };
  return ( iter != s_cSoundHash.end() ) ? iter->second : nullptr;
}


void rumSound::Free()
{
  FreeInternal();
  return super::Free();
}


void rumSound::FreeInternal()
{
  Remove( GetGameID() );
}


// static
void rumSound::Init()
{
  rumAssert( rumScript::GetCurrentVMType() != rumScript::VM_SERVER );

  if( !s_bInitialized )
  {
    Logger::LogStandard( "Initializing sound system..." );
  }
}


bool rumSound::IsPaused()
{
  return ( Paused_SoundState == m_eState );
}


bool rumSound::IsPlaying()
{
  return ( Playing_SoundState == m_eState );
}


bool rumSound::IsStopped()
{
  return ( Stopped_SoundState == m_eState );
}


bool rumSound::IsStopping()
{
  return ( Stopping_SoundState == m_eState );
}


// override
void rumSound::Manage()
{
  const rumUniqueID uiGameID{ GetGameID() };
  rumAssert( uiGameID != INVALID_GAME_ID );
  if( uiGameID != INVALID_GAME_ID )
  {
    RUM_COUT_IFDEF_DBG( SOUND_DEBUG,
                        "Managing Sound " << GetName() << " [" << rumStringUtils::ToHexString64( GetGameID() ) <<
                        "]\n" );
    s_cSoundHash.insert( { uiGameID, this } );
  }
}


// virtual
bool rumSound::Play( float i_fVolume )
{
  return ManageScriptObject( GetScriptInstance() );
}


// virtual
bool rumSound::Play3D( const rumPosition& i_rcPos, float i_fVolume )
{
  return ManageScriptObject( GetScriptInstance() );
}


// override
void rumSound::Unmanage()
{
  RUM_COUT_IFDEF_DBG( SOUND_DEBUG,
                      "Unmanaging Sound " << GetName() << " [" << rumStringUtils::ToHexString64( GetGameID() ) <<
                      "]\n" );
  s_cSoundHash.erase( GetGameID() );
}


// static
void rumSound::OnAssetDataChanged( const rumSoundAsset& i_rcAsset )
{
  for( const auto& iter : s_cSoundHash )
  {
    rumSound* pcSound{ iter.second };
    rumAssert( pcSound );

    if( pcSound->GetAssetID() == i_rcAsset.GetAssetID() )
    {
      // No need to re-init data, just stop the sound
      pcSound->Stop( 2.f );
    }
  }
}


// override
void rumSound::OnCreated()
{
  if( m_pcAsset )
  {
    rumSoundAssetAttributes cSoundAttributes;
    m_pcAsset->GetAttributes( cSoundAttributes );
    m_cAssetAttributes = cSoundAttributes;

    InitData();

    RUM_COUT_IFDEF_DBG( SOUND_DEBUG,
                        "Sound created " << m_pcAsset->GetName() << " [" <<
                        rumStringUtils::ToHexString64(GetGameID()) << "]\n");
  }

  super::OnCreated();
}


// static
void rumSound::PrintActiveSounds()
{
#if SOUND_DEBUG
  RUM_COUT( "Active sound handles:\n" );

  for( const auto& iter : s_hashSounds )
  {
    const rumSound* pcSound{ iter.second };
    RUM_COUT( "   " << pcSound->GetName() << " [" << rumStringUtils::ToHexString64( iter.first ) << "]\n" );
  }
#endif // SOUND_DEBUG
}


// static
void rumSound::Remove( rumUniqueID i_uiGameID )
{
#if SOUND_DEBUG
  const rumSound* pcSound{ Fetch( i_uiGameID ) };
  if( pcSound )
  {
    RUM_COUT( "Removing sound " << pcSound->GetName() << " [" <<
              rumStringUtils::ToHexString64( i_uiGameID ) << "]\n" );
  }
#endif // SOUND_DEBUG

  s_cSoundHash.erase( i_uiGameID );
}


// static
void rumSound::ScriptBind()
{
  rumAssert( rumScript::GetCurrentVMType() != rumScript::VM_SERVER );

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::DerivedClass<rumSound, rumGameObject, Sqrat::NoConstructor<rumSound>> cSound( pcVM, "rumSound" );
  cSound
    .Func( "IsPaused", &IsPaused )
    .Func( "IsPlaying", &IsPlaying )
    .Func( "IsStopped", &IsStopped )
    .Func( "IsStopping", &IsStopping )
    .Func( "SetVolume", &SetVolume )
    .Func( "GetVolume", &GetVolume )
    .Overload<bool( rumSound::* )( void )>( "Play", &Play )
    .Overload<bool( rumSound::* )( float )>( "Play", &Play )
    .Overload<bool( rumSound::* )( const rumPosition& )>( "Play3D", &Play3D )
    .Overload<bool( rumSound::* )( const rumPosition&, float )>( "Play3D", &Play3D )
    .Overload<bool( rumSound::* )( const rumVector& )>( "Play3DOnListerner", &Play3DOnListener )
    .Overload<bool( rumSound::* )( const rumVector&, float )>( "Play3DOnListener", &Play3DOnListener )
    .Overload<void( rumSound::* )( void )>( "Stop", &Stop )
    .Overload<void( rumSound::* )( float )>( "Stop", &Stop );
  Sqrat::RootTable( pcVM ).Bind( "rumSoundBase", cSound );

  Sqrat::RootTable( pcVM )
    .Func( "rumFetchSoundInstance", Fetch )
    .Func( "rumGetListenerPosition", GetListenerPosition )
    .Func( "rumSetMinMaxSoundDistance", SetMinMaxSoundDistance )
    .Overload<void( * )( void )>( "rumStopAllSounds", StopAll )
    .Overload<void( * )( float )>( "rumStopAllSounds", StopAll )
    .Overload<void( * )( rumAssetID )>( "rumStopAllSoundsOfType", StopAllOfType )
    .Overload<void( * )( rumAssetID, float )>( "rumStopAllSoundsOfType", StopAllOfType );
}


// static
void rumSound::SetMinMaxSoundDistance( float i_fMin, float i_fMax )
{
  s_fMinSoundDistance = i_fMin;
  s_fMaxSoundDistance = i_fMax;

  for( auto& rcIter : s_cSoundHash )
  {
    rcIter.second->OnMinMaxDistanceChanged( i_fMin, i_fMax );
  }
}


void rumSound::SetState( const rumSoundState i_eState )
{
  if( i_eState == m_eState )
  {
    return;
  }

  m_eState = i_eState;

  if( Stopped_SoundState == i_eState )
  {
    HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

    auto sqInstance{ GetScriptInstance() };
    UnmanageScriptObject( sqInstance );

    RUM_COUT_IFDEF_DBG( SOUND_DEBUG,
                        "Sound stopped: " << GetName() << " [" << rumStringUtils::ToHexString64( GetGameID() ) <<
                        "]\n" );

    rumScript::ExecOptionalFunc( Sqrat::RootTable( pcVM ), "OnSoundStopped", sqInstance );
  }
}


// static
void rumSound::Shutdown()
{
  rumAssert( rumScript::GetCurrentVMType() != rumScript::VM_SERVER );
  StopAll();

  if( !s_cSoundHash.empty() )
  {
    // Make a copy of the hash map for iteration and deletion. We can't delete directly from the original hash because
    // the iteration will be destroyed by calls to each object's Free method.
    SoundHash cTempHash{ s_cSoundHash };
    s_cSoundHash.clear();

    for( const auto& iter : cTempHash )
    {
      RUM_COUT_IFDEF( MEMORY_DEBUG, "Freeing Sound: " << iter.second->GetName() << '\n' );

      rumSound* pcObject{ iter.second };
      if( pcObject )
      {
        pcObject->Free();
      }
    }

    cTempHash.clear();
  }
}


void rumSound::Stop( float i_fFadeTime )
{
  m_eState = Stopped_SoundState;
}


// static
void rumSound::StopAll( float i_fFadeTime )
{
  for( const auto& iter : s_cSoundHash )
  {
    rumSound* pcSound{ iter.second };
    if( pcSound )
    {
      pcSound->Stop( i_fFadeTime );
    }
  }
}


// static
void rumSound::StopAllOfType( rumAssetID i_eAssetID, float i_fFadeTime )
{
  RUM_COUT_IFDEF_DBG( SOUND_DEBUG,
                      "Stopping all instances of sound id " << rumStringUtils::ToHexString( i_eAssetID ) << '\n' );

  for( const auto& iter : s_cSoundHash )
  {
    rumSound* pcSound{ iter.second };
    if( pcSound && ( pcSound->GetAssetID() == i_eAssetID ) )
    {
      pcSound->Stop( i_fFadeTime );
    }
  }
}
