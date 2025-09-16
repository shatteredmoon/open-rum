#include <releasemanager.h>
#include <ui_releasemanager.h>

#include <filefilter.h>
#include <mainwindow.h>
#include <smFileSystemModel.h>

#include <QCloseEvent>
#include <QComboBox>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>

#include <network/u_patcher.h>

#include <u_broadcast_asset.h>
#include <u_creature_asset.h>
#include <u_custom_asset.h>
#include <u_datatable.h>
#include <u_db.h>
#include <u_graphic_asset.h>
#include <u_inventory_asset.h>
#include <u_language.h>
#include <u_log.h>
#include <u_map_asset.h>
#include <u_portal_asset.h>
#include <u_property_asset.h>
#include <u_scheduler.h>
#include <u_sound_asset.h>
#include <u_strings.h>
#include <u_tile_asset.h>
#include <u_utility.h>
#include <u_widget_asset.h>
#include <u_zlib.h>

#define RELEASE_TABLE_NAME "release"

int ReleaseManager::s_iNextKey{ 0 };

enum
{
  COL_ACTION_NAME    = 0,
  COL_ACTION_SERVICE = 1
};

enum
{
  TREEVIEW_COL_FILENAME = 0,
  TREEVIEW_COL_SIZE     = 1,
  TREEVIEW_COL_TYPE     = 2,
  TREEVIEW_COL_DATE     = 3
};

enum
{
  ROLE_ID      = Qt::UserRole + 0,
  ROLE_PATH    = Qt::UserRole + 1,
  ROLE_SERVICE = Qt::UserRole + 2
};


ReleaseManager::ReleaseManager( QWidget* i_pcParent )
  : QMainWindow( i_pcParent )
  , m_pcUI( new Ui::ReleaseManager )
{
  m_pcUI->setupUi( this );
  setCentralWidget( m_pcUI->splitter );

  ReadSettings();

  const QIcon cReleaseIcon( ":/ui/resources/release.png" );
  setWindowIcon( cReleaseIcon );

  setMinimumWidth( 1024 );
  setMinimumHeight( 768 );

  // Set up the table widget label names
  QStringList cLabelList;
  cLabelList << "Action Name" << "Folder";

  m_pcUI->tableWidget->setColumnCount( cLabelList.size() );
  m_pcUI->tableWidget->setMinimumWidth( 300 );
  m_pcUI->tableWidget->setHorizontalHeaderLabels( cLabelList );

  m_pcUI->tableWidget->setContextMenuPolicy( Qt::CustomContextMenu );

  m_pcUI->toolBar->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );

  const QIcon cPublishAllIcon( ":/ui/resources/process_all.png" );
  m_pcUI->actionPublish_Release->setIcon( cPublishAllIcon );
  m_pcUI->actionPublish_Release->setEnabled( false );

  const QIcon cPublishIcon( ":/ui/resources/process.png" );
  m_pcUI->actionPublish_Action->setIcon( cPublishIcon );
  m_pcUI->actionPublish_Action->setEnabled( false );

  m_pcUI->actionPublish_Databases->setIcon( cPublishIcon );
  m_pcUI->actionPublish_Databases->setEnabled( true );

  m_pcUI->actionPublish_Executables->setIcon( cPublishIcon );
  m_pcUI->actionPublish_Executables->setEnabled( true );

  const QIcon cAddIcon( ":/ui/resources/add.png" );
  m_pcUI->actionNew_Action->setIcon( cAddIcon );
  m_pcUI->actionNew_Action->setEnabled( true );

  const QIcon cRemoveIcon( ":/ui/resources/remove.png" );
  m_pcUI->actionDelete_Action->setIcon( cRemoveIcon );
  m_pcUI->actionDelete_Action->setEnabled( false );

  const QIcon cSaveIcon( ":/ui/resources/save.png" );
  m_pcUI->actionSave->setIcon( cSaveIcon );
  m_pcUI->actionSave->setEnabled( false );

  InitActionItems();

  // Set up the tree view
  m_pcUI->treeView->setMinimumWidth( 700 );
  m_pcUI->treeView->setEnabled( false );

  // Determine default sizes
  const QHeaderView* pcHorizontalHeader{ m_pcUI->tableWidget->horizontalHeader() };
  const int32_t iTableSize{ std::max( m_pcUI->tableWidget->minimumWidth(), pcHorizontalHeader->length() ) };
  const int32_t iTreeSize{ 1024 - iTableSize };

  QList<int32_t> cSplitterList;
  cSplitterList.append( iTableSize );
  cSplitterList.append( iTreeSize );
  m_pcUI->splitter->setSizes( cSplitterList );

  connect( m_pcUI->tableWidget, SIGNAL( itemSelectionChanged() ),
           this, SLOT( on_tableWidget_itemSelectionChanged() ) );

  connect( m_pcUI->tableWidget, SIGNAL( cellActivated( int32_t, int32_t ) ),
           this, SLOT( OnCellStartEdit( int32_t, int32_t ) ) );
}


ReleaseManager::~ReleaseManager()
{
  s_iNextKey = 0;

  delete m_pcUI;

  // Release any loaded proxy models
  for( const auto& iter : m_cFileProxyHash )
  {
    FileFilterProxyModel* pcProxy{ iter.second };
    SAFE_DELETE( pcProxy );
  }
}


void ReleaseManager::AddActionItem( int32_t i_iKey, const QString& i_strName, ServiceType i_eServiceType ) const
{
  const int32_t iRow{ m_pcUI->tableWidget->rowCount() };
  m_pcUI->tableWidget->setRowCount( iRow + 1 );

  if( i_iKey >= s_iNextKey )
  {
    s_iNextKey = i_iKey + 1;
  }

  // Release action name
  QTableWidgetItem* pcCell{ new QTableWidgetItem };
  pcCell->setData( Qt::DisplayRole, i_strName );
  pcCell->setData( ROLE_ID, QVariant::fromValue( i_iKey ) );
  pcCell->setToolTip( rumStringUtils::ToString( i_iKey ) );
  m_pcUI->tableWidget->setItem( iRow, COL_ACTION_NAME, pcCell );

  // Service type
  QString strServiceType;
  switch( i_eServiceType )
  {
    case Shared_ServiceType: strServiceType = "Root"; break;
    case Server_ServiceType: strServiceType = "Server"; break;
    case Client_ServiceType: strServiceType = "Client"; break;
  }
  pcCell = new QTableWidgetItem();
  pcCell->setText( strServiceType );
  pcCell->setData( ROLE_SERVICE, i_eServiceType );
  pcCell->setToolTip( rumStringUtils::ToString( i_eServiceType ) );
  m_pcUI->tableWidget->setItem( iRow, COL_ACTION_SERVICE, pcCell );
  pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );

  m_pcUI->actionPublish_Release->setEnabled( true );
}


void ReleaseManager::Close()
{
  if( !IsDirty() )
  {
    return;
  }

  // Verify that user wants to keep unsaved changes
  const QString strQuestion( "Release Manager has unsaved changes! Save changes before closing?" );

  if( QMessageBox::Yes == QMessageBox::question( this, "Save changes?", strQuestion,
                                                  QMessageBox::Yes | QMessageBox::No,
                                                  QMessageBox::Yes ) )
  {
    // User wants to save changes
    ExportCSVFiles();
  }
}


