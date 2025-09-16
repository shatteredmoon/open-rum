#include <c_map.h>

#include <c_graphics.h>
#include <c_map_fov.h>
#include <c_pawn.h>

#include <network/u_packet.h>
#include <u_db.h>
#include <u_log.h>
#include <u_map_asset.h>
#include <u_pos_iterator.h>
#include <u_resource.h>
#include <u_tile_asset.h>
#include <u_zlib.h>

// Static initializers
rumClientMap::ArchiveSet rumClientMap::m_archives;


rumClientMap::~rumClientMap()
{
  FreeInternal();
}


// override
bool rumClientMap::AddPawn( rumPawn* i_pcPawn, const rumPosition& i_rcPos )
{
  rumAssert( i_pcPawn );
  if( !i_pcPawn )
  {
    return false;
  }

  if( !super::AddPawn( i_pcPawn, i_rcPos ) )
  {
    return false;
  }

  if( i_pcPawn->GetScriptInstance().GetType() == OT_INSTANCE )
  {
    // Optional script callback
    rumScript::ExecOptionalFunc( i_pcPawn->GetScriptInstance(), "OnAddedToMap" );
  }

  return true;
}


// static
bool rumClientMap::ArchiveAdd( const std::string& i_strFile )
{
  std::string strPath( i_strFile );

  if( rumResource::FileExists( strPath ) || rumResource::FindFile( i_strFile, strPath ) )
  {
    const auto p{ m_archives.insert( strPath ) };
    return p.second;
  }

  return false;
}


// override
void rumClientMap::AllocateGameID( rumUniqueID i_uiGameID )
{
  if( INVALID_GAME_ID == i_uiGameID )
  {
    static uint64_t s_uiAssetType{ ( rumUniqueID( rumMapAsset::GetClassRegistryID() ) ) << 60 };
    static uint64_t s_uiGameID{ ( rumUniqueID( Client_ObjectCreationType ) ) << 56 };
    i_uiGameID = ++s_uiGameID | s_uiAssetType;
  }
  else
  {
    // This object originated from the server, so use that UID
    rumAssertMsg( OBJECT_CREATION_TYPE( i_uiGameID ) == Server_ObjectCreationType,
                  "Creating a map with a non-zero UID that doesn't originate from the server" );
  }

  SetGameID( i_uiGameID );
}


