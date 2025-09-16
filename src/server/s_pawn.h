#ifndef _S_PAWN_H_
#define _S_PAWN_H_

#include <s_astar.h>
#include <network/u_network.h>
#include <u_pawn.h>


class rumServerPawn : public rumPawn
{
public:

  ~rumServerPawn() override;

  // Packet builders
  static bool PackageAttributes( const rumPawn* i_pcPawn, rumNetwork::rumOutboundPacket& o_rcPacket );
  static bool PackageGlobalProperties( const rumPawn* i_pcPawn, rumNetwork::rumOutboundPacket& o_rcPacket );
  static bool PackageRegionalProperties( const rumPawn* i_pcPawn, rumNetwork::rumOutboundPacket& o_rcPacket );

  // Plots a path using A* from the pawn's current position to a specified target position
  bool FindPath( const rumPosition& i_rcPos, uint32_t i_uiMaxDistance, rumDirectionType i_eDirs );

  bool FindPath( const rumPosition& i_rcPos, uint32_t i_uiMaxDistance )
  {
    return FindPath( i_rcPos, i_uiMaxDistance, rumDirectionType::Intercardinal_DirectionType );
  }

  // Deletes any existing A* path
  void ForgetPath();

  // Returns the current position set along the A* path
  rumPosition GetPathPosition() const;

  // Returns the current target position set along the A* path
  rumPosition GetPathTargetPosition() const;

  rumUniqueID GetPersistentID() const
  {
    return m_iPersistentID;
  }

  // Returns true if an A* path was successfully plotted
  bool HasPath() const;

  int32_t InitFromPlayerDB( QueryPtr i_pcQuery ) override;

  // Advances the stored path to the next available position, and returns the new position
  rumPosition PopPathPosition();

  void SetMapID( rumUniqueID i_eMapID, const rumPosition& i_rcStartingPos ) override;

  // Sets the position without checking for map collisions
  // TODO: this does not remove pawn from the map position! Therefore, this function should only ever be called from a
  // map object!
  int32_t SetPos( const rumPosition& i_rcPos ) override;

  void OnPropertyRemoved( rumAssetID i_ePropertyID ) override;
  void OnPropertyUpdated( rumAssetID i_ePropertyID, Sqrat::Object i_sqValue, bool i_bAdded ) override;

  // Modifies and replicates this pawn's light range to other clients (engine-side regional property)
  void SetLightRange( uint32_t i_uiRange ) override;

  // Modifies and replicates this pawn's collision flags (engine-side regional property)
  void SetCollisionFlags( uint32_t i_uiCollisionFlags ) override;

  // Modifies and replicates this pawn's move type (engine-side regional property)
  void SetMoveType( uint32_t i_uiMoveType ) override;

  // Modifies and replicates this pawn's state to other clients (engine-side regional property)
  void SetState( int32_t i_iState ) override;

  // Modifies and replicates this pawn's visibility to other clients (engine-side regional property)
  void SetVisibility( bool i_bVisible ) override;

  // Returns a valid SOCKET if this pawn is controlled by a player
  SOCKET GetSocket() const;

  static void ScriptBind();
  int32_t ScriptSendPacket( Sqrat::Object i_sqInstance );

private:

  bool InitPropertiesFromDB( QueryPtr i_pcQuery );

  bool PopPacket();

  int32_t Serialize( rumResource& io_rcResource ) final;

  struct PacketInfo
  {
    // For priority queue comparator
    bool operator<( const PacketInfo& i_rcPacketInfo ) const
    {
      return( m_iPriority < i_rcPacketInfo.m_iPriority );
    }

    rumAssetID m_ePropertyID{ INVALID_ASSET_ID };
    int32_t m_iPriority{ 0 };
    Sqrat::Object m_sqObject;
  };

  rumAstarPath *m_pcAstarPath{ nullptr };

  rumUniqueID m_iPersistentID{ INVALID_GAME_ID };

  using super = rumPawn;
};


// These classes exist only so that we can provide a Pawn Type without requiring end users to properly provide them in
// their scripts. Once they extend a script class from a native class, the type is embedded here upon construction
// automatically.

class rumServerCreature : public rumServerPawn
{
public:

  PawnType GetPawnType() const override
  {
    return rumPawn::Creature_PawnType;
  }

protected:

  void AllocateGameID( rumUniqueID i_uiGameID = INVALID_GAME_ID ) override;
};


class rumServerPortal : public rumServerPawn
{
public:

  PawnType GetPawnType() const override
  {
    return rumPawn::Portal_PawnType;
  }

protected:

  void AllocateGameID( rumUniqueID i_uiGameID = INVALID_GAME_ID ) override;
};


class rumServerWidget : public rumServerPawn
{
public:

  PawnType GetPawnType() const override
  {
    return rumPawn::Widget_PawnType;
  }

protected:

  void AllocateGameID( rumUniqueID i_uiGameID = INVALID_GAME_ID ) override;

private:

  // When true, this widget will not be replicated to the client
  bool m_bServerOnly{ false };
};

#endif // _S_PAWN_H_
