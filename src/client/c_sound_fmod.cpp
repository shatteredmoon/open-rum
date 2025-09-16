#ifdef USE_FMOD

#include <c_sound_fmod.h>

#include <u_log.h>

#define FMOD_MAX_CHANNELS 128

// Static initializers
rumClientSound::FinishedSoundContainer rumClientSound::s_cFinishedSoundsQueue;
rumClientSound::FadeInfoContainer rumClientSound::s_cFadingSoundsList;
rumClientSound::ChannelHash rumClientSound::s_cChannelHash;
FMOD::System* rumClientSound::s_pcSystem;
std::mutex rumClientSound::s_mtxQueue;


FMOD_RESULT F_CALLBACK rumClientSound::ChannelCallback( FMOD_CHANNEL* i_pcChannel, FMOD_CHANNEL_CALLBACKTYPE i_eType,
                                                        void* i_pcData1, void* i_pcData2 )
{
  if( FMOD_CHANNEL_CALLBACKTYPE_END == i_eType )
  {
    const FMOD::Channel* pcChannel{ (FMOD::Channel*)i_pcChannel };
    if( pcChannel )
    {
      rumUniqueID uiGameID{ INVALID_GAME_ID };

      // Reverse lookup
      for( const auto& iter : s_cChannelHash )
      {
        if( iter.second == pcChannel )
        {
          uiGameID = iter.first;
        }
      }

      rumAssert( uiGameID != INVALID_GAME_ID );
      if( uiGameID != INVALID_GAME_ID )
      {
        RUM_COUT_IFDEF_DBG( SOUND_DEBUG,
                            "ChannelCallback sound finished [" << rumStringUtils::ToHexString64( uiGameID ) << "]\n" );

        std::lock_guard<std::mutex> cLockGuard( s_mtxQueue );
        s_cFinishedSoundsQueue.push( uiGameID );
      }
    }
  }

  return FMOD_OK;
}


rumClientSound::~rumClientSound()
{
  FreeInternal();
}


// override
void rumClientSound::Free()
{
  FreeInternal();
  return super::Free();
}


void rumClientSound::FreeInternal()
{
  super::Stop();

  FMOD::Sound* pcSound{ reinterpret_cast<FMOD::Sound*>( m_pcData ) };
  if( pcSound )
  {
    pcSound->release();
    m_pcData = nullptr;
  }
}


float rumClientSound::GetVolume() const
{
  float fVolume{ 1.0f };

  const auto iter{ s_cChannelHash.find( GetGameID() ) };
  if( iter != s_cChannelHash.end() )
  {
    FMOD::Channel* pcChannel{ iter->second };
    if( pcChannel )
    {
      pcChannel->getVolume( &fVolume );
    }
  }

  return fVolume;
}


bool rumClientSound::InitData()
{
  if( m_pcData != nullptr )
  {
    // The data has already been initialized
    return true;
  }

  FMOD::System* pcFMODSystem{ GetFMODSystem() };
  if( !pcFMODSystem )
  {
    RUM_COUT( "Error: FMOD::System has not been initialized\n" );
    return false;
  }

  rumAssert( m_pcAsset );
  if( !m_pcAsset )
  {
    return false;
  }

  FMOD_CREATESOUNDEXINFO cInfo;
  memset( &cInfo, 0, sizeof( FMOD_CREATESOUNDEXINFO ) );
  cInfo.cbsize = sizeof( FMOD_CREATESOUNDEXINFO );
  cInfo.length = m_pcAsset->GetDataAllocSize();

  const rumSoundDataType eSoundDataType{ m_cAssetAttributes.GetDataType() };

  if( MIDI_SoundDataType == eSoundDataType )
  {
    FMOD::Sound* pcSound{ nullptr };
    const FMOD_RESULT eResult{ pcFMODSystem->createSound( (const char*)m_pcAsset->GetData(), FMOD_OPENMEMORY | FMOD_2D,
                                                          &cInfo, &pcSound ) };
    if( FMOD_OK == eResult )
    {
      m_pcData = (void*)pcSound;
    }
    else
    {
      RUM_COUT_DBG( "FMOD Error: " << rumStringUtils::ToHexString( eResult ) << " encountered in " <<
                    __FUNCTION__ << '\n' );
    }
  }
  else if( eSoundDataType != Invalid_SoundDataType )
  {
    FMOD::Sound* pcSound{ nullptr };
    const FMOD_RESULT eResult{ pcFMODSystem->createSound( (const char*)m_pcAsset->GetData(),
                                                          FMOD_OPENMEMORY | FMOD_SOFTWARE | FMOD_3D,
                                                          &cInfo, &pcSound ) };
    if( FMOD_OK == eResult )
    {
      pcSound->setMode( FMOD_3D_LINEARROLLOFF );
      pcSound->set3DMinMaxDistance( s_fMinSoundDistance, s_fMaxSoundDistance );

      m_pcData = (void*)pcSound;
    }
    else
    {
      RUM_COUT_DBG( "FMOD Error: " << rumStringUtils::ToHexString( eResult ) << " encountered in " <<
                    __FUNCTION__ << '\n' );
    }
  }

  return m_pcData != nullptr;
}


