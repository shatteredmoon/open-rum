#include <smMapWidget.h>

#include <e_graphics.h>
#include <e_map.h>
#include <e_pawn.h>

#include <u_enum.h>
#include <u_map_asset.h>
#include <u_pawn_asset.h>
#include <u_pos_iterator.h>
#include <u_resource.h>
#include <u_tile_asset.h>

#include <mainwindow.h>
#include <mapeditor.h>
#include <newmap.h>
#include <smTileWidget.h>
#include <smTreeWidget.h>

#include <QFileInfo>
#include <QKeyEvent>
#include <QMessageBox>
#include <QWheelEvent>

float smMapWidget::s_fCursorColor = 0.f;
float smMapWidget::s_fCursorAnimDelta = 0.25f;

uint32_t smMapWidget::s_iCols = 0;
uint32_t smMapWidget::s_iRows = 0;


smMapWidget::smMapWidget( QWidget* i_pcParent )
  : QOpenGLWidget( i_pcParent )
  , m_pcParent( nullptr )
  , m_cSelectedPos( 0, 0 )
  , m_cSelectedMiniMapPos( 0, 0 )
  , m_fSelectionRect()
  , m_eMapMode( EditMode )
  , m_bPainting( false )
  , m_bMovingPawn( false )
  , m_bHasFocus( false )
  , m_bDirty( false )
{
  m_pcParent = qobject_cast<MapEditor*>( i_pcParent );

  // Allow the GL Window to gain focus so that input events can be handled
  setFocusPolicy( Qt::WheelFocus );

  // By default, mouse events are only sent one button press, so enable constant tracking
  setMouseTracking( true );

  m_pcMap = nullptr;
  m_uiSelectedPawn = INVALID_GAME_ID;
}


smMapWidget::~smMapWidget()
{
  // Cleanup each map and its objects
  m_eBrush = INVALID_ASSET_ID;
  m_eBrushPrev = INVALID_ASSET_ID;
  if( m_pcMap )
  {
    rumGameObject::UnmanageScriptObject( m_pcMap->GetGameID() );
  }
}


void smMapWidget::ApplyBrush()
{
  if( INVALID_ASSET_ID == m_eBrush )
  {
    return;
  }

  const AssetType eAssetType{ (AssetType)RAW_ASSET_TYPE( m_eBrush ) };
  if( eAssetType == Tile_AssetType )
  {
    m_pcMap->SetTileID( m_cSelectedPos.x(), m_cSelectedPos.y(), m_eBrush );
    MarkDirty();
    m_pcParent->GetTileWidget()->Update();
    m_pcParent->OnTilePainted( m_cSelectedPos );

    HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };
    rumScript::ExecOptionalFunc( Sqrat::RootTable( pcVM ), "OnEditorTilePlaced", eAssetType, m_cSelectedPos );
  }
  else if( ( eAssetType == Creature_AssetType ) || ( eAssetType == Portal_AssetType ) ||
           ( eAssetType == Widget_AssetType ) )
  {
    Sqrat::Object sqInstance{ rumGameObject::Create( m_eBrush ) };
    if( sqInstance.GetType() == OT_INSTANCE )
    {
      EditorPawn* pcPawn{ sqInstance.Cast<EditorPawn*>() };
      if( pcPawn )
      {
        if( m_pcMap->AddPawn( pcPawn, rumPosition( m_cSelectedPos.x(), m_cSelectedPos.y() ) ) )
        {
          rumGameObject::ManageScriptObject( sqInstance );

          MarkDirty();
          m_pcParent->GetTileWidget()->Update();
          m_pcParent->OnPawnAdded( pcPawn );

          // Notify scripts that an object was added in the editor
          rumScript::ExecOptionalFunc( sqInstance, "OnEditorPlaced" );
        }
      }
    }

    // Turn off painting if it is on to prevent adding many objects
    m_bPainting = false;
  }
}


void smMapWidget::CenterOnPosition( const QPoint& i_rcPos )
{
  m_eMapMode = EditMode;
  m_cSelectedPos = i_rcPos;

  QPoint cOffset;
  cOffset.setX( i_rcPos.x() - GetNumColumns() / 2 );
  cOffset.setY( i_rcPos.y() - GetNumRows() / 2 );
  SetMapPos( cOffset );
}


void smMapWidget::enterEvent( QEvent* i_pcEvent )
{
  m_bHasFocus = true;
  grabKeyboard();
}


rumAssetID smMapWidget::GetBorderTile() const
{
  return m_pcMap ? m_pcMap->GetBorderTile() : INVALID_ASSET_ID;
}


rumAssetID smMapWidget::GetExitMapID() const
{
  return m_pcMap ? m_pcMap->GetExitMapID() : INVALID_ASSET_ID;
}


QPoint smMapWidget::GetExitMapPos() const
{
  QPoint cPoint( 0, 0 );

  if( m_pcMap )
  {
    const rumPosition& rcPos{ m_pcMap->GetExitPos() };
    cPoint.setX( rcPos.m_iX );
    cPoint.setY( rcPos.m_iY );
  }

  return cPoint;
}


const EditorMap* smMapWidget::GetMap() const
{
  return m_pcMap;
}


QPoint smMapWidget::GetMapPos() const
{
  if( m_pcMap )
  {
    const rumPosition& rcPos{ m_pcMap->GetDrawPosition() };
    return QPoint( rcPos.m_iX, rcPos.m_iY );
  }

  return QPoint( 0, 0 );
}


