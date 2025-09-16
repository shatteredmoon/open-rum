#include <smTreeWidget.h>

#undef TRUE
#undef FALSE

#include <e_graphics_opengl.h>

#include <mainwindow.h>
#include <newmap.h>
#include <newscript.h>
#include <scripteditor.h>

#ifdef WIN32
#include "qt_windows.h"
#include "qwindowdefs_win.h"
#include <shellapi.h>
#endif // WIN32

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>
#include <QTabBar>
#include <QUrl>


smTreeWidget::smTreeWidget( QWidget* i_pcParent )
  : QTreeWidget( i_pcParent )
{}


void smTreeWidget::DoContextMenu( const QPoint& i_rcPos )
{
  if( i_rcPos.isNull() )
  {
    return;
  }

  QTreeWidgetItem* pcItem{ itemAt( i_rcPos ) };
  if( !pcItem )
  {
    return;
  }

  setCurrentItem( pcItem );

  QMenu* pcMenu{ new QMenu( this ) };
  const QAction* pcCurrentAction{ nullptr };

  if( pcItem->type() > TYPE_FOLDER )
  {
    QAction* pcOpenAction{ new QAction( "Open", this ) };
    pcMenu->addAction( pcOpenAction );
    connect( pcOpenAction, SIGNAL( triggered() ), this, SLOT( onOpen() ) );

    pcMenu->addSeparator();

    QAction* pcCopyFilenameAction{ new QAction( "Copy Filename", this ) };
    pcMenu->addAction( pcCopyFilenameAction );
    connect( pcCopyFilenameAction, SIGNAL( triggered() ), this, SLOT( onCopyFilename() ) );
  }

  QAction* pcCopyFullPathAction{ new QAction( "Copy Full Path", this ) };
  pcMenu->addAction( pcCopyFullPathAction );
  connect( pcCopyFullPathAction, SIGNAL( triggered() ), this, SLOT( onCopyFullPath() ) );

  QAction* pcExploreAction{ new QAction( "Open Containing Folder", this ) };
  pcMenu->addAction( pcExploreAction );
  connect( pcExploreAction, SIGNAL( triggered() ), this, SLOT( onExplore() ) );

  pcCurrentAction = pcExploreAction;

  if( pcItem->type() == TYPE_SCRIPT )
  {
    pcCurrentAction = pcMenu->addSeparator();

    QAction* pcScriptAction{ new QAction( "Compile Script", this ) };
    pcMenu->addAction( pcScriptAction );
    connect( pcScriptAction, SIGNAL( triggered() ), this, SLOT( onCompileScripts() ) );

    QAction* pcReloadAction{ new QAction( "Reload Script", this ) };
    pcMenu->addAction( pcReloadAction );
    connect( pcReloadAction, SIGNAL( triggered() ), this, SLOT( onScriptReload() ) );

    pcCurrentAction = pcReloadAction;
  }
  else if( pcItem->type() == TYPE_MAP_FOLDER || pcItem->type() == TYPE_MAP ||
           pcItem->type() == TYPE_AUDIO_FOLDER || pcItem->type() == TYPE_AUDIO ||
           pcItem->type() == TYPE_GRAPHIC_FOLDER || pcItem->type() == TYPE_GRAPHIC )
  {
    pcCurrentAction = pcMenu->addSeparator();

    QAction* pcScriptAction{ new QAction( "Quick Script", this ) };
    pcMenu->addAction( pcScriptAction );
    connect( pcScriptAction, SIGNAL( triggered() ), this, SLOT( onQuickScript() ) );

    pcCurrentAction = pcScriptAction;
  }
  else if( pcItem->type() == TYPE_SCRIPT_FOLDER )
  {
    pcCurrentAction = pcMenu->addSeparator();

    QAction* pcScriptAction{ new QAction( "Compile Scripts", this ) };
    pcMenu->addAction( pcScriptAction );
    connect( pcScriptAction, SIGNAL( triggered() ), this, SLOT( onCompileScripts() ) );

    pcCurrentAction = pcScriptAction;
  }

  // Add a separator if needed
  if( pcCurrentAction && !pcCurrentAction->isSeparator() )
  {
    pcCurrentAction = pcMenu->addSeparator();
  }

  QAction* pcNewMapAction{ new QAction( "New Map", this ) };
  pcMenu->addAction( pcNewMapAction );
  connect( pcNewMapAction, SIGNAL( triggered() ), this, SLOT( onNewMap() ) );

  QAction* pcNewScriptAction{ new QAction( "New Script", this ) };
  pcMenu->addAction( pcNewScriptAction );
  connect( pcNewScriptAction, SIGNAL( triggered() ), this, SLOT( onNewScript() ) );

  pcMenu->addSeparator();

  QAction* pcNewRefreshAction{ new QAction( "Refresh", this ) };
  pcMenu->addAction( pcNewRefreshAction );
  connect( pcNewRefreshAction, SIGNAL( triggered() ), this, SLOT( onRefresh() ) );

  const QPoint cGlobalPos{ this->mapToGlobal( i_rcPos ) };
  pcMenu->exec( cGlobalPos );
}