// static
uint32_t rumClientSound::Init()
{
  super::Init();

  Logger::LogStandard( "Initializing FMOD" );

  // TODO - Device enumeration

  FMOD_RESULT eResult{ FMOD::System_Create( &s_pcSystem ) };
  if( eResult != FMOD_OK )
  {
    s_pcSystem = nullptr;
    return RESULT_FAILED;
  }

  uint32_t uiVersion{ 0 };
  s_pcSystem->getVersion( &uiVersion );
  RUM_COUT( "Initializing FMOD version " << ( uiVersion >> 16 ) << "." << ( ( uiVersion >> 8 ) & 0xf ) << "." <<
            ( uiVersion & 0xf ) << '\n' );

  eResult = s_pcSystem->init( FMOD_MAX_CHANNELS, FMOD_INIT_NORMAL, 0 );
  if( eResult != FMOD_OK )
  {
    s_pcSystem->release();
    s_pcSystem = nullptr;
    return RESULT_FAILED;
  }

  s_bInitialized = true;

  return RESULT_SUCCESS;
}


bool rumClientSound::IsPaused()
{
  bool bPaused{ false };

  const auto iter{ s_cChannelHash.find( GetGameID() ) };
  if( iter != s_cChannelHash.end() )
  {
    FMOD::Channel* pcChannel{ iter->second };
    if( pcChannel )
    {
      pcChannel->getPaused( &bPaused );
    }
  }

  return bPaused;
}


bool rumClientSound::IsPlaying()
{
  const auto iter{ s_cChannelHash.find( GetGameID() ) };
  if( iter != s_cChannelHash.end() )
  {
    FMOD::Channel* pcChannel{ iter->second };
    if( pcChannel )
    {
      bool bPlaying{ false };
      pcChannel->isPlaying( &bPlaying );
      return ( Playing_SoundState == GetState() ) && bPlaying;
    }
  }

  return false;
}


void rumClientSound::OnMinMaxDistanceChanged( float i_fMin, float i_fMax )
{
  FMOD::Sound* pcSound{ reinterpret_cast<FMOD::Sound*>( m_pcData ) };
  rumAssert( pcSound != nullptr );
  if( pcSound != nullptr )
  {
    pcSound->set3DMinMaxDistance( i_fMin, i_fMax );
  }
}


bool rumClientSound::Play( float i_fVolume )
{
  rumAssert( m_pcAsset );

  FMOD::System* pcFMODSystem{ GetFMODSystem() };
  rumAssert( pcFMODSystem != nullptr );
  if( nullptr == pcFMODSystem )
  {
    RUM_COUT( "Error: FMOD::System has not been initialized\n" );
    return false;
  }

  rumAssert( m_pcData != nullptr );
  if( ( Invalid_SoundDataType == m_cAssetAttributes.GetDataType() ) || ( nullptr == m_pcData ) )
  {
    return false;
  }

  FMOD::Sound* pcSound{ reinterpret_cast<FMOD::Sound*>( m_pcData ) };
  rumAssert( pcSound != nullptr );
  if( nullptr == pcSound )
  {
    return false;
  }

  FMOD::Channel* pcChannel{ nullptr };
  if( pcFMODSystem->playSound( FMOD_CHANNEL_FREE, pcSound, true, &pcChannel ) == FMOD_OK )
  {
    pcChannel->setCallback( ChannelCallback );
    pcChannel->setVolume( std::clamp<float>( i_fVolume, 0.f, 1.0f ) );
    pcChannel->setPaused( false );

    RUM_COUT_IFDEF_DBG( SOUND_DEBUG,
                        "Playing sound " << GetName() << " [" <<
                        rumStringUtils::ToHexString64( GetGameID() ) << "]\n" );

    [[maybe_unused]] auto cPair{ s_cChannelHash.insert( std::make_pair( GetGameID(), pcChannel ) ) };
    rumAssert( cPair.second );

    SetState( Playing_SoundState );

    super::Play( i_fVolume );

    return true;
  }

  return false;
}


