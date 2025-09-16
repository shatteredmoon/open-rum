#include <datatablemanager.h>
#include <ui_datatablemanager.h>

#include <mainwindow.h>

#include <QCloseEvent>
#include <QComboBox>
#include <QInputDialog>
#include <QItemSelection>
#include <QLineEdit>
#include <QMessageBox>

#include <combopicker.h>
#include <stringtokenpicker.h>

#include <e_utility.h>
#include <u_strings.h>


enum TableColumns
{
  D_COL_NAME    = 0,
  D_COL_SERVICE = 1
};


DataTableManager::DataTableManager( QWidget* i_pcParent )
  : QMainWindow( i_pcParent )
  , m_pcFilterAction( nullptr )
  , m_pcUI( new Ui::DataTableManager )
{
  m_pcUI->setupUi( this );
  setCentralWidget( m_pcUI->splitter );

  const QIcon cDataTableIcon( ":/ui/resources/datatable.png" );
  setWindowIcon( cDataTableIcon );

  setMinimumWidth( 1024 );
  setMinimumHeight( 768 );

  const QIcon cSaveIcon( ":/ui/resources/save.png" );
  m_pcUI->actionSave->setIcon( cSaveIcon );

  m_pcUI->toolBar->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );

  // Data table -------------------------------------------------------------

  InitDataTables();

  // Single selections only
  m_pcUI->tableWidget->setSelectionBehavior( QAbstractItemView::SelectRows );
  m_pcUI->tableWidget->setSelectionMode( QAbstractItemView::SingleSelection );

  m_pcUI->tableWidget->setContextMenuPolicy( Qt::CustomContextMenu );

  m_pcUI->actionNew_Data_Table->setEnabled( true );

  const QIcon cAddIcon( ":/ui/resources/add.png" );
  m_pcUI->actionNew_Data_Table->setIcon( cAddIcon );

  const QIcon cRemoveIcon( ":/ui/resources/remove.png" );
  m_pcUI->actionRemove_Data_Table->setIcon( cRemoveIcon );

  // Data entry table -------------------------------------------------------

  m_pcUI->actionNew_Data_Column->setEnabled( false );
  m_pcUI->actionNew_Data_Column->setIcon( cAddIcon );

  m_pcUI->actionRemove_Data_Column->setEnabled( false );
  m_pcUI->actionRemove_Data_Column->setIcon( cRemoveIcon );

  m_pcUI->actionNew_Data_Entry->setEnabled( false );
  m_pcUI->actionNew_Data_Entry->setIcon( cAddIcon );

  m_pcUI->actionRemove_Data_Entry->setEnabled( false );
  m_pcUI->actionRemove_Data_Entry->setIcon( cRemoveIcon );

  // Add a line edit on the string's toolbar for filtering
  m_cFilterLabel.setText( "Filter: " );
  m_pcUI->toolBar3->addWidget( &m_cFilterLabel );
  m_pcFilterAction = m_pcUI->toolBar3->addWidget( &m_cFilterEdit );

  m_pcUI->tableWidget_3->setContextMenuPolicy( Qt::CustomContextMenu );

  // Special context menu for the header
  auto pcHeaderWidget{ m_pcUI->tableWidget_3->horizontalHeader() };
  pcHeaderWidget->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( pcHeaderWidget, &QTableWidget::customContextMenuRequested,
           this, &DataTableManager::on_headerWidget_customContextMenuRequested );

  m_cFilterLabel.setEnabled( false );
  m_cFilterEdit.setEnabled( false );

  connect( &m_cFilterEdit, SIGNAL( editingFinished() ), this, SLOT( onFilterChanged() ) );

  connect( m_pcUI->tableWidget_3->selectionModel(),
           SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
           this, SLOT( selectionChanged_DataEntry( const QItemSelection&, const QItemSelection& ) ) );

  // Final setup ------------------------------------------------------------

  // Determine default sizes
  QHeaderView* pcHorizontalHeader{ m_pcUI->tableWidget->horizontalHeader() };
  const int32_t iTableSize{ std::max( m_pcUI->tableWidget->minimumWidth(), pcHorizontalHeader->length() ) };
  const int32_t iViewSize{ 800 - iTableSize };

  pcHorizontalHeader->setStretchLastSection( true );

  QList<int32_t> cSplitterList;
  cSplitterList.append( iTableSize );
  cSplitterList.append( iViewSize );
  m_pcUI->splitter->setSizes( cSplitterList );

  m_pcUI->actionSave->setEnabled( false );
}


DataTableManager::~DataTableManager()
{
  delete m_pcUI;
}


void DataTableManager::AddDataColumn( rumDataTable& i_rcDataTable, uint32_t i_uiColumn )
{
  if( m_pcUI->tableWidget_3->columnCount() < static_cast<int32_t>( i_uiColumn + 1 ) )
  {
    m_pcUI->tableWidget_3->setColumnCount( static_cast<int32_t>( i_uiColumn + 1 ) );
  }

  const auto& rcColumnData{ i_rcDataTable.GetColumnData( i_uiColumn ) };

  const int32_t uiRowCount{ m_pcUI->tableWidget_3->rowCount() };
  for( int32_t i = 0; i < uiRowCount; ++i )
  {
    auto& sqValue{ rcColumnData.m_cValueVector[i] };
    const QString strValue( rumScript::ToString( sqValue ).c_str() );

    QTableWidgetItem* pcCell{ new QTableWidgetItem( strValue ) };
    m_pcUI->tableWidget_3->setItem( i, i_uiColumn, pcCell );
    rumControlUtils::UpdateEditableState( *pcCell, rcColumnData.m_eValueType );
  }

  RefreshHeaderLabels();
}