void smTreeWidget::GetAudio( QTreeWidgetItem* i_pcItem, const QString& i_strPath )
{
  // Recursively find and add all audio under the provided path
  QDir cDir( i_strPath );
  cDir.setFilter( QDir::Dirs | QDir::NoDotAndDotDot | QDir::Files | QDir::NoSymLinks );
  cDir.setSorting( QDir::DirsFirst );

  QStringList cAudioTypesList;
  cAudioTypesList << "snd" << "wav" << "mid" << ".mp3";

  const QFileInfoList cFileInfoList{ cDir.entryInfoList() };
  for( int32_t i = 0; i < cFileInfoList.size(); ++i )
  {
    const QFileInfo cFileInfo{ cFileInfoList.at( i ) };
    if( cFileInfo.isDir() )
    {
      //cout << "Creating folder " << qPrintable(QString("%1 (%2)")
      //        .arg(fileInfo.completeBaseName())
      //        .arg(fileInfo.absoluteFilePath())) << endl;
      QTreeWidgetItem* pcFolder{ new QTreeWidgetItem( i_pcItem, TYPE_AUDIO_FOLDER ) };
      pcFolder->setText( COL_NAME, cFileInfo.fileName() );
      pcFolder->setText( COL_FILE_PATH, cFileInfo.absoluteFilePath() );

      // Recurse the new folder
      GetAudio( pcFolder, cFileInfo.absoluteFilePath() );
    }
    else if( cAudioTypesList.contains( cFileInfo.suffix(), Qt::CaseInsensitive ) )
    {
      //cout << "Adding item " << qPrintable(QString("%1 (%2)")
      //        .arg(fileInfo.completeBaseName())
      //        .arg(fileInfo.absoluteFilePath())) << endl;
      QTreeWidgetItem* pcSubItem{ new QTreeWidgetItem( i_pcItem, TYPE_AUDIO ) };
      pcSubItem->setText( COL_NAME, cFileInfo.fileName() );
      pcSubItem->setText( COL_FILE_PATH, cFileInfo.absoluteFilePath() );
    }
  }
}


void smTreeWidget::GetGraphics( QTreeWidgetItem* i_pcItem, const QString& i_strPath )
{
  // Recursively find and add all graphics under the provided path
  QDir cDir( i_strPath );
  cDir.setFilter( QDir::Dirs | QDir::NoDotAndDotDot | QDir::Files | QDir::NoSymLinks );
  cDir.setSorting( QDir::DirsFirst );

  QStringList cGraphicTypesList;
  cGraphicTypesList << "bmp" << "dds" << "gif" << "jpeg" << "jpg" << "pcx" << "png" << "tga" << "tif";

  const QFileInfoList cList{ cDir.entryInfoList() };
  for( int32_t i = 0; i < cList.size(); ++i )
  {
    const QFileInfo cFileInfo{ cList.at( i ) };
    if( cFileInfo.isDir() )
    {
      //cout << "Creating folder " << qPrintable(QString("%1 (%2)")
      //        .arg(fileInfo.completeBaseName())
      //        .arg(fileInfo.absoluteFilePath())) << endl;
      QTreeWidgetItem* pcFolder{ new QTreeWidgetItem( i_pcItem, TYPE_GRAPHIC_FOLDER ) };
      pcFolder->setText( COL_NAME, cFileInfo.fileName() );
      pcFolder->setText( COL_FILE_PATH, cFileInfo.absoluteFilePath() );

      // Recurse the new folder
      GetGraphics( pcFolder, cFileInfo.absoluteFilePath() );
    }
    else if( cGraphicTypesList.contains( cFileInfo.suffix(), Qt::CaseInsensitive ) )
    {
      //cout << "Adding item " << qPrintable(QString("%1 (%2)")
      //        .arg(fileInfo.completeBaseName())
      //        .arg(fileInfo.absoluteFilePath())) << endl;
      QTreeWidgetItem* pcSubItem{ new QTreeWidgetItem( i_pcItem, TYPE_GRAPHIC ) };
      pcSubItem->setText( COL_NAME, cFileInfo.fileName() );
      pcSubItem->setText( COL_FILE_PATH, cFileInfo.absoluteFilePath() );
    }
  }
}


