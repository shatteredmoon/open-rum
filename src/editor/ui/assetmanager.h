#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>

#include <u_pawn.h>

namespace Ui
{
  class AssetManager;
}

class QItemSelection;
class QTableWidgetItem;

class rumGraphicAsset;
class rumInventoryAsset;
class rumMapAsset;
class rumPawnAsset;
class rumTileAsset;


class AssetManager : public QMainWindow
{
  Q_OBJECT

  using PropertyContainer = rumPropertyContainer::PropertyContainer;
  using TranslationMap = QMap<QString, QString>;

public:

  explicit AssetManager( QWidget* i_pcParent = 0 );
  ~AssetManager();

signals:

  void closed();

private slots:

  void AssetLineEditFinished();

  void ItemComboChanged( const QString& i_strText );
  void ItemComboPropertyChanged( const QString& i_strText );

  void on_actionCopy_All_Properties_triggered();
  void on_actionCopy_Property_triggered();
  void on_actionCopy_Selected_Properties_triggered();

  void on_actionFind_References_triggered();

  void on_actionNew_Asset_triggered();
  void on_actionNew_Property_triggered();

  void on_actionPaste_Properties_Overwrite_triggered();
  void on_actionPaste_Properties_Merge_triggered();

  void on_actionRemove_Asset_triggered();
  void on_actionRemove_Property_triggered();

  void on_actionSave_triggered();

  void on_tableWidget_customContextMenuRequested( const QPoint& i_rcPos );

  void on_tableWidget_3_cellDoubleClicked( int32_t i_iRow, int32_t i_iCol );
  void on_tableWidget_3_customContextMenuRequested( const QPoint& i_rcPos );

  void on_tableWidget_4_customContextMenuRequested( const QPoint& i_rcPos );

  // custom
  void onAssetTableItemChanged( QTableWidgetItem* i_pcItem );

  void OnCellStartEdit( int32_t i_iRow, int32_t i_iCol );
  void OnCellStartPropertyEdit( int32_t i_iRow, int32_t i_iCol );

  void onFilterChanged();
  void onPropertyFilterChanged();

  void OnPropertyAdded( rumAssetID i_ePropertyID );

  void onPropertyTableItemChanged( QTableWidgetItem* i_pcItem );

  void selectionChanged_Asset( const QItemSelection& i_rcSelected, const QItemSelection& i_rcDeselected );
  void selectionChanged_AssetType( const QItemSelection& i_rcSelected, const QItemSelection& i_rcDeselected );
  void selectionChanged_Property( const QItemSelection& i_rcSelected, const QItemSelection& i_rcDeselected );

  void StringTokenPropertyChanged( rumTokenID i_eTokenID );

private:

  void AddAsset( int32_t i_iRow, const rumAsset& i_rcAsset );
  void AddFileAsset( int32_t i_iRow, const rumFileAsset& i_rcAsset );
  void AddGraphicAsset( int32_t i_iRow, const rumGraphicAsset& i_rcAsset );
  void AddInventoryAsset( int32_t i_iRow, const rumInventoryAsset& i_rcAsset );
  void AddMapAsset( int32_t i_iRow, const rumMapAsset& i_rcAsset );
  void AddPawnAsset( int32_t i_iRow, const rumPawnAsset& i_rcAsset );
  void AddPropertyAsset( int32_t i_iRow, const rumPropertyAsset& i_rcAsset, Sqrat::Object i_sqValue );
  void AddTileAsset( int32_t i_iRow, const rumTileAsset& i_rcAsset );

  void AddTable( AssetType i_eAssetType, const QString& i_strName );

  void closeEvent(QCloseEvent* i_pcEvent) override;

  QTableWidgetItem* CreateTableWidgetItemFromProperty( const rumPropertyAsset& i_rcAsset,
                                                       Sqrat::Object i_sqValue ) const;

  rumAsset* GetAsset( int32_t i_iRow ) const;
  rumAssetID GetAssetID( int32_t i_iRow ) const;
  QString GetAssetName( int32_t i_iRow ) const;
  AssetType GetAssetType( int32_t i_iRow ) const;

  rumAsset* GetSelectedAsset() const;
  AssetType GetSelectedAssetType() const;
  Sqrat::Object GetSelectedProperty() const;
  PropertyContainer GetSelectedProperties() const;
  rumAssetID GetSelectedPropertyID() const;
  rumAssetID GetPropertyID( int32_t i_iRow ) const;

  void HandleAssetChange( rumAsset* i_pcAsset, QTableWidgetItem* i_pcItem );
  void HandleFileAssetChange( rumFileAsset* i_pcAsset, QTableWidgetItem* i_pcItem );

  void HandleGraphicAssetChange( rumGraphicAsset* i_pcAsset, QTableWidgetItem* i_pcItem );
  void HandleInventoryAssetChange( rumInventoryAsset* i_pcAsset, QTableWidgetItem* i_pcItem );
  void HandleMapAssetChange( rumMapAsset* i_pcAsset, QTableWidgetItem* i_pcItem );
  void HandlePawnAssetChange( rumPawnAsset* i_pcAsset, QTableWidgetItem* i_pcItem );
  void HandleSoundAssetChange( rumSoundAsset* i_pcAsset, QTableWidgetItem* i_pcItem );
  void HandleTileAssetChange( rumTileAsset* i_pcAsset, QTableWidgetItem* i_pcItem );

  void InitAbstracts();
  void InitBroadcasts();
  void InitCustom();
  void InitGraphics();
  void InitInventory();
  void InitMaps();
  void InitPawns( rumPawn::PawnType i_ePawnType );
  void InitSounds();
  void InitTiles();

  bool IsAssetNameUnique( const QString& i_strName ) const;

  bool IsDirty() const
  {
    return m_bDirty;
  }

  void SetDirty( bool i_bDirty );

  void RefreshPropertyTable();

  void SaveTable( int32_t i_iRow );

  void UpdateFilter();
  void UpdatePropertyFilter();

  template< typename T >
  static T* CreateAsset();

  QLabel m_cFilterLabel;
  QLineEdit m_cFilterEdit;
  QAction* m_pcFilterAction;

  QLabel m_cPropertyFilterLabel;
  QLineEdit m_cPropertyFilterEdit;
  QAction* m_pcPropertyFilterAction;

  bool m_bDirty{ false };

  Ui::AssetManager* m_pcUI;
};

#include <assetmanager.inl>

#endif // ASSETMANAGER_H