void DataTableManager::AddDataEntry( rumDataTable& i_rcDataTable, int32_t i_iRow )
{
  if( m_pcUI->tableWidget_3->rowCount() < i_iRow + 1 )
  {
    m_pcUI->tableWidget_3->setRowCount( i_iRow + 1 );
  }

  const uint32_t uiNumColumns{ i_rcDataTable.GetNumColumns() };
  for( uint32_t i = 0; i < uiNumColumns; ++i )
  {
    const auto& rcColumnData{ i_rcDataTable.GetColumnData( i ) };

    auto& sqValue{ rcColumnData.m_cValueVector[i_iRow] };
    QString strValue( rumScript::ToString( sqValue ).c_str() );

    QTableWidgetItem* pcCell{ new QTableWidgetItem( strValue ) };
    m_pcUI->tableWidget_3->setItem( i_iRow, i, pcCell );
    rumControlUtils::UpdateEditableState( *pcCell, rcColumnData.m_eValueType );
  }
}


void DataTableManager::AddDataTable( const rumDataTable& i_rcDataTable ) const
{
  const int32_t iRow{ m_pcUI->tableWidget->rowCount() };
  m_pcUI->tableWidget->setRowCount( iRow + 1 );

  // Name
  QTableWidgetItem* pcCell{ new QTableWidgetItem };
  pcCell->setData( Qt::DisplayRole, i_rcDataTable.GetName().c_str() );
  pcCell->setData( Qt::UserRole, QVariant::fromValue( (int32_t)i_rcDataTable.GetID() ) );
  m_pcUI->tableWidget->setItem( iRow, D_COL_NAME, pcCell );

  // Service type
  QString strServiceType;
  switch( i_rcDataTable.GetServiceType() )
  {
    case Shared_ServiceType: strServiceType = "Shared"; break;
    case Server_ServiceType: strServiceType = "Server"; break;
    case Client_ServiceType: strServiceType = "Client"; break;
  }
  pcCell = new QTableWidgetItem();
  pcCell->setData( Qt::DisplayRole, strServiceType );
  pcCell->setData( Qt::UserRole, QVariant::fromValue( (int32_t)i_rcDataTable.GetServiceType() ) );
  pcCell->setToolTip( rumStringUtils::ToString( i_rcDataTable.GetServiceType() ) );
  m_pcUI->tableWidget->setItem( iRow, D_COL_SERVICE, pcCell );
  pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
}


void DataTableManager::closeEvent( QCloseEvent* i_pcEvent )
{
  // Warn if dirty
  if( IsDirty() )
  {
    if( QMessageBox::Yes != QMessageBox::question( this, "Close Confirmation?",
                                                   "There are unsaved changes. Are you sure you want to exit?",
                                                   QMessageBox::Yes | QMessageBox::No ) )
    {
      i_pcEvent->ignore();
      return;
    }
  }

  i_pcEvent->accept();
  emit closed();
}


rumAssetID DataTableManager::GetSelectedDataTableID() const
{
  // Get the database type: Server, Client, or Shared
  const int32_t iRow{ m_pcUI->tableWidget->currentRow() };
  const QTableWidgetItem* pcItem{ m_pcUI->tableWidget->item( iRow, D_COL_NAME ) };
  return pcItem ? (rumDataTable::DataTableID)pcItem->data( Qt::UserRole ).toInt() : rumDataTable::INVALID_DATATABLE_ID;
}


QString DataTableManager::GetTableName( int32_t i_iRow ) const
{
  QTableWidgetItem* pcItem{ nullptr };

  if( i_iRow <= m_pcUI->tableWidget->rowCount() )
  {
    pcItem = m_pcUI->tableWidget->item( i_iRow, D_COL_NAME );
  }

  return pcItem ? pcItem->text() : QString();
}


void DataTableManager::InitDataTables()
{
  disconnect( m_pcUI->tableWidget->selectionModel(),
              SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
              this, SLOT( selectionChanged_DataTable( const QItemSelection&, const QItemSelection& ) ) );

  disconnect( m_pcUI->tableWidget, &QTableWidget::itemChanged, this, &DataTableManager::OnDataTableChanged );

  QStringList cLabelList;
  cLabelList << "Data Table" << "Service";

  m_pcUI->tableWidget->setMinimumWidth( 200 );
  m_pcUI->tableWidget->setColumnCount( cLabelList.size() );
  m_pcUI->tableWidget->setRowCount( 0 );
  m_pcUI->tableWidget->setHorizontalHeaderLabels( cLabelList );

  // Fetch all loaded data tables
  const std::vector<rumDataTable::DataTableID> cDataTableIDVector{ rumDataTable::GetDataTableIDs() };
  for( const auto& iter : cDataTableIDVector )
  {
    const rumDataTable::DataTableID uiDataTableID{ iter };
    const rumDataTable& rcDataTable{ rumDataTable::GetDataTable( iter ) };
    AddDataTable( rcDataTable );
  }

  m_pcUI->tableWidget->resizeColumnsToContents();

  connect( m_pcUI->tableWidget->selectionModel(),
           SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
           this, SLOT( selectionChanged_DataTable( const QItemSelection&, const QItemSelection& ) ) );

  connect( m_pcUI->tableWidget, &QTableWidget::itemChanged, this, &DataTableManager::OnDataTableChanged );

  m_pcUI->actionRemove_Data_Table->setEnabled( false );
}


