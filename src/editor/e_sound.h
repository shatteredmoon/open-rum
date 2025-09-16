#ifndef _E_SOUND_H_
#define _E_SOUND_H_

#include <u_sound.h>


class EditorSound : public rumSound
{
public:

  bool Play( float i_fVolume ) override
  {
    return false;
  }

  bool Play3D( const rumPosition& i_rcPos, float i_fVolume ) override
  {
    return false;
  }

  static void ScriptBind();

protected:

  bool InitData() override
  {
    return false;
  }

  void OnMinMaxDistanceChanged( float i_fMin, float i_fMax ) override
  {}

private:

  using super = rumSound;
};

#endif // _E_SOUND_H_
