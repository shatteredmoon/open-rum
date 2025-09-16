#ifndef NEWMAP_H
#define NEWMAP_H

#include <u_enum.h>
#include <QDialog>

class QPoint;

namespace Ui
{
  class NewMap;
}

enum
{
  ANCHOR_TOP_LEFT, ANCHOR_TOP, ANCHOR_TOP_RIGHT,
  ANCHOR_LEFT, ANCHOR_CENTER, ANCHOR_RIGHT,
  ANCHOR_BOTTOM_LEFT, ANCHOR_BOTTOM, ANCHOR_BOTTOM_RIGHT
};


class NewMap : public QDialog
{
  Q_OBJECT

public:

  explicit NewMap( QWidget* i_pcParent = 0,
                   rumAssetID i_eExitMapID = INVALID_ASSET_ID,
                   const QPoint& i_rcExitPos = QPoint(),
                   bool i_bEditMode = false );
  ~NewMap();

  bool IsAdding() const
  {
    return !m_bEditMode;
  }

  bool IsEditing() const
  {
    return m_bEditMode;
  }

  void SetBorderTile( rumAssetID i_eTileID );

  void SetExitPosX( uint32_t i_uiPosX );
  void SetExitPosY( uint32_t i_uiPosY );

  void SetNumColumns( uint32_t i_uiCols );
  void SetNumRows( uint32_t i_uiRows );

signals:

  void mapBorderChanged( rumAssetID );
  void mapCreated( const QString& );
  void mapExitChanged( rumAssetID, const QPoint& );
  void mapResized( uint32_t, uint32_t, int32_t, rumAssetID );

private slots:

  void on_buttonBox_accepted();
  void on_lineEditFile_textEdited( const QString& i_strText );
  void on_listWidgetBaseTile_itemSelectionChanged();
  void on_listWidgetBorderTile_itemSelectionChanged();
  void on_listWidgetMap_itemSelectionChanged();
  void on_pushButtonFile_clicked();
  void on_spinBoxCols_valueChanged( int32_t i_nCols );
  void on_spinBoxPosX_valueChanged( int32_t i_nExitPosX );
  void on_spinBoxPosY_valueChanged( int32_t i_nExitPosY );
  void on_spinBoxRows_valueChanged( int32_t i_nRows );

private:
  void InitMapList();
  void InitTileList();
  bool ValidateInput();

  Ui::NewMap* m_pcUI;

  uint32_t m_uiCols;
  uint32_t m_uiRows;

  uint32_t m_uiExitPosX;
  uint32_t m_uiExitPosY;

  QString m_strAbsoluteFilePath;

  // True when map already exists, but settings are being modified
  bool m_bEditMode;
};

#endif // NEWMAP_H
