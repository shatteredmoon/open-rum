#include <packagemanager.h>
#include <ui_packagemanager.h>

#include <filefilter.h>
#include <mainwindow.h>
#include <smFileSystemModel.h>

#include <QCloseEvent>
#include <QComboBox>
#include <QFileInfo>
#include <QLineEdit>
#include <QMessageBox>

#include <u_rum.h>
#include <u_utility.h>
#include <u_zlib.h>

#include <filesystem>
#include <fstream>

#define PACKAGE_EXTENSION  ".pkg"
#define PACKAGE_TABLE_NAME "package"

enum
{
  COL_NAME = 0,
  COL_TYPE = 1
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
  ROLE_KEY =      Qt::UserRole,
  ROLE_CSV_NAME = Qt::UserRole + 1,
  ROLE_PKG_NAME = Qt::UserRole + 2,
  ROLE_PKG_TYPE = Qt::UserRole + 3
};

int32_t PackageManager::s_iNextKey{ 0 };

PackageManager::PackageManager( QWidget* i_pcParent )
  : QMainWindow( i_pcParent )
  , m_pcUI( new Ui::PackageManager )
{
  m_pcUI->setupUi( this );
  setCentralWidget( m_pcUI->splitter );

  const QIcon cPackageIcon( ":/ui/resources/package.png" );
  setWindowIcon( cPackageIcon );

  setMinimumWidth( 1024 );
  setMinimumHeight( 768 );

  // Set up the table widget
  QStringList cLabelList;
  cLabelList << "Name" << "Type";

  m_pcUI->tableWidget->setMinimumWidth( 200 );
  m_pcUI->tableWidget->setColumnCount( cLabelList.size() );
  m_pcUI->tableWidget->setRowCount( 0 );
  m_pcUI->tableWidget->setHorizontalHeaderLabels( cLabelList );
  m_pcUI->tableWidget->setSortingEnabled( true );

  m_pcUI->toolBar->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );

  const QIcon cBuildIcon( ":/ui/resources/process.png" );
  m_pcUI->actionBuild_Selected->setIcon( cBuildIcon );
  m_pcUI->actionBuild_Selected->setEnabled( false );

  const QIcon cBuildAllIcon( ":/ui/resources/process_all.png" );
  m_pcUI->actionBuild_All->setIcon( cBuildAllIcon );
  m_pcUI->actionBuild_All->setEnabled( false );

  const QIcon cAddIcon( ":/ui/resources/add.png" );
  m_pcUI->actionNew_Package->setIcon( cAddIcon );
  m_pcUI->actionNew_Package->setEnabled( true );

  const QIcon cRemoveIcon( ":/ui/resources/remove.png" );
  m_pcUI->actionDelete_Selected->setIcon( cRemoveIcon );
  m_pcUI->actionDelete_Selected->setEnabled( false );

  const QIcon cSaveIcon( ":/ui/resources/save.png" );
  m_pcUI->actionSave->setIcon( cSaveIcon );
  m_pcUI->actionSave->setEnabled( false );

  m_pcUI->tableWidget->setContextMenuPolicy( Qt::CustomContextMenu );

  Init();

  m_pcUI->tableWidget->resizeColumnsToContents();

  // Single selections only
  m_pcUI->tableWidget->setSelectionBehavior( QAbstractItemView::SelectRows );
  m_pcUI->tableWidget->setSelectionMode( QAbstractItemView::SingleSelection );

  // Set up the tree view
  m_pcUI->treeView->setMinimumWidth( 200 );
  m_pcUI->treeView->setEnabled( false );

  // Determine default sizes
  QHeaderView* pcHorizontalHeader{ m_pcUI->tableWidget->horizontalHeader() };
  pcHorizontalHeader->setStretchLastSection( true );
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


PackageManager::~PackageManager()
{
  s_iNextKey = 0;

  delete m_pcUI;

  // Release any loaded proxy models
  for( auto& iter : m_cFileProxyHash )
  {
    FileFilterProxyModel* pcProxy{ iter.second };
    SAFE_DELETE( pcProxy );
  }
}


