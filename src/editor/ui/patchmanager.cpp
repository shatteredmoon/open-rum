#include <patchmanager.h>
#include <ui_patchmanager.h>

#include <mainwindow.h>

#include <QCloseEvent>
#include <QDirIterator>
#include <QFileDialog>
#include <QItemSelection>
#include <QMessageBox>

#include <u_log.h>

#include <md5.h>

enum class FileColumns : int32_t
{
  FilePath   = 0,
  CRC        = 1,
  Size       = 2,
  NumColumns = 3
};

enum class UserRole : int32_t
{
  PatchCategory = Qt::UserRole
};


PatchManager::PatchManager( QWidget* i_pcParent )
  : QMainWindow( i_pcParent )
  , m_pcFilterAction( nullptr )
  , m_pcUI( new Ui::PatchManager )
{
  m_pcUI->setupUi( this );
  setCentralWidget( m_pcUI->splitter );

  const QIcon cPatchIcon( ":/ui/resources/patch.png" );
  setWindowIcon( cPatchIcon );

  setMinimumWidth( 1024 );
  setMinimumHeight( 768 );

  m_pcUI->toolBar->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );

  const QIcon cSaveIcon( ":/ui/resources/save.png" );
  m_pcUI->actionSave->setIcon( cSaveIcon );
  m_pcUI->actionSave->setEnabled( false );

  const QIcon cGenerateIcon( ":/ui/resources/generate.png" );
  m_pcUI->actionGenerate_Patch->setIcon( cGenerateIcon );
  m_pcUI->actionGenerate_Patch->setEnabled( true );

  const QIcon cAddIcon( ":/ui/resources/add.png" );
  m_pcUI->actionAdd_Entry->setIcon( cAddIcon );
  m_pcUI->actionAdd_Entry->setEnabled( false );

  const QIcon cRemoveIcon( ":/ui/resources/remove.png" );
  m_pcUI->actionRemove_Entry->setIcon( cRemoveIcon );
  m_pcUI->actionRemove_Entry->setEnabled( false );

  const QIcon cClearIcon( ":/ui/resources/delete.png" );
  m_pcUI->actionClear_Patch_Table->setIcon( cClearIcon );
  m_pcUI->actionClear_Patch_Table->setEnabled( true );

  // Categories table ---------------------------------------------------------

  // Set up the table widget label names
  QStringList cLabelList;
  cLabelList << "Category";
  m_pcUI->tableWidget_Categories->setHorizontalHeaderLabels( cLabelList );

  m_pcUI->tableWidget_Categories->setColumnCount( 1 );
  m_pcUI->tableWidget_Categories->setMinimumWidth( 200 );

  connect( m_pcUI->tableWidget_Categories, SIGNAL( itemSelectionChanged() ),
           this, SLOT( on_tableWidget_Categories_itemSelectionChanged() ) );

  // Load existing tables from disk
  Init();

  // Patch table --------------------------------------------------------------

  m_pcUI->tableWidget_PatchFiles->setMinimumWidth( 200 );
  m_pcUI->tableWidget_PatchFiles->setColumnCount( rumUtility::ToUnderlyingType( FileColumns::NumColumns ) );
  m_pcUI->tableWidget_PatchFiles->setRowCount( 0 );

  m_pcUI->tableWidget_PatchFiles->setContextMenuPolicy( Qt::CustomContextMenu );

  m_pcUI->tableWidget_PatchFiles->setSortingEnabled( true );

  connect( m_pcUI->tableWidget_PatchFiles, SIGNAL( cellActivated( int32_t, int32_t ) ),
           this, SLOT( on_tableWidget_PatchFiles_cellActivated( int32_t, int32_t ) ) );

  connect( m_pcUI->tableWidget_PatchFiles, SIGNAL( cellDoubleClicked( int32_t, int32_t ) ),
           this, SLOT( on_tableWidget_PatchFiles_cellActivated( int32_t, int32_t ) ) );

  // Set up the string database widget
  cLabelList.clear();
  cLabelList << "File" << "CRC" << "Bytes";
  m_pcUI->tableWidget_PatchFiles->setHorizontalHeaderLabels( cLabelList );

  // Add a line edit on the string's toolbar for filtering
  m_pcUI->toolBar2->addSeparator();
  m_cFilterLabel.setText( "Filter: " );
  m_pcUI->toolBar2->addWidget( &m_cFilterLabel );
  m_pcFilterAction = m_pcUI->toolBar2->addWidget( &m_cFilterEdit );

  m_cFilterLabel.setEnabled( false );
  m_cFilterEdit.setEnabled( false );

  connect( &m_cFilterEdit, SIGNAL( editingFinished() ), this, SLOT( onFilterChanged() ) );

  // Final setup ------------------------------------------------------------

  // Determine default sizes
  QHeaderView* pcHorizontalHeader{ m_pcUI->tableWidget_Categories->horizontalHeader() };
  const int32_t iTableSize{ std::max( m_pcUI->tableWidget_Categories->minimumWidth(), pcHorizontalHeader->length() ) };
  const int32_t iViewSize{ 1024 - iTableSize };

  pcHorizontalHeader->setStretchLastSection( true );

  pcHorizontalHeader = m_pcUI->tableWidget_PatchFiles->horizontalHeader();
  pcHorizontalHeader->setStretchLastSection( true );

  QList<int32_t> cSplitterList;
  cSplitterList.append( iTableSize );
  cSplitterList.append( iViewSize );
  m_pcUI->splitter->setSizes( cSplitterList );

  RefreshPatchTable();
}