void ReleaseManager::closeEvent( QCloseEvent* i_pcEvent )
{
  if( IsDirty() )
  {
    // Verify that user wants to lose changes
    const QString strQuestion( "Unsaved changes will be discarded on all modified rows!" );

    if( QMessageBox::Abort == QMessageBox::question( this, "Discard changes?", strQuestion,
                                                     QMessageBox::Discard | QMessageBox::Abort,
                                                     QMessageBox::Abort ) )
    {
      // User does not want to lose unsaved changes
      i_pcEvent->ignore();
      return;
    }
  }

  i_pcEvent->accept();
  emit closed();
}


bool ReleaseManager::CopyFileCRC( const QString& i_strSource, const QString& i_strDest )
{
  bool bCopied{ false };

  // Perform size & crc check to determine if file should be copied
  if( FileNeedsUpdate( i_strSource, i_strDest ) )
  {
    const QFileInfo cFileInfo( i_strDest );

    if( QDir().mkpath( cFileInfo.dir().absolutePath() ) )
    {
      bool bRetry{ false };

      do
      {
        bRetry = false;

        if( QFile::exists( i_strDest ) )
        {
          // The file already exists - delete it so that the copy will work
          QFile::remove( i_strDest );
        }

        bCopied = QFile::copy( i_strSource, i_strDest );
        if( !bCopied )
        {
          QString strQuestion{ "Failed to copy " + i_strSource + "\nto " + i_strDest };
          strQuestion += "\n\nRetry?";
          if( QMessageBox::Yes == QMessageBox::question( this, "Copy Failed", strQuestion,
                                                         QMessageBox::Yes | QMessageBox::No,
                                                         QMessageBox::No ) )
          {
            bRetry = true;
          }
        }
      } while( bRetry );
    }
  }

  return bCopied;
}


void ReleaseManager::DeleteAction( int32_t i_iRow )
{
  Q_ASSERT( i_iRow >= 0 && i_iRow < m_pcUI->tableWidget->rowCount() );

  if( i_iRow <= m_pcUI->tableWidget->rowCount() )
  {
    m_pcUI->tableWidget->removeRow( i_iRow );
  }

  if( m_pcUI->tableWidget->rowCount() == 0 )
  {
    m_pcUI->actionPublish_Release->setEnabled( false );
  }
}


void ReleaseManager::ExportCSVFiles()
{
  if( !IsDirty() )
  {
    // Nothing to update
    return;
  }

  std::filesystem::path fsFilepath
  {
    std::filesystem::path( qPrintable( MainWindow::GetProjectDir().canonicalPath() ) ) /
      CSV_FOLDER_NAME / RELEASE_TABLE_NAME
  };

  fsFilepath += CSV_EXTENSION;

  std::filesystem::create_directories( fsFilepath.parent_path() );

  std::ofstream cOutfile( fsFilepath, std::ios::out | std::ios::trunc );
  if( cOutfile.is_open() )
  {
    QList<QString> cOutputList;

    // Visit each release table entry and save out its csv and all of its supported file csvs
    const int32_t iNumRows{ m_pcUI->tableWidget->rowCount() };
    for( int32_t i{ 0 }; i < iNumRows; ++i )
    {
      const QString& strName{ GetActionName( i ) };
      if( strName.isEmpty() )
      {
        continue;
      }

      const ServiceType eServiceType{ GetActionService( i ) };
      const QString strOutput( strName + ',' + rumStringUtils::ToString( eServiceType ) + '\n' );
      cOutputList.push_back( strOutput );

      // Don't resave if the file proxy doesn't exist for the row
      const int32_t iKey{ GetActionKey( i ) };
      const auto& iter{ m_cFileProxyHash.find( iKey ) };
      if( iter != m_cFileProxyHash.end() )
      {
        ExportCSVFiles( i );
      }
    }

    std::sort( cOutputList.begin(), cOutputList.end() );

    QListIterator iter( cOutputList );
    while( iter.hasNext() )
    {
      const QString& strOutput( iter.next() );
      cOutfile << qPrintable( strOutput );
    }

    cOutfile.close();

    // Remove the file stub if nothing was written
    if( std::filesystem::file_size( fsFilepath ) == 0U )
    {
      std::filesystem::remove( fsFilepath );
    }
  }

  SetDirty( false );
}


void ReleaseManager::ExportCSVFiles( int32_t i_iRow )
{
  const QString& strName{ GetActionName( i_iRow ) };
  if( strName.isEmpty() )
  {
    return;
  }

  const QString strPath{ MainWindow::GetProjectDir().canonicalPath() + "/" + CSV_FOLDER_NAME + "/" +
                         RELEASE_TABLE_NAME + "_" + strName + CSV_EXTENSION };

  const std::filesystem::path fsFilepath{ qPrintable( QDir::toNativeSeparators( strPath ) ) };
  std::filesystem::create_directories( fsFilepath.parent_path() );

  std::ofstream cOutfile( fsFilepath, std::ios::out | std::ios::trunc );
  if( cOutfile.is_open() )
  {
    const int32_t iKey{ GetActionKey( i_iRow ) };

    FileFilterProxyModel* pcProxy{ nullptr };

    const auto& iter{ m_cFileProxyHash.find( iKey ) };
    if( iter != m_cFileProxyHash.end() )
    {
      pcProxy = iter->second;
    }

    if( pcProxy )
    {
      const smFileSystemModel* pcModel{ dynamic_cast<smFileSystemModel*>( pcProxy->sourceModel() ) };
      Q_ASSERT( pcModel );

      const smFileSystemModel::FileMap& rcFileMap{ pcModel->GetFileMap() };

      QList<QString> cOutputList;

      typedef QMapIterator<QPersistentModelIndex, Qt::CheckState> FileMapIterator;
      FileMapIterator iter( rcFileMap );
      while( iter.hasNext() )
      {
        iter.next();
        if( iter.key().isValid() )
        {
          // For an item to be saved, it must be fully checked and must not be a directory
          if( iter.value() == Qt::Checked )
          {
            const QFileInfo cFileInfo{ pcModel->fileInfo( iter.key() ) };
            if( !cFileInfo.isDir() )
            {
              const QString& strPath{ MainWindow::GetProjectDir().relativeFilePath( cFileInfo.filePath() ) };
              cOutputList.push_back( strPath );
            }
            else
            {
              // TODO - save a directory if that's possible! So much better than hundreds of checked files!
            }
          }
        }
      }

      std::sort( cOutputList.begin(), cOutputList.end() );

      QListIterator iter2( cOutputList );
      while( iter2.hasNext() )
      {
        const QString& strPath( iter2.next() );
        cOutfile << qPrintable( strPath ) << '\n';
      }
    }

    cOutfile.close();

    // Remove the file stub if nothing was written
    if( std::filesystem::file_size( fsFilepath ) == 0U )
    {
      std::filesystem::remove( fsFilepath );
    }
  }
}