void PackageManager::AddPackage( int32_t i_iKey, const QString& i_strName, AssetTypeHint i_eTypeHint ) const
{
  const int32_t iRow{ m_pcUI->tableWidget->rowCount() };
  m_pcUI->tableWidget->setRowCount( iRow + 1 );

  if( i_iKey >= s_iNextKey )
  {
    s_iNextKey = i_iKey + 1;
  }

  // CSV and package name
  QTableWidgetItem* pcCell{ new QTableWidgetItem() };
  pcCell->setData( Qt::DisplayRole, i_strName );
  pcCell->setData( ROLE_KEY, QVariant::fromValue( i_iKey ) );

  m_pcUI->tableWidget->setItem( iRow, COL_NAME, pcCell );

  // Package type
  pcCell = new QTableWidgetItem();
  QString strTypeHint{ "Any" };
  switch( i_eTypeHint )
  {
    case AssetTypeHint::Misc:    strTypeHint = "Misc";    break;
    case AssetTypeHint::Graphic: strTypeHint = "Graphic"; break;
    case AssetTypeHint::Audio:   strTypeHint = "Audio";   break;
    case AssetTypeHint::Map:     strTypeHint = "Map";     break;
    default:
      rumAssertMsg( false, "Unexpected type hint" );
      break;
  }
  pcCell->setData( Qt::DisplayRole, strTypeHint );
  pcCell->setData( ROLE_PKG_TYPE, i_eTypeHint );
  m_pcUI->tableWidget->setItem( iRow, COL_TYPE, pcCell );
  pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );

  UpdatePackageMeta( iRow );

  m_pcUI->actionBuild_All->setEnabled( true );
}


void PackageManager::Close()
{
  if( IsDirty() )
  {
    // Verify that user wants to keep unsaved changes
    const QString strQuestion( "Package Manager has unsaved changes! Save changes before closing?" );
    if( QMessageBox::Yes == QMessageBox::question( this, "Save changes?", strQuestion,
                                                   QMessageBox::Yes | QMessageBox::No,
                                                   QMessageBox::Yes ) )
    {
      // User wants to save changes
      ExportCSVFiles();
    }
  }
}


void PackageManager::closeEvent( QCloseEvent* i_pcEvent )
{
  Close();
  i_pcEvent->accept();
  emit closed();
}


void PackageManager::DeleteRow( int32_t i_iRow )
{
  if( i_iRow <= m_pcUI->tableWidget->rowCount() )
  {
    m_pcUI->tableWidget->removeRow( i_iRow );
  }

  if( m_pcUI->tableWidget->rowCount() == 0 )
  {
    m_pcUI->actionBuild_All->setEnabled( false );
  }
}


void PackageManager::ExportCSVFiles()
{
  if( !IsDirty() )
  {
    // Nothing to update
    return;
  }

  // Remove the dirty flag
  SetDirty( false );

  std::filesystem::path fsFilepath{ std::filesystem::path( qPrintable( MainWindow::GetProjectDir().canonicalPath() ) ) /
                                    CSV_FOLDER_NAME / PACKAGE_TABLE_NAME };
  fsFilepath += CSV_EXTENSION;

  std::filesystem::create_directories( fsFilepath.parent_path() );

  std::ofstream cOutfile( fsFilepath, std::ios::out | std::ios::trunc );
  if( cOutfile.is_open() )
  {
    // Visit each table entry and save out its csv and all of its supported file csvs
    const int32_t iNumRows{ m_pcUI->tableWidget->rowCount() };
    for( int32_t i{ 0 }; i < iNumRows; ++i )
    {
      const int32_t iKey{ GetPackageKey( i ) };
      const QString& strName{ GetPackageName( i ) };
      const AssetTypeHint eType{ GetPackageType( i ) };

      if( iKey != -1 )
      {
        cOutfile << qPrintable( strName ) << ',' << eType << '\n';
      }

      ExportCSVFiles( i );
    }

    cOutfile.close();

    // Remove the file stub if nothing was written
    if( std::filesystem::file_size( fsFilepath ) == 0U )
    {
      std::filesystem::remove( fsFilepath );
    }
  }
}