uint32_t smMapWidget::GetNumMapColumns() const
{
  return m_pcMap ? m_pcMap->GetCols() : 0;
}


uint32_t smMapWidget::GetNumMapRows() const
{
  return m_pcMap ? m_pcMap->GetRows() : 0;
}


rumPawn* smMapWidget::GetPawnAtCurrentTile()
{
  rumPawn* pcPawn{ nullptr };

  const rumPosition cPos( m_cSelectedPos.x(), m_cSelectedPos.y() );
  rumPositionIterator iter{ m_pcMap->GetPositionIterator( cPos ) };
  if( !iter.Done() )
  {
    pcPawn = iter.GetFirstObjectPtr();
  }

  return pcPawn;
}


bool smMapWidget::HasPawnAtCurrentTile()
{
  const rumPosition cPos( m_cSelectedPos.x(), m_cSelectedPos.y() );
  rumPositionIterator iter{ m_pcMap->GetPositionIterator( cPos ) };
  if( !iter.Done() )
  {
    // At least one pawn exists at this location
    return true;
  }

  return false;
}


rumAssetID smMapWidget::GetTile( const QPoint& i_rcPos ) const
{
  return m_pcMap ? m_pcMap->GetTileID( rumPosition( i_rcPos.x(), i_rcPos.y() ) ) : INVALID_ASSET_ID;
}


void smMapWidget::initializeGL()
{
  // Parent should always be valid at this point
  Q_ASSERT( m_pcParent );

  initializeOpenGLFunctions();

  const uint32_t iHeight{ m_pcParent->GetMapHeight() };
  const uint32_t iWidth{ m_pcParent->GetMapWidth() };

  glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();

  const float fAspectRatio{ iWidth / static_cast<float>( iHeight ) };

  // left, right, bottom, top, znear, zfar
  glOrtho( 0, iWidth * fAspectRatio, iHeight * fAspectRatio, 0, -1, 1 );

  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();

  glDisable( GL_CULL_FACE );
  glDisable( GL_BLEND );
  glDisable( GL_LIGHTING );
  glDisable( GL_DEPTH_TEST );

  // Cache the number of visible rows and columns based on the user specified tile sizes
  s_iCols = iWidth / MainWindow::GetTileWidth();
  s_iRows = iHeight / MainWindow::GetTileHeight();

  UpdateEditMode();
}


bool smMapWidget::keyboardMovement()
{
  bool bHandled{ false };

  Q_ASSERT( m_pcMap );
  const rumPosition& rcPos{ m_pcMap->GetDrawPosition() };

  int32_t iX{ rcPos.m_iX };
  int32_t iY{ rcPos.m_iY };

  // Qt doesn't provide for direct keyboard state queries, so fall back on OS level calls
#ifdef WIN32
  const int32_t iMask{ 0x8000 };

  const bool bUp{    ( ( GetAsyncKeyState( VK_UP ) & iMask )    |
                       ( GetAsyncKeyState( 'W' ) & iMask ) ) != 0 };
  const bool bDown{  ( ( GetAsyncKeyState( VK_DOWN ) & iMask )  |
                       ( GetAsyncKeyState( 'S' ) & iMask ) ) != 0 };
  const bool bLeft{  ( ( GetAsyncKeyState( VK_LEFT ) & iMask )  |
                       ( GetAsyncKeyState( 'A' ) & iMask ) ) != 0 };
  const bool bRight{ ( ( GetAsyncKeyState( VK_RIGHT ) & iMask ) |
                       ( GetAsyncKeyState( 'D' ) & iMask ) ) != 0 };
  const bool bCtrl{    ( GetAsyncKeyState( VK_CONTROL ) & iMask ) != 0 };
#else
#error Handle keystate for other OS - XQueryKeymap
#endif

  if( bUp )
  {
    if( bCtrl )
    {
      iY -= GetNumRows();
    }
    else
    {
      --iY;
    }
    bHandled = true;
  }
  else if( bDown )
  {
    if( bCtrl )
    {
      iY += GetNumRows();
    }
    else
    {
      ++iY;
    }
    bHandled = true;
  }

  if( bLeft )
  {
    if( bCtrl )
    {
      iX -= GetNumColumns();
    }
    else
    {
      --iX;
    }
    bHandled = true;
  }
  else if( bRight )
  {
    if( bCtrl )
    {
      iX += GetNumColumns();
    }
    else
    {
      ++iX;
    }
    bHandled = true;
  }

  if( bHandled )
  {
    // Update the map position
    SetMapPos( iX, iY );
  }

  return bHandled;
}


