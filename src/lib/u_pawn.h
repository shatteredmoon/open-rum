//NOTE: This class should be considered ABSTRACT and should not be directly instantiated

#ifndef _U_PAWN_H_
#define _U_PAWN_H_

#include <u_pawn.h>

#include <u_map.h>
#include <u_object.h>
#include <u_rum.h>
#include <u_structs.h>

#include <list>
#include <set>
#include <unordered_map>

class Resource;
class rumInventory;
class rumInventoryIterator;
class rumMap;
class rumPlayer;

#define SCRIPT_CREATURE_NATIVE_CLASS  "rumCreature"
#define SCRIPT_PORTAL_NATIVE_CLASS    "rumPortal"
#define SCRIPT_WIDGET_NATIVE_CLASS    "rumWidget"

typedef std::unordered_set<rumUniqueID> InventoryContainer;


class rumPawn : public rumGameObject
{
public:

  enum PawnType { Creature_PawnType, Portal_PawnType, Widget_PawnType, NumPawnTypes };

  ~rumPawn();

  void Free() override;

  bool GetBlocksLOS() const
  {
    return m_bBlocksLOS;
  }

  void SetBlocksLOS( bool i_bBlocksLOS )
  {
    m_bBlocksLOS = i_bBlocksLOS;
  }

  virtual float GetDrawOrder() const
  {
    return 0.f;
  }

  virtual PawnType GetPawnType() const = 0;

  const std::string& GetPlayerName() const;

  uint32_t GetNumInventoryItems() const
  {
    return (uint32_t)m_hashInventory.size();
  }

  bool AddInventory( rumInventory* i_pcInventory );
  bool AddInventoryAsset( rumAssetID i_eAssetID );

  // Removes the item and frees it from memory
  bool DeleteInventory( rumInventory* i_pcInventory );
  bool DeleteInventoryID( rumUniqueID i_uiInventoryID );

  rumInventory* FetchInventory( rumUniqueID i_uiInventoryID );
  rumInventoryIterator FetchInventoryIter();
  Sqrat::Object FetchInventoryVM( rumUniqueID i_uiInventoryID );

  // Moves an item from one pawn to another
  bool TransferInventory( rumInventory* i_pcInventory, rumPawn* i_pcTargetPawn );
  bool TransferInventoryID( rumUniqueID i_uiInventoryID, rumPawn* i_pcTargetPawn );

  uint32_t GetLightRange() const
  {
    return m_uiLightRange;
  }

  virtual void SetLightRange( uint32_t i_uiRange );

  int32_t GetPosX() const
  {
    return m_cPos.m_iX;
  }

  int32_t GetPosY() const
  {
    return m_cPos.m_iY;
  }

  rumPosition GetPos() const
  {
    return m_cPos;
  }

  int32_t SetPos( int32_t i_iPosX, int32_t i_iPosY )
  {
    return SetPos( rumPosition( i_iPosX, i_iPosY ) );
  }

  virtual int32_t SetPos( const rumPosition& i_rcPos );

  bool IsCollision( uint32_t i_uiMoveFlags )
  {
    return ( m_uiCollisionFlags & i_uiMoveFlags ) != 0;
  }

  bool IsVisible() const
  {
    return m_bVisible;
  }

  virtual void SetVisibility( bool i_bVisible );

  uint32_t GetCollisionFlags() const
  {
    return m_uiCollisionFlags;
  }

  virtual void SetCollisionFlags( uint32_t i_uiCollisionFlags )
  {
    m_uiCollisionFlags = i_uiCollisionFlags;
  }

  uint32_t GetMoveType() const
  {
    return m_uiMoveType;
  }

  virtual void SetMoveType( uint32_t i_uiMoveType )
  {
    m_uiMoveType = i_uiMoveType;
  }

  virtual void SetState( int32_t i_iState );
  int32_t GetState() const
  {
    return m_iState;
  }

  rumMap* GetMap() const
  {
    return rumMap::Fetch( m_uiMapID );
  }

  Sqrat::Object GetMap_VM() const;

  rumUniqueID GetMapID() const
  {
    return m_uiMapID;
  }

  virtual void SetMapID( rumUniqueID i_uiMapID, const rumPosition& i_rcStartingPos )
  {
    m_uiMapID = i_uiMapID;
    m_cPos = i_rcStartingPos;
  }

  virtual int32_t InitFromMapDB( QueryPtr i_pcQuery, uint32_t i_uiRow );
  virtual int32_t InitFromPlayerDB( QueryPtr i_pcQuery )
  {
    return RESULT_SUCCESS;
  }

  bool IsPlayer() const
  {
    return m_uiPlayerID != INVALID_GAME_ID;
  }

  rumUniqueID GetPlayerID() const
  {
    return m_uiPlayerID;
  }

  void SetPlayerID( rumUniqueID i_uiPlayerID )
  {
    m_uiPlayerID = i_uiPlayerID;
  }

  // Returns the nearest player within an optional radius and whether or not the player is within direct line of
  // sight of the starting position
  rumPlayer* GetNearestPlayer( uint32_t i_uiMaxDistance, rumDirectionType i_eDir, bool i_bCheckLOS );
  rumPlayer* GetNearestPlayer( uint32_t i_uiMaxDistance, rumDirectionType i_eDir );
  rumPlayer* GetNearestPlayer( uint32_t i_uiMaxDistance );
  rumPlayer* GetNearestPlayer();

  // TODO - Probably shouldn't be public, but really don't want to make all of rumMap a friend class
  virtual int32_t Serialize( rumResource& io_rcResource );

  static rumPawn* Fetch( rumUniqueID i_uiGameID );
  static Sqrat::Object FetchVM( rumUniqueID i_uiGameID );

  static void Init()
  {}

  static void Shutdown();

  static void ScriptBind();

  static void Update();

protected:

  void Manage() override;
  void Unmanage() override;

  void OnCreated() override;

private:

  void FreeInternal();

  Sqrat::Object ScriptInstanceRelease() override;

protected:

  typedef std::unordered_map< rumUniqueID, rumPawn* > PawnHash;
  static PawnHash s_hashPawns;

private:

  rumPosition m_cPos;

  // The pawn's inventory container
  InventoryContainer m_hashInventory;

  rumUniqueID m_uiPlayerID{ INVALID_GAME_ID };
  rumUniqueID m_uiMapID{ INVALID_GAME_ID };

  uint32_t m_uiMoveType{ 0 };
  uint32_t m_uiCollisionFlags{ 0 };
  uint32_t m_uiLightRange{ 0 };

  int32_t m_iState{ 0 };

  bool m_bVisible{ true };
  bool m_bBlocksLOS{ false };

  using super = rumGameObject;
};

#endif // _U_PAWN_H_