void PackageManager::ExportCSVFiles( int32_t i_iRow )
{
  const std::filesystem::path fsFilepath{ qPrintable( GetPackageCSVPath( i_iRow ) ) };
  std::filesystem::create_directories( fsFilepath.parent_path() );

  const int32_t iKey{ GetPackageKey( i_iRow ) };

  const FileFilterProxyModel* pcProxy{ nullptr };

  const auto iter{ m_cFileProxyHash.find( iKey ) };
  if( iter != m_cFileProxyHash.end() )
  {
    pcProxy = iter->second;
  }

  if( pcProxy )
  {
    std::ofstream cOutfile( fsFilepath, std::ios::out | std::ios::trunc );
    if( cOutfile.is_open() )
    {
      const smFileSystemModel* pcModel{ dynamic_cast<smFileSystemModel*>( pcProxy->sourceModel() ) };
      Q_ASSERT( pcModel );

      const smFileSystemModel::FileMap& rcFileMap{ pcModel->GetFileMap() };

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
              cOutfile << qPrintable( strPath ) << '\n';
            }
            else
            {
              // TODO - save a directory if that's possible! So much better than hundreds of checked files!
            }
          }
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
}


void PackageManager::ExportPackage( int32_t i_iRow )
{
  const int32_t iKey{ GetPackageKey( i_iRow ) };
  if( iKey == -1 )
  {
    return;
  }

  const auto proxyIter{ m_cFileProxyHash.find( iKey ) };
  if( proxyIter == m_cFileProxyHash.end() )
  {
    return;
  }

  const FileFilterProxyModel* pcProxy{ proxyIter->second };
  if( !pcProxy )
  {
    return;
  }

  const smFileSystemModel* pcModel{ dynamic_cast<smFileSystemModel*>( pcProxy->sourceModel() ) };
  if( !pcModel )
  {
    return;
  }

  // This is temporary... an MDI area with an output window showing exact package progress and individual file
  // pass/fail status and statistics should be shown instead of a MessageBox
  QString strError;

  // Save all selected files to the specified package
  ArchiveWriter cPackage;

  int32_t iNumAdded{ 0 };
  int32_t iNumFailed{ 0 };

  const smFileSystemModel::FileMap& rcFileMap{ pcModel->GetFileMap() };

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
          if( cPackage.AddFile( qPrintable( cFileInfo.filePath() ) ) )
          {
            ++iNumAdded;
          }
          else
          {
            ++iNumFailed;

            if( strError.isEmpty() )
            {
              strError = "The following files failed to be added:\n";
            }

            strError += cFileInfo.filePath() + "\n";
          }
        }
      }
    }
  }

  // Create the package
  const QString strFilePath( GetPackagePKGPath( i_iRow ) );
  cPackage.CreateArchive( qPrintable( strFilePath ) );

  QString strOutput
  {
    QString( "Package build complete, %1 file(s) added, %2 file(s) failed" ).arg( iNumAdded ).arg( iNumFailed )
  };

  if( iNumFailed > 0 )
  {
    strOutput += "\n\n" + strOutput;
  }

  QMessageBox::information( this, tr( "Package Built" ), strOutput );

  /*
  // Test the package
  int32_t result = 0;
  ArchiveReader ar(strFilePath.toLocal8Bit().constData());
  while (result == 0 && ar.hasMoreFileInfo() == true)
  {
      result = ar.extractToFile();
  }*/
}


// static
QString PackageManager::GetCSVPath( const QString& i_strName )
{
  const QString strPath{ MainWindow::GetProjectDir().canonicalPath() + "/" + CSV_FOLDER_NAME + "/" +
                         PACKAGE_TABLE_NAME + "_" + i_strName + CSV_EXTENSION };
  return QDir::toNativeSeparators( strPath );
}


QString PackageManager::GetPackageCSVPath( int32_t i_iRow ) const
{
  QTableWidgetItem* pcItem{ nullptr };

  if( i_iRow <= m_pcUI->tableWidget->rowCount() )
  {
    pcItem = m_pcUI->tableWidget->item( i_iRow, COL_NAME );
  }

  return pcItem ? pcItem->data( ROLE_CSV_NAME ).toString() : 0;
}


int PackageManager::GetPackageKey( int32_t i_iRow ) const
{
  QTableWidgetItem* pcItem{ nullptr };

  if( i_iRow <= m_pcUI->tableWidget->rowCount() )
  {
    pcItem = m_pcUI->tableWidget->item( i_iRow, COL_NAME );
  }

  return pcItem ? pcItem->data( ROLE_KEY ).toInt() : 0;
}


QString PackageManager::GetPackageName( int32_t i_iRow ) const
{
  QTableWidgetItem* pcItem{ nullptr };

  if( i_iRow <= m_pcUI->tableWidget->rowCount() )
  {
    pcItem = m_pcUI->tableWidget->item( i_iRow, COL_NAME );
  }

  return pcItem ? pcItem->text() : QString();
}


