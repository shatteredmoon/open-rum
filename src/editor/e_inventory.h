// NOTE: This class should be considered ABSTRACT and should not be directly instantiated

#ifndef _E_INVENTORY_H_
#define _E_INVENTORY_H_

#include <u_inventory.h>


class EditorInventory : public rumInventory
{
public:

  static void ScriptBind();
};

#endif // _E_INVENTORY_H_
