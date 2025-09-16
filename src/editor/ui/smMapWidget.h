#ifndef SM_MAPWIDGET_H
#define SM_MAPWIDGET_H

#undef TRUE
#undef FALSE

#include <platform.h>
#include <u_script.h>
#include <u_structs.h>

#include <QOpenGLFunctions>
#include <QOpenGLWidget>

class EditorMap;
class MapEditor;
class QKeyEvent;
class rumPawn;


class smMapWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
  Q_OBJECT

public:
  smMapWidget( QWidget* i_pcParent = 0 );
  ~smMapWidget() override;

  void initializeGL() override;
  void paintGL() override;

  // Fetches the map used by the specified VM
  const EditorMap* GetMap() const;

  // The various modes the map widget can be in
  enum MapMode
  {
    EditMode, MiniMode
  };

  rumAssetID GetBorderTile() const;

  MapMode GetMapMode() const
  {
    return m_eMapMode;
  }

  // Returns the number of rows or columns on the underlying map, regardless of map view mode
  uint32_t GetNumMapColumns() const;
  uint32_t GetNumMapRows() const;

  rumAssetID GetExitMapID() const;
  QPoint GetExitMapPos() const;

  rumPawn* GetPawnAtCurrentTile();
  bool HasPawnAtCurrentTile();

  rumAssetID GetTile( const QPoint& i_rcPos ) const;

  // Returns true if this widget has mouse focus
  bool HasFocus() const
  {
    return m_bHasFocus;
  }
  bool IsDirty() const
  {
    return m_bDirty;
  }

  bool LoadMap( const QString &strPath );

  void SetBrush( rumAssetID i_eAssetID );

  QPoint GetMapPos() const;

  void SaveMap( const QString &strPath );

  void SetMapPos( const QPoint& i_rcPos );
  void SetMapPos( int32_t i_iX, int32_t i_iY )
  {
    SetMapPos( QPoint( i_iX, i_iY ) );
  }

  // Returns the coordinates of the currently selected tile
  QPoint GetSelectedPos() const
  {
    return m_cSelectedPos;
  }

  void OnAnimTimer();

  // Returns the number of visible rows or columns as displayed in the edit view mode
  static uint32_t GetNumColumns()
  {
    return s_iCols;
  }

  static uint32_t GetNumRows()
  {
    return s_iRows;
  }

public slots:

  void CenterOnPosition( const QPoint& i_rcPos );
  void CenterOnPosition( int32_t i_iX, int32_t i_iY )
  {
    CenterOnPosition( QPoint( i_iX, i_iY ) );
  }

private slots:

  // Deletes the pawn at the top-most position (the most visible on the client)
  void onDeleteTopPawn();

  // Modifies the map's exit destination
  void OnMapExitChanged( rumAssetID i_eMapID, const QPoint& i_rcPos );

  // Moves the pawn from the current position to the selected destination position
  void onMovePawn();

  // Sets the pawn at the selected position to be the active brush
  void onPickPawn();

  // Sets the tile at the selected position to be the active brush
  void onPickTile();

  // Opens the selected portal's destination map
  void onPortalOpen();

  // Shift the top-most stacked pawn to the left or to the right so that other pawns below it can be accessed
  void onShiftPawnsUp();
  void onShiftPawnsDown();

  int32_t ResizeMap( uint32_t i_iCols, uint32_t i_iRows, int32_t i_eAnchor, rumAssetID i_eTileID );

  void SetBorderTile( rumAssetID i_eTileID );

private:

  // The cached number of visible rows and cols that fits into the map control based on the user specified tile size
  // TODO - this same info might be already cached in mapeditor
  static uint32_t s_iCols;
  static uint32_t s_iRows;

  void ApplyBrush();

  // Marks map as changed, requiring a save for change persistence
  void MarkDirty();

  void MovePawn();

  int32_t ResizeMapInternal( EditorMap* i_pcMap, uint32_t i_iCols, uint32_t i_iRows, int32_t i_eAnchor,
                             rumAssetID i_eTileID );

  // These methods calculate what the paintGL method will display
  void UpdateEditMode( bool i_bUpdateGL = true );
  void UpdateMiniMode( bool i_bUpdateGL = true );

  void UpdateSelectedPos( const QPoint& i_rcMousePos );
  void UpdateSelectedPos( int32_t i_iX, int32_t i_iY )
  {
    UpdateSelectedPos( QPoint( i_iX, i_iY ) );
  }

  void enterEvent( QEvent* i_pcEvent );
  bool keyboardMovement();
  void keyPressEvent( QKeyEvent* i_pcEvent );
  void leaveEvent( QEvent* i_pcEvent );
  void mouseMoveEvent( QMouseEvent* i_pcEvent );
  void mousePressEvent( QMouseEvent* i_pcEvent );
  void mouseReleaseEvent( QMouseEvent* i_pcEvent );
  void wheelEvent( QWheelEvent* i_pcEvent );

  // Pointer to parent widget
  MapEditor* m_pcParent;

  rumAssetID m_eBrush{ INVALID_ASSET_ID };
  rumAssetID m_eBrushPrev{ INVALID_ASSET_ID };

  EditorMap* m_pcMap;

  QPoint m_cSelectedPos;
  QPoint m_cSelectedMiniMapPos;

  // The tile pixel size of the current map view
  QPointF m_fTilePixelSize;

  // The rectangle representing the selection in the various map modes
  QRectF m_fSelectionRect;

  // Holds the current map mode
  MapMode m_eMapMode;

  rumUniqueID m_uiSelectedPawn;

  // True when a user is holding down the left mouse button
  bool m_bPainting;

  // True when a pawn is being moved from one position to another
  bool m_bMovingPawn;

  // True when the mouse cursor is inside of the widget
  bool m_bHasFocus;

  // True when a map change has occurred that will require saving to persist changes
  bool m_bDirty;

  static float s_fCursorColor;
  static float s_fCursorAnimDelta;

  friend class MapEditor;

  using super = QOpenGLWidget;
};

#endif // SM_MAPWIDGET_H
