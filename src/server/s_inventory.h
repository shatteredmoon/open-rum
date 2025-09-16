#ifndef _S_INVENTORY_H_
#define _S_INVENTORY_H_

#include <u_inventory.h>


class rumServerInventory : public rumInventory
{
public:

  int32_t InitFromDB( QueryPtr i_pcQuery, uint32_t i_uiRow ) override;

  bool OnAddedToPawn( rumPawn* i_pcPawn ) override;
  bool OnRemovedFromPawn( rumPawn* i_pcPawn ) override;

  void OnPropertyRemoved( rumAssetID i_ePropertyID ) override;
  void OnPropertyUpdated( rumAssetID i_ePropertyID, Sqrat::Object i_sqValue, bool i_bAdded ) override;

  rumUniqueID GetPersistentID() const
  {
    return m_uiPersistentID;
  }

  void SetPersistentID( rumUniqueID i_uiPersistentID )
  {
    m_uiPersistentID = i_uiPersistentID;
  }

  static void ScriptBind();

protected:

  void AllocateGameID( rumUniqueID i_uiGameID = INVALID_GAME_ID ) override;

private:

  bool Commit( rumUniqueID i_uiPlayerID );
  bool CommitProperty( rumUniqueID i_uiPlayerID, rumAssetID i_ePropertyID, Sqrat::Object i_sqValue, bool i_bAdded );

  bool InitPropertiesFromDB( QueryPtr i_pcQuery );

  rumUniqueID m_uiPersistentID{ INVALID_GAME_ID };

  using super = rumInventory;
};

#endif // _S_INVENTORY_H_