PatchManager::~PatchManager()
{
  delete m_pcUI;
}


void PatchManager::AddCategory( const QString& i_strName )
{
  const int32_t iRow{ m_pcUI->tableWidget_Categories->rowCount() };
  m_pcUI->tableWidget_Categories->setRowCount( iRow + 1 );

  QTableWidgetItem* pcCell{ new QTableWidgetItem( i_strName ) };
  m_pcUI->tableWidget_Categories->setItem( iRow, 0, pcCell );
  pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
  pcCell->setData( rumUtility::ToUnderlyingType( UserRole::PatchCategory ), QVariant::fromValue( iRow ) );
}


void PatchManager::AddFile( const rumPatcher::PatchType i_eCategory,
                            const rumPatcher::rumPatchFileInfo& i_rcPatchFileInfo )
{
  disconnect( m_pcUI->tableWidget_PatchFiles, &QTableWidget::itemChanged, this, &PatchManager::OnTableItemChanged );

  const int32_t iRow{ m_pcUI->tableWidget_PatchFiles->rowCount() };
  m_pcUI->tableWidget_PatchFiles->setRowCount( iRow + 1 );

  // File
  QTableWidgetItem* pcCell{ new QTableWidgetItem( i_rcPatchFileInfo.m_strFile.c_str() ) };
  m_pcUI->tableWidget_PatchFiles->setItem( iRow, rumUtility::ToUnderlyingType( FileColumns::FilePath ), pcCell );
  if( rumPatcher::PatchType::Ignore != i_eCategory && rumPatcher::PatchType::Remove != i_eCategory )
  {
    pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
  }
  
  // CRC
  pcCell = new QTableWidgetItem( i_rcPatchFileInfo.m_strFileCRC.c_str() );
  m_pcUI->tableWidget_PatchFiles->setItem( iRow, rumUtility::ToUnderlyingType( FileColumns::CRC ), pcCell );
  pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
  
  // Size
  pcCell = new QTableWidgetItem( rumStringUtils::ToString64( i_rcPatchFileInfo.m_uiFileSize ) );
  m_pcUI->tableWidget_PatchFiles->setItem( iRow, rumUtility::ToUnderlyingType( FileColumns::Size ), pcCell );
  pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );

  if( ( i_eCategory == rumPatcher::PatchType::Remove ) || ( i_eCategory == rumPatcher::PatchType::Ignore ) )
  {
    connect( m_pcUI->tableWidget_PatchFiles, &QTableWidget::itemChanged, this, &PatchManager::OnTableItemChanged );
  }
}


void PatchManager::Close()
{
}


void PatchManager::closeEvent( QCloseEvent* i_pcEvent )
{
  i_pcEvent->accept();
  emit closed();
}