QString PackageManager::GetPackagePKGPath( int32_t i_iRow ) const
{
  QTableWidgetItem* pcItem{ nullptr };

  if( i_iRow <= m_pcUI->tableWidget->rowCount() )
  {
    pcItem = m_pcUI->tableWidget->item( i_iRow, COL_NAME );
  }

  return pcItem ? pcItem->data( ROLE_PKG_NAME ).toString() : 0;
}


PackageManager::AssetTypeHint PackageManager::GetPackageType( int32_t i_iRow ) const
{
  QTableWidgetItem* pcItem{ nullptr };

  if( i_iRow <= m_pcUI->tableWidget->rowCount() )
  {
    pcItem = m_pcUI->tableWidget->item( i_iRow, COL_TYPE );
  }

  return pcItem ? ( AssetTypeHint)pcItem->data( ROLE_PKG_TYPE ).toInt() : AssetTypeHint::Misc;
}


// static
QString PackageManager::GetPKGPath( const QString& i_strName, AssetTypeHint i_eType )
{
  QString strFolder;

  switch( i_eType )
  {
    case AssetTypeHint::Graphic:
      strFolder = "graphics";
      break;

    case AssetTypeHint::Audio:
      strFolder = "audio";
      break;

    case AssetTypeHint::Map:
      strFolder = "maps";
      break;
  }

  const QString strPath{ MainWindow::GetProjectDir().canonicalPath() + "/" + strFolder + "/" + i_strName +
                         PACKAGE_EXTENSION };
  return QDir::toNativeSeparators( strPath );
}


void PackageManager::Init()
{
  std::filesystem::path fsFilepath
  {
    std::filesystem::path( qPrintable( MainWindow::GetProjectDir().canonicalPath() ) ) /
      CSV_FOLDER_NAME / PACKAGE_TABLE_NAME
  };

  fsFilepath += CSV_EXTENSION;

  std::ifstream cFile( fsFilepath, std::ios::in );
  if( cFile.is_open() )
  {
    std::string strRow;
    enum{ COL_NAME, COL_TYPE };

    while( !cFile.eof() )
    {
      std::getline( cFile, strRow );
      if( !strRow.empty() )
      {
        const auto vFields{ rumStringUtils::ParseCSVRow( strRow ) };

        const QString& strName{ vFields.at( COL_NAME ).c_str() };
        const AssetTypeHint eType{ (AssetTypeHint)rumStringUtils::ToUInt( vFields.at( COL_TYPE ) ) };

        AddPackage( s_iNextKey, strName, eType );
      }
    }

    cFile.close();
  }
}


void PackageManager::ImportCSVFile( int32_t i_iRow )
{
  const int32_t iKey{ GetPackageKey( i_iRow ) };

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

  // File model doesn't already exist, create a new model
  smFileSystemModel* pcModel{ new smFileSystemModel( this ) };

  pcModel->setReadOnly( true );

  // Create the filter proxy
  pcProxy = new FileFilterProxyModel( this );
  pcProxy->setSourceModel( pcModel );

  m_cFileProxyHash.insert( std::make_pair( iKey, pcProxy ) );

  smFileSystemModel::FileSet fileSet;

  const QString strFilePath{ GetPackageCSVPath( i_iRow ) };

  std::ifstream cFile( qPrintable( strFilePath ), std::ios::in );
  if( cFile.is_open() )
  {
    std::string strRow;
    enum { COL_FILE };

    while( !cFile.eof() )
    {
      std::getline( cFile, strRow );
      if( !strRow.empty() )
      {
        const auto cFieldsVector{ rumStringUtils::ParseCSVRow( strRow ) };

        const QString& strFile{ cFieldsVector.at( COL_FILE ).c_str() };
        if( !strFile.isEmpty() )
        {
          fileSet.insert( MainWindow::GetProjectDir().absoluteFilePath( strFile ) );
        }
      }
    }

    // Tell the model to use this check info
    pcModel->SetCheckedFiles( fileSet );
  }
  else
  {
    QMessageBox::warning( nullptr, tr( "Warning" ), "Failed to fetch package info from file" );
  }
}


