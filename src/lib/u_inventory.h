// NOTE: This class should be considered ABSTRACT and should not be directly instantiated

#ifndef _U_INVENTORY_H_
#define _U_INVENTORY_H_

#include <u_object.h>
#include <u_pawn.h>
#include <u_structs.h>
#include <u_utility.h>

#define SCRIPT_INVENTORY_NATIVE_CLASS "rumInventory"


class rumInventory : public rumGameObject
{
public:

  ~rumInventory();

  void Free() override;

  rumUniqueID GetOwner() const
  {
    return m_uiPawnID;
  }

  virtual int32_t InitFromDB( QueryPtr i_pcQuery, uint32_t i_uiRow )
  {
    return RESULT_SUCCESS;
  }

  void SetOwner( rumUniqueID i_uiGameID )
  {
    m_uiPawnID = i_uiGameID;
  }

  void OnCreated() override;

  static rumInventory* Fetch( rumUniqueID i_uiGameID );
  static Sqrat::Object FetchVM( rumUniqueID i_uiGameID );

  virtual bool OnAddedToPawn( rumPawn* i_pcPawn );
  virtual bool OnRemovedFromPawn( rumPawn* i_pcPawn );

  static void Init()
  {}

  static void ScriptBind();
  static void Shutdown();

protected:

  void Manage() override;
  void Unmanage() override;

private:

  void FreeInternal()
  {}

  typedef std::unordered_map< rumUniqueID, rumInventory* > InventoryHash;
  static InventoryHash s_hashInventory;

  // The script defined override for this class
  static Sqrat::Object s_sqClass;

  // Pawn that owns this item
  rumUniqueID m_uiPawnID{ INVALID_GAME_ID };

  using super = rumGameObject;
};

#endif // _U_INVENTORY_H_