bool ReleaseManager::FileNeedsUpdate( const QString& i_strFileSource, const QString& i_strFileDest ) const
{
  bool bUpdate{ false };

  QSettings cSettings;
  const bool bOverwriteNewer{ cSettings.value( "PackageManager/OverwriteNewer", false ).toBool() };

  const QFileInfo cFileInfoSource( i_strFileSource );
  const QFileInfo cFileInfoDest( i_strFileDest );

  if( cFileInfoDest.exists() )
  {
    if( !bOverwriteNewer )
    {
      // Early out if the destination is newer than the source
      if( cFileInfoDest.lastModified() > cFileInfoSource.lastModified() )
      {
        RUM_COUT( qPrintable( i_strFileDest ) << " is newer than the source file, ignoring update\n" );
        return false;
      }
    }

    // Destination file exists, is it the same size as the source
    if( cFileInfoSource.size() == cFileInfoDest.size() )
    {
      // Files are the same size, but are their CRCs the same?
      const QString strFileSourceHash{ MainWindow::GetFileHash( i_strFileSource ) };
      const QString strFileDestHash{ MainWindow::GetFileHash( i_strFileDest ) };
      if( strFileSourceHash.compare( strFileDestHash ) != 0 )
      {
        RUM_COUT( qPrintable( i_strFileDest ) << " does not match source file hash, update required\n" );
        bUpdate = true;
      }
    }
    else
    {
      RUM_COUT( qPrintable( i_strFileDest ) << " does not match source file size, update required\n" );
      bUpdate = true;
    }
  }
  else
  {
    RUM_COUT( qPrintable( i_strFileDest ) << " does not exist, update required\n" );
    bUpdate = true;
  }

  return bUpdate;
}


int ReleaseManager::GetActionKey( int32_t i_iRow ) const
{
  QTableWidgetItem* pcItem{ nullptr };

  if( i_iRow <= m_pcUI->tableWidget->rowCount() )
  {
    pcItem = m_pcUI->tableWidget->item( i_iRow, COL_ACTION_NAME );
  }

  return pcItem ? pcItem->data( ROLE_ID ).toInt() : -1;
}


QString ReleaseManager::GetActionName( int32_t i_iRow ) const
{
  QTableWidgetItem* pcItem{ nullptr };

  if( i_iRow <= m_pcUI->tableWidget->rowCount() )
  {
    pcItem = m_pcUI->tableWidget->item( i_iRow, COL_ACTION_NAME );
  }

  return pcItem ? pcItem->text() : QString();
}


ServiceType ReleaseManager::GetActionService( int32_t i_iRow ) const
{
  QTableWidgetItem* pcItem{ nullptr };

  if( i_iRow <= m_pcUI->tableWidget->rowCount() )
  {
    pcItem = m_pcUI->tableWidget->item( i_iRow, COL_ACTION_SERVICE );
  }

  return pcItem ? (ServiceType)pcItem->data( ROLE_SERVICE ).toInt() : Shared_ServiceType;
}


QString ReleaseManager::GetDefaultPublishPath() const
{
  const QDir cPublishPath{ QCoreApplication::applicationDirPath() + "/export" };
  return cPublishPath.absolutePath();
}


void ReleaseManager::ImportCSVFile( int32_t i_iRow )
{
  const int32_t iKey{ GetActionKey( i_iRow ) };

  FileFilterProxyModel* pcProxy{ nullptr };

  const auto iter{ m_cFileProxyHash.find( iKey ) };
  if( iter != m_cFileProxyHash.end() )
  {
    pcProxy = iter->second;
  }

  if( pcProxy )
  {
    // The CSV for this row has already been imported
    return;
  }

  const QString& strName{ GetActionName( iKey ) };

  // File model doesn't already exist, create a new model
  smFileSystemModel* pcModel{ new smFileSystemModel( this ) };

  pcModel->setReadOnly( true );

  // Create the filter proxy
  pcProxy = new FileFilterProxyModel( this );
  pcProxy->setSourceModel( pcModel );

  m_cFileProxyHash.insert( std::make_pair( iKey, pcProxy ) );

  smFileSystemModel::FileSet cFileSet;

  const QString strFilePath{ MainWindow::GetProjectDir().canonicalPath() + "/" + CSV_FOLDER_NAME + "/" +
                             RELEASE_TABLE_NAME + "_" + strName + CSV_EXTENSION };

  std::ifstream cFile( qPrintable( strFilePath ), std::ios::in );
  if( cFile.is_open() )
  {
    std::string strRow;
    enum{ COL_FILE };

    while( !cFile.eof() )
    {
      std::getline( cFile, strRow );
      if( !strRow.empty() )
      {
        const auto vFields{ rumStringUtils::ParseCSVRow( strRow ) };

        const QString& strFile{ vFields.at( COL_FILE ).c_str() };
        if( !strFile.isEmpty() )
        {
          cFileSet.insert( MainWindow::GetProjectDir().absoluteFilePath( strFile ) );
        }
      }
    }

    // Tell the model to use this check info
    pcModel->SetCheckedFiles( cFileSet );
  }
  else
  {
    QMessageBox::warning( nullptr, tr( "Warning" ), "Failed to fetch package info from file" );
  }
}


void ReleaseManager::InitActionItems()
{
  m_pcUI->tableWidget->clearContents();
  m_pcUI->tableWidget->setRowCount( 0 );

  // Add all release actions
  std::filesystem::path fsFilepath
  {
    std::filesystem::path( qPrintable( MainWindow::GetProjectDir().canonicalPath() ) ) /
      CSV_FOLDER_NAME / RELEASE_TABLE_NAME
  };

  fsFilepath += CSV_EXTENSION;

  std::ifstream cFile( fsFilepath, std::ios::in );
  if( cFile.is_open() )
  {
    std::string strRow;
    enum{ COL_NAME, COL_SERVICE };

    while( !cFile.eof() )
    {
      std::getline( cFile, strRow );
      if( !strRow.empty() )
      {
        const auto cFieldsVector{ rumStringUtils::ParseCSVRow( strRow ) };

        const QString& strName{ cFieldsVector.at( COL_NAME ).c_str() };
        const ServiceType eServiceType{ (ServiceType)rumStringUtils::ToInt( cFieldsVector.at( COL_SERVICE ) ) };

        AddActionItem( s_iNextKey, strName, eServiceType );
      }
    }

    cFile.close();
  }

  m_pcUI->tableWidget->resizeColumnsToContents();
}


// slot
void ReleaseManager::ItemComboChanged( const QString& i_strText )
{
  const int32_t iRow{ m_pcUI->tableWidget->currentRow() };
  const int32_t iCol{ m_pcUI->tableWidget->currentColumn() };
  bool bAcceptChange{ false };

  if( iCol == COL_ACTION_SERVICE )
  {
    QVariant cVariant;

    // Access user data if it is available
    const QWidget* pcWidget{ m_pcUI->tableWidget->cellWidget( iRow, iCol ) };
    if( pcWidget )
    {
      const QComboBox* pcComboBox{ qobject_cast<const QComboBox*>( pcWidget ) };
      if( pcComboBox )
      {
        cVariant = pcComboBox->currentData();
      }

      m_pcUI->tableWidget->removeCellWidget( iRow, iCol );
    }

    QTableWidgetItem* pcCell{ new QTableWidgetItem() };
    pcCell->setData( Qt::DisplayRole, i_strText );

    if( cVariant.isValid() )
    {
      pcCell->setData( Qt::UserRole, cVariant );
    }

    m_pcUI->tableWidget->setItem( iRow, iCol, pcCell );
    pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
  }
  else
  {
    rumAssertMsg( false, "Unexpected column modified" );
  }

  SetDirty( true );
}