bool PackageManager::IsPackageNameUnique( const QString& i_strName ) const
{
  bool bUnique{ true };
  const int32_t iSelectedRow{ m_pcUI->tableWidget->currentRow() };

  // Visit each table row and compare names
  const int32_t iNumRows{ m_pcUI->tableWidget->rowCount() };
  for( int32_t iRow{ 0 }; iRow < iNumRows && bUnique; ++iRow )
  {
    if( iRow != iSelectedRow )
    {
      const QString& strName{ GetPackageName( iRow ) };
      if( strName.compare( i_strName, Qt::CaseInsensitive ) == 0 )
      {
        bUnique = false;
      }
    }
  }

  return bUnique;
}


// slot
void PackageManager::ItemComboChanged( const QString& i_strText )
{
  const int32_t iRow{ m_pcUI->tableWidget->currentRow() };
  const int32_t iCol{ m_pcUI->tableWidget->currentColumn() };

  if( iCol == COL_TYPE )
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
    pcCell->setData( ROLE_PKG_NAME, i_strText );

    if( cVariant.isValid() )
    {
      pcCell->setData( ROLE_PKG_TYPE, cVariant );
    }

    m_pcUI->tableWidget->setItem( iRow, iCol, pcCell );
    pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );

    UpdatePackageMeta( iRow );
    SetDirty( true );
  }
  else
  {
    rumAssertMsg( false, "Unexpected column modified" );
  }
}


void PackageManager::LineEditFinished()
{
  const int32_t iCol{ m_pcUI->tableWidget->currentColumn() };

  if( iCol == COL_NAME )
  {
    const int32_t iRow{ m_pcUI->tableWidget->currentRow() };

    const QWidget* pcWidget{ m_pcUI->tableWidget->cellWidget( iRow, iCol ) };
    if( pcWidget )
    {
      const QLineEdit* pcLineEdit{ qobject_cast<const QLineEdit*>( pcWidget ) };
      if( pcLineEdit )
      {
        disconnect( pcLineEdit, SIGNAL( editingFinished() ), this, SLOT( LineEditFinished() ) );

        const QString& strName{ pcLineEdit->text() };
        if( IsPackageNameUnique( strName ) )
        {
          QTableWidgetItem* pcItem{ m_pcUI->tableWidget->item( iRow, iCol ) };
          if( pcItem )
          {
            pcItem->setText( strName );
            UpdatePackageMeta( iRow );
            SetDirty( true );
          }
        }
        else
        {
          const QString strError{ "Package names must be unique" };
          QMessageBox::critical( this, tr( "Error" ), strError );
        }
      }

      m_pcUI->tableWidget->removeCellWidget( iRow, iCol );
    }
  }
}


void PackageManager::on_actionBuild_All_triggered()
{
  // Warn that all package files will be overwritten
  if( QMessageBox::Cancel == QMessageBox::question( this, "Overwrite?",
                                                    "Warning: All existing package files will be overwritten",
                                                    QMessageBox::Ok | QMessageBox::Cancel,
                                                    QMessageBox::Cancel ) )
  {
    // Do not overwrite
    return;
  }

  // Visit each table row
  const int32_t iNumRows{ m_pcUI->tableWidget->rowCount() };
  for( int32_t iRow{ 0 }; iRow < iNumRows; ++iRow )
  {
    ImportCSVFile( iRow );
    ExportPackage( iRow );
  }
}


void PackageManager::on_actionBuild_Selected_triggered()
{
  const int32_t iRow{ m_pcUI->tableWidget->currentRow() };
  const smFileSystemModel::FileMap& rcFileMap{ m_pcCurrentModel->GetFileMap() };

  const QString strFilePath{ GetPackagePKGPath( iRow ) };

  const QFileInfo cFileInfo( strFilePath );
  if( cFileInfo.exists() )
  {
    // Warn that the file will be overwritten
    if( QMessageBox::No == QMessageBox::question( this, "Overwrite?",
                                                  "Package " + strFilePath + " already exists, overwrite?",
                                                  QMessageBox::Yes | QMessageBox::No,
                                                  QMessageBox::Yes ) )
    {
      // Do not overwrite the currently existing package
      return;
    }
  }

  ExportPackage( iRow );
}