bool rumClientSound::Play3D( const rumPosition& i_rcPos, float i_fVolume )
{
  rumAssert( m_pcAsset );

  FMOD::System* pcFMODSystem{ GetFMODSystem() };
  rumAssert( pcFMODSystem != nullptr );
  if( nullptr == pcFMODSystem )
  {
    RUM_COUT( "Error: FMOD::System has not been initialized\n" );
    return false;
  }

  rumAssert( m_pcData != nullptr );
  if( ( Invalid_SoundDataType == m_cAssetAttributes.GetDataType() ) || ( nullptr == m_pcData ) )
  {
    return false;
  }

  FMOD::Sound* pcSound{ reinterpret_cast<FMOD::Sound*>( m_pcData ) };
  rumAssert( pcSound != nullptr );
  if( nullptr == pcSound )
  {
    return false;
  }

  FMOD::Channel* pcChannel{ nullptr };
  if( pcFMODSystem->playSound( FMOD_CHANNEL_FREE, pcSound, true, &pcChannel ) == FMOD_OK )
  {
    FMOD_VECTOR vPos{ (float)i_rcPos.m_iX, (float)i_rcPos.m_iY, 0.0f };
    pcChannel->set3DAttributes( &vPos, nullptr );
    pcChannel->setCallback( ChannelCallback );
    pcChannel->setVolume( std::clamp<float>( i_fVolume, 0.f, 1.0f ) );
    pcChannel->setPaused( false );

    RUM_COUT_IFDEF_DBG( SOUND_DEBUG,
                        "Playing sound " << GetName() << " [" <<
                        rumStringUtils::ToHexString64( GetGameID() ) << "]\n" );

    [[maybe_unused]] auto cPair{ s_cChannelHash.insert( std::make_pair( GetGameID(), pcChannel ) ) };
    rumAssert( cPair.second );

    SetState( Playing_SoundState );

    super::Play3D( i_rcPos, i_fVolume );

    return true;
  }

  return false;
}


// static
void rumClientSound::ProcessQueue()
{
  if( s_cFinishedSoundsQueue.empty() )
  {
    return;
  }

  // Since this call takes place on a separate thread, sound instance removal is gated.
  std::lock_guard<std::mutex> cLockGuard( s_mtxQueue );

  do
  {
    const rumUniqueID uiGameID{ s_cFinishedSoundsQueue.front() };
    s_cFinishedSoundsQueue.pop();

    // The sound is no longer playing
    rumSound* pcSound{ rumSound::Fetch( uiGameID ) };
    if( pcSound )
    {
      RUM_COUT_IFDEF_DBG( SOUND_DEBUG,
                          "Sound stopped " << pcSound->GetName() << " [" <<
                          rumStringUtils::ToHexString64( uiGameID ) << "]\n" );

      s_cChannelHash.erase( uiGameID );

      pcSound->SetState( Stopped_SoundState );
    }
  } while( !s_cFinishedSoundsQueue.empty() );
}


// static
void rumClientSound::ScriptBind()
{
  super::ScriptBind();

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::DerivedClass<rumClientSound, rumSound> cClientSound( pcVM, "rumClientSound" );
  Sqrat::RootTable( pcVM ).Bind( "rumSound", cClientSound );

  Sqrat::RootTable( pcVM )
    .Overload<void( * )( int32_t, int32_t )>( "rumSetListenerPosition", SetListenerPosition )
    .Overload<void( * )( const rumPosition& )>( "rumSetListenerPosition", SetListenerPosition );
}


// static
void rumClientSound::SetListenerPosition( int32_t i_iPosX, int32_t i_iPosY )
{
  SetListenerPosition( rumPosition( i_iPosX, i_iPosY ) );
}


// static
void rumClientSound::SetListenerPosition( const rumPosition& i_rcPos )
{
  super::SetListenerPosition( i_rcPos );

  FMOD::System* pcFMODSystem{ GetFMODSystem() };
  rumAssert( pcFMODSystem != nullptr );
  if( nullptr == pcFMODSystem )
  {
    RUM_COUT( "Error: FMOD::System has not been initialized\n" );
    return;
  }

  FMOD_VECTOR ciListenerPos{ (float)i_rcPos.m_iX, (float)i_rcPos.m_iY, 0.0f };
  pcFMODSystem->set3DListenerAttributes( 0, &ciListenerPos, nullptr, nullptr, nullptr );
}