void smTreeWidget::GetMaps( QTreeWidgetItem* i_pcItem, const QString& i_strPath )
{
  // Recursively find and add all maps under the provided path
  QDir cDir( i_strPath );
  cDir.setFilter( QDir::Dirs | QDir::NoDotAndDotDot | QDir::Files | QDir::NoSymLinks );
  cDir.setSorting( QDir::DirsFirst );

  const QFileInfoList cList{ cDir.entryInfoList() };
  for( int32_t i = 0; i < cList.size(); ++i )
  {
    QFileInfo cFileInfo{ cList.at( i ) };
    if( cFileInfo.isDir() )
    {
      //cout << "Creating folder " << qPrintable(QString("%1 (%2)")
      //        .arg(fileInfo.completeBaseName())
      //        .arg(fileInfo.absoluteFilePath())) << endl;
      QTreeWidgetItem* pcFolder{ new QTreeWidgetItem( i_pcItem, TYPE_MAP_FOLDER ) };
      pcFolder->setText( COL_NAME, cFileInfo.fileName() );
      pcFolder->setText( COL_FILE_PATH, cFileInfo.absoluteFilePath() );

      // Recurse the new folder
      GetMaps( pcFolder, cFileInfo.absoluteFilePath() );
    }
    else if( cFileInfo.suffix().compare( "map", Qt::CaseInsensitive ) == 0 )
    {
      //cout << "Adding item " << qPrintable(QString("%1 (%2)")
      //        .arg(fileInfo.completeBaseName())
      //        .arg(fileInfo.absoluteFilePath())) << endl;
      QTreeWidgetItem* pcSubItem{ new QTreeWidgetItem( i_pcItem, TYPE_MAP ) };
      pcSubItem->setText( COL_NAME, cFileInfo.fileName() );
      pcSubItem->setText( COL_FILE_PATH, cFileInfo.absoluteFilePath() );
    }
  }
}


void smTreeWidget::GetScripts( QTreeWidgetItem* i_pcItem, const QString& i_strPath )
{
  // Recursively find and add all scripts under the provided path
  QDir cDir( i_strPath );
  cDir.setFilter( QDir::Dirs | QDir::NoDotAndDotDot | QDir::Files | QDir::NoSymLinks );
  cDir.setSorting( QDir::DirsFirst );

  const QFileInfoList cList{ cDir.entryInfoList() };
  for( int32_t i = 0; i < cList.size(); ++i )
  {
    QFileInfo cFileInfo{ cList.at( i ) };
    if( cFileInfo.isDir() )
    {
      //cout << "Creating folder " << qPrintable(QString("%1 (%2)")
      //        .arg(fileInfo.completeBaseName())
      //        .arg(fileInfo.absoluteFilePath())) << endl;
      QTreeWidgetItem* pcFolder{ new QTreeWidgetItem( i_pcItem, TYPE_SCRIPT_FOLDER ) };
      pcFolder->setText( COL_NAME, cFileInfo.fileName() );
      pcFolder->setText( COL_FILE_PATH, cFileInfo.absoluteFilePath() );

      // Recurse the new folder
      GetScripts( pcFolder, cFileInfo.absoluteFilePath() );
    }
    else if( cFileInfo.suffix().compare( "nut", Qt::CaseInsensitive ) == 0 )
    {
      //cout << "Adding item " << qPrintable(QString("%1 (%2)")
      //        .arg(fileInfo.completeBaseName())
      //        .arg(fileInfo.absoluteFilePath())) << endl;
      QTreeWidgetItem* pcSubItem{ new QTreeWidgetItem( i_pcItem, TYPE_SCRIPT ) };
      pcSubItem->setText( COL_NAME, cFileInfo.fileName() );
      pcSubItem->setText( COL_FILE_PATH, cFileInfo.absoluteFilePath() );
    }
  }
}