void ReleaseManager::on_actionDelete_Action_triggered()
{
  const int32_t iRow{ m_pcUI->tableWidget->currentRow() };

  QString strQuestion{ "Are you sure you want to delete release action " };
  strQuestion += GetActionName( iRow );
  strQuestion += "?";

  // Verify delete
  if( QMessageBox::Yes == QMessageBox::question( this, "Verify Delete", strQuestion,
                                                 QMessageBox::Yes | QMessageBox::No,
                                                 QMessageBox::No ) )
  {
    DeleteAction( iRow );
  }
}


void ReleaseManager::on_actionNew_Action_triggered()
{
  QString strName{ "Action_" };
  strName += rumStringUtils::ToString( s_iNextKey );

  AddActionItem( s_iNextKey, strName, Shared_ServiceType );
}


void ReleaseManager::on_actionPublish_Action_triggered()
{
  const int32_t iRow{ m_pcUI->tableWidget->currentRow() };

  QSettings cSettings;
  const QString strPublishPath{ cSettings.value( "PackageManager/PublishPath", GetDefaultPublishPath() ).toString() };
  bool bOverwriteNewer{ cSettings.value( "PackageManager/OverwriteNewer", false ).toBool() };

  QString strQuestion{ "Are you sure you want to publish " };
  strQuestion += GetActionName( iRow );
  strQuestion += "?\n\nWARNING: This process will overwrite existing files at the destination:\n\n";
  strQuestion += strPublishPath;
  strQuestion += "\n";

  if( bOverwriteNewer )
  {
    strQuestion += "\n*All files will be overwritten, even if newer than the source.";
  }
  else
  {
    strQuestion += "\n*Destination files will not be overwritten if they are newer than the source.";
  }

  if( QMessageBox::Yes == QMessageBox::question( this, "Publish Action", strQuestion,
                                                 QMessageBox::Yes | QMessageBox::No,
                                                 QMessageBox::No ) )
  {
    PublishAction( iRow );
  }
}


void ReleaseManager::on_actionPublish_Databases_triggered()
{
  QSettings cSettings;
  const QString strPublishPath{ cSettings.value( "PackageManager/PublishPath", GetDefaultPublishPath() ).toString() };

  QString strQuestion{ "Are you sure you want to publish databases?\n\n"
                       "WARNING: This process will overwrite existing files at the destination:\n\n" };
  strQuestion += strPublishPath;
  strQuestion += "\n";

  if( QMessageBox::Yes == QMessageBox::question( this, "Publish Databases", strQuestion,
                                                 QMessageBox::Yes | QMessageBox::No,
                                                 QMessageBox::No ) )
  {
    PublishDatabases();
  }
}


void ReleaseManager::on_actionPublish_Executables_triggered()
{
  QSettings cSettings;
  const QString strPublishPath{ cSettings.value( "PackageManager/PublishPath", GetDefaultPublishPath() ).toString() };
  const bool bOverwriteNewer{ cSettings.value( "PackageManager/OverwriteNewer", false ).toBool() };
  const bool bCopyDebug{ cSettings.value( "PackageManager/CopyDebug", false ).toBool() };

  QString strQuestion{ "Are you sure you want to publish executables?\n\n"
                       "WARNING: This process will overwrite existing files at the destination:\n\n" };
  strQuestion += strPublishPath;
  strQuestion += "\n";

  if( bOverwriteNewer )
  {
    strQuestion += "\n*All files will be overwritten, even if newer than the source.";
  }
  else
  {
    strQuestion += "\n*Destination files will not be overwritten if they are newer than the source.";
  }

  if( bCopyDebug )
  {
    strQuestion += "\n*Debug executables will be provided.";
  }

  if( QMessageBox::Yes == QMessageBox::question( this, "Publish Executables", strQuestion,
                                                 QMessageBox::Yes | QMessageBox::No,
                                                 QMessageBox::No ) )
  {
    PublishExecutables();
  }
}


void ReleaseManager::on_actionPublish_Release_triggered()
{
  QSettings cSettings;
  const QString strPublishPath{ cSettings.value( "PackageManager/PublishPath", GetDefaultPublishPath() ).toString() };
  const bool bCleanTarget{ cSettings.value( "PackageManager/CleanTarget", true ).toBool() };
  const bool bOverwriteNewer{ cSettings.value( "PackageManager/OverwriteNewer", false ).toBool() };
  const bool bCopyDebug{ cSettings.value( "PackageManager/CopyDebug", false ).toBool() };
  const bool bCompileScripts{ cSettings.value( "PackageManager/CompileScripts", false ).toBool() };

  QString strQuestion{ "Are you sure you want to publish a full release?\n\n"
                       "WARNING: This process will overwrite existing files at the destination:\n\n" };
  strQuestion += strPublishPath;
  strQuestion += "\n";

  if( bCleanTarget )
  {
    strQuestion += "\n*All files in the destination folder will be removed prior to the publish.";
  }

  if( bOverwriteNewer )
  {
    strQuestion += "\n*All files will be overwritten, even if newer than the source.";
  }
  else
  {
    strQuestion += "\n*Destination files will not be overwritten if they are newer than the source.";
  }

  if( bCopyDebug )
  {
    strQuestion += "\n*Debug executables will be provided.";
  }

  if( QMessageBox::No == QMessageBox::question( this, "Publish Release", strQuestion,
                                                QMessageBox::Yes | QMessageBox::No,
                                                QMessageBox::No ) )
  {
    return;
  }

  MainWindow* pcWindow{ MainWindow::GetMainWindow() };
  rumAssert( pcWindow != nullptr );
  pcWindow->ShowOutputDialog();
  pcWindow->ShowOnOutputDialog( "Publish Release" );

  if( bCleanTarget )
  {
    // Delete everything at the destination
    QDir dir( strPublishPath );
    dir.removeRecursively();
  }

  // Visit and execute each defined publish action
  for( int32_t i{ 0 }; i < m_pcUI->tableWidget->rowCount(); ++i )
  {
    ImportCSVFile( i );
    PublishAction( i );
  }

  PublishExecutables();
  PublishDatabases();

  const std::filesystem::path fsPublishPath( qPrintable( GetDefaultPublishPath() ) );

  if( bCompileScripts )
  {
    // Disable the scheduler during script compilation
    rumScheduler::EnableScheduling( false );

    {
      // Create the client game script from the exported scripts
      const std::filesystem::path fsClientScriptPath( fsPublishPath / "client" / rumScript::GetFolderName() );
      rumScript::Init( rumScript::VM_CLIENT, fsClientScriptPath.generic_string() );
      BindScripts();
      rumScript::Compile( rumScript::VM_CLIENT, fsClientScriptPath.generic_string() );
      rumScript::ShutdownVM( rumScript::VM_CLIENT );

      // Recursively delete all of the non-binary script files in the script subfolder
      const QString strPath( fsClientScriptPath.generic_string().c_str() );
      QDir cDir( strPath );
      cDir.setNameFilters( QStringList() << "*.nut" );
      cDir.setFilter( QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks );

      foreach( const QString & strFile, cDir.entryList() )
      {
        cDir.remove( strFile );
      }

      // Delete every subfolder under the script subfolder
      const std::filesystem::path fsSubClientScriptPath( fsClientScriptPath / "client" );
      const QString strSubPath( fsSubClientScriptPath.generic_string().c_str() );
      QDir cSubDir( strSubPath );
      cSubDir.setFilter( QDir::AllEntries | QDir::NoDotAndDotDot | QDir::NoSymLinks );
      cSubDir.removeRecursively();
    }

    {
      // Create the server game script from the exported scripts
      const std::filesystem::path fsServerScriptPath( fsPublishPath / "server" / rumScript::GetFolderName() );
      rumScript::Init( rumScript::VM_SERVER, fsServerScriptPath.generic_string() );
      BindScripts();
      rumScript::Compile( rumScript::VM_SERVER, fsServerScriptPath.generic_string() );
      rumScript::ShutdownVM( rumScript::VM_SERVER );

      // Recursively delete all of the non-binary script files in the script subfolder
      const QString strPath( fsServerScriptPath.generic_string().c_str() );
      QDir cDir( strPath );
      cDir.setNameFilters( QStringList() << "*.nut" );
      cDir.setFilter( QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks );

      foreach( const QString & strFile, cDir.entryList() )
      {
        cDir.remove( strFile );
      }

      // Delete every subfolder under the script subfolder
      const std::filesystem::path fsSubServerScriptPath( fsServerScriptPath / "server" );
      const QString strSubPath( fsSubServerScriptPath.generic_string().c_str() );
      QDir cSubDir( strSubPath );
      cSubDir.setFilter( QDir::AllEntries | QDir::NoDotAndDotDot | QDir::NoSymLinks );
      cSubDir.removeRecursively();
    }

    // Re-enable scheduling
    rumScheduler::EnableScheduling( true );

    rumScript::SetCurrentVMType( rumScript::VM_EDITOR );
  }

  pcWindow->ShowOnOutputDialog( "\nPublish Release - completed\n" );
}


