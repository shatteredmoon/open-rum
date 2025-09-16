#ifndef _U_BROADCAST_H_
#define _U_BROADCAST_H_

#include <u_broadcast_asset.h>
#include <u_object.h>
#include <u_script.h>
#include <u_structs.h>

#define SCRIPT_BROADCAST_NATIVE_CLASS "rumBroadcast"


class rumBroadcast : public rumGameObject
{
public:

  ~rumBroadcast();

  static void Init()
  {}

  static void ScriptBind();
  static void Shutdown();

protected:

  void AllocateGameID( rumUniqueID i_uiGameID = INVALID_GAME_ID ) override;

  void Manage() override;
  void Unmanage() override;

  typedef std::unordered_map< rumUniqueID, rumBroadcast* > BroadcastHash;
  static BroadcastHash s_hashBroadcasts;

private:

  static Sqrat::Object s_sqClass;
};

#endif // _U_BROADCAST_H_
