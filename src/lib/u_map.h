#ifndef _U_MAP_H_
#define _U_MAP_H_

#include <u_enum.h>
#include <u_object.h>
#include <u_structs.h>
#include <u_utility.h>

#include <list>
#include <string>
#include <unordered_map>

namespace rumNetwork
{
  class rumOutboundPacket;
}

class rumPawn;
class rumPlayer;
class rumPositionIterator;
class rumResource;

#define SCRIPT_MAP_NATIVE_CLASS "rumMap"


class rumMap : public rumGameObject
{
public:

  ~rumMap();

  struct PawnData
  {
    PawnData( float i_fDrawOrder, rumUniqueID i_iGameID )
      : m_fDrawOrder( i_fDrawOrder )
      , m_iPawnID( i_iGameID )
    {}

    // Draw order designates drawing priority or z-order. The lower the number, higher the pawn will be on the stack
    // of items in the cell, and therefore closer to the viewer.
    float m_fDrawOrder;

    // Look up pawns by their id
    rumUniqueID m_iPawnID;
  };

  typedef std::unordered_map< rumUniqueID, rumMap* > MapHash;
  typedef std::list<PawnData> PawnDataList;

  // A specific coordinate of the map that is represented by a base tile and a list of all pawns located there
  struct PositionData
  {
    bool InsertPawn( rumPawn* i_pcPawn );
    bool RemovePawn( rumPawn* i_pcPawn );

    void ShiftPawnsUp();
    void ShiftPawnsDown();

  private:
    // Disallow copy by assignment
    PositionData* operator=( const PositionData* );

  public:
    rumAssetID m_eTileID;
    PawnDataList m_cPawnDataList;
  };

  bool IsLoaded() const
  {
    return m_pcData != nullptr;
  }

  // Returns base tile and list of all objects on a particular map location
  // For a non-const version of this call, see AccessPositionData
  const PositionData* GetPositionData( const rumPosition& i_rcPos ) const
  {
    return AccessPositionData( i_rcPos );
  }

  rumPositionIterator GetPositionIterator( const rumPosition& i_rcPos ) const;
  rumPositionIterator GetPositionIterator( int32_t x, int32_t y ) const;

  void WrapPosition( rumPosition& io_rcPos ) const;
  rumPosition::PositionValidationEnum ValidatePosition( rumPosition& io_rcPos ) const;

  bool Wraps() const
  {
    return m_bWraps;
  }

  // Checks a position to see if it lies within the map bounds (without applying wrapping).
  // Returns true if the position is within the map boundary, false otherwise.
  bool IsValidPos( const rumPosition& i_rcPos ) const
  {
    return( i_rcPos.m_iX >= 0 &&
            i_rcPos.m_iX < (int32_t)m_uiCols &&
            i_rcPos.m_iY >= 0 &&
            i_rcPos.m_iY < (int32_t)m_uiRows );
  }

  // Returns base tile type at a particular map location
  rumAssetID GetTileID( const rumPosition& i_rcPos ) const
  {
    if( IsValidPos( i_rcPos ) )
    {
      return m_pcData[i_rcPos.m_iY][i_rcPos.m_iX].m_eTileID;
    }

    return 0;
  }

  rumAssetID GetTileID( int32_t i_iX, int32_t i_iY ) const
  {
    return GetTileID( rumPosition( i_iX, i_iY ) );
  }

  bool SetTileID( const rumPosition& i_rcPos, rumAssetID i_eTileID )
  {
    if( IsValidPos( i_rcPos ) )
    {
      m_pcData[i_rcPos.m_iY][i_rcPos.m_iX].m_eTileID = i_eTileID;
      return true;
    }

    return false;
  }

  bool SetTileID( int32_t i_iX, int32_t i_iY, rumAssetID i_eTileID )
  {
    return SetTileID( { i_iX, i_iY }, i_eTileID );
  }

  bool PositionBlocksSight( int32_t i_iX, int32_t i_iY ) const
  {
    return PositionBlocksSight( rumPosition( i_iX, i_iY ) );
  }

  bool PositionBlocksSight( const rumPosition& i_rcPos ) const;

  uint32_t GetCols() const
  {
    return m_uiCols;
  }

