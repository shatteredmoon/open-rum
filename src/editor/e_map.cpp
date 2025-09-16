#include <e_map.h>

#include <e_graphics_opengl.h>
#include <e_pawn.h>

#include <u_assert.h>
#include <u_log.h>
#include <u_map_asset.h>
#include <u_pos_iterator.h>
#include <u_resource.h>

#include <filesystem>

// Static initializers
uint32_t EditorMap::s_uiNextAvailableScriptID = 0;


EditorMap::EditorMap()
{
  // Set the new map row and column sizes
  m_uiCols = 1;
  m_uiRows = 1;

  m_uiMapSize = m_uiRows * m_uiCols;

  // Allocate map in two dimensions
  m_pcData = new PositionData*[m_uiRows];
  if( m_pcData )
  {
    for( uint32_t i = 0; i < m_uiRows; ++i )
    {
      m_pcData[i] = new PositionData[m_uiCols];
    }
  }

  m_uiEditorMapID = s_uiNextAvailableScriptID++;
}


EditorMap::~EditorMap()
{
  Free();
}


// override
void EditorMap::AllocateGameID( rumUniqueID i_uiGameID )
{
  // Newly created editor maps should not already have a UID
  rumAssertMsg( INVALID_GAME_ID == i_uiGameID, "Creating editor pawn with a non-zero GameID" );

  static rumUniqueID s_uiAssetType{ ( rumUniqueID( rumMapAsset::GetClassRegistryID() ) ) << 60 };
  static rumUniqueID s_uiGameID{ ( rumUniqueID( Neutral_ObjectCreationType ) ) << 56 };

  i_uiGameID = ++s_uiGameID | s_uiAssetType;
  SetGameID( i_uiGameID );
}


void EditorMap::AddColsLeft( uint32_t i_uiNumCols, rumAssetID i_eTileID )
{
  if( GetCols() + i_uiNumCols > 65536 )
  {
    return;
  }

  const uint32_t uiNewColSize{ GetCols() + i_uiNumCols };
  const uint32_t uiNewRowSize{ GetRows() };
  const uint32_t uiNewMapSize{ uiNewColSize * uiNewRowSize };

  // Allocate map in two dimensions
  PositionData** pcNewData{ new PositionData * [uiNewRowSize] };
  if( pcNewData && m_pcData )
  {
    for( uint32_t j = 0; j < uiNewRowSize; ++j )
    {
      pcNewData[j] = new PositionData[uiNewColSize];

      for( uint32_t i = 0; i < uiNewColSize; ++i )
      {
        if( i < i_uiNumCols )
        {
          // New cell, just add the tile class
          pcNewData[j][i].m_eTileID = i_eTileID;
        }
        else
        {
          // Existing cell, copy from existing map data
          pcNewData[j][i].m_eTileID = m_pcData[j][i - i_uiNumCols].m_eTileID;
        }
      }
    }

    // Destroy existing map data - this frees up old allocations
    FreeCellData();

    // Set the new map row and column sizes
    m_uiCols = uiNewColSize;
    m_uiRows = uiNewRowSize;

    // Point to new map cell data
    m_pcData = pcNewData;

    // Add pawns to the new cells at their offset position
    AddPawnListWithOffset( i_uiNumCols, 0 );
  }
}


void EditorMap::AddColsRight( uint32_t i_uiNumCols, rumAssetID i_eTileID )
{
  if( GetCols() + i_uiNumCols > 65536 )
  {
    return;
  }

  const uint32_t uiNewColSize{ GetCols() + i_uiNumCols };
  const uint32_t uiNewRowSize{ GetRows() };
  const uint32_t uiNewMapSize{ uiNewColSize * uiNewRowSize };

  // Allocate map in two dimensions
  PositionData** pcNewData{ new PositionData * [uiNewRowSize] };
  if( pcNewData && m_pcData )
  {
    for( uint32_t j = 0; j < uiNewRowSize; ++j )
    {
      pcNewData[j] = new PositionData[uiNewColSize];

      for( uint32_t i = 0; i < uiNewColSize; ++i )
      {
        if( i >= GetCols() )
        {
          // New cell, just add the tile class
          pcNewData[j][i].m_eTileID = i_eTileID;
        }
        else
        {
          // Existing cell, copy from existing map data
          pcNewData[j][i].m_eTileID = m_pcData[j][i].m_eTileID;
        }
      }
    }

    // Destroy existing map data - this frees up old allocations
    FreeCellData();

    // Set the new map row and column sizes
    m_uiCols = uiNewColSize;
    m_uiRows = uiNewRowSize;

    // Point to new map cell data
    m_pcData = pcNewData;

    // Add pawns to the new cells at their offset position
    AddPawnListWithOffset( 0, 0 );
  }
}


