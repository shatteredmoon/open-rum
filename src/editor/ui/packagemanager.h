#ifndef PACKAGEMANAGER_H
#define PACKAGEMANAGER_H

#include <QMainWindow>
#include <QModelIndex>

#include <platform.h>

namespace Ui
{
  class PackageManager;
}

class FileFilterProxyModel;
class smFileSystemModel;

class PackageManager : public QMainWindow
{
  Q_OBJECT

public:

  // When a hint is specified, file type filters will be applied to file selection (like *.bmp) for instance
  enum AssetTypeHint
  {
    Misc    = 0,
    Graphic = 1,
    Audio   = 2,
    Map     = 3
  };

  explicit PackageManager( QWidget* i_pcParent = 0 );
  ~PackageManager();

  // Programmatic close
  void Close();

  QString GetPackageCSVPath( int32_t i_iRow ) const;
  int32_t GetPackageKey( int32_t i_iRow ) const;
  QString GetPackageName( int32_t i_iRow ) const;
  QString GetPackagePKGPath( int32_t i_iRow ) const;
  AssetTypeHint GetPackageType( int32_t i_iRow ) const;

signals:

  void closed();

private slots:

  void on_actionBuild_All_triggered();
  void on_actionBuild_Selected_triggered();
  void on_actionDelete_Selected_triggered();
  void on_actionNew_Package_triggered();
  void on_actionSave_triggered();

  void on_tableWidget_cellDoubleClicked( int32_t i_iRow, int32_t i_iCol );
  void on_tableWidget_customContextMenuRequested( const QPoint& i_rcPos );
  void on_tableWidget_itemSelectionChanged();

  // Custom slots
  void ItemComboChanged( const QString& i_strText );
  void LineEditFinished();
  void OnCellStartEdit( int32_t i_iRow, int32_t i_iCol );
  void onDirectoryLoaded( const QString& i_strPath );
  void onTreeDataChanged();
  void onModelLayoutChanged();

private:

  static QString GetCSVPath( const QString& i_strName );
  static QString GetPKGPath( const QString& i_strName, AssetTypeHint i_eType );

  void AddPackage( int32_t i_iKey, const QString& i_strName, AssetTypeHint i_eTypeHint ) const;

  void closeEvent( QCloseEvent* i_pcEvent ) override;

  void DeleteRow( int32_t i_iRow );

  // Clears all packages and reloads info
  void Init();

  bool IsDirty() const
  {
    return m_bDirty;
  }

  bool IsPackageNameUnique( const QString& i_strName ) const;

  void SetDirty( bool i_bDirty );

  void ExportCSVFiles();
  void ExportCSVFiles( int32_t i_iRow );

  void ExportPackage( int32_t i_iRow );

  void ImportCSVFile( int32_t i_iRow );

  QModelIndex SetPackagePath( smFileSystemModel* i_pcModel, FileFilterProxyModel* i_pcProxy );

  void UpdatePackageMeta( int32_t i_iRow ) const;

  Ui::PackageManager* m_pcUI{ nullptr };

  smFileSystemModel* m_pcCurrentModel{ nullptr };

  // Note: This used to be stored in table cell UserRole data, but Qt had a very fickle way of handling 64-byte
  // pointers in QVariants. Dropped that in favor of just keeping track of the ProxyModel separately here.
  std::unordered_map<int32_t, FileFilterProxyModel*> m_cFileProxyHash;

  bool m_bDirty{ false };

  // When this is true, the dirty flag will not be set
  bool m_bInternalUpdate{ false };

  static int32_t s_iNextKey;
};

#endif // PACKAGEMANAGER_H