void smMapWidget::keyPressEvent( QKeyEvent* i_pcEvent )
{
  if( !m_bHasFocus )
  {
    super::keyPressEvent( i_pcEvent );
    return;
  }

  // Process movement keys
  if( keyboardMovement() )
  {
    // Early out since movement has been processed
    return;
  }

  switch( i_pcEvent->key() )
  {
    case Qt::Key_Delete:
      onDeleteTopPawn();
      break;

    case Qt::Key_Home:
      SetMapPos( 0, 0 );
      break;

    case Qt::Key_End:
      if( m_eMapMode == EditMode )
      {
        SetMapPos( GetNumMapColumns() - ( m_pcParent->GetMapWidth() / MainWindow::GetTileWidth() ),
                   GetNumMapRows() - ( m_pcParent->GetMapHeight() / MainWindow::GetTileHeight() ) );
      }
      else if( m_eMapMode == MiniMode )
      {
        // Center the mini-mode view around the edit mode
        const QPointF cVisibleTiles( m_pcParent->GetMapWidth() / m_fTilePixelSize.x(),
                                     m_pcParent->GetMapHeight() / m_fTilePixelSize.y() );

        const QPoint& rcMapPos{ GetMapPos() };
        QPoint cNewMapPos( (int32_t)( GetNumMapColumns() - cVisibleTiles.x() ),
                           (int32_t)( GetNumMapRows() - cVisibleTiles.y() ) );

        // Check that bottom-right portion of map is only showing the editable area, except here we favor the
        // top-left not showing uneditable portions of the map over bottom-right
        cNewMapPos.setX( qMax( 0, cNewMapPos.x() ) );
        cNewMapPos.setY( qMax( 0, cNewMapPos.y() ) );

        SetMapPos( cNewMapPos );
      }
      break;

    case Qt::Key_Minus:
    case Qt::Key_PageUp:
      onShiftPawnsUp();
      break;

    case Qt::Key_Plus:
    case Qt::Key_Equal:
    case Qt::Key_PageDown:
      onShiftPawnsDown();
      break;

    case Qt::Key_M:
      onMovePawn();
      break;

    case Qt::Key_P:
      onPickPawn();
      break;

    case Qt::Key_T:
      onPickTile();
      break;

    case Qt::Key_Space:
      if( MiniMode == m_eMapMode )
      {
        m_eMapMode = EditMode;
        m_cSelectedPos = m_cSelectedMiniMapPos;
        SetMapPos( m_cSelectedPos );
      }
      else if( EditMode == m_eMapMode )
      {
        m_eMapMode = MiniMode;
        m_cSelectedMiniMapPos = m_cSelectedPos;

        // Update mini mode settings, but refrain from updating GL because there is more work to do
        UpdateMiniMode( false );

        // Center the mini-mode view around the edit mode
        const QPointF cVisibleTiles( m_pcParent->GetMapWidth() / m_fTilePixelSize.x(),
                                     m_pcParent->GetMapHeight() / m_fTilePixelSize.y() );
        const QPointF cTileOffset( ( cVisibleTiles.x() - GetNumColumns() ) / 2,
                                   ( cVisibleTiles.y() - GetNumRows() ) / 2 );

        const QPoint& rcMapPos{ GetMapPos() };
        QPoint cNewMapPos( (int32_t)( rcMapPos.x() - cTileOffset.x() ), (int32_t)( rcMapPos.y() - cTileOffset.y() ) );

        // Keep the map within editable boundaries if possible (the map may be smaller than the max view size and may
        // therefore always show an uneditable portion)
        const QPoint cMaxMapPos( GetNumMapColumns() - cVisibleTiles.x(), GetNumMapRows() - cVisibleTiles.y() );

        // Check that bottom-right portion of map is only showing the editable area
        if( cNewMapPos.x() > cMaxMapPos.x() )
        {
          cNewMapPos.setX( cMaxMapPos.x() );
        }

        if( cNewMapPos.y() > cMaxMapPos.y() )
        {
          cNewMapPos.setY( cMaxMapPos.y() );
        }

        // Check that bottom-right portion of map is only showing the editable area, except here we favor the top-left
        // not showing uneditable portions of the map over bottom-right
        cNewMapPos.setX( qMax( 0, cNewMapPos.x() ) );
        cNewMapPos.setY( qMax( 0, cNewMapPos.y() ) );

        SetMapPos( cNewMapPos );
      }
      break;

    default:
      super::keyPressEvent( i_pcEvent );
      break;
  }
}


void smMapWidget::leaveEvent( QEvent* i_pcEvent )
{
  m_bHasFocus = false;
  releaseKeyboard();
}


bool smMapWidget::LoadMap( const QString& i_strFilePath )
{
  // Get the filename without path information
  const QFileInfo cFileInfo( i_strFilePath );
  const QString strFile{ cFileInfo.fileName() };

  rumMapAsset* pcAsset{ nullptr };

  for( const auto& iter : rumMapAsset::GetAssetHash() )
  {
    pcAsset = iter.second;
    if( strFile.compare( pcAsset->GetFilename().c_str(), Qt::CaseInsensitive ) == 0 )
    {
      break;
    }
  }

  if( !pcAsset )
  {
    QMessageBox::critical( this, tr( "Map Load Failure" ), "Failed to load map: " + strFile );
    return false;
  }

  EditorMap* pcMap = nullptr;

  // Create the map
  Sqrat::Object sqObject{ rumGameObject::Create( pcAsset->GetAssetID() ) };
  if( sqObject.GetType() == OT_INSTANCE )
  {
    pcMap = sqObject.Cast<EditorMap*>();
    if( pcMap->Load() == RESULT_SUCCESS )
    {
      rumGameObject::ManageScriptObject( sqObject );
    }
    else
    {
      pcMap = nullptr;
      QMessageBox::critical( this, tr( "Map Load Failure" ), "Failed to load map: " + i_strFilePath );
    }
  }
  else
  {
    QMessageBox::critical( this, tr( "Map Load Failure" ), "Failed to create an instance of map: " + i_strFilePath );
  }

  m_pcMap = pcMap;

  return m_pcMap != nullptr;
}


void smMapWidget::MarkDirty()
{
  if( IsDirty() )
  {
    // The map is already dirty
    return;
  }

  m_bDirty = true;

  // Update this map's tab to reflect dirty state
  MainWindow* pcMain{ MainWindow::GetMainWindow() };
  Q_ASSERT( pcMain );

  QIcon cSaveIcon( ":/ui/resources/save.png" );
  pcMain->SetTabIcon( pcMain->GetCurrentTabIndex(), cSaveIcon );
}