bool DataTableManager::IsTableNameUnique( const QString& i_strName ) const
{
  bool bUnique{ true };

  const int32_t iSelectedRow{ m_pcUI->tableWidget->currentRow() };

  // Visit each table row and compare names
  const int32_t iNumRows{ m_pcUI->tableWidget->rowCount() };
  for( int32_t iRow{ 0 }; iRow < iNumRows && bUnique; ++iRow )
  {
    if( iRow != iSelectedRow )
    {
      const QString& strName{ GetTableName( iRow ) };
      if( strName.compare( i_strName, Qt::CaseInsensitive ) == 0 )
      {
        bUnique = false;
      }
    }
  }

  return bUnique;
}


// slot
void DataTableManager::OnDataEntryChanged( QTableWidgetItem* i_pcItem )
{
  const int32_t iCol{ i_pcItem->column() };
  const int32_t iRow{ i_pcItem->row() };

  const rumDataTable::DataTableID uiDataTableID{ GetSelectedDataTableID() };
  rumDataTable& rcDataTable{ rumDataTable::GetDataTable( uiDataTableID ) };

  auto& rcColumnData{ rcDataTable.AccessColumnData( iCol ) };

  if( rcColumnData.m_cValueVector.size() > iRow )
  {
    Sqrat::Object sqValue;

    if( rcColumnData.m_eValueType >= PropertyValueType::FirstAssetRef &&
        rcColumnData.m_eValueType <= PropertyValueType::LastAssetRef )
    {
      const rumAssetID eAssetID{ static_cast<rumAssetID>( i_pcItem->data( Qt::UserRole ).toInt() ) };
      rumScript::SetValue( sqValue, eAssetID );
    }
    else if( PropertyValueType::StringToken == rcColumnData.m_eValueType )
    {
      const rumTokenID uiTokenID{ static_cast<rumTokenID>( i_pcItem->data( Qt::UserRole ).toInt() ) };
      rumScript::SetValue( sqValue, uiTokenID );
    }
    else if( PropertyValueType::String == rcColumnData.m_eValueType )
    {
      rumScript::SetValue( sqValue, i_pcItem->text() );
    }
    else if( PropertyValueType::Integer == rcColumnData.m_eValueType )
    {
      if( i_pcItem->data( Qt::UserRole ).isValid() )
      {
        rumScript::SetValue( sqValue, i_pcItem->data( Qt::UserRole ).toInt() );
      }
      else
      {
        const auto iValue{ rumScript::ToValue( rcColumnData.m_eValueType, qPrintable( i_pcItem->text() ) ) };
        rumScript::SetValue( sqValue, iValue );
      }
    }
    else
    {
      sqValue = rumScript::ToValue( rcColumnData.m_eValueType, qPrintable( i_pcItem->text() ) );
    }


    rcColumnData.m_cValueVector[iRow] = sqValue;
  }

  SetDirty( true );
}


// slot
void DataTableManager::OnDataEntryComboChanged( const QString& i_strText )
{
  const int32_t iRow{ m_pcUI->tableWidget_3->currentRow() };
  const int32_t iCol{ m_pcUI->tableWidget_3->currentColumn() };

  QVariant cVariant;

  // Access user data if it is available
  const QWidget* pcWidget{ m_pcUI->tableWidget_3->cellWidget( iRow, iCol ) };
  if( pcWidget )
  {
    const QComboBox* pcComboBox{ qobject_cast<const QComboBox*>( pcWidget ) };
    if( pcComboBox )
    {
      cVariant = pcComboBox->currentData();
    }

    m_pcUI->tableWidget_3->removeCellWidget( iRow, iCol );
  }

  QTableWidgetItem* pcCell{ new QTableWidgetItem() };
  pcCell->setData( Qt::DisplayRole, i_strText );

  if( cVariant.isValid() )
  {
    pcCell->setData( Qt::UserRole, cVariant );
  }

  m_pcUI->tableWidget_3->setItem( iRow, iCol, pcCell );
  pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
}


// slot
void DataTableManager::OnDataEntryStringTokenChanged( rumTokenID i_eTokenID )
{
  const int32_t iRow{ m_pcUI->tableWidget_3->currentRow() };
  const int32_t iCol{ m_pcUI->tableWidget_3->currentColumn() };

  const auto& strToken{ rumStringTable::GetTokenName( i_eTokenID ) };

  QTableWidgetItem* pcCell{ new QTableWidgetItem() };
  pcCell->setData( Qt::DisplayRole, strToken.c_str() );

  QVariant cVariant{ i_eTokenID };
  if( cVariant.isValid() )
  {
    pcCell->setData( Qt::UserRole, cVariant );
  }

  m_pcUI->tableWidget_3->setItem( iRow, iCol, pcCell );
  pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
}