void PackageManager::on_actionDelete_Selected_triggered()
{
  const int32_t iRow{ m_pcUI->tableWidget->currentRow() };
  const QString strName{ GetPackageName( iRow ) };

  QString strQuestion = "Are you sure you want to delete package ";
  strQuestion += strName;
  strQuestion += "?";

  // Verify delete
  if( QMessageBox::Yes == QMessageBox::question( this, "Verify Delete", strQuestion,
                                                 QMessageBox::Yes | QMessageBox::No,
                                                 QMessageBox::No ) )
  {
    DeleteRow( m_pcUI->tableWidget->currentRow() );
  }
}


void PackageManager::on_actionNew_Package_triggered()
{
  QString strName{ "Package_" };
  strName += rumStringUtils::ToString( s_iNextKey );

  AddPackage( s_iNextKey, strName, AssetTypeHint::Misc );
}


void PackageManager::on_actionSave_triggered()
{
  ExportCSVFiles();
}


void PackageManager::onDirectoryLoaded( const QString &path )
{
  m_pcUI->treeView->resizeColumnToContents( COL_NAME );
}


void PackageManager::onTreeDataChanged()
{
  SetDirty( true );
}


void PackageManager::on_tableWidget_cellDoubleClicked( int32_t i_iRow, int32_t i_iCol )
{
  if( COL_NAME == i_iCol )
  {
    // Only allow numbers, letters, dash, and underscore up to 32 characters
    const QRegExp cRegExp( "[\\w-]{1,32}" );
    const QRegExpValidator* pcValidator{ new QRegExpValidator( cRegExp, this ) };

    QLineEdit* pcLineEdit{ new QLineEdit };
    pcLineEdit->setText( GetPackageName( i_iRow ) );
    pcLineEdit->setValidator( pcValidator );

    connect( pcLineEdit, SIGNAL( editingFinished() ), this, SLOT( LineEditFinished() ) );

    m_pcUI->tableWidget->setCellWidget( i_iRow, i_iCol, pcLineEdit );
  }
}


void PackageManager::on_tableWidget_customContextMenuRequested( const QPoint& i_rcPos )
{
  if( i_rcPos.isNull() )
  {
    return;
  }

  QMenu* pcMenu{ new QMenu( m_pcUI->tableWidget ) };
  pcMenu->addAction( m_pcUI->actionBuild_All );
  pcMenu->addAction( m_pcUI->actionBuild_Selected );
  pcMenu->addSeparator();
  pcMenu->addAction( m_pcUI->actionNew_Package );
  pcMenu->addAction( m_pcUI->actionDelete_Selected );

  pcMenu->exec( m_pcUI->tableWidget->mapToGlobal( i_rcPos ) );
}