void PatchManager::GeneratePatch()
{
  // Clear the table
  m_pcUI->tableWidget_PatchFiles->clearContents();
  m_pcUI->tableWidget_PatchFiles->setRowCount( 0 );

  QList<rumPatcher::rumPatchFileInfo> cPatchEntries;

  // Backup the original patch map
  rumPatcher::PatchMap& rcPatchMap{ rumPatcher::AccessPatchMap() };
  auto cPatchEntriesBackup{ rcPatchMap };

  // Clear all existing entries
  rcPatchMap.clear();

  // TODO - Some type of patching mechanism needs to exist for patching the client executables provided by RUM
  //QDir cPublishPath{ QCoreApplication::applicationDirPath() + "/export" };
  //cPublishPath.setFilter( QDir::AllEntries | QDir::NoDotAndDotDot | QDir::NoSymLinks );
  //GeneratePatchEntriesForPath( cPublishPath, cPatchEntries );

  // Patch only the files under the client folder
  QDir cClientPath{ QCoreApplication::applicationDirPath() + "/export/client" };
  cClientPath.setFilter( QDir::AllEntries | QDir::NoDotAndDotDot | QDir::NoSymLinks );

  GeneratePatchEntriesForPath( cClientPath, cPatchEntries, QDirIterator::Subdirectories );

  // Create the new tables based on the files that were just harvested. We compare against the old list to determine
  // what has changed and update accordingly.
  for( const auto& iter : cPatchEntries )
  {
    const auto& strFile{ iter.m_strFile };

    if( strFile.compare( rumPatcher::GetPatchDatabaseName().data() ) == 0 )
    {
      // This is the one file that is never patched by the patcher
      continue;
    }

    const auto IsIgnoredFile
    {
      [&strFile]( const auto& i_rcPatchFileInfo )
      {
        if( i_rcPatchFileInfo.m_strFile.find( "*" ) != std::string::npos )
        {
          // Convert the wildcard entry to a regular expression and test for a match
          const QString strWildcardExp
          {
            QRegularExpression::wildcardToRegularExpression( i_rcPatchFileInfo.m_strFile.c_str() )
          };

          QRegularExpression RegExp( QRegularExpression::anchoredPattern( strWildcardExp ),
                                     QRegularExpression::CaseInsensitiveOption );
          return RegExp.match( strFile.c_str() ).hasMatch();
        }

        return ( i_rcPatchFileInfo == strFile );
      }
    };

    const auto& rcIgnoreList{ cPatchEntriesBackup[rumPatcher::PatchType::Ignore] };
    const auto iterIgnore{ std::find_if( rcIgnoreList.begin(), rcIgnoreList.end(), IsIgnoredFile ) };
    if( iterIgnore != rcIgnoreList.end() )
    {
      // This file should be ignored
      continue;
    }
    else
    {
      const auto& rcConditionalList{ cPatchEntriesBackup[rumPatcher::PatchType::Conditional] };
      const auto iterConditional{ std::find_if( rcConditionalList.begin(), rcConditionalList.end(),
                                                [&strFile]( const auto& i_rcPatchFileInfo )
                                                {
                                                  return i_rcPatchFileInfo == strFile;
                                                } ) };
      if( iterConditional != rcConditionalList.end() )
      {
        // The file already exists as a conditional, so it remains conditional
        rcPatchMap[rumPatcher::PatchType::Conditional].push_back( iter );
        cPatchEntriesBackup[rumPatcher::PatchType::Conditional].erase( iterConditional );
      }
      else
      {
        // No matter what happens next, this is going to be a file that should be updated on the client
        rcPatchMap[rumPatcher::PatchType::Standard].push_back( iter );

        // Determine if the file was previously marked as a file that should be deleted on the client
        const auto& rcDeleteList{ cPatchEntriesBackup[rumPatcher::PatchType::Remove] };
        const auto iterDelete{ std::find_if( rcDeleteList.begin(), rcDeleteList.end(),
                                             [&strFile]( const auto& i_rcPatchFileInfo )
                                             {
                                               return i_rcPatchFileInfo == strFile;
                                             } ) };
        if( iterDelete != rcDeleteList.end() )
        {
          // The file was previously marked as a file that should be deleted, but it's back so remove the delete entry
          cPatchEntriesBackup[rumPatcher::PatchType::Remove].erase( iterDelete );
        }
        else
        {
          // If the file was in the old update list, remove it so that it's not marked for delete later
          const auto& rcUpdateList{ cPatchEntriesBackup[rumPatcher::PatchType::Standard] };
          const auto iterUpdate{ std::find_if( rcUpdateList.begin(), rcUpdateList.end(),
                                               [&strFile]( const auto& i_rcPatchFileInfo )
                                               {
                                                 return i_rcPatchFileInfo == strFile;
                                               } ) };
          if( iterUpdate != rcUpdateList.end() )
          {
            cPatchEntriesBackup[rumPatcher::PatchType::Standard].erase( iterUpdate );
          }
        }
      }
    }
  }

  // Copy an existing updates to delete
  const auto& cBackupUpdateList{ cPatchEntriesBackup[rumPatcher::PatchType::Standard] };
  for( const auto& rcPatchFileInfo : cBackupUpdateList )
  {
    rcPatchMap[rumPatcher::PatchType::Remove].push_back( rcPatchFileInfo );
  }

  // Copy an existing conditionals to delete
  const auto& cBackupConditionalList{ cPatchEntriesBackup[rumPatcher::PatchType::Conditional] };
  for( const auto& rcPatchFileInfo : cBackupConditionalList )
  {
    rcPatchMap[rumPatcher::PatchType::Remove].push_back( rcPatchFileInfo );
  }

  // Copy the existing delete list
  const auto& cBackupDeleteList{ cPatchEntriesBackup[rumPatcher::PatchType::Remove] };
  for( const auto& rcPatchFileInfo : cBackupDeleteList )
  {
    rcPatchMap[rumPatcher::PatchType::Remove].push_back( rcPatchFileInfo );
  }

  // Copy the existing ignore list
  const auto& cBackupIgnoreList{ cPatchEntriesBackup[rumPatcher::PatchType::Ignore] };
  for( const auto& rcPatchFileInfo : cBackupIgnoreList )
  {
    rcPatchMap[rumPatcher::PatchType::Ignore].push_back( rcPatchFileInfo );
  }

  SetDirty( true );

  RefreshPatchTable();
}


