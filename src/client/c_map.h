#ifndef _C_MAP_H_
#define _C_MAP_H_

#include <u_map.h>
#include <set>
#include <unordered_map>

namespace rumNetwork
{
  class rumInboundPacket;
}

class RayCaster;
class rumPlayer;


struct MapDrawProps
{
  bool LineOfSight{ true };
  bool m_bUseLighting{ false };
  bool ScaleTiles{ false };
  uint32_t HorizontalTiles{ 11 };
  uint32_t VerticalTiles{ 11 };
  uint32_t HorizontalTileOffset{ 32 };
  uint32_t VerticalTileOffset{ 32 };
  float TileHorizontalScale{ 1.f };
  float TileVerticalScale{ 1.f };
};


class rumClientMap : public rumMap
{
public:
  ~rumClientMap() override;

  bool AddPawn( rumPawn* i_pcPawn, const rumPosition& i_rcPos ) override;

  void Free() override;

  // Screen position to draw top-left corner of map and tile position to center on
  void Draw( int32_t i_iScreenX, int32_t i_iScreenY, int32_t i_iTileX, int32_t i_iTileY );

  inline void Draw( const rumPoint& i_rcScreen, int32_t i_iTileX, int32_t i_iTileY )
  {
    Draw( i_rcScreen.m_iX, i_rcScreen.m_iY, i_iTileX, i_iTileY );
  }

  inline void Draw( int32_t i_iScreenX, int32_t i_iScreenY, const rumPoint& i_rcTile )
  {
    Draw( i_iScreenX, i_iScreenY, i_rcTile.m_iX, i_rcTile.m_iY );
  }

  inline void Draw( const rumPoint& i_rcScreen, const rumPoint& i_rcTile )
  {
    Draw( i_rcScreen.m_iX, i_rcScreen.m_iY, i_rcTile.m_iX, i_rcTile.m_iY );
  }

  void DrawLOS( int32_t i_iScreenX, int32_t i_iScreenY, int32_t i_iTileX, int32_t i_iTileY );

  inline void DrawLOS( const rumPoint& i_rcScreen, int32_t i_iTileX, int32_t i_iTileY )
  {
    DrawLOS( i_rcScreen.m_iX, i_rcScreen.m_iY, i_iTileX, i_iTileY );
  }

  inline void DrawLOS( int32_t i_iScreenX, int32_t i_iScreenY, rumPoint& i_rcTile )
  {
    DrawLOS( i_iScreenX, i_iScreenY, i_rcTile.m_iX, i_rcTile.m_iY );
  }

  inline void DrawLOS( const rumPoint& i_rcScreen, rumPoint& i_rcTile )
  {
    DrawLOS( i_rcScreen.m_iX, i_rcScreen.m_iY, i_rcTile.m_iX, i_rcTile.m_iY );
  }

  bool GenerateLightMap( const rumPosition& i_rcStartPos );
  void InitializeLightMap( uint32_t i_iRows, uint32_t i_iCols );
  void LightArea( const rumPosition& i_rcStartPos, const rumPosition& i_rcLightPos, uint32_t i_uiLightRange );

  bool PositionLit( const rumPoint& i_rcPoint ) const;

  bool PositionLit( int32_t i_iScreenX, int32_t i_iScreenY ) const
  {
    return PositionLit( rumPoint( i_iScreenX, i_iScreenY ) );
  }

  void ResetLightMap();

  // Returns a copy
  MapDrawProps GetDrawProps() const
  {
    return m_cMapDrawProps;
  }

  void ScriptDrawLOS( HSQUIRRELVM i_pcVM );

  void SetDrawProps( const MapDrawProps& i_rcMapDrawProps )
  {
    m_cMapDrawProps = i_rcMapDrawProps;
  }

  static bool ArchiveAdd( const std::string& i_strFile );
  static bool ArchiveRemove( const std::string& i_strFile )
  {
    return ( m_archives.erase( i_strFile ) > 0 );
  }

  static rumClientMap* Fetch( rumUniqueID i_uiGameID )
  {
    return SafeCastNative( super::Fetch( i_uiGameID ) );
  }

  static rumClientMap* SafeCastNative( rumGameObject* i_pcObject )
  {
    return dynamic_cast<rumClientMap*>( i_pcObject );
  }

  static void ScriptBind();

  static int32_t ServerPawnUpdates( rumMap* i_pcMap, const rumNetwork::rumInboundPacket& i_rcPacket );

  typedef std::set<std::string> ArchiveSet;

protected:

  void AllocateGameID( rumUniqueID i_uiGameID = INVALID_GAME_ID ) override;

private:

  void FreeInternal();
  void FreeLightMap();

  int32_t Load() override;

  int32_t Serialize( rumResource& io_rcResource ) final;
  int32_t SerializePawns( rumResource& io_rcResource ) final;

  typedef rumMap super;

  static ArchiveSet m_archives;

  RayCaster *m_pcRayCaster{ nullptr };

  float **m_pcLightMap{ nullptr };
  uint32_t m_iLightMapVerticalTiles{ 0 };
  uint32_t m_iLightMapHorizontalTiles{ 0 };

  MapDrawProps m_cMapDrawProps;

  typedef rumMap super;
};

#endif // _C_MAP_H_