void smMapWidget::mouseMoveEvent( QMouseEvent* i_pcEvent )
{
  if( EditMode == m_eMapMode )
  {
    UpdateEditMode();
    UpdateSelectedPos( i_pcEvent->pos() );

    if( m_bPainting )
    {
      ApplyBrush();
    }
  }
  else if( MiniMode == m_eMapMode )
  {
    UpdateMiniMode();
  }
}


void smMapWidget::mousePressEvent( QMouseEvent* i_pcEvent )
{
  // releaseKeyboard???

  if( i_pcEvent->button() == Qt::LeftButton )
  {
    if( MiniMode == m_eMapMode )
    {
      m_eMapMode = EditMode;
      m_cSelectedPos = m_cSelectedMiniMapPos;
      SetMapPos( m_cSelectedPos );
    }
    else if( EditMode == m_eMapMode )
    {
      UpdateSelectedPos( i_pcEvent->pos() );

      if( m_bMovingPawn )
      {
        MovePawn();
      }
      else
      {
        m_bPainting = true;
        ApplyBrush();
      }
    }
  }
}


void smMapWidget::mouseReleaseEvent( QMouseEvent * i_pcEvent )
{
  if( i_pcEvent->button() == Qt::LeftButton )
  {
    m_bPainting = false;
  }
}


void smMapWidget::MovePawn()
{
  if( !m_pcMap )
  {
    return;
  }

  rumPawn* pcPawn{ rumPawn::Fetch( m_uiSelectedPawn ) };
  m_pcMap->MovePawn( pcPawn, rumPosition( m_cSelectedPos.x(), m_cSelectedPos.y() ),
                    rumMoveFlags( IgnoreTileCollision_MoveFlag | IgnorePawnCollision_MoveFlag |
                                  IgnoreDistance_MoveFlag ),
                    0 );
  SetBrush( m_eBrushPrev );
  MarkDirty();
  m_pcParent->GetTileWidget()->Update();

  m_bMovingPawn = false;
}


void smMapWidget::OnAnimTimer()
{
  s_fCursorColor += s_fCursorAnimDelta;
  if( s_fCursorColor > 1.f )
  {
    // Begin fading to black
    s_fCursorColor = 1.f;
    s_fCursorAnimDelta *= -1.f;
  }
  else if( s_fCursorColor < 0.f )
  {
    // Begin fading to white
    s_fCursorColor = 0.f;
    s_fCursorAnimDelta *= -1.f;
  }

  update();
}


void smMapWidget::onDeleteTopPawn()
{
  if( !m_pcMap )
  {
    return;
  }

  rumPawn* pcPawn{ GetPawnAtCurrentTile() };
  if( pcPawn )
  {
    m_pcParent->OnPawnRemoved( pcPawn );
    m_pcMap->RemovePawn( pcPawn );
    MarkDirty();
    m_pcParent->GetTileWidget()->Update();
  }
}


void smMapWidget::OnMapExitChanged( rumAssetID i_eMapID, const QPoint& i_rcPos )
{
  // Update map with the new settings
  if( !m_pcMap )
  {
    return;
  }

  if( m_pcMap->GetExitMapID() != i_eMapID )
  {
    m_pcMap->SetExitMapID( i_eMapID );
    MarkDirty();
  }

  const rumPosition& rcCurrentPos{ m_pcMap->GetExitPos() };
  if( rcCurrentPos.m_iX != i_rcPos.x() || rcCurrentPos.m_iY != i_rcPos.y() )
  {
    m_pcMap->SetExitPos( rumPosition( i_rcPos.x(), i_rcPos.y() ) );
    MarkDirty();
  }
}


void smMapWidget::onMovePawn()
{
  if( !m_pcMap )
  {
    return;
  }

  const rumPawn* pcPawn{ GetPawnAtCurrentTile() };
  if( pcPawn )
  {
    m_uiSelectedPawn = pcPawn->GetGameID();
    SetBrush( pcPawn->GetAssetID() );
  }

  m_bMovingPawn = true;
}


void smMapWidget::onPickPawn()
{
  if( !m_pcMap )
  {
    return;
  }

  rumPawn* pcPawn{ GetPawnAtCurrentTile() };
  if( pcPawn )
  {
    m_uiSelectedPawn = pcPawn->GetGameID();
    SetBrush( pcPawn->GetAssetID() );
    m_pcParent->OnPawnPicked( pcPawn );
  }
}


void smMapWidget::onPickTile()
{
  if( !m_pcMap )
  {
    return;
  }

  const rumAssetID eTileID{ GetTile( QPoint{ m_cSelectedPos.x(), m_cSelectedPos.y() } ) };
  SetBrush( eTileID );
  m_pcParent->OnTilePicked( eTileID );
}


