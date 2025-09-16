#ifndef SM_TREEWIDGET_H
#define SM_TREEWIDGET_H

#include <QtWidgets/QTreeWidget>


enum TreeWidgetColumnType
{
  COL_NAME = 0,
  COL_FILE_PATH = 1,
  NUM_COL = 2
};


enum TreeWidgetItemType
{
  TYPE_SCRIPT_FOLDER = QTreeWidgetItem::UserType,
  TYPE_MAP_FOLDER,
  TYPE_AUDIO_FOLDER,
  TYPE_GRAPHIC_FOLDER,
  TYPE_FOLDER,
  TYPE_DB,
  TYPE_DB_TABLE,
  TYPE_SCRIPT,
  TYPE_MAP,
  TYPE_AUDIO,
  TYPE_GRAPHIC
};


class smTreeWidget : public QTreeWidget
{
  Q_OBJECT

public:

  smTreeWidget( QWidget* i_pcParent = 0 );

  void DoContextMenu( const QPoint& i_rcPos );

  void Init();

  void Refresh()
  {
    Init();
  }

  void Shutdown()
  {
    Reset();
  }

  void Open( QTreeWidgetItem* i_pcItem );

public slots:

  void onCompileScripts();
  void onCopyFilename() const;
  void onCopyFullPath() const;
  void onExportTable();
  void onExplore() const;
  void onImportTable();
  void onNewMap();
  void onNewScript();
  void onOpen()
  {
    Open( currentItem() );
  }
  void onQuickScript();
  void onScriptReload();

private slots:

  // Emitted by NewMap on successful map creation
  void onMapCreated( const QString& );
  void onRefresh()
  {
    Refresh();
  }
  void onScriptCreated( const QString& );

private:

  void GetAudio( QTreeWidgetItem* i_pcItem, const QString& i_strPath );
  void GetGraphics( QTreeWidgetItem* i_pcItem, const QString& i_strPath );
  void GetMaps( QTreeWidgetItem* i_pcItem, const QString& i_strPath );
  void GetScripts( QTreeWidgetItem* i_pcItem, const QString& i_strPath );

  // Recursively generates script for all child items
  void QuickScript( const QTreeWidgetItem* i_pcItem, QString& i_strScript );

  // Clears all data for reuse of this widget
  void Reset()
  {
    clear();
  }
};

#endif // SM_TREEWIDGET_H
