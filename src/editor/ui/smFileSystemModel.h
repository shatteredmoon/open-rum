#ifndef SM_FILESYSTEMMODEL_H
#define SM_FILESYSTEMMODEL_H

#include <QFileSystemModel>
#include <QSet>

class smFileSystemModel : public QFileSystemModel
{
  Q_OBJECT

public:

  smFileSystemModel( QObject* i_pcParent = 0 );
  ~smFileSystemModel();

  typedef QMap<QPersistentModelIndex, Qt::CheckState> FileMap;
  const FileMap& GetFileMap() const
  {
    return m_cFileMap;
  }

  typedef QSet<QString> FileSet;
  const FileSet& GetFileSet() const
  {
    return m_cFileSet;
  }

  bool IsChecked( const QModelIndex& i_rcIndex ) const;
  void ResetCheckedFiles();
  void SetCheckedFiles( const FileSet& i_rcFileSet );

  void SetFilter( const QString& i_strFilter )
  {
    m_strFilter = i_strFilter;
  }

  // Returns true if all subitems of the specified index match the given CheckState value
  bool SubitemsMatch( const QModelIndex& i_rcIndex,
                      const Qt::CheckState i_eCheckState,
                      bool i_bCheckSubdirs );

  QModelIndex SetRootPath( const QString& i_strPath );

signals:

  void onDataChanged( const QModelIndex& i_rcIndex );

private:

  QVariant data( const QModelIndex& i_rcIndex, int32_t i_eRole ) const override;
  Qt::ItemFlags flags( const QModelIndex& i_rcIndex ) const override;
  bool setData( const QModelIndex& i_rcIndex, const QVariant& i_rcVariant, int32_t i_eRole ) override;

  // Called on parent items when a child item is checked or unchecked to determine the new CheckState of the parent
  void ParentCheck( const QModelIndex& i_rcIndex,
                    const Qt::CheckState i_eCheckState,
                    bool i_bCheckSubdirs );

  // Called on all subitems when a directory is checked or unchecked to determine the new CheckState of each child
  void RecursiveCheck( const QModelIndex& i_rcIndex, const Qt::CheckState i_eCheckState );

  // Persistant model index to checkstate mappings for all files in the model
  FileMap m_cFileMap;

  // File path names of files that are currently checked
  FileSet m_cFileSet;

  // A set of directory indexes that should be re-examined for checkstate after a restore from database is performed
  typedef QSet<QModelIndex> RestoreDirSet;
  RestoreDirSet m_cRestoreDirSet;

  QModelIndex m_cRootIndex;

  QString m_strFilter;

  // True when restoring checkstate info from storage
  bool m_bRestoring{ false };

  // Reentrancy guard for recursive passes
  bool m_bRecursivePass{ false };

  typedef QFileSystemModel super;
};

#endif // SM_FILESYSTEMMODEL_H