void smMapWidget::onPortalOpen()
{
  bool bSuccess{ false };

  rumPawn* pcPawn{ GetPawnAtCurrentTile() };
  if( pcPawn && ( pcPawn->GetPawnType() == rumPawn::Portal_PawnType ) )
  {
// Editor must provide EditorOpenPortal function, with a dialog for choosing which in the event there is an alt
#pragma message( "TODO - Support engine level properties? Or call a script function to provide the info?" )

    Sqrat::Object sqDestMapID{ pcPawn->GetProperty( Map_ID_PropertyID ) };

    // Get the map name and script id
    if( sqDestMapID.GetType() != OT_INTEGER )
    {
      return;
    }

    const rumMapAsset* pcMapAsset{ rumMapAsset::Fetch( (rumAssetID)sqDestMapID.Cast<int32_t>() ) };
    rumAssert( pcMapAsset );
    if( !pcMapAsset )
    {
      return;
    }

    std::string strFilePath;
    if( rumResource::FindFile( pcMapAsset->GetFilename(), strFilePath ) )
    {
      const QFileInfo cFileInfo( strFilePath.c_str() );

      MainWindow* pcMain{ MainWindow::GetMainWindow() };
      Q_ASSERT( pcMain );
      QWidget* pcWidget{ pcMain->AddOrOpenTab( cFileInfo.canonicalFilePath(), TYPE_MAP ) };
      if( pcWidget )
      {
        MapEditor* pcMapEditor{ qobject_cast<MapEditor*>( pcWidget ) };
        Q_ASSERT( pcMapEditor );

        QPoint cPos( 0, 0 );

        Sqrat::Object sqPosX{ pcPawn->GetProperty( Map_PosX_PropertyID ) };
        if( sqPosX.GetType() == OT_INTEGER )
        {
          cPos.setX( sqPosX.Cast<int32_t>() );
        }

        Sqrat::Object sqPosY{ pcPawn->GetProperty( Map_PosY_PropertyID ) };
        if( sqPosY.GetType() == OT_INTEGER )
        {
          cPos.setY( sqPosY.Cast<int32_t>() );
        }

        pcMapEditor->GotoMapPosition( cPos );
        bSuccess = true;
      }
    }
  }

  if( !bSuccess )
  {
    QMessageBox::critical( this, tr( "Portal Open Failure" ), "Error: The portal destination is invalid." );
  }
}


void smMapWidget::onShiftPawnsUp()
{
  const rumPosition cPos( m_cSelectedPos.x(), m_cSelectedPos.y() );

  if( !m_pcMap )
  {
    return;
  }

  m_pcMap->ShiftPawnsUp( cPos );
  MarkDirty();
  m_pcParent->GetTileWidget()->Update();
}


void smMapWidget::onShiftPawnsDown()
{
  const rumPosition cPos( m_cSelectedPos.x(), m_cSelectedPos.y() );

  if( !m_pcMap )
  {
    return;
  }

  m_pcMap->ShiftPawnsDown( cPos );
  MarkDirty();
  m_pcParent->GetTileWidget()->Update();
}


void smMapWidget::paintGL()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };
  rumScript::ExecOptionalFunc( Sqrat::RootTable( pcVM ), "OnFrameStart", GetElapsedTime() );

  rumPawn::Update();

  const QSizeF cWidgetSize{ size() };
  if( cWidgetSize.width() != m_pcParent->GetMapWidth() || cWidgetSize.height() != m_pcParent->GetMapHeight() )
  {
    resize( m_pcParent->GetMapWidth(), m_pcParent->GetMapHeight() );
  }

  glClearColor( 0.f, 0.f, 0.f, 0.f );
  glClear( GL_COLOR_BUFFER_BIT );

  glLoadIdentity();
  glTranslatef( 1, 1, 0 );

  const QPoint& rcMapPos{ GetMapPos() };

  if( MiniMode == m_eMapMode )
  {
    glEnable( GL_TEXTURE_2D );
    Q_ASSERT( m_pcMap );
    m_pcMap->Draw( rumPosition(), rumPosition( rcMapPos.x(), rcMapPos.y() ) );
    glDisable( GL_TEXTURE_2D );

    if( m_bHasFocus )
    {
      glPushAttrib( GL_CURRENT_BIT );
      glColor3f( s_fCursorColor, s_fCursorColor, s_fCursorColor );
      glBegin( GL_LINE_LOOP );
      glVertex2f( m_fSelectionRect.left(), m_fSelectionRect.top() );
      glVertex2f( m_fSelectionRect.right(), m_fSelectionRect.top() );
      glVertex2f( m_fSelectionRect.right(), m_fSelectionRect.bottom() );
      glVertex2f( m_fSelectionRect.left(), m_fSelectionRect.bottom() );
      glEnd();
      glPopAttrib();
    }
  }
  else if( EditMode == m_eMapMode )
  {
    glEnable( GL_TEXTURE_2D );
    Q_ASSERT( m_pcMap );
    m_pcMap->Draw( rumPosition(), rumPosition( rcMapPos.x(), rcMapPos.y() ) );
    glDisable( GL_TEXTURE_2D );

    if( m_bHasFocus && m_eBrush != INVALID_ASSET_ID )
    {
      // Draw the brush asset with the cursor
      rumAssetID eGraphicID{ INVALID_ASSET_ID };

      const rumTileAsset* pcTile{ rumTileAsset::Fetch( m_eBrush ) };
      if( pcTile )
      {
        eGraphicID = pcTile->GetGraphicID();
      }
      else
      {
        const rumPawnAsset* pcPawn{ rumPawnAsset::Fetch( m_eBrush ) };
        if( pcPawn )
        {
          eGraphicID = pcPawn->GetGraphicID();
        }
      }

      if( eGraphicID != INVALID_ASSET_ID )
      {
        rumGraphic* pcGraphic{ rumGraphic::Fetch( eGraphicID ) };
        if( pcGraphic )
        {
          glEnable( GL_TEXTURE_2D );

          // Set to 90% opaque, draw, then restore old level
          uint32_t iOldLevel{ pcGraphic->GetTransparentLevel() };
          pcGraphic->SetTransparentLevel( RUM_ALPHA_OPAQUE * 0.9f );
          pcGraphic->DrawAnimation( rumPoint( m_fSelectionRect.left(), m_fSelectionRect.top() ) );
          pcGraphic->SetTransparentLevel( iOldLevel );

          glDisable( GL_TEXTURE_2D );
        }
      }
    }

    // Highlight the selected square
    glPushAttrib( GL_CURRENT_BIT );
    glColor3f( s_fCursorColor, s_fCursorColor, s_fCursorColor );
    glBegin( GL_LINE_LOOP );
    glVertex2f( m_fSelectionRect.left(), m_fSelectionRect.top() );
    glVertex2f( m_fSelectionRect.right(), m_fSelectionRect.top() );
    glVertex2f( m_fSelectionRect.right(), m_fSelectionRect.bottom() );
    glVertex2f( m_fSelectionRect.left(), m_fSelectionRect.bottom() );
    glEnd();
    glPopAttrib();
  }
}