  uint32_t GetRows() const
  {
    return m_uiRows;
  }

  rumAssetID GetExitMapID() const
  {
    return m_eExitMapID;
  }

  const rumPosition& GetExitPos() const
  {
    return m_cExitPos;
  }

  void SetExitMapID( rumAssetID i_eAssetID )
  {
    m_eExitMapID = i_eAssetID;
  }

  void SetExitPos( const rumPosition& i_rcPos )
  {
    m_cExitPos = i_rcPos;
  }

  bool HasClearPath( const rumPosition& i_rcOrigin, const rumPosition& i_rcTarget, rumMoveFlags i_uiMovementFlags ) const;

  // Determines if the target position is within plain view of a starting position within a given distance
  bool HasLineOfSight( const rumPosition& i_rcOrigin, const rumPosition& i_rcTarget, uint32_t i_uiMaxTileDistance,
                       rumDirectionType i_eDir ) const;

  bool HasLineOfSight( const rumPosition& i_rcOrigin, const rumPosition& i_rcTarget, uint32_t i_uiMaxTileDistance ) const
  {
    return HasLineOfSight( i_rcOrigin, i_rcTarget, i_uiMaxTileDistance, Intercardinal_DirectionType );
  }

  bool HasLineOfSight( const rumPosition& i_rcOrigin, const rumPosition& i_rcTarget ) const
  {
    return HasLineOfSight( i_rcOrigin, i_rcTarget, UINT_MAX, Intercardinal_DirectionType );
  }

  rumCollisionType IsCollision( const rumPosition& i_rcPos, rumPawn* i_pcPawn, rumMoveFlags i_uiMovementFlags ) const;
  bool IsHarmful( const rumPosition& i_rcPos, rumPawn* i_pcPawn ) const;

  virtual bool AddPawn( rumPawn* i_pcPawn, const rumPosition& i_rcPos );
  bool AddPawn( rumUniqueID i_uiPawnID, const rumPosition& i_rcPos );

  // Adds a reference to the script object
  bool AddPawnVM( Sqrat::Object i_sqObject, const rumPosition& i_rcPos );

  rumAssetID GetBorderTile() const
  {
    return m_eBorderTileID;
  }

  void SetBorderTile( rumAssetID i_eTileID )
  {
    m_eBorderTileID = i_eTileID;
  }

  virtual bool RemovePawn( rumPawn* i_pcPawn );

  // Releases the managed script reference
  bool RemovePawnVM( Sqrat::Object i_sqObject );

  // Moves pawn to a new position, checking collision and valid movement only when bChecked is true. The position is
  // only updated if bPreview is set to false.
  virtual rumMoveResultType MovePawn( rumPawn* i_pcPawn, const rumPosition& i_rcPos, rumMoveFlags i_uiMovementFlags,
                                      uint32_t uiTileDistance );

  rumMoveResultType MovePawn( rumPawn* i_pcPawn, const rumPosition& i_rcPos )
  {
    constexpr uint32_t uiTileDistance{ 1 };
    return MovePawn( i_pcPawn, i_rcPos, Default_MoveFlag, uiTileDistance );
  }

  rumMoveResultType MovePawn( rumPawn* pcPawn, const rumPosition& i_rcPos, rumMoveFlags i_uiMovementFlags )
  {
    constexpr uint32_t uiTileDistance{ 1 };
    return MovePawn( pcPawn, i_rcPos, i_uiMovementFlags, uiTileDistance );
  }

  rumMoveResultType MovePawn( rumPawn* i_pcPawn, int32_t i_iX, int32_t i_iY, rumMoveFlags i_uiMovementFlags,
                              uint32_t i_uiTileDistance )
  {
    return MovePawn( i_pcPawn, rumPosition( i_iX, i_iY ), i_uiMovementFlags, i_uiTileDistance );
  }

  rumMoveResultType OffsetPawn( rumPawn* i_pcPawn, const rumVector& i_vOffset, rumMoveFlags i_uiMovementFlags,
                                uint32_t i_uiTileDistance );