void ReleaseManager::on_actionSave_triggered()
{
  ExportCSVFiles();
}


void ReleaseManager::OnCellStartEdit( int32_t i_iRow, int32_t i_iCol )
{
  if( COL_ACTION_SERVICE == i_iCol )
  {
    QComboBox* pcCombo{ new QComboBox };
    pcCombo->addItem( "Root", ServiceType::Shared_ServiceType );
    pcCombo->addItem( "Server", ServiceType::Server_ServiceType );
    pcCombo->addItem( "Client", ServiceType::Client_ServiceType );

    const int32_t iIndex{ pcCombo->findData( GetActionService( i_iRow ) ) };
    pcCombo->setCurrentIndex( iIndex );
    pcCombo->view()->setMinimumWidth( pcCombo->minimumSizeHint().width() );

    connect( pcCombo, SIGNAL( currentIndexChanged( const QString& ) ),
             this, SLOT( ItemComboChanged( const QString& ) ) );

    m_pcUI->tableWidget->setCellWidget( i_iRow, i_iCol, pcCombo );
  }
}


void ReleaseManager::onDirectoryLoaded( const QString& i_rcPath )
{
  m_pcUI->treeView->resizeColumnToContents( COL_ACTION_NAME );
}


void ReleaseManager::onTreeDataChanged()
{
  SetDirty( true );
}


void ReleaseManager::on_checkBox_stateChanged( int32_t i_eState )
{
  QSettings cSettings;
  cSettings.setValue( "PackageManager/CleanTarget", m_pcUI->checkBox->isChecked() );
}


void ReleaseManager::on_checkBox_2_stateChanged( int32_t i_eState )
{
  QSettings cSettings;
  cSettings.setValue( "PackageManager/OverwriteNewer", m_pcUI->checkBox_2->isChecked() );
}


void ReleaseManager::on_checkBox_3_stateChanged( int32_t i_eState )
{
  QSettings cSettings;
  cSettings.setValue( "PackageManager/CopyDebug", m_pcUI->checkBox_3->isChecked() );
}


void ReleaseManager::on_checkBox_4_stateChanged( int32_t i_eState )
{
  QSettings cSettings;
  cSettings.setValue( "PackageManager/CompileScripts", m_pcUI->checkBox_4->isChecked() );
}


void ReleaseManager::on_lineEdit_textEdited( const QString& i_strText )
{
  if( i_strText.isEmpty() )
  {
    m_pcUI->lineEdit->setText( GetDefaultPublishPath() );
  }

  QSettings cSettings;
  cSettings.setValue( "PackageManager/PublishPath", m_pcUI->lineEdit->text() );
}


void ReleaseManager::on_pushButton_clicked()
{
  const QString strPath{ QFileDialog::getExistingDirectory( this, tr( "Publish Folder" ) ) };
  m_pcUI->lineEdit->setText( strPath );
}


void ReleaseManager::on_pushButton_Filter_clicked()
{
  QAbstractItemModel* pcModel{ m_pcUI->treeView->model() };
  FileFilterProxyModel* pcFilterProxy{ dynamic_cast<FileFilterProxyModel*>( pcModel ) };

  const QString strFilter{ m_pcUI->lineEdit_Filter->text() };
  pcFilterProxy->FilterUpdated( strFilter );

  // Not sure how to get the proxy model from a QFileSystemModel, so this will have to do
  m_pcCurrentModel->SetFilter( strFilter );
}


void ReleaseManager::on_tableWidget_customContextMenuRequested( const QPoint& i_rcPos )
{
  if( i_rcPos.isNull() )
  {
    return;
  }

  QMenu* pcMenu{ new QMenu( m_pcUI->tableWidget ) };
  pcMenu->addAction( m_pcUI->actionPublish_Release );
  pcMenu->addAction( m_pcUI->actionPublish_Action );
  pcMenu->addAction( m_pcUI->actionNew_Action );
  pcMenu->addAction( m_pcUI->actionDelete_Action );

  const QPoint cPos{ m_pcUI->tableWidget->mapToGlobal( i_rcPos ) };
  pcMenu->exec( cPos );
}