void rumClientMap::Draw( int32_t i_iScreenX, int32_t i_iScreenY, int32_t i_iTileX, int32_t i_iTileY )
{
  if( m_cMapDrawProps.LineOfSight )
  {
    DrawLOS( i_iScreenX, i_iScreenY, i_iTileX, i_iTileY );
    return;
  }

  // The width and height must be odd!
  if( !( ( m_cMapDrawProps.HorizontalTiles & 0x1 ) && ( m_cMapDrawProps.VerticalTiles & 0x1 ) ) )
  {
    return;
  }

  // Draw location of current tile
  rumPosition cDrawPos( 0, 0 );

  const uint32_t hOffset{ m_cMapDrawProps.HorizontalTiles / 2 };
  const uint32_t vOffset{ m_cMapDrawProps.VerticalTiles / 2 };

  // The top-left most square to be drawn
  const rumPosition cSourcePos( i_iTileX - hOffset, i_iTileY - vOffset );

  if( m_cMapDrawProps.m_bUseLighting )
  {
    if( !GenerateLightMap( cSourcePos ) )
    {
      // Nothing on the light map is visible, so early out
      return;
    }
  }

  // Current position
  rumPosition cPos( cSourcePos );
  rumMap::PositionData* pcPosition{ nullptr };
  Sqrat::Object sqObj;

  // Draw all map tiles
  for( uint32_t j = 0; j < m_cMapDrawProps.VerticalTiles; ++j, ++cPos.m_iY )
  {
    for( uint32_t i = 0; i < m_cMapDrawProps.HorizontalTiles; ++i, ++cPos.m_iX )
    {
      cDrawPos.m_iX = ( i * m_cMapDrawProps.HorizontalTileOffset ) + i_iScreenX;
      cDrawPos.m_iY = ( j * m_cMapDrawProps.VerticalTileOffset ) + i_iScreenY;

      if( m_cMapDrawProps.m_bUseLighting && m_pcLightMap[j][i] <= 0.f )
      {
        continue;
      }

      rumAssetID eTileID{ INVALID_ASSET_ID };

      if( !Wraps() &&
          ( cPos.m_iX < 0 || cPos.m_iY < 0 || cPos.m_iX >= (int32_t)m_uiCols || cPos.m_iY >= (int32_t)m_uiRows ) )
      {
        pcPosition = nullptr;
        eTileID = m_eBorderTileID;
      }
      else
      {
        if( Wraps() )
        {
          WrapPosition( cPos );
        }

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
        if( !cGraphicAttributes.m_bHidden )
        {
          rumGraphicAttributes cGraphicAttributesBackup{ cGraphicAttributes };
          cGraphicAttributes.m_bDrawMasked = true;

          if( m_cMapDrawProps.ScaleTiles )
          {
            cGraphicAttributes.m_bDrawScaled = true;
            cGraphicAttributes.m_fVerticalScale = m_cMapDrawProps.TileVerticalScale;
            cGraphicAttributes.m_fHorizontalScale = m_cMapDrawProps.TileHorizontalScale;
          }

          if( m_cMapDrawProps.m_bUseLighting )
          {
            cGraphicAttributes.m_bDrawLit = true;
            cGraphicAttributes.m_fLightLevel = m_pcLightMap[j][i];
          }

          pcGraphic->SetAttributes( cGraphicAttributes );

          // Draw the map tiles
          pcGraphic->DrawAnimation( cDrawPos );

          // Restore the original props
          pcGraphic->SetAttributes( cGraphicAttributesBackup );
        }
      }

      // Show all pawns at this location
      rumPositionIterator iter{ GetPositionIterator( cPos ) };
      while( !iter.Done() )
      {
        sqObj = iter.GetNextObject();
        if( sqObj.GetType() == OT_INSTANCE )
        {
          const rumClientPawn* pcPawn{ sqObj.Cast<rumClientPawn*>() };
          if( pcPawn->IsVisible() )
          {
            pcGraphic = rumGraphic::Fetch( pcPawn->GetGraphicID() );
            if( pcGraphic )
            {
              // Copy the map draw props to the graphic draw props
              rumGraphicAttributes cGraphicAttributes{ pcGraphic->GetAttributes() };
              if( !cGraphicAttributes.m_bHidden )
              {
                rumGraphicAttributes cGraphicAttributesBackup{ cGraphicAttributes };
                cGraphicAttributes.m_bDrawMasked = true;
                cGraphicAttributes.m_uiTransparentLevel = pcPawn->GetTransparencyLevel();
                cGraphicAttributes.m_eBlendType = pcPawn->GetBlendType();
                cGraphicAttributes.m_bRestoreAlphaPostBlend = pcPawn->GetRestoreAlphaPostBlend();
                cGraphicAttributes.m_cBlendColor = pcPawn->GetBlendColor();
                cGraphicAttributes.m_cBufferColor = pcPawn->GetBufferColor();

                if( m_cMapDrawProps.ScaleTiles )
                {
                  cGraphicAttributes.m_bDrawScaled = true;
                  cGraphicAttributes.m_fVerticalScale = m_cMapDrawProps.TileVerticalScale;
                  cGraphicAttributes.m_fHorizontalScale = m_cMapDrawProps.TileHorizontalScale;
                }

                if( m_cMapDrawProps.m_bUseLighting )
                {
                  cGraphicAttributes.m_bDrawLit = true;
                  cGraphicAttributes.m_fLightLevel = m_pcLightMap[j][i];
                }

                pcGraphic->SetAttributes( cGraphicAttributes );

                // Draw the map pawns
                pcGraphic->DrawAnimation( cDrawPos, pcPawn->GetAnimSet(), pcPawn->GetFrame() );

                // Restore the original props
                pcGraphic->SetAttributes( cGraphicAttributesBackup );
              }
            }
          }
        }
      }
    }

    // Reset the horizontal position
    cPos.m_iX = cSourcePos.m_iX;
  }
}