  rumMoveResultType OffsetPawn( rumPawn* i_pcPawn, const rumVector& i_vOffset, rumMoveFlags i_uiMovementFlags )
  {
    constexpr uint32_t uiTileDistance{ 1 };
    return OffsetPawn( i_pcPawn, i_vOffset, i_uiMovementFlags, uiTileDistance );
  }

  rumMoveResultType OffsetPawn( rumPawn* i_pcPawn, const rumVector& i_vOffset )
  {
    constexpr uint32_t uiTileDistance{ 1 };
    return OffsetPawn( i_pcPawn, i_vOffset, Default_MoveFlag, uiTileDistance );
  }

  bool TransferPawn( rumPawn* i_pcPawn, rumMap* i_pcMap, const rumPosition& i_rcPos, bool i_bForce );

  bool TransferPawn( rumPawn* i_pcPawn, rumMap* i_pcMap, const rumPosition& i_rcPos )
  {
    return TransferPawn( i_pcPawn, i_pcMap, i_rcPos, false /* do not force */ );
  }

  // Returns the real distance (using the distance formula) between two map positions, taking map wrapping into
  // consideration
  float GetDistance( const rumPosition& i_rcOrigin, const rumPosition& rTarget ) const;

  // Returns the number of tiles between two map positions, taking map wrapping and allowed movement directions into
  // consideration
  uint32_t GetTileDistance( const rumPosition& i_rcOrigin, const rumPosition& rTarget, rumDirectionType eDirs ) const;

  uint32_t GetTileDistance( const rumPosition& i_rcOrigin, const rumPosition& rTarget ) const
  {
    return GetTileDistance( i_rcOrigin, rTarget, Intercardinal_DirectionType );
  }

  // Return true if the position is within the maps bounds
  bool IsWithinBounds( const rumPosition& i_rcPos ) const;

  // Returns true if a target position is within the real radius (using the distance formula) of a map position,
  // taking map wrapping into consideration
  bool IsWithinRadius( const rumPosition& i_rcOrigin, const rumPosition& i_rcTarget, float i_fRadius ) const;

  // Returns true if a target position is within the specified amount of tiles from the origin position, taking map
  // wrapping into consideration
  bool IsWithinTileDistance( const rumPosition& i_rcOrigin, const rumPosition& i_rcTarget,
                             uint32_t i_uiMaxTileDistance, const rumDirectionType i_eDir ) const;

  // Script entry method for IsWithinTileDistance
  bool IsWithinTileDistance( const rumPosition& i_rcOrigin, const rumPosition& i_rcTarget,
                             uint32_t i_uiMaxTileDistance ) const
  {
    return IsWithinTileDistance( i_rcOrigin, i_rcTarget, i_uiMaxTileDistance, Intercardinal_DirectionType );
  }

  const GameIDHash& GetPlayers() const
  {
    return m_hashPlayers;
  }

  // Returns the nearest player within an optional distance and whether or not the player is within direct line of
  // sight of the starting position
  rumPlayer* GetNearestPlayer( const rumPosition& i_rcPos, uint32_t i_uiMaxTileDistance, rumDirectionType i_eDir,
                               bool i_bCheckLOS ) const;

  rumPlayer* GetNearestPlayer( const rumPosition& i_rcPos ) const
  {
    return GetNearestPlayer( i_rcPos, INT32_MAX, Intercardinal_DirectionType, false );
  }

  rumPlayer* GetNearestPlayer( const rumPosition& i_rcPos, uint32_t i_uiMaxTileDistance ) const
  {
    return GetNearestPlayer( i_rcPos, i_uiMaxTileDistance, Intercardinal_DirectionType, false );
  }

  rumPlayer* GetNearestPlayer( const rumPosition& i_rcPos, uint32_t i_uiMaxTileDistance,
                               rumDirectionType i_eDir ) const
  {
    return GetNearestPlayer( i_rcPos, i_uiMaxTileDistance, i_eDir, false );
  }

  Sqrat::Object ScriptGetAllPawns();
  Sqrat::Object ScriptGetPawns( const rumPosition& i_rcOrigin, const uint32_t i_uiMaxTileDistance,
                                const bool i_bCheckLOS );

  Sqrat::Object ScriptGetAllPlayers();
  Sqrat::Object ScriptGetPlayers( const rumPosition& i_rcOrigin, const uint32_t i_uiMaxTileDistance,
                                  const bool i_bCheckLOS );

