#ifndef RELEASEMANAGER_H
#define RELEASEMANAGER_H

#include <platform.h>
#include <u_enum.h>

#include <QMainWindow>
#include <QModelIndex>

namespace Ui
{
  class ReleaseManager;
}

class FileFilterProxyModel;
class smFileSystemModel;
class QItemSelection;

class ReleaseManager : public QMainWindow
{
  Q_OBJECT

public:

  explicit ReleaseManager( QWidget* i_pcParent = 0 );
  ~ReleaseManager();

  // Programmatic close
  void Close();

signals:

  void closed();

private slots:

  void on_actionDelete_Action_triggered();
  void on_actionNew_Action_triggered();
  void on_actionPublish_Action_triggered();
  void on_actionPublish_Databases_triggered();
  void on_actionPublish_Executables_triggered();
  void on_actionPublish_Release_triggered();
  void on_actionSave_triggered();
  void on_checkBox_stateChanged( int32_t i_eState );
  void on_checkBox_2_stateChanged( int32_t i_eState );
  void on_checkBox_3_stateChanged( int32_t i_eState );
  void on_checkBox_4_stateChanged( int32_t i_eState );
  void on_lineEdit_textEdited( const QString& i_strText );
  void on_pushButton_clicked();
  void on_pushButton_Filter_clicked();
  void on_tableWidget_customContextMenuRequested( const QPoint& i_rcPos );
  void on_tableWidget_itemSelectionChanged();

  // Custom slots
  void ItemComboChanged( const QString& i_strText );
  void OnCellStartEdit( int32_t i_iRow, int32_t i_iCol );
  void onDirectoryLoaded( const QString& i_strPath );
  void onTreeDataChanged();
  void onModelLayoutChanged();

private:

  void AddActionItem( int32_t i_iKey, const QString& i_strName, ServiceType i_eServiceType ) const;

  void closeEvent( QCloseEvent* i_pcEvent ) override;

  bool CopyFileCRC( const QString& i_strSource, const QString& i_strDest );

  void DeleteAction( int32_t i_iRow );

  void ExportCSVFiles();
  void ExportCSVFiles( int32_t i_iRow );

  // Returns true if file fails size or CRC check
  bool FileNeedsUpdate( const QString& i_strSource, const QString& i_strDest ) const;

  int32_t GetActionKey( int32_t i_iRow ) const;
  QString GetActionName( int32_t i_iRow ) const;
  ServiceType GetActionService( int32_t i_iRow ) const;

  QString GetDefaultPublishPath() const;

  void ImportCSVFile( int32_t i_iRow );

  // Handles Release and Action item displays
  void InitActionItems();

  bool IsDirty() const
  {
    return m_bDirty;
  }

  void SetDirty( bool i_bDirty );

  // Loads checked file info from the game database
  void LoadFromDatabase();

  void PublishAction( int32_t i_iRow );
  void PublishDatabases();
  void PublishExecutables();

  void ReadSettings();

  QModelIndex SetReleasePath( smFileSystemModel* i_pcModel, FileFilterProxyModel* i_pcProxy );

  Ui::ReleaseManager* m_pcUI{ nullptr };

  smFileSystemModel* m_pcCurrentModel{ nullptr };

  // Note: This used to be stored in table cell UserRole data, but Qt had a very fickle way of handling 64-byte
  // pointers in QVariants. Dropped that in favor of just keeping track of the ProxyModel separately here.
  std::unordered_map<int32_t, FileFilterProxyModel*> m_cFileProxyHash;

  bool m_bDirty{ false };

  // When this is true, the dirty flag will not be set
  bool m_bInternalUpdate{ false };

  static int32_t s_iNextKey;
};

#endif // RELEASEMANAGER_H