void rumClientMap::DrawLOS( int32_t i_iScreenX, int32_t i_iScreenY, int32_t i_iTileX, int32_t i_iTileY )
{
  if( !m_cMapDrawProps.LineOfSight )
  {
    Draw( i_iScreenX, i_iScreenY, i_iTileX, i_iTileY );
    return;
  }

  // The width and height must be odd!
  if( !( ( m_cMapDrawProps.HorizontalTiles & 0x1 ) && ( m_cMapDrawProps.VerticalTiles & 0x1 ) ) )
  {
    return;
  }

  // Draw location of current tile
  rumPosition cDrawPos( 0, 0 );

  const uint32_t hOffset{ m_cMapDrawProps.HorizontalTiles / 2 };
  const uint32_t vOffset{ m_cMapDrawProps.VerticalTiles / 2 };

  // The top-left most square to be drawn
  const rumPosition cSourcePos( i_iTileX - hOffset, i_iTileY - vOffset );

  // TODO - this method of map drawing could take advantage of the light map by not visiting tiles during LOS
  // generation that aren't visible by light
  if( m_cMapDrawProps.m_bUseLighting )
  {
    if( !GenerateLightMap( cSourcePos ) )
    {
      // Nothing on the light map is visible, so early out
      return;
    }
  }

  const rumRectangle cRangeRect( cSourcePos, m_cMapDrawProps.HorizontalTiles, m_cMapDrawProps.VerticalTiles );

  Sqrat::Object sqObj;

  // Determine tile visibility
  m_pcRayCaster->CastRays( rumPosition( i_iTileX, i_iTileY ), cRangeRect );

  RayData* pcRay{ nullptr };
  const RayMap& rcRayMap{ m_pcRayCaster->GetRayMap() };

  for( const auto& iter : rcRayMap )
  {
    pcRay = iter.second;
    if( !pcRay->IsVisible() )
    {
      continue;
    }

    rumPosition cPos( iter.first.m_iX, iter.first.m_iY );

    cDrawPos.m_iX = ( ( cPos.m_iX - cSourcePos.m_iX ) * m_cMapDrawProps.HorizontalTileOffset ) + i_iScreenX;
    cDrawPos.m_iY = ( ( cPos.m_iY - cSourcePos.m_iY ) * m_cMapDrawProps.VerticalTileOffset ) + i_iScreenY;

    const int32_t iMapOffsetX{ cPos.m_iX - cSourcePos.m_iX };
    const int32_t iMapOffsetY{ cPos.m_iY - cSourcePos.m_iY };
    if( m_cMapDrawProps.m_bUseLighting && m_pcLightMap[iMapOffsetY][iMapOffsetX] <= 0.f )
    {
      // Nothing on the light map is visible, so early out
      continue;
    }

    rumAssetID eTileID{ INVALID_ASSET_ID };

    if( !Wraps() &&
        ( cPos.m_iX < 0 || cPos.m_iY < 0 || cPos.m_iX >= (int32_t)m_uiCols || cPos.m_iY >= (int32_t)m_uiRows ) )
    {
      eTileID = m_eBorderTileID;
    }
    else
    {
      if( Wraps() )
      {
        WrapPosition( cPos );
      }

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
      if( !cGraphicAttributes.m_bHidden )
      {
        rumGraphicAttributes cGraphicAttributesBackup{ cGraphicAttributes };
        cGraphicAttributes.m_bDrawMasked = true;

        if( m_cMapDrawProps.ScaleTiles )
        {
          cGraphicAttributes.m_bDrawScaled = true;
          cGraphicAttributes.m_fVerticalScale = m_cMapDrawProps.TileVerticalScale;
          cGraphicAttributes.m_fHorizontalScale = m_cMapDrawProps.TileHorizontalScale;
        }

        if( m_cMapDrawProps.m_bUseLighting )
        {
          cGraphicAttributes.m_bDrawLit = true;
          cGraphicAttributes.m_fLightLevel = m_pcLightMap[iMapOffsetY][iMapOffsetX];
        }

        pcGraphic->SetAttributes( cGraphicAttributes );

        // Draw the map tiles
        pcGraphic->DrawAnimation( cDrawPos );

        // Restore the original props
        pcGraphic->SetAttributes( cGraphicAttributesBackup );
      }
    }

    // Show all pawns at this location
    rumPositionIterator loc_iter{ GetPositionIterator( cPos ) };
    while( !loc_iter.Done() )
    {
      sqObj = loc_iter.GetNextObject();
      if( sqObj.GetType() == OT_INSTANCE )
      {
        const rumClientPawn* pcPawn{ sqObj.Cast<rumClientPawn*>() };
        if( pcPawn->IsVisible() )
        {
          pcGraphic = rumGraphic::Fetch( pcPawn->GetGraphicID() );
          if( pcGraphic )
          {
            // Copy the map draw props to the graphic draw props
            rumGraphicAttributes cGraphicAttributes{ pcGraphic->GetAttributes() };
            if( !cGraphicAttributes.m_bHidden )
            {
              rumGraphicAttributes cGraphicAttributesBackup{ cGraphicAttributes };
              cGraphicAttributes.m_bDrawMasked = true;
              cGraphicAttributes.m_uiTransparentLevel = pcPawn->GetTransparencyLevel();
              cGraphicAttributes.m_eBlendType = pcPawn->GetBlendType();
              cGraphicAttributes.m_bRestoreAlphaPostBlend = pcPawn->GetRestoreAlphaPostBlend();
              cGraphicAttributes.m_cBlendColor = pcPawn->GetBlendColor();
              cGraphicAttributes.m_cBufferColor = pcPawn->GetBufferColor();

              if( m_cMapDrawProps.ScaleTiles )
              {
                cGraphicAttributes.m_bDrawScaled = true;
                cGraphicAttributes.m_fVerticalScale = m_cMapDrawProps.TileVerticalScale;
                cGraphicAttributes.m_fHorizontalScale = m_cMapDrawProps.TileHorizontalScale;
              }

              if( m_cMapDrawProps.m_bUseLighting )
              {
                cGraphicAttributes.m_bDrawLit = true;
                cGraphicAttributes.m_fLightLevel = m_pcLightMap[iMapOffsetY][iMapOffsetX];
              }

              pcGraphic->SetAttributes( cGraphicAttributes );

              // Draw the map pawns
              pcGraphic->DrawAnimation( cDrawPos, pcPawn->GetAnimSet(), pcPawn->GetFrame() );

              // Restore the original props
              pcGraphic->SetAttributes( cGraphicAttributesBackup );
            }
          }
        }
      }
    }
  }
}