void smTreeWidget::Init()
{
  Reset();

  setHeaderLabel( "Project" );

  // ---- Add client, server, and shared scripts ----------------------------
  const QString strScriptPath{ MainWindow::GetProjectScriptDir().canonicalPath() };
  QTreeWidgetItem* pcScriptItem{ new QTreeWidgetItem( this, TYPE_SCRIPT_FOLDER ) };
  pcScriptItem->setText( COL_NAME, "Scripts" );
  pcScriptItem->setText( COL_FILE_PATH, strScriptPath );
  pcScriptItem->setExpanded( true );

  GetScripts( pcScriptItem, strScriptPath );

  // ---- Add maps ----------------------------------------------------------
  const QString strMapPath{ MainWindow::GetProjectMapDir().canonicalPath() };
  QTreeWidgetItem* pcMapItem{ new QTreeWidgetItem( this, TYPE_MAP_FOLDER ) };
  pcMapItem->setText( COL_NAME, "Maps" );
  pcMapItem->setText( COL_FILE_PATH, strMapPath );
  pcMapItem->setExpanded( true );

  GetMaps( pcMapItem, strMapPath );

  // ---- Add graphics ------------------------------------------------------
  const QString strGraphicPath{ MainWindow::GetProjectGraphicDir().canonicalPath() };
  QTreeWidgetItem* pcGraphicItem{ new QTreeWidgetItem( this, TYPE_GRAPHIC_FOLDER ) };
  pcGraphicItem->setText( COL_NAME, "Graphics" );
  pcGraphicItem->setText( COL_FILE_PATH, strGraphicPath );
  pcGraphicItem->setExpanded( true );

  GetGraphics( pcGraphicItem, strGraphicPath );

  // ---- Add audio ---------------------------------------------------------
  const QString strAudioPath{ MainWindow::GetProjectAudioDir().canonicalPath() };
  QTreeWidgetItem* pcAudioItem{ new QTreeWidgetItem( this, TYPE_AUDIO_FOLDER ) };
  pcAudioItem->setText( COL_NAME, "Audio" );
  pcAudioItem->setText( COL_FILE_PATH, strAudioPath );
  pcAudioItem->setExpanded( true );

  GetAudio( pcAudioItem, strAudioPath );
}


void smTreeWidget::onCompileScripts()
{
  const QTreeWidgetItem* pcItem{ currentItem() };
  if( !pcItem )
  {
    return;
  }

#pragma message("Allow manual script compilation?")
  /*const int32_t RESULT_SUCCESS = 0;

  // Get the full path info
  const QString strPath{ QDir::toNativeSeparators( pcItem->text( COL_FILE_PATH ) ) };
  const int32_t eResult{ rumScript::Compile( qPrintable( strPath ) ) };
  if( eResult != RESULT_SUCCESS )
  {
    const QString &strCompileError = rumScript::GetLastCompilerError().c_str();
    QMessageBox::critical( this, "Script Compilation Failure",
                            strPath + "\nScript(s) failed compilation\n\n" + strCompileError );
  }
  else
  {
    QMessageBox::information( this, "Script Compilation Success", strPath + "\nScript(s) compiled successfully" );
  }*/

  rumAssert( false );
}


void smTreeWidget::onCopyFilename() const
{
  const QTreeWidgetItem* pcItem{ currentItem() };
  if( !pcItem )
  {
    return;
  }

  QClipboard* pcClipBoard{ QApplication::clipboard() };
  if( !pcClipBoard )
  {
    return;
  }

  QString strFile;

  if( pcItem->type() == TYPE_DB_TABLE )
  {
    // Get the parent item filename
    const QTreeWidgetItem* pcParent{ pcItem->parent() };
    if( pcParent )
    {
      strFile = pcParent->text( COL_NAME );
    }
  }
  else
  {
    strFile = pcItem->text( COL_NAME );
  }

  pcClipBoard->setText( strFile );
}


void smTreeWidget::onCopyFullPath() const
{
  const QTreeWidgetItem* pcItem{ currentItem() };
  if( !pcItem )
  {
    return;
  }

  QClipboard* pcClipBoard{ QApplication::clipboard() };
  if( !pcClipBoard )
  {
    return;
  }

  // Get the full path info
  QString strPath{ pcItem->text( COL_FILE_PATH ) };

  // Database tables contain a '|' symbol, strip it
  const int32_t iIndex{ strPath.indexOf( '|' ) };
  if( iIndex >= 0 )
  {
    strPath = strPath.left( iIndex );
  }

  pcClipBoard->setText( strPath );
}