void PatchManager::GeneratePatchEntriesForPath( const QDir& i_rcPath,
                                                QList<rumPatcher::rumPatchFileInfo>& io_cPatchEntries,
                                                QDirIterator::IteratorFlags i_eIteratorFlags )
{
  QDirIterator iter( i_rcPath, i_eIteratorFlags );
  while( iter.hasNext() )
  {
    const QFileInfo cFileInfo{ iter.fileInfo() };
    if( cFileInfo.isFile() )
    {
      rumPatcher::rumPatchFileInfo cPatchFileInfo;
      cPatchFileInfo.m_strFile = qPrintable( i_rcPath.relativeFilePath( cFileInfo.filePath() ) );
      cPatchFileInfo.m_uiFileSize = cFileInfo.size();

      // TODO: Add some logging?
      // RUM_COUT( "===FILE: " << qPrintable( cPatchFileInfo.m_strFile ) << " ===\n" );

      ifstream cInfile;
      cInfile.open( qPrintable( cFileInfo.filePath() ), ios::binary );
      if( cInfile.is_open() )
      {
        MD5 cContext( cInfile );
        cPatchFileInfo.m_strFileCRC = cContext.hex_digest();
        cInfile.close();

        io_cPatchEntries.append( cPatchFileInfo );
      }
      else
      {
        QString strWarning{ "Failed to generate an MD5 hash for file " };
        strWarning += cPatchFileInfo.m_strFile.c_str();
        QMessageBox::warning( this, tr( "Warning" ), strWarning );
      }
    }

    iter.next();
  }
}


rumPatcher::PatchType PatchManager::GetSelectedCategoryType() const
{
  const QTableWidgetItem* pcItem
  {
    m_pcUI->tableWidget_Categories->item( m_pcUI->tableWidget_Categories->currentRow(), 0 )
  };

  return pcItem != nullptr
    ? static_cast<rumPatcher::PatchType>(
         pcItem->data( rumUtility::ToUnderlyingType( UserRole::PatchCategory ) ).toInt() )
    : rumPatcher::PatchType::Standard;
}