  uint32_t GetNumPlayers() const
  {
    return (uint32_t)m_hashPlayers.size();
  }

  rumVector GetDirectionVector( const rumPosition& i_rcOrigin, const rumPosition& i_rcTarget );

  bool IsPawnOnMap( rumUniqueID i_uiGameID )
  {
    return m_hashPawns.find( i_uiGameID ) != m_hashPawns.end();
  }

  virtual int32_t Load();

  static rumMap* Fetch( rumUniqueID i_uiGameID );

  static Sqrat::Object Fetch_VM( rumUniqueID i_uiGameID )
  {
    rumMap* pcMap{ Fetch( i_uiGameID ) };
    return pcMap ? pcMap->GetScriptInstance() : Sqrat::Object();
  }

  static const MapHash& GetMaps()
  {
    return s_hashMaps;
  }

  static void Init()
  {}

  static void ScriptBind();
  static void Shutdown();

protected:

  // Deallocates the map
  void Free() override;
  void FreeCellData();

  // Returns base tile and list of all objects on a particular map location
  // For a const version of this call, see GetPositionData
  PositionData* AccessPositionData( const rumPosition& i_rcPos ) const;

  bool IsPawnCollision( const rumPosition& i_rcPos, rumPawn* i_pcPawn ) const;

  bool IsTileCollision( const rumPosition& i_rcPos, uint32_t i_uiMoveFlags ) const;
  bool IsTileCollision( const rumPosition& i_rcPos, rumPawn* i_pcPawn ) const;

  void Manage() override;
  void Unmanage() override;

  // Called when a map is created
  void OnCreated() override;

  virtual void OnLoaded();

  virtual int32_t AddPlayer( rumPlayer* i_pcPlayer );
  virtual int32_t RemovePlayer( rumPlayer* i_pcPlayer );

  virtual int32_t Serialize( rumResource& io_rcResource );
  virtual int32_t SerializePawns( rumResource& io_rcResource );

private:

  // Appends all 8 possible combinations of the provided position shifted off of the map in each direction
  void AppendWrappedPositions( const rumPosition& i_rcPos, std::vector<rumPosition>& i_rcPositions ) const;

  void FreeInternal();

  // The internal versions do not account for map wrapping
  bool IsWithinRadiusInternal( const rumPosition& i_rcOrigin, const rumPosition& i_rcTarget, float i_fRadius ) const;

  bool HasClearPathInternal( const rumPosition& i_rcOrigin, const rumPosition& i_rcTarget,
                             rumMoveFlags i_uiMovementFlags ) const;

  // The internal version does not account for map wrapping and makes no attempt to validate positions
  bool HasLineOfSightInternal( const rumPosition& i_rcOrigin, const rumPosition& i_rcTarget ) const;

  // The internal versions do not account for map wrapping and make no attempt to validate positions
  bool IsWithinTileDistanceInternal( const rumPosition& i_rcOrigin, const rumPosition& i_rcTarget,
                                     uint32_t i_uiTileDistance, rumDirectionType i_eDir ) const;

protected:

  //static const std::string FOLDER;
  static constexpr uint32_t s_uiMaxCols{ 0xffff };
  static constexpr uint32_t s_uiMaxRows{ 0xffff };

  static Sqrat::Object s_sqClass;

  typedef std::unordered_map< rumUniqueID, rumMap* > MapHash;
  static MapHash s_hashMaps;

  uint32_t m_uiRows{ 0 };
  uint32_t m_uiCols{ 0 };
  uint32_t m_uiMapSize{ 0 };

  bool m_bWraps{ false };

  rumAssetID m_eExitMapID{ INVALID_GAME_ID };
  rumPosition m_cExitPos{ 0, 0 };

  PositionData **m_pcData{ nullptr };

  // The tile represented around the boundary of the map
  rumAssetID m_eBorderTileID;

  // Set of pawn IDs existing on this map
  GameIDHash m_hashPawns;

  // Hash of player IDs representing players actively using this map
  GameIDHash m_hashPlayers;

private:

  using super = rumGameObject;
};

#endif // _U_MAP_H_