void smTreeWidget::onExportTable()
{
  const QTreeWidgetItem* pcItem{ currentItem() };
  if( !pcItem )
  {
    return;
  }

  // Export a .csv file
  bool bExport{ false };

  // Get a file name to export to
  const QString strFile{ QFileDialog::getSaveFileName( this ) };
  if( !QFile::exists( strFile ) )
  {
    // Create file dialog
    if( QMessageBox::Yes == QMessageBox::question( this, "Export Database Table",
                                                   "File " + strFile + " does not exist, create?",
                                                   QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes ) )
    {
      bExport = true;
    }
  }

  if( bExport )
  {
    // Get the database filename
    const QTreeWidgetItem* pcParent{ pcItem->parent() };
    const QString strDatabase{ pcParent->text( COL_NAME ) };

    const rumDatabase::DatabaseID eDatabaseID{ GetDatabaseIDFromFilename( strDatabase.toLocal8Bit().data() ) };
    const rumDatabase* pcDatabase{ rumDatabase::GetSynchronousDB( eDatabaseID ) };
    if( pcDatabase )
    {
      if( pcDatabase->TableExport( pcItem->text( COL_NAME ).toLocal8Bit().data(), strFile.toLocal8Bit().data() ) )
      {
        QMessageBox::information( this, "Export Database Table",
                                  "Database table successfully exported to file " + strFile );
      }
      else
      {
        QMessageBox::critical( this, "Export Database Table", "Database table failed to export to file " + strFile );
      }
    }
    else
    {
      QMessageBox::critical( this, "Export Database Table", "Database not available: " + strDatabase );
    }
  }
}


void smTreeWidget::onExplore() const
{
  const QTreeWidgetItem* pcItem{ currentItem() };
  if( !pcItem )
  {
    return;
  }

  QString strPath{ pcItem->text( COL_FILE_PATH ) };

  // Database tables contain a '|' symbol, strip it
  const int32_t iIndex{ strPath.indexOf( '|' ) };
  if( iIndex >= 0 )
  {
    strPath = strPath.left( iIndex );
  }

  QFileInfo cFileInfo( strPath );
  if( cFileInfo.isFile() )
  {
    // Get the parent folder
    strPath = cFileInfo.path();

    // Update the file info
    cFileInfo.setFile( strPath );
  }

  // Only continue if we have a directory
  if( cFileInfo.isDir() )
  {
#ifdef WIN32
    ShellExecute( NULL, "explore", strPath.toLocal8Bit().data(), NULL, NULL, SW_SHOWNORMAL );
#else
#error Platform not supported!
#endif
  }
}


void smTreeWidget::onImportTable()
{
  const QTreeWidgetItem* pcItem{ currentItem() };
  if( !pcItem )
  {
    return;
  }

  bool bImport{ true };

  // Get a file name to export to
  const QString strFile{ QFileDialog::getOpenFileName( this ) };
  if( !QFile::exists( strFile ) )
  {
    // Create file dialog
    QMessageBox::critical( this, "Import Database Table", "Could not find or open file " + strFile );
    bImport = false;
  }

  if( bImport )
  {
    // Get the database filename
    const QTreeWidgetItem* pcParent{ pcItem->parent() };
    const QString strDatabase{ pcParent->text( COL_NAME ) };

    const rumDatabase::DatabaseID eDatabaseID{ GetDatabaseIDFromFilename( strDatabase.toLocal8Bit().data() ) };
    rumDatabase* pcDatabase{ rumDatabase::GetSynchronousDB( eDatabaseID ) };
    if( pcDatabase )
    {
      if( pcDatabase->TableImport( pcItem->text( COL_NAME ).toLocal8Bit().data(), strFile.toLocal8Bit().data() ) )
      {
        QMessageBox::information( this, "Import Database Table",
                                  "Database table successfully imported from file " + strFile );
      }
      else
      {
        QMessageBox::critical( this, "Import Database Table", "Database table failed to import to file " + strFile );
      }
    }
    else
    {
      QMessageBox::critical( this, "Import Database Table", "Database not available: " + strDatabase );
    }
  }
}


void smTreeWidget::onMapCreated( const QString& i_strFilePath )
{
  Refresh();
}


void smTreeWidget::onNewMap()
{
  NewMap* pcDialog{ new NewMap( this ) };

  connect( pcDialog, SIGNAL( mapCreated( const QString & ) ), this, SLOT( onMapCreated( const QString & ) ) );

  pcDialog->setModal( true );
  pcDialog->show();
}


void smTreeWidget::onNewScript()
{
  NewScript* pcDialog{ new NewScript( this ) };

  connect( pcDialog, SIGNAL( scriptCreated( const QString & ) ), this, SLOT( onScriptCreated( const QString & ) ) );

  pcDialog->setModal( true );
  pcDialog->show();
}