// virtual
void rumClientMap::Free()
{
  FreeInternal();
  return super::Free();
}


void rumClientMap::FreeInternal()
{
  SAFE_DELETE( m_pcRayCaster );
  FreeLightMap();
}


void rumClientMap::FreeLightMap()
{
  // Free the light map
  for( uint32_t i = 0; i < m_iLightMapVerticalTiles; ++i )
  {
    if( m_pcLightMap[i] )
    {
      delete[] m_pcLightMap[i];
      m_pcLightMap[i] = nullptr;
    }
  }

  if( m_pcLightMap )
  {
    delete[] m_pcLightMap;
    m_pcLightMap = nullptr;
  }
}


bool rumClientMap::GenerateLightMap( const rumPosition& i_rcStartPos )
{
  bool bLit{ false };

  if( m_iLightMapVerticalTiles != m_cMapDrawProps.VerticalTiles ||
      m_iLightMapHorizontalTiles != m_cMapDrawProps.HorizontalTiles )
  {
    InitializeLightMap( m_cMapDrawProps.VerticalTiles, m_cMapDrawProps.HorizontalTiles );
  }

  ResetLightMap();

  const rumRectangle cRect( i_rcStartPos, m_iLightMapHorizontalTiles, m_iLightMapVerticalTiles );

  // Visit all on-screen pawns to determine if anything is emitting light
  for( const auto iter : m_hashPawns )
  {
    const rumUniqueID uiPawnID{ iter };
    const rumPawn* pcPawn{ rumPawn::Fetch( uiPawnID ) };
    rumAssert( pcPawn );
    if( pcPawn && pcPawn->IsVisible() && pcPawn->GetLightRange() > 0 )
    {
      const rumCircle cCircle( pcPawn->GetPos(), (float)pcPawn->GetLightRange() );
      if( cCircle.Intersects( cRect ) )
      {
        bLit = true;
        LightArea( i_rcStartPos, pcPawn->GetPos(), pcPawn->GetLightRange() );
      }
    }
  }

  return bLit;
}


void rumClientMap::InitializeLightMap( uint32_t i_iRows, uint32_t i_iCols )
{
  FreeLightMap();

  m_iLightMapVerticalTiles = i_iRows;
  m_iLightMapHorizontalTiles = i_iCols;

  // Allocate light map in two dimensions
  m_pcLightMap = new float*[i_iRows];
  if( m_pcLightMap )
  {
    for( uint32_t i = 0; i < i_iRows; ++i )
    {
      m_pcLightMap[i] = new float[i_iCols];
    }
  }
}