QString PatchManager::GetSelectedCategoryName() const
{
  const QTableWidgetItem* pcItem
  {
    m_pcUI->tableWidget_Categories->item( m_pcUI->tableWidget_Categories->currentRow(), 0 )
  };

  return pcItem ? pcItem->text() : QString();
}


QString PatchManager::GetSelectedFilePath() const
{
  const QTableWidgetItem* pcItem
  {
    m_pcUI->tableWidget_PatchFiles->item( m_pcUI->tableWidget_PatchFiles->currentRow(),
                                          rumUtility::ToUnderlyingType( FileColumns::FilePath ) )
  };

  return pcItem ? pcItem->text() : QString();
}


void PatchManager::Init()
{
  for( int32_t iIndex = 0; iIndex < rumUtility::ToUnderlyingType( rumPatcher::PatchType::NumTypes ); ++iIndex )
  {
    const auto ePatchType{ static_cast<rumPatcher::PatchType>( iIndex ) };
    AddCategory( rumPatcher::GetTypeName( ePatchType ).data() );
  }
}


void PatchManager::MoveSelectedEntryToTable( rumPatcher::PatchType i_eTargetTable )
{
  const rumPatcher::PatchType ePatchType{ GetSelectedCategoryType() };
  const std::string strFile{ qPrintable( GetSelectedFilePath() ) };

  rumPatcher::PatchMap& rcPatchMap{ rumPatcher::AccessPatchMap() };

  // Get the relevant vector from the map
  auto& rcPatchFileInfoList{ rcPatchMap[ePatchType] };
  const auto iter{ std::find_if( rcPatchFileInfoList.begin(), rcPatchFileInfoList.end(),
                                 [&strFile]( const auto& i_rcPatchFileInfo )
                                 {
                                   // Return true if the files match
                                   return i_rcPatchFileInfo == strFile;
                                 } ) };
  if( iter != rcPatchFileInfoList.end() )
  {
    if( i_eTargetTable != rumPatcher::PatchType::None )
    {
      rcPatchMap[i_eTargetTable].push_back( *iter );
    }

    rcPatchFileInfoList.erase( iter );
  }

  const auto iRow{ m_pcUI->tableWidget_PatchFiles->currentRow() };
  m_pcUI->tableWidget_PatchFiles->removeRow( iRow );

  SetDirty( true );
}


// slot
void PatchManager::on_actionClear_Patch_Table_triggered()
{
  const QString strQuestion{ "Clear all patch table entries for \"" + GetSelectedCategoryName() + "\"?" };

  // Verify clear
  if( QMessageBox::Yes == QMessageBox::question( this, "Verify", strQuestion, QMessageBox::Yes | QMessageBox::No,
                                                 QMessageBox::No ) )
  {
    // Clear the table
    m_pcUI->tableWidget_PatchFiles->clearContents();
    m_pcUI->tableWidget_PatchFiles->setRowCount( 0 );

    const rumPatcher::PatchType eCategory{ GetSelectedCategoryType() };
    rumPatcher::PatchMap& rcPatchMap{ rumPatcher::AccessPatchMap() };
    rcPatchMap[eCategory].clear();

    SetDirty( true );
  }
}


// slot
void PatchManager::on_actionGenerate_Patch_triggered()
{
  GeneratePatch();
}


// slot
void PatchManager::on_actionAdd_Entry_triggered()
{
  // Clear the filter
  m_cFilterEdit.clear();

  // Add a new row to the table widget
  const int32_t iRow{ m_pcUI->tableWidget_PatchFiles->rowCount() };

  // We don't want a change notification for newly added rows
  disconnect( m_pcUI->tableWidget_PatchFiles, &QTableWidget::itemChanged, this, &PatchManager::OnTableItemChanged );

  AddFile( GetSelectedCategoryType(), {} );

  m_pcUI->tableWidget_PatchFiles->selectRow( iRow );

  // Scroll to the newly added row
  // NOTE: The row id might have just changed because of sorting, so use currentRow
  const QTableWidgetItem* pcItem
  {
    m_pcUI->tableWidget_PatchFiles->item( m_pcUI->tableWidget_PatchFiles->currentRow(), 0 )
  };

  if( pcItem )
  {
    m_pcUI->tableWidget_PatchFiles->scrollToItem( pcItem );
  }

  SetDirty( true );
}


