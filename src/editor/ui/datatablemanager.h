#ifndef DATATABLEMANAGER_H
#define DATATABLEMANAGER_H

#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QSet>
#include <QTimer>

#include <platform.h>

#include <u_datatable.h>

namespace Ui
{
  class DataTableManager;
}

class QItemSelection;
class QNetworkReply;
class QTableWidgetItem;


class DataTableManager : public QMainWindow
{
  Q_OBJECT

public:

  explicit DataTableManager( QWidget* i_pcParent = 0 );
  ~DataTableManager();

  bool IsDirty() const
  {
    return m_bDirty;
  }

  rumDataTable::DataTableID GetSelectedDataTableID() const;

  QString GetTableName( int32_t i_iRow ) const;

private:

  void SaveAllTables();

  void on_headerWidget_customContextMenuRequested( const QPoint& pos );

signals:

  void closed();

private slots:

  void on_actionChange_Data_Column_Type_triggered();

  void on_actionNew_Data_Column_triggered();
  void on_actionNew_Data_Entry_triggered();
  void on_actionNew_Data_Table_triggered();

  void on_actionRemove_Data_Column_triggered();
  void on_actionRemove_Data_Entry_triggered();
  void on_actionRemove_Data_Table_triggered();

  void on_actionRename_Data_Column_triggered();

  void on_actionSave_triggered();

  void on_tableWidget_cellDoubleClicked( int32_t i_iRow, int32_t i_iCol );
  void on_tableWidget_customContextMenuRequested( const QPoint& i_rcPos );

  void on_tableWidget_3_cellDoubleClicked( int32_t i_iRow, int32_t i_iCol );
  void on_tableWidget_3_customContextMenuRequested( const QPoint& i_rcPos );

  // custom
  void OnComboPickerItemSelected( int32_t );

  void OnDataEntryChanged( QTableWidgetItem* i_pcItem );
  void OnDataEntryComboChanged( const QString& i_strText );
  void OnDataEntryStringTokenChanged( rumTokenID i_eTokenID );
  void OnDataTableChanged( QTableWidgetItem* i_pcItem );
  void OnDataTableComboChanged( const QString& i_strText );

  void onFilterChanged();

  void selectionChanged_DataEntry( const QItemSelection& i_rcSelected, const QItemSelection& i_rcDeselected );
  void selectionChanged_DataTable( const QItemSelection& i_rcSelected, const QItemSelection& i_rcDeselected );

  void TableLineEditFinished();

private:

  void AddDataColumn( rumDataTable& i_rcDataTable, uint32_t i_uiColumn );
  void AddDataEntry( rumDataTable& i_rcDataTable, int32_t i_iRow );
  void AddDataTable( const rumDataTable& i_rcDataTable ) const;

  void closeEvent( QCloseEvent* i_pcEvent ) override;

  void InitDataTables();

  bool IsTableNameUnique( const QString& i_strName ) const;

  // Called when a new data table is selected
  void RefreshDataTable();

  void RefreshHeaderLabels();

  void SetDirty( bool i_bDirty );

  void UpdateFilter();

  QLabel m_cFilterLabel;
  QLineEdit m_cFilterEdit;
  QAction* m_pcFilterAction;

  Ui::DataTableManager* m_pcUI;

  bool m_bDirty{ false };
};

#endif // DATATABLEMANAGER_H