void rumClientMap::ResetLightMap()
{
  for( uint32_t i = 0; i < m_iLightMapVerticalTiles; ++i )
  {
    memset( m_pcLightMap[i], 0, sizeof( float ) * m_iLightMapHorizontalTiles );
  }
}


void rumClientMap::LightArea( const rumPosition& i_rcStartPos, const rumPosition& i_rcLightPos,
                              uint32_t i_uiLightRange )
{
  // Start by increasing the specified light range by one - this is done so that the tiles at the outer edge of the
  // the range have values > 0.
  ++i_uiLightRange;

  // Light one quadrant and reflect the values to the others
  for( uint32_t j = 0; j < i_uiLightRange; ++j )
  {
    for( uint32_t i = 0; i < i_uiLightRange; ++i )
    {
      // The tile position currently being processed
      const uint32_t x{ i_rcLightPos.m_iX + i };
      const uint32_t y{ i_rcLightPos.m_iY + j };

      // TODO - map wrapping?

      // The offset from the map starting position, upper-left from the player
      int32_t iMapOffsetX{ (int32_t)( x - i_rcStartPos.m_iX ) };
      int32_t iMapOffsetY{ (int32_t)( y - i_rcStartPos.m_iY ) };

      if( ( 0 == i ) && ( 0 == j ) )
      {
        if( iMapOffsetX >= 0 && iMapOffsetX < (int32_t)m_iLightMapHorizontalTiles &&
            iMapOffsetY >= 0 && iMapOffsetY < (int32_t)m_iLightMapVerticalTiles )
        {
          // The light source itself is always fully lit
          m_pcLightMap[iMapOffsetY][iMapOffsetX] = 1.f;
        }
        continue;
      }

      // Distance forumula - i & j reflect how far the light is offset
      const float fDistance{ rumNumberUtils::GetSqrtValue( SQR( i ) + SQR( j ) ) };

      // Light intensity decreases as the position moves farther away from the source
      const float fLightIntensity{ ( (float)i_uiLightRange - fDistance ) / (float)i_uiLightRange };

      // Is the current position inside of the light map?
      if( iMapOffsetX >= 0 && iMapOffsetX < (int32_t)m_iLightMapHorizontalTiles &&
          iMapOffsetY >= 0 && iMapOffsetY < (int32_t)m_iLightMapVerticalTiles )
      {
        if( m_pcLightMap[iMapOffsetY][iMapOffsetX] < fLightIntensity )
        {
          m_pcLightMap[iMapOffsetY][iMapOffsetX] = fLightIntensity;
        }
      }

      // Reflect along the vertical axis
      int32_t iLightReflectedPosX{ (int32_t)( i_rcLightPos.m_iX - i ) };
      if( iLightReflectedPosX != i_rcLightPos.m_iX )
      {
        iMapOffsetX = iLightReflectedPosX - i_rcStartPos.m_iX;
        if( iMapOffsetX >= 0 && iMapOffsetX < (int32_t)m_iLightMapHorizontalTiles &&
            iMapOffsetY >= 0 && iMapOffsetY < (int32_t)m_iLightMapVerticalTiles )
        {
          if( m_pcLightMap[iMapOffsetY][iMapOffsetX] < fLightIntensity )
          {
            m_pcLightMap[iMapOffsetY][iMapOffsetX] = fLightIntensity;
          }
        }
      }

      // Reflect along the horizontal axis
      int32_t iLightReflectedPosY{ (int32_t)( i_rcLightPos.m_iY - j ) };
      if( iLightReflectedPosY != i_rcLightPos.m_iY )
      {
        iMapOffsetY = iLightReflectedPosY - i_rcStartPos.m_iY;
        if( iMapOffsetX >= 0 && iMapOffsetX < (int32_t)m_iLightMapHorizontalTiles &&
            iMapOffsetY >= 0 && iMapOffsetY < (int32_t)m_iLightMapVerticalTiles )
        {
          if( m_pcLightMap[iMapOffsetY][iMapOffsetX] < fLightIntensity )
          {
            m_pcLightMap[iMapOffsetY][iMapOffsetX] = fLightIntensity;
          }
        }
      }

      // Reflect along the vertical axis
      iLightReflectedPosX = i_rcLightPos.m_iX + i;
      if( iLightReflectedPosY != i_rcLightPos.m_iY && iLightReflectedPosX != i_rcLightPos.m_iX )
      {
        iMapOffsetX = iLightReflectedPosX - i_rcStartPos.m_iX;
        if( iMapOffsetX >= 0 && iMapOffsetX < (int32_t)m_iLightMapHorizontalTiles &&
            iMapOffsetY >= 0 && iMapOffsetY < (int32_t)m_iLightMapVerticalTiles )
        {
          if( m_pcLightMap[iMapOffsetY][iMapOffsetX] < fLightIntensity )
          {
            m_pcLightMap[iMapOffsetY][iMapOffsetX] = fLightIntensity;
          }
        }
      }
    }
  }
}