// slot
void PatchManager::on_actionMove_Conditional_triggered()
{
  MoveSelectedEntryToTable( rumPatcher::PatchType::Conditional );
}


// slot
void PatchManager::on_actionMove_Ignore_triggered()
{
  MoveSelectedEntryToTable( rumPatcher::PatchType::Ignore );
}


// slot
void PatchManager::on_actionMove_Remove_triggered()
{
  MoveSelectedEntryToTable( rumPatcher::PatchType::Remove );
}


// slot
void PatchManager::on_actionMove_Standard_triggered()
{
  MoveSelectedEntryToTable( rumPatcher::PatchType::Standard );
}


// slot
void PatchManager::on_actionRemove_Entry_triggered()
{
  MoveSelectedEntryToTable( rumPatcher::PatchType::None );
}


// slot
void PatchManager::on_actionSave_triggered()
{
  if( !IsDirty() )
  {
    // Nothing to update
    return;
  }

  rumPatcher::ExportCSVFiles( qPrintable( MainWindow::GetProjectDir().canonicalPath() ) );

  SetDirty( false );
}


// slot
void PatchManager::on_tableWidget_Categories_itemSelectionChanged()
{
  RefreshPatchTable();
}


// slot
void PatchManager::on_tableWidget_PatchFiles_customContextMenuRequested( const QPoint& i_rcPos )
{
  if( i_rcPos.isNull() )
  {
    return;
  }

  QMenu* pcMenu{ new QMenu( m_pcUI->tableWidget_PatchFiles ) };

  const rumPatcher::PatchType ePatchType{ GetSelectedCategoryType() };
  if( ePatchType == rumPatcher::PatchType::Standard )
  {
    pcMenu->addAction( m_pcUI->actionMove_Conditional );
    pcMenu->addAction( m_pcUI->actionMove_Remove );
    pcMenu->addAction( m_pcUI->actionMove_Ignore );
    pcMenu->addAction( m_pcUI->actionRemove_Entry );
  }
  else if( ePatchType == rumPatcher::PatchType::Conditional )
  {
    pcMenu->addAction( m_pcUI->actionMove_Standard );
    pcMenu->addAction( m_pcUI->actionMove_Remove );
    pcMenu->addAction( m_pcUI->actionMove_Ignore );
    pcMenu->addAction( m_pcUI->actionRemove_Entry );
  }
  else if( ePatchType == rumPatcher::PatchType::Remove )
  {
    pcMenu->addAction( m_pcUI->actionAdd_Entry );
    pcMenu->addAction( m_pcUI->actionMove_Ignore );
    pcMenu->addAction( m_pcUI->actionRemove_Entry );
  }
  else if( ePatchType == rumPatcher::PatchType::Ignore )
  {
    pcMenu->addAction( m_pcUI->actionAdd_Entry );
    pcMenu->addAction( m_pcUI->actionMove_Remove );
    pcMenu->addAction( m_pcUI->actionRemove_Entry );
  }

  pcMenu->exec( m_pcUI->tableWidget_PatchFiles->mapToGlobal( i_rcPos ) );
}


// slot
void PatchManager::on_tableWidget_PatchFiles_itemSelectionChanged()
{
  m_pcUI->actionRemove_Entry->setEnabled( true );
}


