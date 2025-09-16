#ifdef USE_FMOD

#ifndef _C_SOUND_FMOD_H_
#define _C_SOUND_FMOD_H_

#include <u_sound.h>

#include <fmod.hpp>
#include <mutex>
#include <queue>


class rumClientSound : public rumSound
{
public:

  ~rumClientSound();

  void Free() override;

  float GetVolume() const override;
  void SetVolume( float i_fVolume ) override;

  bool IsPaused() override;
  bool IsPlaying() override;

  bool Play( float i_fVolume ) override;
  bool Play3D( const rumPosition& i_rcPos, float i_fVolume ) override;

  void Stop( float i_fFadeTime ) override;

  static FMOD_RESULT F_CALLBACK ChannelCallback( FMOD_CHANNEL* i_pcChannel, FMOD_CHANNEL_CALLBACKTYPE i_eType,
                                                 void* i_pcData1, void* i_pcData2 );

  static FMOD::System* GetFMODSystem()
  {
    return s_pcSystem;
  }

  static uint32_t Init();

  static void ProcessQueue();

  static void ScriptBind();

  static void SetListenerPosition( int32_t i_iPosX, int32_t i_iPosY );
  static void SetListenerPosition( const rumPosition& i_rcPos );

  static void Shutdown();

  static void Update();

  struct rumFadeInfo
  {
    rumUniqueID m_uiGameID{ INVALID_GAME_ID };
    double m_tFadeStart{ 0.0 };
    double m_tFadeEnd{ 0.0 };
  };

protected:

  // Init from asset data
  bool InitData() override;

  void OnMinMaxDistanceChanged( float i_fMin, float i_fMax ) override;

private:

  void FreeInternal();

  // Sounds in this queue are finished playing and pending cleanup
  typedef std::queue<rumUniqueID> FinishedSoundContainer;
  static FinishedSoundContainer s_cFinishedSoundsQueue;

  // Sounds in this list are currently fading out
  typedef std::list<rumFadeInfo> FadeInfoContainer;
  static FadeInfoContainer s_cFadingSoundsList;

  typedef std::unordered_map<rumUniqueID, FMOD::Channel*> ChannelHash;
  static ChannelHash s_cChannelHash;

  static FMOD::System* s_pcSystem;

  static std::mutex s_mtxQueue;

  typedef rumSound super;
};

#endif // _C_SOUND_FMOD_H_

#endif // USE_FMOD