// override
int32_t rumClientMap::Load()
{
  rumAssert( m_pcAsset );

  int32_t eResult{ RESULT_FAILED };

  // On the client, try to load the map from any available archives first
  if( !m_archives.empty() )
  {
    // Try to load the file from the map archives
    ArchiveSet::const_iterator iter{ m_archives.begin() };
    const ArchiveSet::const_iterator end{ m_archives.end() };
    for( ; iter != end && ( RESULT_FAILED == eResult ); ++iter )
    {
      rumResourceLoader cResource;
      const std::string& strArchivePath{ *iter };
      if( cResource.LoadFileFromArchive( m_pcAsset->GetFilename(), strArchivePath ) > 0 )
      {
        auto sqObject{ m_pcAsset->GetProperty( Map_Wraps_PropertyID ) };
        if( sqObject.GetType() == OT_BOOL )
        {
          m_bWraps = sqObject.Cast<bool>();
        }

        eResult = Serialize( cResource );
      }
    }
  }

  if( eResult != RESULT_SUCCESS )
  {
    // Try the default loader
    eResult = super::Load();
  }
  else
  {
    OnLoaded();
  }

  if( RESULT_SUCCESS == eResult )
  {
    // Create the RayCaster
    m_pcRayCaster = new RayCaster( this );
  }

  return eResult;
}


bool rumClientMap::PositionLit( const rumPoint& i_rcPoint ) const
{
  bool bLit{ false };

  if( m_cMapDrawProps.m_bUseLighting )
  {
    if( i_rcPoint.m_iX >= 0 && i_rcPoint.m_iX < (int32_t)m_cMapDrawProps.HorizontalTiles &&
        i_rcPoint.m_iY >= 0 && i_rcPoint.m_iY < (int32_t)m_cMapDrawProps.VerticalTiles &&
        m_pcLightMap[i_rcPoint.m_iY][i_rcPoint.m_iX] > 0.f )
    {
      bLit = true;
    }
  }
  else
  {
    // Map does not use lighting, so the tile is always lit
    bLit = true;
  }

  return bLit;
}


void rumClientMap::ScriptBind()
{
  super::ScriptBind();

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::Class<MapDrawProps> cMapDrawProps( pcVM, "rumMapDrawProps" );
  cMapDrawProps
    .Var( "LineOfSight", &MapDrawProps::LineOfSight )
    .Var( "Lighting", &MapDrawProps::m_bUseLighting )
    .Var( "ScaleTiles", &MapDrawProps::ScaleTiles )
    .Var( "HorizontalTiles", &MapDrawProps::HorizontalTiles )
    .Var( "VerticalTiles", &MapDrawProps::VerticalTiles )
    .Var( "HorizontalTileOffset", &MapDrawProps::HorizontalTileOffset )
    .Var( "VerticalTileOffset", &MapDrawProps::VerticalTileOffset )
    .Var( "TileVerticalScale", &MapDrawProps::TileVerticalScale )
    .Var( "TileHorizontalScale", &MapDrawProps::TileHorizontalScale );
  Sqrat::RootTable( pcVM ).Bind( "rumMapDrawProps", cMapDrawProps );

  Sqrat::DerivedClass<rumClientMap, rumMap> cClientMap( pcVM, "rumClientMap" );
  cClientMap
    .Func( "GetDrawProps", &GetDrawProps )
    .Func( "SetDrawProps", &SetDrawProps )
    .Overload<bool( rumClientMap::* )( const rumPoint& ) const>( "IsPositionLit", &PositionLit )
    .Overload<bool( rumClientMap::* )( int32_t, int32_t ) const>( "IsPositionLit", &PositionLit )
    .Overload<void( rumClientMap::* )( const rumPoint&, const rumPoint& )>( "Draw", &Draw )
    .Overload<void( rumClientMap::* )( const rumPoint&, int32_t, int32_t )>( "Draw", &Draw )
    // int32, int32, rumPoint& - TODO - how to support this?
    .Overload<void( rumClientMap::* )( int32_t, int32_t, int32_t, int32_t )>( "Draw", &Draw );
  Sqrat::RootTable( pcVM ).Bind( SCRIPT_MAP_NATIVE_CLASS, cClientMap );

  Sqrat::RootTable( pcVM )
    .Func( "rumAddMapArchive", ArchiveAdd )
    .Func( "rumRemoveMapArchive", ArchiveRemove );
}