void ReleaseManager::on_tableWidget_itemSelectionChanged()
{
  if( m_pcUI->tableWidget->selectedItems().empty() )
  {
    m_pcUI->actionDelete_Action->setEnabled( false );
    return;
  }

  disconnect( m_pcUI->tableWidget, SIGNAL( itemSelectionChanged() ),
              this, SLOT( on_tableWidget_itemSelectionChanged() ) );

  const int32_t iRow{ m_pcUI->tableWidget->currentRow() };

  m_pcUI->actionPublish_Action->setEnabled( true );
  m_pcUI->actionDelete_Action->setEnabled( true );

  if( !m_pcUI->treeView->isEnabled() )
  {
    m_pcUI->treeView->setEnabled( true );
  }

  ImportCSVFile( iRow );

  FileFilterProxyModel* pcProxy{ nullptr };

  // Get the stored file system model from the table data
  const int32_t iKey{ GetActionKey( iRow ) };
  const auto iter{ m_cFileProxyHash.find( iKey ) };
  if( iter != m_cFileProxyHash.end() )
  {
    pcProxy = iter->second;
  }

  if( !pcProxy )
  {
    return;
  }

  // Access the file model
  smFileSystemModel* pcModel{ dynamic_cast<smFileSystemModel*>( pcProxy->sourceModel() ) };
  if( pcModel )
  {
    m_pcCurrentModel = pcModel;

    disconnect( pcModel, SIGNAL( directoryLoaded( const QString& ) ),
                this, SLOT( onDirectoryLoaded( const QString& ) ) );
    disconnect( pcModel, SIGNAL( onDataChanged( const QModelIndex& ) ),
                this, SLOT( onTreeDataChanged() ) );
    disconnect( pcModel, SIGNAL( layoutChanged() ),
                this, SLOT( onModelLayoutChanged() ) );
  }

  pcProxy->ClearDirFilter();
  pcProxy->ClearFileFilter();

  // For whatever reason, this has to be called now to keep the program from crashing (as of 5.15 upgrade)
  m_pcUI->treeView->reset();

  m_pcUI->treeView->sortByColumn( TREEVIEW_COL_TYPE, Qt::AscendingOrder );
  m_pcUI->treeView->setSortingEnabled( true );

  // Finally, set our file model (via the proxy)
  m_pcUI->treeView->setModel( pcProxy );

  // Reflect package settings in our file view
  const QModelIndex cRootModelIndex{ SetReleasePath( pcModel, pcProxy ) };
  if( cRootModelIndex.isValid() )
  {
    // Fix the TreeView on the Root Path of the Model, note that we have to map the index of the file view to the
    // filter proxy because it's our current model
    m_pcUI->treeView->setRootIndex( pcProxy->mapFromSource( cRootModelIndex ) );
  }

  m_pcUI->treeView->header()->setSectionResizeMode( TREEVIEW_COL_FILENAME, QHeaderView::ResizeToContents );

  connect( m_pcUI->tableWidget, SIGNAL( itemSelectionChanged() ),
           this, SLOT( on_tableWidget_itemSelectionChanged() ) );

  if( pcModel )
  {
    connect( pcModel, SIGNAL( directoryLoaded( const QString& ) ),
             this, SLOT( onDirectoryLoaded( const QString& ) ) );
    connect( pcModel, SIGNAL( onDataChanged( const QModelIndex& ) ),
             this, SLOT( onTreeDataChanged() ) );
    connect( pcModel, SIGNAL( layoutChanged() ),
             this, SLOT( onModelLayoutChanged() ) );
  }
}


void ReleaseManager::PublishAction( int32_t i_iRow )
{
  MainWindow* pcWindow{ MainWindow::GetMainWindow() };
  rumAssert( pcWindow != nullptr );
  pcWindow->ShowOutputDialog();

  const QTableWidgetItem* pcItem{ m_pcUI->tableWidget->item( i_iRow, COL_ACTION_NAME ) };
  const QString& strAction{ GetActionName( i_iRow ) };

  pcWindow->ShowOnOutputDialog( "\nPublishing action " + strAction );

  const QDir cDirProject{ MainWindow::GetProjectDir() };

  QSettings cSettings;
  QString strPublishPath{ cSettings.value( "PackageManager/PublishPath", GetDefaultPublishPath() ).toString() };

  const ServiceType eServiceType{ GetActionService( i_iRow ) };
  switch( eServiceType )
  {
    case Server_ServiceType:
      strPublishPath += "/server/";
      break;

    case Client_ServiceType:
      strPublishPath += "/client/";
      break;
  }

  const QDir cPublishDir( strPublishPath );
  strPublishPath = cPublishDir.absoluteFilePath( QDir::cleanPath( strPublishPath ) );

  FileFilterProxyModel* pcProxy{ nullptr };

  const int32_t iKey{ GetActionKey( i_iRow ) };

  const auto iter{ m_cFileProxyHash.find( iKey ) };
  if( iter != m_cFileProxyHash.end() )
  {
    pcProxy = iter->second;
  }

  if( !pcProxy )
  {
    return;
  }

  const smFileSystemModel* pcModel{ dynamic_cast<smFileSystemModel*>( pcProxy->sourceModel() ) };
  Q_ASSERT( pcModel );

  const smFileSystemModel::FileMap& rcFileMap{ pcModel->GetFileMap() };

  QList<QString> cOutputList;

  typedef QMapIterator<QPersistentModelIndex, Qt::CheckState> FileMapIterator;
  FileMapIterator iter2( rcFileMap );
  while( iter2.hasNext() )
  {
    iter2.next();

      // For an item to be saved, it must be fully checked and must not be a directory
    if( iter2.key().isValid() && ( iter2.value() == Qt::Checked ) )
    {
      const QFileInfo cFileInfo{ pcModel->fileInfo( iter2.key() ) };
      if( !cFileInfo.isDir() )
      {
        //const QString& strPath{ dirExport.relativeFilePath( cFileInfo.filePath() ) };
        cOutputList.push_back( cFileInfo.filePath() );
      }
    }
  }

  std::sort( cOutputList.begin(), cOutputList.end() );

  QListIterator iter3( cOutputList );
  while( iter3.hasNext() )
  {
    const QString& strSourcePath( iter3.next() );
    const QString strRelativePath( cDirProject.relativeFilePath( strSourcePath ) );
    const QString strDestinationPath{ QDir::cleanPath( strPublishPath + "/" + strRelativePath ) };
    pcWindow->ShowOnOutputDialog( "Copying " + strSourcePath + " to " + strDestinationPath );

    CopyFileCRC( strSourcePath, strDestinationPath );
  }

  pcWindow->ShowOnOutputDialog( "Publishing action " + strAction + " - completed" );
}