void EditorMap::AddRowsTop( uint32_t i_uiNumRows, rumAssetID i_eTileID )
{
  if( GetRows() + i_uiNumRows > 65536 )
  {
    return;
  }

  const uint32_t uiNewColSize{ GetCols() };
  const uint32_t uiNewRowSize{ GetRows() + i_uiNumRows };
  const uint32_t uiNewMapSize{ uiNewColSize * uiNewRowSize };

  // Allocate map in two dimensions
  PositionData** pcNewData{ new PositionData * [uiNewRowSize] };
  if( pcNewData && m_pcData )
  {
    for( uint32_t j = 0; j < uiNewRowSize; ++j )
    {
      pcNewData[j] = new PositionData[uiNewColSize];

      for( uint32_t i = 0; i < uiNewColSize; ++i )
      {
        if( j < i_uiNumRows )
        {
          // New cell, just add the tile class
          pcNewData[j][i].m_eTileID = i_eTileID;
        }
        else
        {
          // Existing cell, copy from existing map data
          pcNewData[j][i].m_eTileID = m_pcData[j - i_uiNumRows][i].m_eTileID;
        }
      }
    }

    // Destroy existing map data - this frees up old allocations
    FreeCellData();

    // Set the new map row and column sizes
    m_uiCols = uiNewColSize;
    m_uiRows = uiNewRowSize;

    // Point to new map cell data
    m_pcData = pcNewData;

    // Add pawns to the new cells at their offset position
    AddPawnListWithOffset( 0, i_uiNumRows );
  }
}


void EditorMap::AddRowsBottom( uint32_t i_uiNumRows, rumAssetID i_eTileID )
{
  if( GetRows() + i_uiNumRows > 65536 )
  {
    return;
  }

  const uint32_t uiNewColSize{ GetCols() };
  const uint32_t uiNewRowSize{ GetRows() + i_uiNumRows };
  const uint32_t uiNewMapSize{ uiNewColSize * uiNewRowSize };

  // Allocate map in two dimensions
  PositionData** pcNewData{ new PositionData * [uiNewRowSize] };
  if( pcNewData && m_pcData )
  {
    for( uint32_t j = 0; j < uiNewRowSize; ++j )
    {
      pcNewData[j] = new PositionData[uiNewColSize];

      for( uint32_t i = 0; i < uiNewColSize; ++i )
      {
        if( j >= GetRows() )
        {
          // New cell, just add the tile class
          pcNewData[j][i].m_eTileID = i_eTileID;
        }
        else
        {
          // Existing cell, copy from existing map data
          pcNewData[j][i].m_eTileID = m_pcData[j][i].m_eTileID;
        }
      }
    }

    // Destroy existing map data - this frees up old allocations
    FreeCellData();

    // Set the new map row and column sizes
    m_uiCols = uiNewColSize;
    m_uiRows = uiNewRowSize;

    // Point to new map cell data
    m_pcData = pcNewData;

    // Add pawns to the new cells at their offset position
    AddPawnListWithOffset( 0, 0 );
  }
}


void EditorMap::AddPawnListWithOffset( int32_t i_iX, int32_t i_iY )
{
  // Make a copy of the pawn container and clear the original so that set collisions do not occur when the pawns are
  // re-added to the map
  const GameIDHash cHash{ m_hashPawns };
  m_hashPawns.clear();

  // Visit each pawn and offset its position using the deltas provided
  for( const auto& iter : cHash )
  {
    rumPawn* pcPawn{ rumPawn::Fetch( iter ) };
    if( pcPawn )
    {
      const rumPosition cPos( pcPawn->GetPosX() + i_iX, pcPawn->GetPosY() + i_iY );
      if( !AddPawn( pcPawn, cPos ) )
      {
        // This pawn has moved off of the map, destroy it
        pcPawn->Free();
      }
    }
  }
}