int smMapWidget::ResizeMap( uint32_t i_uiCols, uint32_t i_uiRows, int32_t i_eAnchor, rumAssetID i_eTileID )
{
  const int32_t eResult{ ResizeMapInternal( m_pcMap, i_uiCols, i_uiRows, i_eAnchor, i_eTileID ) };

  // Reset to origin since a previously visible part of the map may no longer exist
  SetMapPos( 0, 0 );

  return eResult;
}


int smMapWidget::ResizeMapInternal( EditorMap* i_pcMap, uint32_t i_uiCols, uint32_t i_uiRows, int32_t i_eAnchor,
                                    rumAssetID i_eTileID )
{
  if( !i_pcMap )
  {
    return RESULT_FAILED;
  }

  // Add or remove columns
  if( i_uiCols > i_pcMap->GetCols() )
  {
    // Add columns
    if( ( i_eAnchor == ANCHOR_TOP_LEFT ) || ( i_eAnchor == ANCHOR_LEFT ) || ( i_eAnchor == ANCHOR_BOTTOM_LEFT ) )
    {
      i_pcMap->AddColsRight( i_uiCols - i_pcMap->GetCols(), i_eTileID );
      MarkDirty();
    }
    else if( ( i_eAnchor == ANCHOR_TOP ) || ( i_eAnchor == ANCHOR_CENTER ) || ( i_eAnchor == ANCHOR_BOTTOM ) )
    {
      const uint32_t iNewTotal{ i_uiCols - i_pcMap->GetCols() };
      const uint32_t iNewLeft{ iNewTotal / 2 };
      uint32_t iNewRight{ iNewLeft };

      if( ( iNewLeft + iNewRight ) < iNewTotal )
      {
        // Prefer adds to the right
        ++iNewRight;
      }

      Q_ASSERT( ( iNewLeft + iNewRight ) == iNewTotal );

      i_pcMap->AddColsLeft( iNewLeft, i_eTileID );
      i_pcMap->AddColsRight( iNewRight, i_eTileID );
      MarkDirty();
    }
    else
    {
      i_pcMap->AddColsLeft( i_uiCols - i_pcMap->GetCols(), i_eTileID );
      MarkDirty();
    }
  }
  else if( i_uiCols < i_pcMap->GetCols() )
  {
    // Remove columns
    if( ( i_eAnchor == ANCHOR_TOP_LEFT ) || ( i_eAnchor == ANCHOR_LEFT ) || ( i_eAnchor == ANCHOR_BOTTOM_LEFT ) )
    {
      i_pcMap->DelColsRight( i_pcMap->GetCols() - i_uiCols );
      MarkDirty();
    }
    else if( ( i_eAnchor == ANCHOR_TOP ) || ( i_eAnchor == ANCHOR_CENTER ) || ( i_eAnchor == ANCHOR_BOTTOM ) )
    {
      const uint32_t iDelTotal{ i_uiCols - i_pcMap->GetCols() };
      uint32_t iDelLeft{ iDelTotal / 2 };
      const uint32_t iDelRight{ iDelLeft };

      if( ( iDelLeft + iDelRight ) < iDelTotal )
      {
        // Prefer removal to the left
        ++iDelLeft;
      }

      Q_ASSERT( ( iDelLeft + iDelRight ) == iDelTotal );

      i_pcMap->DelColsLeft( iDelLeft );
      i_pcMap->DelColsRight( iDelRight );
      MarkDirty();
    }
    else
    {
      i_pcMap->DelColsLeft( i_pcMap->GetCols() - i_uiCols );
      MarkDirty();
    }
  }

  // Add or remove rows
  if( i_uiRows > i_pcMap->GetRows() )
  {
    // Add rows
    if( ( i_eAnchor == ANCHOR_TOP_LEFT ) || ( i_eAnchor == ANCHOR_TOP ) || ( i_eAnchor == ANCHOR_TOP_RIGHT ) )
    {
      i_pcMap->AddRowsBottom( i_uiRows - i_pcMap->GetRows(), i_eTileID );
      MarkDirty();
    }
    else if( ( i_eAnchor == ANCHOR_LEFT ) || ( i_eAnchor == ANCHOR_CENTER ) || ( i_eAnchor == ANCHOR_RIGHT ) )
    {
      const uint32_t iNewTotal{ i_uiRows - i_pcMap->GetRows() };
      const uint32_t iNewTop{ iNewTotal / 2 };
      uint32_t iNewBottom{ iNewTop };

      if( ( iNewTop + iNewBottom ) < iNewTotal )
      {
        // Prefer adds on the bottom
        ++iNewBottom;
      }

      Q_ASSERT( ( iNewTop + iNewBottom ) == iNewTotal );

      i_pcMap->AddRowsTop( iNewTop, i_eTileID );
      i_pcMap->AddRowsBottom( iNewBottom, i_eTileID );
      MarkDirty();
    }
    else
    {
      i_pcMap->AddRowsTop( i_uiRows - i_pcMap->GetRows(), i_eTileID );
      MarkDirty();
    }
  }
  else if( i_uiRows < i_pcMap->GetRows() )
  {
    // Remove rows
    if( ( i_eAnchor == ANCHOR_TOP_LEFT ) || ( i_eAnchor == ANCHOR_TOP ) || ( i_eAnchor == ANCHOR_TOP_RIGHT ) )
    {
      i_pcMap->DelRowsBottom( i_pcMap->GetRows() - i_uiRows );
      MarkDirty();
    }
    else if( ( i_eAnchor == ANCHOR_LEFT ) || ( i_eAnchor == ANCHOR_CENTER ) || ( i_eAnchor == ANCHOR_RIGHT ) )
    {
      const uint32_t iDelTotal{ i_uiCols - i_pcMap->GetCols() };
      uint32_t iDelTop{ iDelTotal / 2 };
      const uint32_t iDelBottom{ iDelTop };

      if( ( iDelTop + iDelBottom ) < iDelTotal )
      {
        // Prefer removal at the Top
        ++iDelTop;
      }

      Q_ASSERT( ( iDelTop + iDelBottom ) == iDelTotal );

      i_pcMap->DelRowsTop( iDelTop );
      i_pcMap->DelRowsBottom( iDelBottom );
      MarkDirty();
    }
    else
    {
      i_pcMap->DelRowsTop( i_pcMap->GetRows() - i_uiRows );
      MarkDirty();
    }
  }

  return RESULT_SUCCESS;
}