void PackageManager::on_tableWidget_itemSelectionChanged()
{
  if( m_pcUI->tableWidget->selectedItems().empty() )
  {
    return;
  }

  disconnect( m_pcUI->tableWidget, SIGNAL( itemSelectionChanged() ),
              this, SLOT( on_tableWidget_itemSelectionChanged() ) );

  const int32_t iRow{ m_pcUI->tableWidget->currentRow() };

  m_pcUI->actionBuild_Selected->setEnabled( true );
  m_pcUI->actionDelete_Selected->setEnabled( true );

  m_pcUI->actionSave->setEnabled( IsDirty() );

  if( !m_pcUI->treeView->isEnabled() )
  {
    m_pcUI->treeView->setEnabled( true );
  }

  ImportCSVFile( iRow );

  // Get the stored file system model from the table data
  const int32_t iKey{ GetPackageKey( iRow ) };

  FileFilterProxyModel* pcProxy{ nullptr };

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

    disconnect( pcModel, SIGNAL(directoryLoaded(const QString&) ),
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
  const QModelIndex cModelIndex{ SetPackagePath( pcModel, pcProxy ) };
  if( cModelIndex.isValid() )
  {
    // Fix the TreeView on the Root Path of the Model, note that we have to map the index of the file view to the
    // filter proxy because it's our current model
    m_pcUI->treeView->setRootIndex( pcProxy->mapFromSource( cModelIndex ) );
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


void PackageManager::OnCellStartEdit( int32_t i_iRow, int32_t i_iCol )
{
  if( COL_TYPE == i_iCol )
  {
    const AssetTypeHint eType{ GetPackageType( i_iRow ) };

    QComboBox* pcCombo{ new QComboBox };
    pcCombo->addItem( "Misc", AssetTypeHint::Misc );
    pcCombo->addItem( "Graphic", AssetTypeHint::Graphic );
    pcCombo->addItem( "Audio", AssetTypeHint::Audio );
    pcCombo->addItem( "Map", AssetTypeHint::Map );

    const int32_t iIndex{ pcCombo->findData( eType ) };
    pcCombo->setCurrentIndex( iIndex );
    pcCombo->view()->setMinimumWidth( pcCombo->minimumSizeHint().width() );

    connect( pcCombo, SIGNAL( currentIndexChanged( const QString& ) ),
             this, SLOT( ItemComboChanged( const QString& ) ) );
    m_pcUI->tableWidget->setCellWidget( i_iRow, i_iCol, pcCombo );
  }
}


void PackageManager::onModelLayoutChanged()
{
  if( m_pcCurrentModel )
  {
    m_bInternalUpdate = true;
    m_pcCurrentModel->ResetCheckedFiles();
    m_bInternalUpdate = false;
  }
}


void PackageManager::SetDirty( bool i_bDirty )
{
  if( !m_bInternalUpdate && m_bDirty != i_bDirty )
  {
    m_bDirty = i_bDirty;
    m_pcUI->actionSave->setEnabled( i_bDirty );
  }
}


QModelIndex PackageManager::SetPackagePath( smFileSystemModel* i_pcModel, FileFilterProxyModel* i_pcProxy )
{
  QModelIndex cModelIndex{ i_pcModel->SetRootPath( MainWindow::GetProjectDir().canonicalPath() ) };
  if( !i_pcProxy )
  {
    return cModelIndex;
  }

  const int32_t iRow{ m_pcUI->tableWidget->currentRow() };
  const AssetTypeHint eTypeHint{ GetPackageType( iRow ) };
  if( eTypeHint == AssetTypeHint::Graphic )
  {
    cModelIndex = i_pcModel->SetRootPath( MainWindow::GetProjectGraphicDir().canonicalPath() );

    i_pcProxy->AddFileFilter( ".bmp" );
    i_pcProxy->AddFileFilter( ".dds" );
    i_pcProxy->AddFileFilter( ".gif" );
    i_pcProxy->AddFileFilter( ".jpeg" );
    i_pcProxy->AddFileFilter( ".jpg" );
    i_pcProxy->AddFileFilter( ".pcx" );
    i_pcProxy->AddFileFilter( ".png" );
    i_pcProxy->AddFileFilter( ".tga" );
    i_pcProxy->AddFileFilter( ".tif" );
  }
  else if( eTypeHint == AssetTypeHint::Audio )
  {
    cModelIndex = i_pcModel->SetRootPath( MainWindow::GetProjectAudioDir().canonicalPath() );

    i_pcProxy->AddFileFilter( ".mid" );
    i_pcProxy->AddFileFilter( ".mp3" );
    i_pcProxy->AddFileFilter( ".ogg" );
    i_pcProxy->AddFileFilter( ".snd" );
    i_pcProxy->AddFileFilter( ".wav" );
  }
  else if( eTypeHint == AssetTypeHint::Map )
  {
    cModelIndex = i_pcModel->SetRootPath( MainWindow::GetProjectMapDir().canonicalPath() );

    i_pcProxy->AddFileFilter( ".map" );
  }

  return cModelIndex;
}


void PackageManager::UpdatePackageMeta( int32_t i_iRow ) const
{
  if( i_iRow > m_pcUI->tableWidget->rowCount() )
  {
    return;
  }

  QTableWidgetItem* pcItem{ m_pcUI->tableWidget->item( i_iRow, COL_NAME ) };
  if( pcItem )
  {
    const QString& strName{ pcItem->text() };
    const int32_t iKey{ pcItem->data( ROLE_KEY ).toInt() };

    const QString& strCSVPath{ GetCSVPath( strName ) };
    pcItem->setData( ROLE_CSV_NAME, strCSVPath );

    const AssetTypeHint eType{ GetPackageType( i_iRow ) };

    const QString& strPKGPath{ GetPKGPath( strName, eType ) };
    pcItem->setData( ROLE_PKG_NAME, strPKGPath );

    const QString strTooltip
    {
      QString( "Key: %1\nSaves as: %2\nExports as: %3" ).arg( QString::number( iKey ), strCSVPath, strPKGPath )
    };

    pcItem->setToolTip( strTooltip );
  }
}