// slot
void DataTableManager::OnDataTableChanged( QTableWidgetItem* i_pcItem )
{
  const int32_t iCol{ i_pcItem->column() };

  const rumDataTable::DataTableID uiDataTableID{ GetSelectedDataTableID() };
  rumDataTable& rcDataTable{ rumDataTable::GetDataTable( uiDataTableID ) };

  if( iCol == D_COL_NAME )
  {
    rcDataTable.SetName( qPrintable( i_pcItem->text() ) );
  }
  else if( iCol == D_COL_SERVICE )
  {
    const ServiceType eServiceType{ (ServiceType)i_pcItem->data( Qt::UserRole ).toInt() };
    rcDataTable.SetServiceType( eServiceType );
  }

  SetDirty( true );
}


// slot
void DataTableManager::OnDataTableComboChanged( const QString& i_strText )
{
  const int32_t iRow{ m_pcUI->tableWidget->currentRow() };
  const int32_t iCol{ m_pcUI->tableWidget->currentColumn() };

  if( D_COL_SERVICE == iCol )
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


// slot
void DataTableManager::on_actionChange_Data_Column_Type_triggered()
{
  ComboPicker::ComboPickerVector cComboVector
  {
    { "Integer",     rumUtility::ToUnderlyingType( PropertyValueType::Integer ) },
    { "Bool",        rumUtility::ToUnderlyingType( PropertyValueType::Bool ) },
    { "Float",       rumUtility::ToUnderlyingType( PropertyValueType::Float ) },
    { "String",      rumUtility::ToUnderlyingType( PropertyValueType::String ) },
    { "Bitfield",    rumUtility::ToUnderlyingType( PropertyValueType::Bitfield ) },
    { "Asset",       rumUtility::ToUnderlyingType( PropertyValueType::AssetRef ) },
    { "Creature",    rumUtility::ToUnderlyingType( PropertyValueType::CreatureRef ) },
    { "Portal",      rumUtility::ToUnderlyingType( PropertyValueType::PortalRef ) },
    { "Widget",      rumUtility::ToUnderlyingType( PropertyValueType::WidgetRef ) },
    { "Broadcast",   rumUtility::ToUnderlyingType( PropertyValueType::BroadcastRef ) },
    { "Tile",        rumUtility::ToUnderlyingType( PropertyValueType::TileRef ) },
    { "Map",         rumUtility::ToUnderlyingType( PropertyValueType::MapRef ) },
    { "Graphic",     rumUtility::ToUnderlyingType( PropertyValueType::GraphicRef ) },
    { "Sound",       rumUtility::ToUnderlyingType( PropertyValueType::SoundRef ) },
    { "Property",    rumUtility::ToUnderlyingType( PropertyValueType::PropertyRef ) },
    { "Inventory",   rumUtility::ToUnderlyingType( PropertyValueType::InventoryRef ) },
    { "Custom",      rumUtility::ToUnderlyingType( PropertyValueType::CustomRef ) },
    { "StringToken", rumUtility::ToUnderlyingType( PropertyValueType::StringToken ) },
  };

  ComboPicker* pcDialog{ new ComboPicker( "Select type:", cComboVector, this ) };

  connect( pcDialog, SIGNAL( ComboPickerItemSelected( int32_t ) ),
           this, SLOT( OnComboPickerItemSelected( int32_t ) ) );

  pcDialog->setModal( true );
  pcDialog->show();
}


// slot
void DataTableManager::on_actionNew_Data_Column_triggered()
{
  const rumDataTable::DataTableID uiTableID{ GetSelectedDataTableID() };
  rumDataTable& rcDataTable{ rumDataTable::GetDataTable( uiTableID ) };

  // Determine a new default column name based on the current number of columns
  const uint32_t uiNumColumns{ rcDataTable.GetNumColumns() };
  std::string strName{ "Data" };
  strName += rumStringUtils::ToString( uiNumColumns );

  // Add the new column
  const uint32_t uiIndex{ rcDataTable.AddDataColumn( strName, PropertyValueType::Integer ) };
  AddDataColumn( rcDataTable, uiIndex );
}


// slot
void DataTableManager::on_actionNew_Data_Entry_triggered()
{
  // Clear the filter
  m_cFilterEdit.clear();

  UpdateFilter();

  // Disable sorting
  m_pcUI->tableWidget_3->setSortingEnabled( false );

  const rumAssetID uiDataTableID{ GetSelectedDataTableID() };
  rumDataTable& rcDataTable{ rumDataTable::GetDataTable( uiDataTableID ) };

  disconnect( m_pcUI->tableWidget_3, &QTableWidget::itemChanged, this, &DataTableManager::OnDataEntryChanged );

  rcDataTable.AddDataEntry();

  // Add a new row to the table widget
  const int32_t iRow{ m_pcUI->tableWidget_3->rowCount() };

  //AddDataEntry( rcDataTable, iRow );
  connect( m_pcUI->tableWidget_3, &QTableWidget::itemChanged, this, &DataTableManager::OnDataEntryChanged );

  m_pcUI->tableWidget_3->selectRow( iRow );

  // Re-enable sorting
  //m_pcUI->tableWidget_3->setSortingEnabled( true );

  // Scroll to the newly added row
  // NOTE: The row id might have just changed because of sorting, so use currentRow
  const QTableWidgetItem* pcItem{ m_pcUI->tableWidget_3->item( m_pcUI->tableWidget_3->currentRow(), D_COL_NAME ) };
  if( pcItem )
  {
    m_pcUI->tableWidget_3->scrollToItem( pcItem );
  }

  SetDirty( true );

  RefreshDataTable();
}


// slot
void DataTableManager::on_actionNew_Data_Table_triggered()
{
  const rumDataTable::DataTableID uiDataTableID{ rumDataTable::GetNextDataTableID() };
  std::string strName{ "table" };
  strName += rumStringUtils::ToString( uiDataTableID );

  if( rumDataTable::AddDataTable( uiDataTableID, strName, ServiceType::Shared_ServiceType, true /* new table */) )
  {
    InitDataTables();
    RefreshDataTable();
  }
}


void DataTableManager::on_actionRemove_Data_Column_triggered()
{
  const QString strQuestion{ "Are you sure you want to remove this column?" };

  // Verify delete
  if( QMessageBox::Yes == QMessageBox::question( this, "Verify", strQuestion,
                                                 QMessageBox::Yes | QMessageBox::No,
                                                 QMessageBox::No ) )
  {
    const int32_t iCol{ m_pcUI->tableWidget_3->currentColumn() };

    rumDataTable& rcDataTable{ rumDataTable::GetDataTable( GetSelectedDataTableID() ) };
    rcDataTable.RemoveDataColumn( iCol );

    RefreshDataTable();
    SetDirty( true );
  }
}


void DataTableManager::on_actionRemove_Data_Entry_triggered()
{
  const QString strQuestion{ "Are you sure you want to remove this row?" };

  // Verify delete
  if( QMessageBox::Yes == QMessageBox::question( this, "Verify", strQuestion,
                                                 QMessageBox::Yes | QMessageBox::No,
                                                 QMessageBox::No ) )
  {
    const int32_t iRow{ m_pcUI->tableWidget_3->currentRow() };

    rumDataTable& rcDataTable{ rumDataTable::GetDataTable( GetSelectedDataTableID() ) };
    rcDataTable.RemoveDataEntry( iRow );

    SetDirty( true );

    // Remove the table widget entry
    m_pcUI->tableWidget_3->removeRow( iRow );
  }
}


// slot
void DataTableManager::on_actionRemove_Data_Table_triggered()
{
  const rumAssetID uiDataTableID{ GetSelectedDataTableID() };
  rumDataTable& rcDataTable{ rumDataTable::GetDataTable( uiDataTableID ) };

  QString strQuestion{ "Are you sure you want to remove data table " };
  strQuestion += rcDataTable.GetName().c_str();
  strQuestion += "?\n\nWARNING: This will remove all data associated with this table!\n"
                 "Be sure to backup your files before removing a data table.";

  // Verify delete
  if( QMessageBox::Yes == QMessageBox::question( this, "Verify", strQuestion,
                                                 QMessageBox::Yes | QMessageBox::No,
                                                 QMessageBox::No ) )
  {
    if( rcDataTable.RemoveDataTable( uiDataTableID ) )
    {
      // Remove the table widget row
      const int32_t iRow{ m_pcUI->tableWidget->currentRow() };
      m_pcUI->tableWidget->removeRow( iRow );
    }
  }
}


// slot
void DataTableManager::on_actionRename_Data_Column_triggered()
{
  rumAssert( m_pcUI->tableWidget_3->columnCount() > 0 );

  int32_t iCol{ m_pcUI->tableWidget_3->currentColumn() };
  if( iCol < 0 )
  {
    iCol = 0;
  }

  const QString strCurrentName{ m_pcUI->tableWidget_3->horizontalHeaderItem( iCol )->text() };

  bool bOK{ false };
  const QString strNewName{ QInputDialog::getText( this,
                                                   "Rename Column ",
                                                   "Rename column '" + strCurrentName + "' to:",
                                                   QLineEdit::Normal,
                                                   strCurrentName,
                                                   &bOK ) };
  if( bOK && !strNewName.isEmpty() )
  {
    m_pcUI->tableWidget_3->setHorizontalHeaderItem( iCol, new QTableWidgetItem( strNewName ) );

    const rumDataTable::DataTableID uiDataTableID{ GetSelectedDataTableID() };
    rumDataTable& rcDataTable{ rumDataTable::GetDataTable( uiDataTableID ) };

    auto& rcColumnData{ rcDataTable.AccessColumnData( iCol ) };
    rcColumnData.m_strName = qPrintable( strNewName );

    SetDirty( true );
  }
}


// slot
void DataTableManager::on_actionSave_triggered()
{
  SaveAllTables();
}


void DataTableManager::on_headerWidget_customContextMenuRequested( const QPoint& i_rcPos )
{
  const QTableWidgetItem* pcItem{ m_pcUI->tableWidget_3->itemAt( i_rcPos ) };
  if( pcItem != nullptr )
  {
    m_pcUI->tableWidget_3->selectColumn( pcItem->column() );
  }

  QMenu* pcMenu{ new QMenu( this ) };
  pcMenu->addAction( m_pcUI->actionChange_Data_Column_Type );
  pcMenu->addAction( m_pcUI->actionRename_Data_Column );

  pcMenu->popup( m_pcUI->tableWidget_3->horizontalHeader()->mapToGlobal( i_rcPos ) );
}


void DataTableManager::on_tableWidget_cellDoubleClicked( int32_t i_iRow, int32_t i_iCol )
{
  if( D_COL_NAME == i_iCol )
  {
    // Only allow numbers, letters, dash, and underscore up to 32 characters
    const QRegExp cRegExp( "[\\w-]{1,32}" );
    const QRegExpValidator* pcValidator{ new QRegExpValidator( cRegExp, this ) };

    QLineEdit* pcLineEdit{ new QLineEdit };
    pcLineEdit->setText( GetTableName( i_iRow ) );
    pcLineEdit->setValidator( pcValidator );

    connect( pcLineEdit, SIGNAL( editingFinished() ), this, SLOT( TableLineEditFinished() ) );

    m_pcUI->tableWidget->setCellWidget( i_iRow, i_iCol, pcLineEdit );
  }
  else if( D_COL_SERVICE == i_iCol )
  {
    QComboBox* pcCombo{ new QComboBox };
    pcCombo->addItem( "Shared", ServiceType::Shared_ServiceType );
    pcCombo->addItem( "Server", ServiceType::Server_ServiceType );
    pcCombo->addItem( "Client", ServiceType::Client_ServiceType );

    const rumAssetID uiDataTableID{ GetSelectedDataTableID() };
    const rumDataTable& rcDataTable{ rumDataTable::GetDataTable( uiDataTableID ) };

    const int32_t iIndex{ pcCombo->findData( rcDataTable.GetServiceType() ) };
    pcCombo->setCurrentIndex( iIndex );
    pcCombo->view()->setMinimumWidth( pcCombo->minimumSizeHint().width() );

    connect( pcCombo, SIGNAL( currentIndexChanged( const QString& ) ),
             this, SLOT( OnDataTableComboChanged( const QString& ) ) );

    m_pcUI->tableWidget->setCellWidget( i_iRow, i_iCol, pcCombo );
  }
}


void DataTableManager::on_tableWidget_customContextMenuRequested( const QPoint& i_rcPos )
{
  if( i_rcPos.isNull() )
  {
    return;
  }

  QMenu* pcMenu{ new QMenu( m_pcUI->tableWidget ) };
  pcMenu->addAction( m_pcUI->actionNew_Data_Table );
  pcMenu->addAction( m_pcUI->actionRemove_Data_Table );

  pcMenu->exec( m_pcUI->tableWidget->mapToGlobal( i_rcPos ) );
}


void DataTableManager::on_tableWidget_3_cellDoubleClicked( int32_t i_iRow, int32_t i_iCol )
{
  const rumDataTable::DataTableID uiDataTableID{ GetSelectedDataTableID() };
  rumDataTable& rcDataTable{ rumDataTable::GetDataTable( uiDataTableID ) };

  const auto& rcColumnData{ rcDataTable.GetColumnData( i_iCol ) };
  const QTableWidgetItem* pcCell{ m_pcUI->tableWidget_3->item( i_iRow, i_iCol ) };

  QComboBox* pcCombo{ nullptr };
  bool bUseCombo{ false };

  if( rcColumnData.m_eValueType >= PropertyValueType::FirstAssetRef &&
      rcColumnData.m_eValueType <= PropertyValueType::LastAssetRef )
  {
    // Provide each asset as an entry in the combo
    pcCombo = new QComboBox;
    rumControlUtils::BuildComboFromAssetType( *pcCombo, rcColumnData.m_eValueType );

    // Select the index in the combo box that matches the selection in the table prior to this edit
    const int32_t iIndex{ pcCombo->findText( pcCell->text() ) };
    if( iIndex != -1 )
    {
      pcCombo->setCurrentIndex( iIndex );
    }
  }
  else if( rcColumnData.m_eValueType == PropertyValueType::StringToken )
  {
    auto sqObject{ rcColumnData.m_cValueVector.at( i_iRow ) };
    const rumTokenID eTokenID{ static_cast<rumTokenID>( sqObject.Cast<rumTokenID>() ) };

    // Use the string token picker dialog for the new selection
    StringTokenPicker* pcDialog{ new StringTokenPicker( eTokenID, this ) };

    connect( pcDialog, SIGNAL( NewStringTokenSelected( rumTokenID ) ),
             this, SLOT( OnDataEntryStringTokenChanged( rumTokenID ) ) );

    pcDialog->setModal( true );
    pcDialog->show();
  }
  else if( rcColumnData.m_eValueType == PropertyValueType::Bool )
  {
    // Simple true/false combo
    pcCombo = new QComboBox;
    pcCombo->addItem( "false", false );
    pcCombo->addItem( "true", true );
    pcCombo->view()->setMinimumWidth( pcCombo->minimumSizeHint().width() );

    // Select the index in the combo box that matches the selection in the table prior to this edit
    const QVariant cVariant{ pcCell->data( Qt::DisplayRole ) };
    const int32_t iIndex{ pcCombo->findData( cVariant ) };
    if( iIndex != -1 )
    {
      pcCombo->setCurrentIndex( iIndex );
    }
  }
  else if( rcColumnData.m_eValueType == PropertyValueType::Integer )
  {
    const QVariant cVariant{ pcCell->data( Qt::UserRole ) };
    if( cVariant.toBool() )
    {
      // Provide the enum values in a combo
      pcCombo = new QComboBox;
      rumControlUtils::BuildComboFromEnumType( *pcCombo, rcColumnData.m_strName );

      // Select the index in the combo box that matches the selection in the table prior to this edit
      const int32_t iIndex{ pcCombo->findText( pcCell->text() ) };
      if( iIndex != -1 )
      {
        pcCombo->setCurrentIndex( iIndex );
      }
    }
  }

  if( pcCombo )
  {
    connect( pcCombo, SIGNAL( currentIndexChanged( const QString& ) ),
             this, SLOT( OnDataEntryComboChanged( const QString& ) ) );

    m_pcUI->tableWidget_3->setCellWidget( i_iRow, i_iCol, pcCombo );
  }
}


void DataTableManager::on_tableWidget_3_customContextMenuRequested( const QPoint& i_rcPos )
{
  if( i_rcPos.isNull() )
  {
    return;
  }

  const QTableWidgetItem* pcItem{ m_pcUI->tableWidget_3->itemAt( i_rcPos ) };
  if( pcItem != nullptr )
  {
    m_pcUI->tableWidget_3->selectColumn( pcItem->column() );
  }

  QMenu* pcMenu{ new QMenu( m_pcUI->tableWidget_3 ) };
  pcMenu->addAction( m_pcUI->actionNew_Data_Entry );
  pcMenu->addAction( m_pcUI->actionRemove_Data_Entry );
  pcMenu->addAction( m_pcUI->actionNew_Data_Column );
  pcMenu->addAction( m_pcUI->actionRemove_Data_Column );

  pcMenu->exec( m_pcUI->tableWidget_3->mapToGlobal( i_rcPos ) );
}


// slot
void DataTableManager::OnComboPickerItemSelected( int32_t i_iItem )
{
  const rumDataTable::DataTableID uiDataTableID{ GetSelectedDataTableID() };
  rumDataTable& rcDataTable{ rumDataTable::GetDataTable( uiDataTableID ) };
  if( rcDataTable.GetNumEntries() == 0 )
  {
    return;
  }

  const int32_t iCol{ m_pcUI->tableWidget_3->currentColumn() };

  auto& rcColumnData{ rcDataTable.AccessColumnData( iCol ) };

  const PropertyValueType eOldValueType{ rcColumnData.m_eValueType };
  rcColumnData.m_eValueType = static_cast<PropertyValueType>( i_iItem );

  // Convert existing values to their new type
  for( auto& sqValue : rcColumnData.m_cValueVector )
  {
    rumValueUtils::ConvertPropertyValue( sqValue, eOldValueType, rcColumnData.m_eValueType );
  }

  RefreshDataTable();
  SetDirty( true );
}


// slot
void DataTableManager::onFilterChanged()
{
  UpdateFilter();
}


void DataTableManager::RefreshDataTable()
{
  m_pcUI->tableWidget_3->setEnabled( false );

  const rumDataTable::DataTableID uiDataTableID{ GetSelectedDataTableID() };

  // Early out if one of the tables is not selected
  if( uiDataTableID == rumDataTable::INVALID_DATATABLE_ID )
  {
    m_pcUI->actionNew_Data_Entry->setEnabled( false );
    m_pcUI->actionNew_Data_Column->setEnabled( false );
    m_cFilterLabel.setEnabled( false );
    m_cFilterEdit.setEnabled( false );
    return;
  }

  rumDataTable& rcDataTable{ rumDataTable::GetDataTable( uiDataTableID ) };
  if( rcDataTable.GetNumEntries() == 0 )
  {
    rcDataTable.LoadDataTable( uiDataTableID );
  }

  const int32_t iNumColumns{ static_cast<int32_t>( rcDataTable.GetNumColumns() ) };

  m_pcUI->tableWidget_3->setMinimumWidth( 200 );
  m_pcUI->tableWidget_3->setSortingEnabled( false );
  m_pcUI->tableWidget_3->setColumnCount( iNumColumns );
  m_pcUI->tableWidget_3->setRowCount( 0 );
  m_pcUI->tableWidget_3->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
  m_pcUI->tableWidget_3->setHorizontalScrollMode( QAbstractItemView::ScrollPerPixel );

  RefreshHeaderLabels();

  QHeaderView* pcHorizontalHeader{ m_pcUI->tableWidget_3->horizontalHeader() };
  pcHorizontalHeader->setMaximumSectionSize( 300 );
  pcHorizontalHeader->setStretchLastSection( true );

  disconnect( m_pcUI->tableWidget_3, &QTableWidget::itemChanged, this, &DataTableManager::OnDataEntryChanged );

  const int32_t iTableRow{ m_pcUI->tableWidget->currentRow() };

  m_pcUI->actionNew_Data_Entry->setEnabled( true );
  m_pcUI->actionNew_Data_Column->setEnabled( true );
  m_cFilterLabel.setEnabled( true );
  m_cFilterEdit.setEnabled( true );

  int32_t iRow{ 0 };
  m_pcUI->tableWidget_3->setRowCount( rcDataTable.GetNumEntries() );

  for( int32_t iCol{ 0 }; iCol < iNumColumns; ++iCol )
  {
    iRow = 0;

    const auto& rcColumnData{ rcDataTable.GetColumnData( iCol ) };
    bool bIsEnum{ false };

    // If the column is integer based, see if the name matches an enum and if so, use that enum for a value
    if( rcColumnData.m_eValueType == PropertyValueType::Integer )
    {
      Sqrat::Object sqEnumObj{ Sqrat::ConstTable().GetSlot( rcColumnData.m_strName.c_str() ) };
      bIsEnum = !sqEnumObj.IsNull();
    }

    for( const auto sqValue : rcColumnData.m_cValueVector )
    {
      QTableWidgetItem* pcCell{ nullptr };

      if( bIsEnum )
      {
        const std::string strEnum{ rumScript::EnumToString( rcColumnData.m_strName, sqValue.Cast<int32_t>(), false ) };
        pcCell = new QTableWidgetItem( strEnum.c_str() );
        pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
      }
      else
      {
        pcCell = new QTableWidgetItem( rumScript::ToDisplayString( rcColumnData.m_eValueType, sqValue ).c_str() );
        rumControlUtils::UpdateEditableState( *pcCell, rcColumnData.m_eValueType );
      }

      // Set whether this is an enum as the user role for this column
      pcCell->setData( Qt::UserRole, QVariant::fromValue( bIsEnum ) );

      m_pcUI->tableWidget_3->setItem( iRow++, iCol, pcCell );
    }
  }

  UpdateFilter();

  m_pcUI->tableWidget_3->setSortingEnabled( false );
  m_pcUI->tableWidget_3->setEnabled( true );

  // Subscribe to change notifications
  connect( m_pcUI->tableWidget_3, &QTableWidget::itemChanged, this, &DataTableManager::OnDataEntryChanged );
}


void DataTableManager::RefreshHeaderLabels()
{
  const rumDataTable::DataTableID uiDataTableID{ GetSelectedDataTableID() };
  rumDataTable& rcDataTable{ rumDataTable::GetDataTable( uiDataTableID ) };
  if( rcDataTable.GetNumEntries() == 0 )
  {
    rcDataTable.LoadDataTable( uiDataTableID );
  }

  QStringList cLabelList;
  uint32_t uiNumColumns{ rcDataTable.GetNumColumns() };
  for( uint32_t i{ 0 }; i < uiNumColumns; ++i )
  {
    const auto& rcColumnData{ rcDataTable.GetColumnData( i ) };
    cLabelList << rcColumnData.m_strName.c_str();
  }

  m_pcUI->tableWidget_3->setHorizontalHeaderLabels( cLabelList );
}


void DataTableManager::SaveAllTables()
{
  rumDataTable::ExportCSVFiles( qPrintable( MainWindow::GetProjectDir().canonicalPath() ) );
  SetDirty( false );
}


// slot
void DataTableManager::selectionChanged_DataEntry( const QItemSelection& selected, const QItemSelection& deselected )
{
  m_pcUI->actionRemove_Data_Entry->setEnabled( !m_pcUI->tableWidget_3->selectedItems().empty() );
  m_pcUI->actionRemove_Data_Column->setEnabled( !m_pcUI->tableWidget_3->selectedItems().empty() );
}


// slot
void DataTableManager::selectionChanged_DataTable( const QItemSelection& selected, const QItemSelection& deselected )
{
  m_pcUI->actionRemove_Data_Table->setEnabled( !m_pcUI->tableWidget->selectedItems().empty() );
  RefreshDataTable();
}


void DataTableManager::SetDirty( bool i_bDirty )
{
  // Only do work if the current dirty state doesn't equal the requested dirty state
  if( m_bDirty != i_bDirty )
  {
    m_bDirty = i_bDirty;
    m_pcUI->actionSave->setEnabled( m_bDirty );
  }
}


void DataTableManager::TableLineEditFinished()
{
  const int32_t iCol{ m_pcUI->tableWidget->currentColumn() };

  if( iCol == D_COL_NAME )
  {
    const int32_t iRow{ m_pcUI->tableWidget->currentRow() };

    const QWidget* pcWidget{ m_pcUI->tableWidget->cellWidget( iRow, iCol ) };
    if( pcWidget )
    {
      const QLineEdit* pcLineEdit{ qobject_cast<const QLineEdit*>( pcWidget ) };
      if( pcLineEdit )
      {
        disconnect( pcLineEdit, SIGNAL( editingFinished() ), this, SLOT( TableLineEditFinished() ) );

        const QString& strName{ pcLineEdit->text() };
        if( IsTableNameUnique( strName ) )
        {
          QTableWidgetItem* pcItem{ m_pcUI->tableWidget->item( iRow, iCol ) };
          if( pcItem )
          {
            pcItem->setText( strName );
            SetDirty( true );
          }
        }
        else
        {
          const QString strError{ "DataTable names must be unique" };
          QMessageBox::critical( this, tr( "Error" ), strError );
        }
      }

      m_pcUI->tableWidget->removeCellWidget( iRow, iCol );
    }
  }
}


void DataTableManager::UpdateFilter()
{
  // Hide anything that doesn't pass the filter
  const QString& strFilter{ m_cFilterEdit.text() };

  // Visit each item in the table and hide anything that doesn't match the filter settings
  const int32_t iCols{ m_pcUI->tableWidget_3->columnCount() };
  const int32_t iRows{ m_pcUI->tableWidget_3->rowCount() };
  for( int32_t i{ 0 }; i < iRows; ++i )
  {
    if( !strFilter.isEmpty() )
    {
      bool bShow{ false };

      for( int32_t j = 0; j < iCols; ++j )
      {
        const QTableWidgetItem* pcItem{ m_pcUI->tableWidget_3->item( i, j ) };
        bShow |= pcItem->text().contains( strFilter, Qt::CaseInsensitive );
      }

      bShow ? m_pcUI->tableWidget_3->showRow( i ) : m_pcUI->tableWidget_3->hideRow( i );
    }
    else
    {
      m_pcUI->tableWidget_3->showRow( i );
    }
  }

  m_pcUI->tableWidget_3->resizeColumnsToContents();
}