void EditorMap::DelColsLeft( uint32_t i_uiNumCols )
{
  if( i_uiNumCols > GetCols() )
  {
    return;
  }

  const uint32_t uiNewColSize{ GetCols() - i_uiNumCols };
  const uint32_t uiNewRowSize{ GetRows() };
  const uint32_t uiNewMapSize{ uiNewColSize * uiNewRowSize };

  // Allocate map in two dimensions
  PositionData** pcNewData{ new PositionData * [uiNewRowSize] };
  if( pcNewData && m_pcData )
  {
    for( uint32_t j = 0; j < uiNewRowSize; ++j )
    {
      pcNewData[j] = new PositionData[uiNewColSize];

      for( uint32_t i = 0; i < uiNewColSize; ++i )
      {
        // Copy from existing map data
        pcNewData[j][i].m_eTileID = m_pcData[j][i + i_uiNumCols].m_eTileID;
      }
    }

    // Destroy existing map data - this frees up old allocations
    FreeCellData();

    // Set the new map row and column sizes
    m_uiCols = uiNewColSize;
    m_uiRows = uiNewRowSize;

    // Point to new map cell data
    m_pcData = pcNewData;

    // Add pawns to the new cells at their offset position
    AddPawnListWithOffset( -( (int32_t)i_uiNumCols ), 0 );
  }
}


void EditorMap::DelColsRight( uint32_t i_uiNumCols )
{
  if( i_uiNumCols > GetCols() )
  {
    return;
  }

  const uint32_t uiNewColSize{ GetCols() - i_uiNumCols };
  const uint32_t uiNewRowSize{ GetRows() };
  const uint32_t uiNewMapSize{ uiNewColSize * uiNewRowSize };

  // Allocate map in two dimensions
  PositionData** pcNewData{ new PositionData * [uiNewRowSize] };
  if( pcNewData && m_pcData )
  {
    for( uint32_t j = 0; j < uiNewRowSize; ++j )
    {
      pcNewData[j] = new PositionData[uiNewColSize];

      for( uint32_t i = 0; i < uiNewColSize; ++i )
      {
        // Copy from existing map data
        pcNewData[j][i].m_eTileID = m_pcData[j][i].m_eTileID;
      }
    }

    // Destroy existing map data - this frees up old allocations
    FreeCellData();

    // Set the new map row and column sizes
    m_uiCols = uiNewColSize;
    m_uiRows = uiNewRowSize;

    // Point to new map cell data
    m_pcData = pcNewData;

    // Add pawns to the new cells at their offset position
    AddPawnListWithOffset( 0, 0 );
  }
}


void EditorMap::DelRowsTop( uint32_t i_uiNumRows )
{
  if( i_uiNumRows > GetRows() )
  {
    return;
  }

  const uint32_t uiNewColSize{ GetCols() };
  const uint32_t uiNewRowSize{ GetRows() - i_uiNumRows };
  const uint32_t uiNewMapSize{ uiNewColSize * uiNewRowSize };

  // Allocate map in two dimensions
  PositionData** pcNewData{ new PositionData * [uiNewRowSize] };
  if( pcNewData && m_pcData )
  {
    for( uint32_t j = 0; j < uiNewRowSize; ++j )
    {
      pcNewData[j] = new PositionData[uiNewColSize];

      for( uint32_t i = 0; i < uiNewColSize; ++i )
      {
        // Copy from existing map data
        pcNewData[j][i].m_eTileID = m_pcData[j + i_uiNumRows][i].m_eTileID;
      }
    }

    // Destroy existing map data - this frees up old allocations
    FreeCellData();

    // Set the new map row and column sizes
    m_uiCols = uiNewColSize;
    m_uiRows = uiNewRowSize;

    // Point to new map cell data
    m_pcData = pcNewData;

    // Add pawns to the new cells at their offset position
    AddPawnListWithOffset( 0, -( (int32_t)i_uiNumRows ) );
  }
}


