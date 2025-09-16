#ifndef _U_SOUND_H_
#define _U_SOUND_H_

#include <u_object.h>
#include <u_script.h>
#include <u_sound_asset.h>
#include <u_structs.h>

#include <unordered_map>

#include <iostream>

#define SCRIPT_SOUND_NATIVE_CLASS "rumSound"

#define DEFAULT_SAMPLE_RATE 44100


// Note on the lifetime of sounds: When a sound object is created, scripts can either hold on to the object themselves,
// or more customarily, call Play() or Play3D() on the object. When either of those functions are called, a reference
// to the sound is temporarily held and released again when the sound is finished playing. The callback function
// OnSoundStopped() is called with the script object, and scripts once again have the opportunity to store a reference
// to the sound, or call some variation of Play() to keep the object alive.

class rumSound : public rumGameObject
{
public:

  enum rumSoundState
  {
    Playing_SoundState,
    Paused_SoundState,
    Stopping_SoundState,
    Stopped_SoundState
  };

  ~rumSound();

  // Fetches a sound by its unique playing handle (not the asset id!)
  static rumSound* Fetch( rumUniqueID i_uiGameID );

  void Free() override;

  rumSoundState GetState() const
  {
    return m_eState;
  }

  void SetState( const rumSoundState i_eState );

  virtual float GetVolume() const
  {
    return m_fVolume;
  }

  virtual void SetVolume( float i_fVolume )
  {
    m_fVolume = i_fVolume;
  }

  virtual bool IsPaused();
  virtual bool IsPlaying();
  virtual bool IsStopped();
  virtual bool IsStopping();

  // Plays an ambient sound sample
  virtual bool Play( float i_fVolume );

  bool Play()
  {
    return Play( 1.0f );
  }

  // Plays a sound using attenuation based on distance from the listener. Scripts can specify the location via Vector,
  // which generates a position based on the stored listener offset. Any time integer components are passed, they are
  // interpreted as a Position (not a Vector).
  virtual bool Play3D( const rumPosition& i_rcPos, float i_fVolume );

  bool Play3D( const rumPosition& i_rcPos )
  {
    return Play3D( i_rcPos, 1.0f );
  }

  bool Play3D( int32_t i_iPosX, int32_t i_iPosY, float i_fVolume )
  {
    return Play3D( rumPosition( i_iPosX, i_iPosY ), i_fVolume );
  }

  bool Play3D( int32_t i_iPosX, int32_t i_iPosY )
  {
    return Play3D( rumPosition( i_iPosX, i_iPosY ), 1.0f );
  }

  bool Play3DOnListener( const rumVector& i_rcVector, float i_fVolume )
  {
    return Play3D( GetListenerPosition() + i_rcVector, i_fVolume );
  }

  bool Play3DOnListener( const rumVector& i_rcVector )
  {
    return Play3D( GetListenerPosition() + i_rcVector, 1.0f );
  }

  // Stops the sound with a fade (in seconds)
  virtual void Stop( float i_fFadeTime );

  void Stop()
  {
    Stop( 0.f );
  }

  static rumPosition GetListenerPosition()
  {
    return s_posListener;
  }

  static void Init();

  static void OnAssetDataChanged( const rumSoundAsset& i_rcAsset );

  static void SetListenerPosition( const rumPosition& i_rcPos )
  {
    s_posListener = i_rcPos;
  }

  static void PrintActiveSounds();

  static void Shutdown();

  static void ScriptBind();

  static void SetMinMaxSoundDistance( float i_fMin, float i_fMax );

  // Stops all sounds with a fade (in seconds)
  static void StopAll( float i_fFadeTime );

  static void StopAll()
  {
    StopAll( 0.f );
  }

  // Stops all sounds of a given type with a fade (in seconds)
  static void StopAllOfType( rumAssetID i_eAssetID, float i_fFadeTime );

  static void StopAllOfType( rumAssetID i_eAssetID )
  {
    StopAllOfType( i_eAssetID, 0.f );
  }

private:

  void FreeInternal();

  void Manage() override;
  void Unmanage() override;

  virtual void OnMinMaxDistanceChanged( float i_fMin, float i_fMax ) = 0;

protected:

  void AllocateGameID( rumUniqueID i_uiGameID = INVALID_GAME_ID ) override;

  // Init data from buffer
  virtual bool InitData() = 0;

  void OnCreated() override;

  // Remove a sound instance from the SoundHash
  static void Remove( rumUniqueID i_uiGameID );

private:

  // The script defined override for this class
  static Sqrat::Object s_sqClass;

  // Hash of playing sounds
  typedef std::unordered_map<rumUniqueID, rumSound*> SoundHash;
  static SoundHash s_cSoundHash;

protected:

  static bool s_bInitialized;

  // Listener coordinates for 3D sounds
  static rumPosition s_posListener;

  static float s_fMaxSoundDistance;
  static float s_fMinSoundDistance;

  // The actual sound data
  void* m_pcData{ nullptr };

  float m_fVolume{ 0.f };

  rumSoundAssetAttributes m_cAssetAttributes;

private:

  rumSoundState m_eState{ Stopped_SoundState };

  using super = rumGameObject;
};

#endif // _U_SOUND_H_
