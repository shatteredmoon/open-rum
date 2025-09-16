#ifndef PROPERTYMANAGER_H
#define PROPERTYMANAGER_H

#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>

#include <u_enum.h>

namespace Ui
{
  class PropertyManager;
}

class QItemSelection;
class QTableWidgetItem;
class rumPropertyAsset;


class PropertyManager : public QMainWindow
{
  Q_OBJECT

  typedef QMap<QString, QString> TranslationMap;

public:
  explicit PropertyManager( QWidget* i_pcParent = 0 );
  ~PropertyManager();

  static QString GetPropertyValueTypeString( PropertyValueType i_eValueType );

signals:

  void closed();

private slots:

  void on_actionFind_References_triggered();
  void on_actionNew_Property_triggered();
  void on_actionRemove_Property_triggered();
  void on_actionSave_triggered();

  void on_tableWidget_3_customContextMenuRequested( const QPoint& i_rcPos );

  // custom
  void onCellStartEdit( int32_t i_iRow, int32_t i_iCol );

  void onFilterChanged();

  void onTableItemChanged( QTableWidgetItem* i_pcItem );
  void itemComboChanged_Property( const QString& i_rcItem );

  void selectionChanged_Property( const QItemSelection& i_rcSelected, const QItemSelection& i_rcDeselected );

private:

  // Called when the string content table is changed or refreshed
  void AddProperty( int32_t i_iRow, rumAssetID i_eAssetID );
  void AddProperty( int32_t i_iRow, const rumPropertyAsset& i_rcAsset );

  void closeEvent( QCloseEvent* i_pcEvent ) override;

  rumPropertyAsset* GetAsset( int32_t i_iRow ) const;
  rumAssetID GetAssetID( int32_t i_iRow ) const;
  QString GetName( int32_t i_iRow ) const;
  rumPropertyAsset* GetSelectedProperty() const;

  bool IsDirty( int32_t i_iRow ) const
  {
    return m_bDirty;
  }

  void SetDirty( bool i_bDirty );

  void RefreshTable();

  void UpdateFilter();

  QLabel m_cFilterLabel;
  QLineEdit m_cFilterEdit;
  QAction* m_pcFilterAction;

  Ui::PropertyManager* m_pcUI;

  bool m_bDirty{ false };
};

#endif // PROPERTYMANAGER_H
