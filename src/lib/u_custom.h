#ifndef _U_CUSTOM_H_
#define _U_CUSTOM_H_

#include <u_custom_asset.h>
#include <u_object.h>
#include <u_script.h>
#include <u_structs.h>

#define SCRIPT_CUSTOM_NATIVE_CLASS "rumCustom"


class rumCustom : public rumGameObject
{
public:

  ~rumCustom();

  static rumCustom* Fetch( rumUniqueID i_uiGameID );

  static void Init()
  {}

  static void ScriptBind();
  static void Shutdown();

protected:

  void Manage() override;
  void Unmanage() override;

private:

  // Hash of custom objects
  typedef std::unordered_map<rumUniqueID, rumCustom*> CustomHash;
  static CustomHash s_hashCustomObjects;

  static Sqrat::Object s_sqClass;
};

#endif // _U_CUSTOM_H_
