#ifndef E_MAP_H
#define E_MAP_H

#include <u_map.h>
#include <u_tile_asset.h>

#include <mainwindow.h>
#include <smMapWidget.h>


struct MapDrawProps
{
  bool m_bScaleTiles{ false };
  uint32_t m_uiHorizontalTiles{ smMapWidget::GetNumColumns() };
  uint32_t m_uiVerticalTiles{ smMapWidget::GetNumRows() };
  float m_fHorizontalTileOffset{ (float)MainWindow::GetTileWidth() };
  float m_fVerticalTileOffset{ (float)MainWindow::GetTileHeight() };
  float m_fTileHorizontalScale{ 1.f };
  float m_fTileVerticalScale{ 1.f };
};


class EditorMap : public rumMap
{
public:
  EditorMap();
  ~EditorMap() override;

  void AddColsLeft( uint32_t i_uiNumCols, rumAssetID i_eTileID );
  void AddColsRight( uint32_t i_uiNumCols, rumAssetID i_eTileID );
  void AddRowsTop( uint32_t i_uiNumRows, rumAssetID i_eTileID );
  void AddRowsBottom( uint32_t i_uiNumRows, rumAssetID i_eTileID );

  void DelColsLeft( uint32_t i_uiNumCols );
  void DelColsRight( uint32_t i_uiNumCols );
  void DelRowsTop( uint32_t i_uiNumRows );
  void DelRowsBottom( uint32_t i_uiNumRows );

  // Always adds and deletes to the right & bottom
  void ResizeMap( uint32_t i_uiCols, uint32_t i_uiRows, rumAssetID i_eTileID );

  void Draw( const rumPosition& i_rcScreenPos, const rumPosition& i_rcTilePos );

  const rumPosition& GetDrawPosition() const
  {
    return m_cDrawPos;
  }

  void SetDrawPosition( const rumPosition& i_rcPos )
  {
    m_cDrawPos = i_rcPos;
  }

  int32_t Resave();
  int32_t Save( const std::string& i_strFilename );

  // Script attempts to move pawns are ignored on editor maps
  virtual int32_t ScriptPawnMove( HSQUIRRELVM i_pcVM )
  {
    return RESULT_FAILED;
  }

  MapDrawProps GetDrawProps() const
  {
    return m_cMapAttributes;
  }

  void SetDrawProps( const MapDrawProps& i_rcMapAttributes )
  {
    m_cMapAttributes = i_rcMapAttributes;
  }

  int32_t GetMapID() const
  {
    return m_uiEditorMapID;
  }

  const GameIDHash& GetPawnHash() const
  {
    return m_hashPawns;
  }

  void ShiftPawnsUp( const rumPosition& i_rcPos );
  void ShiftPawnsDown( const rumPosition& i_rcPos );

  static void ScriptBind();

protected:

  void AllocateGameID( rumUniqueID i_uiGameID = INVALID_GAME_ID ) override;

private:

  // Used during map resizing to adjust pawn positions
  void AddPawnListWithOffset( int32_t i_iX, int32_t i_iY );

  MapDrawProps m_cMapAttributes;

  // The top-left tile drawn on-screen
  rumPosition m_cDrawPos;

  // A temporary ID used only by the editor since the map may not yet have a script class defined
  rumAssetID m_uiEditorMapID;

  // Used for assigning temporary script IDs
  static uint32_t s_uiNextAvailableScriptID;

  using super = rumMap;
};

#endif // E_MAP_H