void ReleaseManager::PublishDatabases()
{
  using PatchType = rumPatcher::PatchType;

  QSettings cSettings;

  MainWindow* pcWindow{ MainWindow::GetMainWindow() };
  rumAssert( pcWindow != nullptr );
  pcWindow->ShowOutputDialog();
  pcWindow->ShowOnOutputDialog( "\nPublishing databases" );

  const QString strPublishPath{ cSettings.value( "PackageManager/PublishPath", GetDefaultPublishPath() ).toString() };
  const QString strClientPath{ strPublishPath + "/client/"};
  const QString strServerPath{ strPublishPath + "/server/" };

  if( !QDir().mkpath( strClientPath ) || !QDir().mkpath( strServerPath ) )
  {
    return;
  }

  // Copy the game db to both client and server
  {
    const QDir cDirProject{ MainWindow::GetProjectDir() };
    const QString strSourceFilePath{ cDirProject.absolutePath() + "/game.rum" };

    const QString strClientDBFilePath{ strClientPath + "game.rum" };
    pcWindow->ShowOnOutputDialog( "Copying " + strSourceFilePath + " to " + strClientDBFilePath );
    CopyFileCRC( strSourceFilePath, strClientDBFilePath );

    const QFileInfo cClientDBFile( strClientDBFilePath );
    QFile::remove( cClientDBFile.baseName() + ".db-shm" );
    QFile::remove( cClientDBFile.baseName() + ".db-wal" );

    const QString strServerDBFilePath{ strServerPath + "game.rum" };
    pcWindow->ShowOnOutputDialog( "Copying " + strSourceFilePath + " to " + strServerDBFilePath );
    CopyFileCRC( strSourceFilePath, strServerDBFilePath );

    const QFileInfo cServerDBFile( strServerDBFilePath );
    QFile::remove( cServerDBFile.baseName() + ".rum-shm" );
    QFile::remove( cServerDBFile.baseName() + ".rum-wal" );
  }

  {
    // Create the client string database
    const QString strClientStringsFilePath{ strClientPath + "strings.db" };
    const QFileInfo cClientStringsFileInfo( strClientStringsFilePath );

    // Remove the existing database files
    QFile::remove( strClientStringsFilePath );
    QFile::remove( cClientStringsFileInfo.baseName() + ".db-shm" );
    QFile::remove( cClientStringsFileInfo.baseName() + ".db-wal" );

    const std::filesystem::path cClientStringsDBPath( qPrintable( cClientStringsFileInfo.absoluteFilePath() ) );
    if( rumDatabase::CreateConnection( rumDatabase::Strings_DatabaseID, cClientStringsDBPath.string() ) )
    {
      pcWindow->ShowOnOutputDialog( "Creating " + strClientStringsFilePath );

      rumStringTable::ExportDB( qPrintable( MainWindow::GetProjectDir().canonicalPath() ),
                                { Shared_ServiceType, Client_ServiceType } );
      rumDatabase::CloseConnection( rumDatabase::Strings_DatabaseID );
    }
  }

  {
    // Create the server string database
    const QString strServerStringsFilePath{ strServerPath + "strings.db" };
    const QFileInfo cServerStringsFileInfo( strServerStringsFilePath );

    // Remove the existing database files
    QFile::remove( strServerStringsFilePath );
    QFile::remove( cServerStringsFileInfo.baseName() + ".db-shm" );
    QFile::remove( cServerStringsFileInfo.baseName() + ".db-wal" );

    const std::filesystem::path cServerStringsDBPath( qPrintable( cServerStringsFileInfo.absoluteFilePath() ) );
    if( rumDatabase::CreateConnection( rumDatabase::Strings_DatabaseID, cServerStringsDBPath.string() ) )
    {
      pcWindow->ShowOnOutputDialog( "Creating " + strServerStringsFilePath );

      rumStringTable::ExportDB( qPrintable( MainWindow::GetProjectDir().canonicalPath() ),
                                { Shared_ServiceType, Server_ServiceType, Client_ServiceType } );
      rumDatabase::CloseConnection( rumDatabase::Strings_DatabaseID );
    }
  }

  {
    // Create the client datatable database
    const QString strClientDataTablesFilePath{ strClientPath + "datatables.db" };
    const QFileInfo cClientDataTablesFileInfo( strClientDataTablesFilePath );

    // Remove the existing database files
    QFile::remove( strClientDataTablesFilePath );
    QFile::remove( cClientDataTablesFileInfo.baseName() + ".db-shm" );
    QFile::remove( cClientDataTablesFileInfo.baseName() + ".db-wal" );

    const std::filesystem::path cClientDataTablesDBPath( qPrintable( cClientDataTablesFileInfo.absoluteFilePath() ) );
    if( rumDatabase::CreateConnection( rumDatabase::DataTable_DatabaseID, cClientDataTablesDBPath.string() ) )
    {
      pcWindow->ShowOnOutputDialog( "Creating " + strClientDataTablesFilePath );

      rumDataTable::ExportDB( qPrintable( MainWindow::GetProjectDir().canonicalPath() ),
                              { Shared_ServiceType, Client_ServiceType } );
      rumDatabase::CloseConnection( rumDatabase::DataTable_DatabaseID );
    }
  }

  {
    // Create the server datatable database
    const QString strServerDataTablesFilePath{ strServerPath + "datatables.db" };
    const QFileInfo cServerDataTablesFileInfo( strServerDataTablesFilePath );

    // Remove the existing database files
    QFile::remove( strServerDataTablesFilePath );
    QFile::remove( cServerDataTablesFileInfo.baseName() + ".db-shm" );
    QFile::remove( cServerDataTablesFileInfo.baseName() + ".db-wal" );

    const std::filesystem::path cServerDataTablesDBPath( qPrintable( cServerDataTablesFileInfo.absoluteFilePath() ) );
    if( rumDatabase::CreateConnection( rumDatabase::DataTable_DatabaseID, cServerDataTablesDBPath.string() ) )
    {
      pcWindow->ShowOnOutputDialog( "Creating " + strServerDataTablesFilePath );

      rumDataTable::ExportDB( qPrintable( MainWindow::GetProjectDir().canonicalPath() ),
                              { Shared_ServiceType, Server_ServiceType } );
      rumDatabase::CloseConnection( rumDatabase::DataTable_DatabaseID );
    }
  }

  {
    // Create the server patch database
    const QString strServerPatchFilePath{ strServerPath + rumPatcher::GetPatchDatabaseName().data() };
    const QFileInfo cServerPatchFileInfo( strServerPatchFilePath );

    // Remove the existing database files
    QFile::remove( strServerPatchFilePath );
    QFile::remove( cServerPatchFileInfo.baseName() + ".db-shm" );
    QFile::remove( cServerPatchFileInfo.baseName() + ".db-wal" );

    const std::filesystem::path cServerPatchDBPath( qPrintable( cServerPatchFileInfo.absoluteFilePath() ) );
    if( rumDatabase::CreateConnection( rumDatabase::Patch_DatabaseID, cServerPatchDBPath.string() ) )
    {
      pcWindow->ShowOnOutputDialog( "Creating " + strServerPatchFilePath );

      rumPatcher::ExportDBTables( { PatchType::Standard, PatchType::Conditional, PatchType::Remove } );
      rumDatabase::CloseConnection( rumDatabase::Patch_DatabaseID );
    }
  }

  {
    // Create the client asset database
    const QString strClientAssetsFilePath{ strClientPath + "assets.db" };
    const QFileInfo cClientAssetsFileInfo( strClientAssetsFilePath );

    // Remove the existing database files
    QFile::remove( strClientAssetsFilePath );
    QFile::remove( cClientAssetsFileInfo.baseName() + ".db-shm" );
    QFile::remove( cClientAssetsFileInfo.baseName() + ".db-wal" );

    const std::filesystem::path cClientAssetsDBPath( qPrintable( cClientAssetsFileInfo.absoluteFilePath() ) );
    if( !rumDatabase::CreateConnection( rumDatabase::Assets_DatabaseID, cClientAssetsDBPath.string() ) )
    {
      std::string strError{ "Failed to connect to database " };
      strError += cClientAssetsDBPath.string();
      Logger::LogStandard( strError );
      pcWindow->ShowOnOutputDialog( strError.c_str() );
      rumAssertMsg( false, strError );
      return;
    }

    pcWindow->ShowOnOutputDialog( "Creating " + strClientAssetsFilePath );

    constexpr ServiceType eServiceType{ Client_ServiceType };

    rumAsset::ExportDBTables<rumBroadcastAsset>( eServiceType );
    rumAsset::ExportDBTables<rumCreatureAsset>( eServiceType );
    rumAsset::ExportDBTables<rumCustomAsset>( eServiceType );
    rumAsset::ExportDBTables<rumGraphicAsset>( eServiceType );
    rumAsset::ExportDBTables<rumInventoryAsset>( eServiceType );
    rumAsset::ExportDBTables<rumMapAsset>( eServiceType );
    rumAsset::ExportDBTables<rumPortalAsset>( eServiceType );
    rumAsset::ExportDBTables<rumPropertyAsset>( eServiceType );
    rumAsset::ExportDBTables<rumSoundAsset>( eServiceType );
    rumAsset::ExportDBTables<rumTileAsset>( eServiceType );
    rumAsset::ExportDBTables<rumWidgetAsset>( eServiceType );

    rumDatabase::Query( rumDatabase::Assets_DatabaseID, "VACUUM" );
    rumDatabase::CloseConnection( rumDatabase::Assets_DatabaseID );
  }

  {
    // Create server asset tables
    const QString strServerAssetsFilePath{ strServerPath + "assets.db" };
    const QFileInfo cServerAssetsFileInfo( strServerAssetsFilePath );

    // Remove the existing database files
    QFile::remove( strServerAssetsFilePath );
    QFile::remove( cServerAssetsFileInfo.baseName() + ".db-shm" );
    QFile::remove( cServerAssetsFileInfo.baseName() + ".db-wal" );

    const std::filesystem::path cServerAssetsDBPath( qPrintable( cServerAssetsFileInfo.absoluteFilePath() ) );
    if( !rumDatabase::CreateConnection( rumDatabase::Assets_DatabaseID, cServerAssetsDBPath.string() ) )
    {
      std::string strError{ "Failed to connect to database " };
      strError += cServerAssetsDBPath.string();
      Logger::LogStandard( strError );
      pcWindow->ShowOnOutputDialog( strError.c_str() );
      rumAssertMsg( false, strError );
      return;
    }

    pcWindow->ShowOnOutputDialog( "Creating " + strServerAssetsFilePath );

    constexpr ServiceType eServiceType{ Server_ServiceType };

    rumAsset::ExportDBTables<rumBroadcastAsset>( eServiceType );
    rumAsset::ExportDBTables<rumCreatureAsset>( eServiceType );
    rumAsset::ExportDBTables<rumCustomAsset>( eServiceType );
    rumAsset::ExportDBTables<rumInventoryAsset>( eServiceType );
    rumAsset::ExportDBTables<rumMapAsset>( eServiceType );
    rumAsset::ExportDBTables<rumPortalAsset>( eServiceType );
    rumAsset::ExportDBTables<rumPropertyAsset>( eServiceType );
    rumAsset::ExportDBTables<rumTileAsset>( eServiceType );
    rumAsset::ExportDBTables<rumWidgetAsset>( eServiceType );

    rumDatabase::Query( rumDatabase::Assets_DatabaseID, "VACUUM" );
    rumDatabase::CloseConnection( rumDatabase::Assets_DatabaseID );
  }

  pcWindow->ShowOnOutputDialog( "Publishing databases - completed" );
}