void smTreeWidget::onQuickScript()
{
  const QTreeWidgetItem* pcItem{ currentItem() };
  if( !pcItem )
  {
    return;
  }

  QString strScript;
  QuickScript( pcItem, strScript );

  MainWindow* pcMain{ MainWindow::GetMainWindow() };
  if( pcMain )
  {
    QWidget* pcWidget{ pcMain->AddOrOpenTab( "", TYPE_SCRIPT, strScript ) };
    if (pcWidget)
    {
      ScriptEditor* pcScriptEditor{ qobject_cast<ScriptEditor*>( pcWidget ) };
      Q_ASSERT(pcScriptEditor);
    }
  }
}


void smTreeWidget::QuickScript( const QTreeWidgetItem* i_pcItem, QString& i_strScript )
{
  // If this item is a folder, recurse into it
  const int32_t iCount{ i_pcItem->childCount() };
  for( int32_t i = 0; i < iCount; ++i )
  {
    QuickScript( i_pcItem->child( i ), i_strScript );
  }

  if( i_pcItem->type() > TYPE_FOLDER )
  {
    const QFileInfo cFileInfo{ i_pcItem->text( COL_FILE_PATH ) };
    i_strScript += "class " + cFileInfo.baseName();

    if( i_pcItem->type() == TYPE_AUDIO )
    {
      i_strScript += "_Sound extends Sound\n{\n    static s_strFilename = ";
      i_strScript += cFileInfo.fileName();
      i_strScript += ";\n";
    }
    else if( i_pcItem->type() == TYPE_GRAPHIC )
    {
      i_strScript += "_Graphic extends Graphic\n{\n    static s_strFilename = ";
      i_strScript += cFileInfo.fileName();
      i_strScript += ";\n";

      // None of this is necessary anymore...
      rumAssert( false );

      /*const QString strFilePath = info.absoluteFilePath();

      // Create a new graphic object
      EditorGraphic* pcGraphic = new EditorGraphic();
      if( !pcGraphic && pcGraphic->Create( strFilePath ) )
      {
        int32_t height = pcGraphic->GetHeight();
        int32_t width = pcGraphic->GetWidth();
        int32_t frames = height / MainWindow::GetTileHeight();
        int32_t sets = width / MainWindow::GetTileWidth();

        strScript += "\n    // width = " + QString::number( width );
        strScript += "\n    // height = " + QString::number( height );
        strScript += "\n\n    static s_iNumFrames = " + QString::number( frames );
        strScript += ";\n    static s_iNumSets = " + QString::number( sets );
        strScript += ";\n";
      }

      delete pcGraphic;*/
    }
    else if( i_pcItem->type() == TYPE_MAP )
    {
      i_strScript += "_Map extends Map\n{\n    static s_strFilename = ";
      i_strScript += cFileInfo.fileName();
      i_strScript += ";\n";
    }

    i_strScript += "}\n\n";
  }
}


void smTreeWidget::onScriptCreated( const QString& i_strFilePath )
{
  Refresh();
}


void smTreeWidget::onScriptReload()
{
  const QTreeWidgetItem* pcItem{ currentItem() };
  if( !pcItem )
  {
    return;
  }

  // Determine the relative path from the script folder
  const QString& strFilePath{ pcItem->text( COL_FILE_PATH ) };
  const QString strRelativePath{ MainWindow::GetProjectScriptDir().relativeFilePath( strFilePath ) };

  // Reload the script
  rumScript::Load( qPrintable( strRelativePath ) );

  // Handle script re-registration, id embedding, etc.
  //rumScript::ProcessScriptChanges();
}


void smTreeWidget::Open( QTreeWidgetItem* i_pcItem )
{
  Q_ASSERT( i_pcItem );

  if( i_pcItem->type() == TYPE_MAP || i_pcItem->type() == TYPE_SCRIPT || i_pcItem->type() == TYPE_DB_TABLE )
  {
    // Get this object's main window
    MainWindow* pcMain{ MainWindow::GetMainWindow() };
    if( pcMain )
    {
      // Open the tab using the info stored in the specified column
      pcMain->AddOrOpenTab( i_pcItem );
    }
  }
  else
  {
    const QString& strFilePath{ i_pcItem->text( COL_FILE_PATH ) };
    QDesktopServices::openUrl( QUrl( "file:///" + strFilePath, QUrl::TolerantMode ) );
  }
}