void smMapWidget::SaveMap( const QString& i_strPath )
{
  if( m_pcMap->Save( qPrintable( i_strPath ) ) == RESULT_SUCCESS )
  {
    m_bDirty = false;

    // Update this map's tab to reflect dirty state
    MainWindow* pcMain{ MainWindow::GetMainWindow() };
    Q_ASSERT( pcMain );
    pcMain->SetTabIcon( pcMain->GetCurrentTabIndex(), QIcon() );
  }
  else
  {
    QMessageBox::critical( this, tr( "Map Save Failure" ), "Failed to save map!" );
  }
}


void smMapWidget::SetBrush( rumAssetID i_eAssetID )
{
  m_eBrushPrev = m_eBrush;
  m_eBrush = i_eAssetID;
}


void smMapWidget::SetBorderTile( rumAssetID i_eTileID )
{
  Q_ASSERT( m_pcMap );
  if( !m_pcMap )
  {
    return;
  }

  if( m_pcMap->GetBorderTile() != i_eTileID )
  {
    m_pcMap->SetBorderTile( i_eTileID );
    MarkDirty();
  }
}


void smMapWidget::SetMapPos( const QPoint& i_rcMapPos )
{
  Q_ASSERT( m_pcMap );
  if( !m_pcMap )
  {
    return;
  }

  m_pcMap->SetDrawPosition( rumPosition( i_rcMapPos.x(), i_rcMapPos.y() ) );

  if( MiniMode == m_eMapMode )
  {
    UpdateMiniMode( false );
  }
  else if( EditMode == m_eMapMode )
  {
    UpdateEditMode( false );
  }

  // Recalculate which tile the mouse is now hovering over
  const QPoint& rcMousePos{ mapFromGlobal( QCursor::pos() ) };
  UpdateSelectedPos( rcMousePos );
}


void smMapWidget::UpdateEditMode( bool i_bUpdateGL )
{
  m_fTilePixelSize.setX( MainWindow::GetTileWidth() );
  m_fTilePixelSize.setY( MainWindow::GetTileHeight() );

  const QPoint& rcMapPos{ GetMapPos() };

  m_fSelectionRect.setX( ( m_cSelectedPos.x() - rcMapPos.x() ) * m_fTilePixelSize.x() );
  m_fSelectionRect.setWidth( m_fTilePixelSize.x() );
  m_fSelectionRect.setY( ( m_cSelectedPos.y() - rcMapPos.y() ) * m_fTilePixelSize.y() );
  m_fSelectionRect.setHeight( m_fTilePixelSize.y() );

  MapDrawProps cDrawProps;
  cDrawProps.m_uiHorizontalTiles = GetNumColumns();
  cDrawProps.m_uiVerticalTiles = GetNumRows();
  cDrawProps.m_bScaleTiles = false;
  cDrawProps.m_fTileHorizontalScale = 1.0f;
  cDrawProps.m_fTileVerticalScale = 1.0f;
  cDrawProps.m_fHorizontalTileOffset = m_fTilePixelSize.x();
  cDrawProps.m_fVerticalTileOffset = m_fTilePixelSize.y();
  
  if( m_pcMap )
  {
    m_pcMap->SetDrawProps( cDrawProps );
  }

  if( i_bUpdateGL )
  {
    update();
  }
}