void ReleaseManager::PublishExecutables()
{
  MainWindow* pcWindow{ MainWindow::GetMainWindow() };
  rumAssert( pcWindow != nullptr );
  pcWindow->ShowOutputDialog();

  pcWindow->ShowOnOutputDialog( "\nPublishing executables" );

  const QString strPath{ QCoreApplication::applicationDirPath() };

  QSettings cSettings;
  const QString strPublishPath{ cSettings.value( "PackageManager/PublishPath", GetDefaultPublishPath() ).toString() };
  if( !QDir().mkpath( strPublishPath ) )
  {
    pcWindow->ShowOnOutputDialog( "Failed to create path: " + strPublishPath );
    return;
  }

  const std::vector<QString> cExecutableFilesVector
  {
    "rum_client.exe",
    "rum_client.ini",
    "rum_server.exe",
    "rum_server.ini",
    "fmodex64.dll"
  };

  for( const auto& iter : cExecutableFilesVector )
  {
    const QString strFile{ iter };
    const QFileInfo cFileSource( strPath + "/" + strFile );
    const QFileInfo cFileDest( strPublishPath + "/" + strFile );

    const QString strSourceFilePath{ cFileSource.absoluteFilePath() };
    const QString strDestFilePath{ cFileDest.absoluteFilePath() };

    pcWindow->ShowOnOutputDialog( "Copying " + strSourceFilePath + " to " + strDestFilePath );

    CopyFileCRC( cFileSource.absoluteFilePath(), cFileDest.absoluteFilePath() );
  }

  bool bCopyDebug{ cSettings.value( "PackageManager/CopyDebug", false ).toBool() };
  if( bCopyDebug )
  {
    const std::vector<QString> cExecutableDebugFilesVector
    {
      "rum_client_debug.exe",
      "rum_client_debug.pdb",
      "rum_server_debug.exe",
      "rum_server_debug.pdb"
    };

    for( const auto& iter : cExecutableDebugFilesVector )
    {
      const QString strFile{ iter };
      const QFileInfo cFileSource( strPath + "/" + strFile );
      const QFileInfo cFileDest( strPublishPath + "/" + strFile );

      const QString strSourceFilePath{ cFileSource.absoluteFilePath() };
      const QString strDestFilePath{ cFileDest.absoluteFilePath() };

      pcWindow->ShowOnOutputDialog( "Copying " + strSourceFilePath + " to " + strDestFilePath );

      CopyFileCRC( cFileSource.absoluteFilePath(), cFileDest.absoluteFilePath() );
    }
  }

  pcWindow->ShowOnOutputDialog( "Publishing executables - completed" );
}


void ReleaseManager::ReadSettings()
{
  QSettings cSettings;

  const QString strPublishPath{ cSettings.value( "PackageManager/PublishPath", GetDefaultPublishPath() ).toString() };
  m_pcUI->lineEdit->setText( strPublishPath );

  const bool bCleanTarget{ cSettings.value( "PackageManager/CleanTarget", true ).toBool() };
  m_pcUI->checkBox->setChecked( bCleanTarget );

  const bool bOverwriteNewer{ cSettings.value( "PackageManager/OverwriteNewer", false ).toBool() };
  m_pcUI->checkBox_2->setChecked( bOverwriteNewer );

  const bool bCopyDebug{ cSettings.value( "PackageManager/CopyDebug", false ).toBool() };
  m_pcUI->checkBox_3->setChecked( bCopyDebug );

  const bool bCompileScripts{ cSettings.value( "PackageManager/CompileScripts", false ).toBool() };
  m_pcUI->checkBox_4->setChecked( bCompileScripts );
}


QModelIndex ReleaseManager::SetReleasePath( smFileSystemModel* i_pcModel, FileFilterProxyModel* i_pcProxy )
{
  return i_pcModel->SetRootPath( MainWindow::GetProjectDir().canonicalPath() );
}


void ReleaseManager::SetDirty( bool i_bDirty )
{
  if( !m_bInternalUpdate && m_bDirty != i_bDirty )
  {
    m_bDirty = i_bDirty;
    m_pcUI->actionSave->setEnabled( i_bDirty );
  }
}


void ReleaseManager::onModelLayoutChanged()
{
  if( m_pcCurrentModel )
  {
    m_bInternalUpdate = true;
    m_pcCurrentModel->ResetCheckedFiles();
    m_bInternalUpdate = false;
  }
}