void EditorMap::DelRowsBottom( uint32_t i_uiNumRows )
{
  if( i_uiNumRows > GetRows() )
  {
    return;
  }

  const uint32_t uiNewColSize{ GetCols() };
  const uint32_t uiNewRowSize{ GetRows() - i_uiNumRows };
  const uint32_t uiNewMapSize{ uiNewColSize * uiNewRowSize };

  // Allocate map in two dimensions
  PositionData** pcNewData{ new PositionData * [uiNewRowSize] };
  if( pcNewData && m_pcData )
  {
    for( uint32_t j = 0; j < uiNewRowSize; ++j )
    {
      pcNewData[j] = new PositionData[uiNewColSize];

      for( uint32_t i = 0; i < uiNewColSize; ++i )
      {
        // Copy from existing map data
        pcNewData[j][i].m_eTileID = m_pcData[j][i].m_eTileID;
      }
    }

    // Destroy existing map data - this frees up old allocations
    FreeCellData();

    // Set the new map row and column sizes
    m_uiCols = uiNewColSize;
    m_uiRows = uiNewRowSize;

    // Point to new map cell data
    m_pcData = pcNewData;

    // Add pawns to the new cells at their offset position
    AddPawnListWithOffset( 0, 0 );
  }
}


void EditorMap::Draw( const rumPosition& i_rcScreenPos, const rumPosition& i_rcTilePos )
{
  /*if (mdp.LineOfSight)
  {
      DrawLOS(screenx, screeny, tilex, tiley);
      return;
  }*/

  Sqrat::Object sqObj;

  // Draw location of current tile
  rumPosition cDrawPos( 0, 0 );

  // The top-left most square to be drawn
  const rumPosition cSrcPos( i_rcTilePos );

  // Current position
  rumPosition cPos( cSrcPos );

  // Draw all map tiles
  for( uint32_t j = 0; j < m_cMapAttributes.m_uiVerticalTiles; ++j, ++cPos.m_iY )
  {
    for( uint32_t i = 0; i < m_cMapAttributes.m_uiHorizontalTiles; ++i, ++cPos.m_iX )
    {
      EditorGraphic::EnableBlending( false );

      cDrawPos.m_iX = static_cast<int32_t>( ( i * m_cMapAttributes.m_fHorizontalTileOffset ) + i_rcScreenPos.m_iX );
      cDrawPos.m_iY = static_cast<int32_t>( ( j * m_cMapAttributes.m_fVerticalTileOffset ) + i_rcScreenPos.m_iY );

      rumAssetID eTileID{ INVALID_ASSET_ID };

      if( cPos.m_iX < 0 || cPos.m_iY < 0 || cPos.m_iX >= (int32_t)GetCols() || cPos.m_iY >= (int32_t)GetRows() )
      {
        eTileID = m_eBorderTileID;
      }
      else
      {
        eTileID = GetTileID( cPos );
      }

      rumAssetID eGraphicID{ INVALID_ASSET_ID };

      const rumTileAsset* pcTile{ rumTileAsset::Fetch( eTileID ) };
      if( pcTile )
      {
        eGraphicID = pcTile->GetGraphicID();
      }

      rumGraphic* pcGraphic{ rumGraphic::Fetch( eGraphicID ) };
      if( pcGraphic )
      {
        // Copy the map draw props to the graphic draw props
        rumGraphicAttributes cGraphicAttributes{ pcGraphic->GetAttributes() };
        const rumGraphicAttributes cAttributeBackup{ cGraphicAttributes };
        cGraphicAttributes.m_bDrawMasked = true;
        if( m_cMapAttributes.m_bScaleTiles )
        {
          cGraphicAttributes.m_bDrawScaled = true;
          cGraphicAttributes.m_fVerticalScale = m_cMapAttributes.m_fTileVerticalScale;
          cGraphicAttributes.m_fHorizontalScale = m_cMapAttributes.m_fTileHorizontalScale;
        }
        pcGraphic->SetAttributes( cGraphicAttributes );

        // Draw the map tiles
        pcGraphic->DrawAnimation( cDrawPos );

        // Restore the original props
        pcGraphic->SetAttributes( cAttributeBackup );
      }

      EditorGraphic::EnableBlending( true );

      // Show all pawns at this location
      rumPositionIterator iter( GetPositionIterator( cPos ) );
      while( !iter.Done() )
      {
        Sqrat::Object sqWidget{ iter.GetNextObject() };
        if( sqWidget.GetType() == OT_INSTANCE )
        {
          const EditorPawn* pcPawn{ sqWidget.Cast<EditorPawn*>() };
          rumGraphic* pcGraphic{ rumGraphic::Fetch( pcPawn->GetGraphicID() ) };
          if( pcGraphic )
          {
            // Copy the map draw props to the graphic draw props
            rumGraphicAttributes cGraphicAttributes{ pcGraphic->GetAttributes() };
            rumGraphicAttributes cAttributeBackup{ cGraphicAttributes };
            cGraphicAttributes.m_bDrawMasked = true;
            if( m_cMapAttributes.m_bScaleTiles )
            {
              cGraphicAttributes.m_bDrawScaled = true;
              cGraphicAttributes.m_fVerticalScale = m_cMapAttributes.m_fTileVerticalScale;
              cGraphicAttributes.m_fHorizontalScale = m_cMapAttributes.m_fTileHorizontalScale;
            }
            pcGraphic->SetAttributes( cGraphicAttributes );

            // Draw the map pawns
            pcGraphic->DrawAnimation( cDrawPos, pcPawn->GetAnimSet(), pcPawn->GetFrame() );

            // Restore the original props
            pcGraphic->SetAttributes( cAttributeBackup );
          }
        }
      }
    }

    // Reset the horizontal position
    cPos.m_iX = cSrcPos.m_iX;
  }
}


