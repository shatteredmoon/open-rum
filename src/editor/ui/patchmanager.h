#ifndef PATCHMANAGER_H
#define PATCHMANAGER_H

#include <network/u_patcher.h>

#include <QDirIterator>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>

namespace Ui
{
  class PatchManager;
}

class QDir;
class QItemSelection;
class QTableWidgetItem;


class PatchManager : public QMainWindow
{
  Q_OBJECT

public:

  explicit PatchManager( QWidget* i_pcParent = 0 );
  ~PatchManager();

  // Programmatic close
  void Close();

  rumPatcher::PatchType GetSelectedCategoryType() const;
  QString GetSelectedCategoryName() const;

  QString GetSelectedFilePath() const;

signals:

  void closed();

private slots:

  void on_actionClear_Patch_Table_triggered();
  void on_actionGenerate_Patch_triggered();
  void on_actionAdd_Entry_triggered();
  void on_actionMove_Conditional_triggered();
  void on_actionMove_Ignore_triggered();
  void on_actionMove_Remove_triggered();
  void on_actionMove_Standard_triggered();
  void on_actionRemove_Entry_triggered();
  void on_actionSave_triggered();

  void on_tableWidget_Categories_itemSelectionChanged();
  void on_tableWidget_PatchFiles_cellActivated( int32_t i_iRow, int32_t i_iCol );
  void on_tableWidget_PatchFiles_customContextMenuRequested( const QPoint& i_rcPos );
  void on_tableWidget_PatchFiles_itemSelectionChanged();

  // custom
  void onFilterChanged();
  void OnTableItemChanged( QTableWidgetItem* i_pcItem );

private:

  void AddCategory( const QString& i_strName );

  void AddFile( const rumPatcher::PatchType i_eCategory, const rumPatcher::rumPatchFileInfo& i_rcPatchFileInfo );

  void closeEvent( QCloseEvent* i_pcEvent ) override;

  void GeneratePatch();
  void GeneratePatchEntriesForPath( const QDir& i_rcPublishPath,
                                    QList<rumPatcher::rumPatchFileInfo>& io_cPatchEntries,
                                    QDirIterator::IteratorFlags i_eIteratorFlags = QDirIterator::NoIteratorFlags );
 
  void ImportCSVFile( rumPatcher::PatchType i_ePatchType );

  void Init();

  bool IsDirty() const
  {
    return m_bDirty;
  }

  void MoveSelectedEntryToTable( rumPatcher::PatchType i_eTargetTable );

  void RefreshPatchTable();

  void SetDirty( bool i_bDirty );

  QLabel m_cFilterLabel;
  QLineEdit m_cFilterEdit;
  QAction* m_pcFilterAction;

  // Stores the last text used to filter
  QString m_strActiveFilterText;

  // Stores the last edited field before it was changed
  std::string m_strLastEdit;

  Ui::PatchManager* m_pcUI;

  bool m_bDirty{ false };
};

#endif // PATCHMANAGER_H
