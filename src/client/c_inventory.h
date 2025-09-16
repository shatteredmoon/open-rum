#ifndef _C_INVENTORY_H_
#define _C_INVENTORY_H_

#include <u_inventory.h>


class rumClientInventory : public rumInventory
{
public:

  static void ScriptBind();

protected:

  void AllocateGameID( rumUniqueID i_uiGameID = INVALID_GAME_ID ) override;
};

#endif // _C_INVENTORY_H_