// slot
void PatchManager::onFilterChanged()
{
  const QString& strText{ m_cFilterEdit.text() };

  // A bug in the signal for editingFinished() causes the function to be called twice. To cut down on redundant work,
  // the filter string is stored here. If we're already filtering on this string, we early out.
  if( strText.compare( m_strActiveFilterText, Qt::CaseInsensitive ) == 0 )
  {
    return;
  }

  m_strActiveFilterText = strText;

  const int32_t rowCount{ m_pcUI->tableWidget_PatchFiles->rowCount() };

  if( strText.isEmpty() )
  {
    // Show all items
    for( int32_t i{ 0 }; i < rowCount; ++i )
    {
      m_pcUI->tableWidget_PatchFiles->showRow( i );
    }
  }
  else
  {
    QList<int32_t> cRowList;
    const QList<QTableWidgetItem*> pcItemList{ m_pcUI->tableWidget_PatchFiles->findItems( strText,
                                                                                          Qt::MatchContains ) };
    for( int32_t i{ 0 }; i < pcItemList.count(); ++i )
    {
      cRowList << pcItemList.at( i )->row();
    }

    // Hide any item matching the filter
    for( int32_t i{ 0 }; i < rowCount; ++i )
    {
      if( !cRowList.contains( i ) )
      {
        m_pcUI->tableWidget_PatchFiles->hideRow( i );
      }
    }
  }
}


// slot
void PatchManager::OnTableItemChanged( QTableWidgetItem* i_pcItem )
{
  const rumPatcher::PatchType eCategory{ GetSelectedCategoryType() };

  rumPatcher::rumPatchFileInfo cPatchFileInfo;
  cPatchFileInfo.m_strFile = qPrintable( i_pcItem->text() );

  rumPatcher::PatchMap& rcPatchMap{ rumPatcher::AccessPatchMap() };
  rcPatchMap[eCategory].push_back( std::move( cPatchFileInfo ) );

  if( !m_strLastEdit.empty() )
  {
    // Get the relevant vector from the map
    auto& rcPatchFileInfoList{ rcPatchMap[eCategory] };
    rcPatchFileInfoList.erase( std::remove_if( rcPatchFileInfoList.begin(), rcPatchFileInfoList.end(),
                                               [this](const auto& i_rcPatchFileInfo)
                                               {
                                                 // Return true if the files match
                                                 return i_rcPatchFileInfo == m_strLastEdit;
                                               }),
                                               rcPatchFileInfoList.end() );

    m_strLastEdit.clear();
  }

  SetDirty( true );
}


// slot
void PatchManager::on_tableWidget_PatchFiles_cellActivated( int32_t i_iRow, int32_t i_iCol )
{
  m_strLastEdit = qPrintable( GetSelectedFilePath() );
}


void PatchManager::RefreshPatchTable()
{
  // Clear the table
  m_pcUI->tableWidget_PatchFiles->clearContents();
  m_pcUI->tableWidget_PatchFiles->setRowCount( 0 );

  m_pcUI->actionRemove_Entry->setEnabled( false );

  m_cFilterLabel.setEnabled( true );
  m_cFilterEdit.setEnabled( true );

  const rumPatcher::PatchType eCategory{ GetSelectedCategoryType() };
  const rumPatcher::PatchMap& rcPatchMap{ rumPatcher::AccessPatchMap() };

  const auto iter{ rcPatchMap.find( eCategory ) };
  if( iter != rcPatchMap.end() )
  {
    const auto& rcOutputList{ iter->second };

    for( const auto& rcPatchFileInfo : rcOutputList )
    {
      AddFile( eCategory, rcPatchFileInfo );
    }
  }

  m_pcUI->tableWidget_PatchFiles->resizeColumnsToContents();

  if( rumPatcher::PatchType::Standard == eCategory )
  {
    m_pcUI->tableWidget_PatchFiles->showColumn( rumUtility::ToUnderlyingType( FileColumns::CRC ) );
    m_pcUI->tableWidget_PatchFiles->showColumn( rumUtility::ToUnderlyingType( FileColumns::Size ) );

    m_pcUI->actionAdd_Entry->setEnabled( false );
  }
  else
  {
    m_pcUI->tableWidget_PatchFiles->hideColumn( rumUtility::ToUnderlyingType( FileColumns::CRC ) );
    m_pcUI->tableWidget_PatchFiles->hideColumn( rumUtility::ToUnderlyingType( FileColumns::Size ) );

    m_pcUI->actionAdd_Entry->setEnabled( true );
  }
}


void PatchManager::SetDirty( bool i_bDirty )
{
  if( m_bDirty != i_bDirty )
  {
    m_bDirty = i_bDirty;
    m_pcUI->actionSave->setEnabled( i_bDirty );
  }
}