void smMapWidget::UpdateMiniMode( bool i_bUpdateGL )
{
  const uint32_t iMaxMiniMapTiles{ 64U };
  int32_t iNumVisibleTiles{ 0 };

  // Determine aspect ratios
  if( m_pcParent->GetMapWidth() > m_pcParent->GetMapHeight() )
  {
    iNumVisibleTiles = qMin( iMaxMiniMapTiles, GetNumMapRows() );

    // Fit to height - how many tiles can we fit vertically?
    const float fTileSizeRatio{ MainWindow::GetTileWidth() / static_cast<float>( MainWindow::GetTileHeight() ) };
    m_fTilePixelSize.setY( m_pcParent->GetMapHeight() / (float)iNumVisibleTiles );
    m_fTilePixelSize.setX( m_fTilePixelSize.y() * fTileSizeRatio );
  }
  else
  {
    iNumVisibleTiles = qMin( iMaxMiniMapTiles, GetNumMapColumns() );

    // Fit to width - how many tiles can we fit horizontally?
    const float fTileSizeRatio{ MainWindow::GetTileHeight() / static_cast<float>( MainWindow::GetTileWidth() ) };
    m_fTilePixelSize.setX( m_pcParent->GetMapWidth() / (float)iNumVisibleTiles );
    m_fTilePixelSize.setY( m_fTilePixelSize.x() * fTileSizeRatio );
  }

  // The mini map selection size in pixels determined by the zoom level of the mini map view
  QPointF cMiniSelectionPixelSize;
  cMiniSelectionPixelSize.setX( m_fTilePixelSize.x() * GetNumColumns() );
  cMiniSelectionPixelSize.setY( m_fTilePixelSize.y() * GetNumRows() );

  MapDrawProps cDrawProps;
  cDrawProps.m_uiHorizontalTiles = iNumVisibleTiles;
  cDrawProps.m_uiVerticalTiles = iNumVisibleTiles;
  cDrawProps.m_bScaleTiles = true;
  cDrawProps.m_fTileHorizontalScale = m_fTilePixelSize.x() / MainWindow::GetTileWidth();
  cDrawProps.m_fTileVerticalScale = m_fTilePixelSize.y() / MainWindow::GetTileHeight();
  cDrawProps.m_fHorizontalTileOffset = m_fTilePixelSize.x();
  cDrawProps.m_fVerticalTileOffset = m_fTilePixelSize.y();
  m_pcMap->SetDrawProps( cDrawProps );

  // Recalculate which tile the mouse is now hovering over
  const QPoint& rcMousePos{ mapFromGlobal( QCursor::pos() ) };

  // Center the selection rectangle around the mouse cursor
  m_fSelectionRect.setX( (float)rcMousePos.x() - ( cMiniSelectionPixelSize.x() / 2.f ) );
  m_fSelectionRect.setY( (float)rcMousePos.y() - ( cMiniSelectionPixelSize.y() / 2.f ) );

  const QPoint& rcMapPos{ GetMapPos() };

  m_cSelectedMiniMapPos.setX( rcMapPos.x() + m_fSelectionRect.x() / m_fTilePixelSize.x() );
  m_cSelectedMiniMapPos.setY( rcMapPos.y() + m_fSelectionRect.y() / m_fTilePixelSize.y() );

  m_fSelectionRect.setWidth( cMiniSelectionPixelSize.x() );
  m_fSelectionRect.setHeight( cMiniSelectionPixelSize.y() );

  if( i_bUpdateGL )
  {
    update();
  }

  // There is a new selected position - update screen
  const QString& strStatus
  {
    QString( " Pos X %1 Y %2 | %3x%4 " )
      .arg( m_cSelectedPos.x() )
      .arg( m_cSelectedPos.y() )
      .arg( GetNumMapColumns() )
      .arg( GetNumMapRows() )
  };

  MainWindow::SetStatusBarText( strStatus );
}


void smMapWidget::UpdateSelectedPos( const QPoint& i_rcMousePos )
{
  if( !m_pcMap )
  {
    return;
  }

  const QPoint& rcMapPos{ GetMapPos() };
  const QPoint cOldSelectedPos( m_cSelectedPos );
  m_cSelectedPos.setX( ( i_rcMousePos.x() / MainWindow::GetTileWidth() ) + rcMapPos.x() );
  m_cSelectedPos.setY( ( i_rcMousePos.y() / MainWindow::GetTileHeight() ) + rcMapPos.y() );

  if( m_cSelectedPos != cOldSelectedPos )
  {
    // There is a new selected position - update screen
    const QString& strStatus
    {
      QString( " Pos X %1 Y %2 | %3x%4 " )
        .arg( m_cSelectedPos.x() )
        .arg( m_cSelectedPos.y() )
        .arg( GetNumMapColumns() )
        .arg( GetNumMapRows() )
    };

    MainWindow::SetStatusBarText( strStatus );

    update();
    m_pcParent->GetTileWidget()->Update();
  }
}


void smMapWidget::wheelEvent( QWheelEvent* i_pcEvent )
{
  /*if( MiniMode == m_eMapMode )
  {
    int32_t iNumDegrees = pcEvent->delta() / 8;
    int32_t iNumSteps = iNumDegrees / 15;

    if( iNumDegrees > 0 )
    {
      m_pcParent->GetMapWidth() += MainWindow::GetTileWidth();
      m_pcParent->GetMapHeight() += MainWindow::GetTileHeight();
    }
    else
    {
      m_pcParent->GetMapWidth() -= MainWindow::GetTileWidth();
      m_pcParent->GetMapHeight() -= MainWindow::GetTileHeight();
    }

    pcEvent->accept();
    UpdateMiniMode();
  }*/
}