void rumClientSound::SetVolume( float i_fVolume )
{
  if( GetState() == Stopping_SoundState )
  {
    // Volume control on sounds that are fading out is restricted
    return;
  }

  rumNumberUtils::Clamp( i_fVolume, 0.f, 1.f );

  const auto iter{ s_cChannelHash.find( GetGameID() ) };
  if( iter != s_cChannelHash.end() )
  {
    FMOD::Channel* pcChannel{ iter->second };
    if( pcChannel )
    {
      pcChannel->setVolume( i_fVolume );
      m_fVolume = i_fVolume;
    }
  }
}


// static
void rumClientSound::Shutdown()
{
  super::Shutdown();
  ProcessQueue();
}


void rumClientSound::Stop( float i_fFadeTime )
{
  if( GetState() == Stopping_SoundState )
  {
    return;
  }

  const auto iter{ s_cChannelHash.find( GetGameID() ) };
  if( iter != s_cChannelHash.end() )
  {
    FMOD::Channel* pcChannel{ iter->second };
    if( pcChannel )
    {
      if( i_fFadeTime > 0.f )
      {
        RUM_COUT_IFDEF_DBG( SOUND_DEBUG,
                            "Fading sound " << " [" << rumStringUtils::ToHexString64( GetGameID() ) << "]\n" );

        rumFadeInfo cFadeInfo;
        cFadeInfo.m_uiGameID = GetGameID();
        cFadeInfo.m_tFadeStart = GetElapsedTime();
        cFadeInfo.m_tFadeEnd = cFadeInfo.m_tFadeStart + i_fFadeTime;

        // Fading handled in Update()
        s_cFadingSoundsList.push_front( cFadeInfo );

        SetState( Stopping_SoundState );
      }
      else
      {
        RUM_COUT_IFDEF_DBG( SOUND_DEBUG,
                            "Stopping sound " << " [" << rumStringUtils::ToHexString64( GetGameID() ) << "]\n" );

        pcChannel->stop();
        s_cChannelHash.erase( GetGameID() );

        SetState( Stopped_SoundState );
      }
    }
  }

  PrintActiveSounds();
}


// static
void rumClientSound::Update()
{
  if( s_pcSystem != nullptr )
  {
    s_pcSystem->update();
  }

  ProcessQueue();

  // Handle fading sounds
  FadeInfoContainer::iterator iter( s_cFadingSoundsList.begin() );
  const FadeInfoContainer::iterator end( s_cFadingSoundsList.end() );
  while( iter != end )
  {
    rumFadeInfo& rcInfo{ *iter };

    const auto iterChannel{ s_cChannelHash.find( rcInfo.m_uiGameID ) };
    if( iterChannel == s_cChannelHash.end() )
    {
      ++iter;
      continue;
    }

    FMOD::Channel* pcChannel{ iterChannel->second };
    if( pcChannel )
    {
      // Determine volume level
      double tCurrent{ GetElapsedTime() };
      float fVolume{ 1.0f };
      pcChannel->getVolume( &fVolume );
      double dVolume{ ( rcInfo.m_tFadeEnd - tCurrent ) / ( rcInfo.m_tFadeEnd - rcInfo.m_tFadeStart ) * fVolume };
      if( dVolume < 0.0001 )
      {
        RUM_COUT_IFDEF_DBG( SOUND_DEBUG,
                            "Stopping sound " << " [" <<
                            rumStringUtils::ToHexString64( rcInfo.m_uiGameID ) << "]\n" );

        // The sound is no longer audible
        pcChannel->stop();
        s_cChannelHash.erase( iterChannel );

        rumSound* pcSound{ rumSound::Fetch( rcInfo.m_uiGameID ) };
        rumAssert( pcSound != nullptr );
        if( pcSound )
        {
          pcSound->SetState( Stopped_SoundState );
        }

        // Remove this sound from the fading info container
        iter = s_cFadingSoundsList.erase( iter );
      }
      else
      {
        // Set new volume level
        pcChannel->setVolume( (float)dVolume );
        ++iter;
      }
    }
    else
    {
      ++iter;
    }
  }
}

#endif // USE_FMOD
