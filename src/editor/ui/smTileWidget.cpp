#include <smTileWidget.h>

#include <e_graphics.h>
#include <e_map.h>
#include <e_pawn.h>

#include <u_pos_iterator.h>

#include <mainwindow.h>
#include <mapeditor.h>
#include <smMapWidget.h>

float smTileWidget::s_fCursorColor = 0.f;
float smTileWidget::s_fCursorAnimDelta = 0.25f;
uint32_t smTileWidget::s_iHeight = 0;
uint32_t smTileWidget::s_iWidth = 0;


smTileWidget::smTileWidget( QWidget* i_pcParent )
  : QOpenGLWidget( i_pcParent )
  , m_pcParent( nullptr )
{
  m_pcParent = qobject_cast<MapEditor*>( i_pcParent );
}


void smTileWidget::initializeGL()
{
  // Parent should always be valid at this point
  Q_ASSERT( m_pcParent );

  initializeOpenGLFunctions();

  const uint32_t iHeight{ m_pcParent->GetMapHeight() };
  const uint32_t iWidth{ MainWindow::GetTileWidth() };

  glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

  // Resize the underlying widget
  resize( iWidth, iHeight );

  glViewport( 0, 0, iWidth, iHeight );

  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();

  // left, right, bottom, top, znear, zfar
  glOrtho( 0, iWidth, iHeight, 0, -1, 1 );

  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();

  glDisable( GL_CULL_FACE );
  glDisable( GL_BLEND );
  glDisable( GL_LIGHTING );
  glDisable( GL_DEPTH_TEST );
}


void smTileWidget::OnAnimTimer()
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


void smTileWidget::paintGL()
{
  const smMapWidget* pcMapWidget{ m_pcParent->GetMapWidget() };

  glClearColor( 0.f, 0.f, 0.f, 1.f );
  glClear( GL_COLOR_BUFFER_BIT );

  if( !pcMapWidget->HasFocus() || pcMapWidget->GetMapMode() != smMapWidget::EditMode )
  {
    return;
  }

  const uint32_t iTileHeight{ MainWindow::GetTileHeight() };
  const uint32_t iTileWidth{ MainWindow::GetTileWidth() };

  const EditorMap* pcMap{ pcMapWidget->GetMap() };
  const QPoint& rcMapPos{ pcMapWidget->GetSelectedPos() };
  const rumPosition cPos( rcMapPos.x(), rcMapPos.y() );

  Q_ASSERT( pcMap );

  glEnable( GL_TEXTURE_2D );

  int32_t iYOffset{ 0 };
  uint32_t iSelectedY1{ 0 };

  // Draw the base tile
  const rumAssetID eTileID{ pcMap->GetTileID( cPos ) };
  const rumTileAsset* pcTile{ rumTileAsset::Fetch( eTileID ) };
  if( pcTile )
  {
    const rumAssetID eGraphicID{ pcTile->GetGraphicID() };
    const rumGraphic* pcGraphic{ rumGraphic::Fetch( eGraphicID ) };
    if( pcGraphic )
    {
      pcGraphic->DrawAnimation( rumPoint( 0, iYOffset ) );
      iYOffset += iTileHeight;
    }
  }

  // Draw all pawns at this location
  rumPositionIterator iter{ pcMap->GetPositionIterator( cPos ) };
  while( !iter.Done() )
  {
    // Move the selection to the beginning of the pawn display
    iSelectedY1 = iTileHeight;

    Sqrat::Object sqInstance{ iter.GetNextObject() };
    if( sqInstance.GetType() == OT_INSTANCE )
    {
      const EditorPawn* pcPawn{ sqInstance.Cast<EditorPawn*>() };
      if( pcPawn )
      {
        const rumGraphic* pcGraphic{ rumGraphic::Fetch( pcPawn->GetGraphicID() ) };
        if( pcGraphic )
        {
          pcGraphic->DrawAnimation( rumPoint( 0, iYOffset ) );
          iYOffset += iTileHeight;
        }
      }
    }
  }

  glDisable( GL_TEXTURE_2D );

  const uint32_t iSelectedY2{ iSelectedY1 + iTileHeight };

  // Highlight the selected square
  glPushAttrib( GL_CURRENT_BIT );
  glColor3f( s_fCursorColor, s_fCursorColor, s_fCursorColor );
  glBegin( GL_LINE_LOOP );
  glVertex2f( 0.f, iSelectedY1 + 1 );
  glVertex2f( iTileWidth - 1, iSelectedY1 + 1 );
  glVertex2f( iTileWidth - 1, iSelectedY2 );
  glVertex2f( 0.f, iSelectedY2 );
  glEnd();
  glPopAttrib();
}