// final
int32_t rumClientMap::Serialize( rumResource& io_rcResource )
{
  // The client should never save maps
  rumAssert( io_rcResource.IsLoading() );
  return io_rcResource.IsLoading() ? super::Serialize( io_rcResource ) : RESULT_FAILED;
}


// final
int32_t rumClientMap::SerializePawns( rumResource& io_rcResource )
{
  // The client should never load or save pawns
  rumAssert( false );
  return RESULT_FAILED;
}


int32_t rumClientMap::ServerPawnUpdates( rumMap* i_pcMap, const rumNetwork::rumInboundPacket& i_rcPacket )
{
  uint32_t eResult{ RESULT_FAILED };

  rumQWord uiGameID = 0;
  rumDWord eAssetID = 0;
  rumDWord iPosX = 0;
  rumDWord iPosY = 0;
  rumDWord uiCollisionFlags = 0;
  rumDWord uiMoveType = 0;
  rumDWord uiLightRange = 0;
  rumDWord uiState = 0;
  rumDWord uiNumPawns = 0;
  rumByte ePawnType = 0;
  rumByte bVisible = 0;

  i_rcPacket << uiNumPawns;
  for( uint32_t i = 0; i < (uint32_t)uiNumPawns; ++i )
  {
    i_rcPacket << uiGameID << ePawnType << eAssetID << iPosX << iPosY << bVisible << uiCollisionFlags << uiMoveType
               << uiLightRange << uiState;

    rumPawn* pcPawn{ nullptr };

    bool bOk{ false };

    // Find the existing pawn on the map
    if( !i_pcMap->IsPawnOnMap( uiGameID ) )
    {
      // Does the pawn exist outside of the current map?
      if( pcPawn = rumPawn::Fetch( uiGameID ) )
      {
        // Force the pawn onto this map
        i_pcMap->TransferPawn( pcPawn, i_pcMap, rumPosition( iPosX, iPosY ), true /* force */ );
      }
      else
      {
        // The pawn does not already exist, so create it
        Sqrat::Object sqInstance{ Create( (rumAssetID)eAssetID, Sqrat::Table(), uiGameID ) };
        if( sqInstance.GetType() == OT_INSTANCE )
        {
          pcPawn = sqInstance.Cast<rumPawn*>();

          // Position and add the pawn to the map
          pcPawn->SetPos( iPosX, iPosY );

          eResult = i_pcMap->AddPawn( pcPawn, pcPawn->GetPos() ) ? RESULT_SUCCESS : RESULT_FAILED;
          if( eResult == RESULT_SUCCESS )
          {
            ManageScriptObject( sqInstance );
          }
        }
      }
    }
    else
    {
      // The pawn already exists (likely a player's pawn)
      pcPawn = rumPawn::Fetch( uiGameID );
      rumAssert( pcPawn );

      // Update the pawn's position
      const rumMoveFlags eMoveFlags{ (rumMoveFlags)( IgnoreTileCollision_MoveFlag | IgnorePawnCollision_MoveFlag |
                                                     IgnoreDistance_MoveFlag ) };
      i_pcMap->MovePawn( pcPawn, rumPosition( iPosX, iPosY ), eMoveFlags, 0 );
    }

    if( pcPawn )
    {
      pcPawn->SetVisibility( bVisible ? true : false );
      pcPawn->SetCollisionFlags( uiCollisionFlags );
      pcPawn->SetMoveType( uiMoveType );
      pcPawn->SetLightRange( uiLightRange );
      pcPawn->SetState( (int32_t)uiState );
    }
    else
    {
      std::string strError{ "Invalid pawn (" };
      strError += rumStringUtils::ToHexString( eAssetID );
      strError += ") encountered during pawn updates";
      Logger::LogStandard( strError );
    }
  }

  return eResult;
}
