#ifndef MAPEDITOR_H
#define MAPEDITOR_H

#include <QLabel>
#include <QLineEdit>
#include <QWidget>

#undef TRUE
#undef FALSE

#include <u_pawn.h>

class QItemSelection;
class QListWidget;
class QListWidgetItem;
class QTableWidget;
class QTableWidgetItem;
class smMapWidget;
class smTileWidget;

namespace Ui
{
  class MapEditor;
}


class MapEditor : public QWidget
{
  Q_OBJECT

  using PropertyContainer = rumPropertyContainer::PropertyContainer;

public:
  MapEditor( QWidget* i_pcParent = 0 );
  MapEditor( const QString& i_strFilePath, QWidget* i_pcParent = 0 );
  ~MapEditor() override;

  void CopyPawnProperties( rumPawn* i_pcPawn );
  void PastePawnProperties( rumPawn* i_pcPawn );

  void EditPawn( rumPawn* i_pcPawn );

  const smMapWidget* GetMapWidget() const;
  smTileWidget* GetTileWidget() const;

  // Returns the height of the map area in pixels - this is used by other systems to match the map height
  uint32_t GetMapHeight() const;
  uint32_t GetMapWidth() const;

  void InspectTile( rumAssetID i_eTileID );

  // Returns true if the map widget has been modified
  bool IsDirty() const;

  void OnPawnAdded( rumPawn* i_pcPawn );
  void OnPawnPicked( rumPawn* i_pcPawn );
  void OnPawnRemoved( const rumPawn* i_pcPawn );

  void OnTilePainted( const QPoint& i_rcPos );
  void OnTilePicked( rumAssetID i_eTileID );

  // If dirty, this gives the user a chance to save or cancel
  // Returns true unless the user presses cancel
  bool RequestClose();

  void GotoMapPosition( const QPoint& i_rcPos );
  void Save();

  const QString& GetMapFilePath() const
  {
    return m_strFilePath;
  }

  // TODO - change to slot?
  void OnAnimTimer();

  bool FailedLoad() const
  {
    return m_bFailure;
  }

private slots:

  void ItemComboPropertyChanged( const QString& i_strText );
  void OnAssetFilterChanged();
  void OnCellStartPropertyEdit( int32_t i_iRow, int32_t i_iCol );
  void OnCopyPawnProperties();
  void OnEditPawn();
  void OnInspectTile();
  void OnListWidgetItemClicked( QListWidgetItem* i_pcItem );
  void OnPastePawnProperties();
  void OnPropertyAdded( rumAssetID i_ePropertyID );
  void OnPropertyFilterChanged();
  void onPropertyTableItemChanged( QTableWidgetItem* i_pcItem );

  void on_actionSettings_triggered();
  void on_actionGoto_Pos_triggered();
  void on_actionNew_Property_triggered();
  void on_actionRemove_Property_triggered();
  void on_actionSave_triggered();
  void on_lineEdit_textChanged( const QString& i_rcArg );
  void on_mapWidget_customContextMenuRequested( const QPoint& i_rcPos );
  void on_toolButton_clicked();

  void selectionChanged_Property( const QItemSelection& i_rcSelected, const QItemSelection& i_rcDeselected );

  void StringTokenPropertyChanged( rumTokenID i_eTokenID );

private:

  void AddAssetProperty( int32_t i_iRow, const QString& i_strAttribute, const QString& i_strValue,
                         const QString& i_strTooltip );
  bool AddInstanceProperty( int32_t i_iRow, const rumPropertyAsset& i_rcAsset, Sqrat::Object i_sqValue );

  void FilterListWidget( QListWidget* i_pcListWidget, const QString& i_strFilter ) const;

  rumAssetID GetPropertyID( int32_t i_iRow ) const;

  void Init();

  template<typename T>
  void InitListWidget( QListWidget* i_pcListWidget );

  void keyPressEvent( QKeyEvent* i_pcEvent );

  bool Load( const QString& i_strPath );

  // The user selected an item from one of the four list item boxes
  void OnBrushChanged( const QListWidgetItem* i_pcItem );

  bool Open( const QString& i_strFilePath );

  void UpdateAssetFilter();
  void UpdatePropertyFilter();

  Ui::MapEditor* m_pcUI;

  // The number of map columns and rows shown in the editor
  static int32_t s_iEditorCols;
  static int32_t s_iEditorRows;

  QString m_strFilePath;

  QLabel m_cAssetFilterLabel;
  QLineEdit m_cAssetFilterEdit;
  QAction* m_pcAssetFilterAction;

  QLabel m_cPropertyFilterLabel;
  QLineEdit m_cPropertyFilterEdit;
  QAction* m_pcPropertyFilterAction;

  rumPawn* m_pcCurrentPawn{ nullptr };

  PropertyContainer m_cPropertyClipboard;

  bool m_bFailure;
};

#include <mapeditor.inl>

#endif // MAPEDITOR_H