void EditorMap::ResizeMap( uint32_t i_uiCols, uint32_t i_uiRows, rumAssetID i_eTiledID )
{
  rumNumberUtils::Clamp<uint32_t>( i_uiCols, 1, 65536 );
  rumNumberUtils::Clamp<uint32_t>( i_uiRows, 1, 65536 );

  if( i_uiCols > GetCols() )
  {
    AddColsRight( i_uiCols - GetCols(), i_eTiledID );
  }
  else if( i_uiCols < GetCols() )
  {
    DelColsRight( GetCols() - i_uiCols );
  }

  if( i_uiRows > GetRows() )
  {
    AddRowsBottom( i_uiRows - GetRows(), i_eTiledID );
  }
  else if( i_uiRows < GetRows() )
  {
    DelRowsBottom( GetRows() - i_uiRows );
  }
}


int32_t EditorMap::Resave()
{
  if( Load() == RESULT_SUCCESS )
  {
    std::string strFilePath;
    if( rumResource::FindFile( m_pcAsset->GetFilename(), strFilePath ) )
    {
      Save( strFilePath );
    }

#pragma message("TODO - Free maps after resave")
    //Free();

    return RESULT_SUCCESS;
  }

  return RESULT_FAILED;
}


int32_t EditorMap::Save( const std::string& i_strFilePath )
{
  int32_t eResult{ RESULT_FAILED };

  std::filesystem::path fsPath( i_strFilePath );

  rumResourceSaver cResource;
  if( RESULT_SUCCESS == Serialize( cResource ) )
  {
    // Save the map file
    eResult = cResource.SaveFile( fsPath.string() );

    // Any pawns to save?
    if( ( RESULT_SUCCESS == eResult ) && !m_hashPawns.empty() )
    {
      cResource.Reset();

      // Prepare a pawn file to save
      fsPath.replace_extension( ".pwn" );
      if( RESULT_SUCCESS == SerializePawns( cResource ) )
      {
        // Save the pawn file
        eResult = cResource.SaveFile( fsPath.string() );
      }
    }
  }

  return eResult;
}


// static
void EditorMap::ScriptBind()
{
  super::ScriptBind();

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::DerivedClass<EditorMap, rumMap> cEditorMap( pcVM, "EditorMap" );
  Sqrat::RootTable( pcVM ).Bind( SCRIPT_MAP_NATIVE_CLASS, cEditorMap );

}


void EditorMap::ShiftPawnsUp( const rumPosition& i_rcPos )
{
  PositionData* pcCell{ AccessPositionData( i_rcPos ) };
  if( pcCell )
  {
    pcCell->ShiftPawnsUp();
  }
}


void EditorMap::ShiftPawnsDown( const rumPosition& i_rcPos )
{
  PositionData* pcCell{ AccessPositionData( i_rcPos ) };
  if( pcCell )
  {
    pcCell->ShiftPawnsDown();
  }
}
