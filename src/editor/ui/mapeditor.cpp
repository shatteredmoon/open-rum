#include <mapeditor.h>
#include <ui_mapeditor.h>

#include <e_graphics.h>
#include <e_pawn.h>

#include <u_broadcast_asset.h>
#include <u_creature_asset.h>
#include <u_custom_asset.h>
#include <u_enum.h>
#include <u_graphic_asset.h>
#include <u_inventory_asset.h>
#include <u_map_asset.h>
#include <u_portal_asset.h>
#include <u_sound_asset.h>
#include <u_strings.h>
#include <u_tile_asset.h>
#include <u_widget_asset.h>

#undef TRUE
#undef FALSE

#include <assetpicker.h>
#include <mainwindow.h>
#include <mapgoto.h>
#include <newmap.h>
#include <stringtokenpicker.h>

#include <QComboBox>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>

enum ObjectAttributeColumns
{
  ASSET_COL_NAME  = 0,
  ASSET_COL_VALUE = 1
};

enum PropertyTableColumns
{
  PROP_COL_NAME  = 0,
  PROP_COL_VALUE = 1
};


MapEditor::MapEditor( QWidget* i_pcParent )
  : QWidget( i_pcParent )
  , m_pcUI( new Ui::MapEditor )
  , m_bFailure( false )
{
  m_pcUI->setupUi( this );
  Init();
}


MapEditor::MapEditor( const QString& i_strFilePath, QWidget* i_pcParent )
  : QWidget( i_pcParent )
  , m_pcUI( new Ui::MapEditor )
  , m_bFailure( false )
{
  m_pcUI->setupUi( this );
  Init();

  // Attempt to open the specified path
  if( !Open( i_strFilePath ) )
  {
    // Set failure state so that this UI can be cleaned up
    m_bFailure = true;
  }
}


MapEditor::~MapEditor()
{
  delete m_pcUI;
}


void MapEditor::AddAssetProperty( int32_t i_iRow, const QString& i_strAttribute, const QString& i_strValue,
                                  const QString& i_strTooltip )
{
  if( m_pcUI->tableWidget->rowCount() < i_iRow + 1 )
  {
    m_pcUI->tableWidget->setRowCount( i_iRow + 1 );
  }

  // Name
  QTableWidgetItem* pcCell{ new QTableWidgetItem( i_strAttribute ) };
  m_pcUI->tableWidget->setItem( i_iRow, ASSET_COL_NAME, pcCell );
  pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
  pcCell->setBackground( Qt::lightGray );

  // Value
  pcCell = new QTableWidgetItem( i_strValue );
  m_pcUI->tableWidget->setItem( i_iRow, ASSET_COL_VALUE, pcCell );
  pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
  pcCell->setBackground( Qt::lightGray );
  if( !i_strTooltip.isEmpty() )
  {
    pcCell->setToolTip( i_strTooltip );
  }
}


bool MapEditor::AddInstanceProperty( int32_t i_iRow, const rumPropertyAsset& i_rcAsset, Sqrat::Object i_sqValue )
{
  bool bCreate{ true };
  QTableWidgetItem* pcValueCell{ nullptr };

  // Has this property already been added? If so, just update the value.
  for( int32_t iRow{ 0 }; iRow < m_pcUI->tableWidget_3->rowCount(); ++iRow )
  {
    const rumAssetID eAssetID{ MapEditor::GetPropertyID( iRow ) };
    if( i_rcAsset.GetAssetID() == eAssetID )
    {
      auto* pcItem{ m_pcUI->tableWidget_3->item( iRow, PROP_COL_VALUE ) };
      if( pcItem )
      {
        // Property already exists in the table
        pcValueCell = pcItem;
        i_iRow = iRow;
        bCreate = false;
        break;
      }
    }
  }

  if( nullptr == pcValueCell )
  {
    if( m_pcUI->tableWidget_3->rowCount() < i_iRow + 1 )
    {
      m_pcUI->tableWidget_3->setRowCount( i_iRow + 1 );
    }

    // Name & ID
    QTableWidgetItem* pcCell{ new QTableWidgetItem() };
    pcCell->setText( i_rcAsset.GetName().c_str() );
    pcCell->setData( Qt::UserRole, i_rcAsset.GetAssetID() );
    pcCell->setToolTip( rumStringUtils::ToHexString( i_rcAsset.GetAssetID() ) );
    m_pcUI->tableWidget_3->setItem( i_iRow, PROP_COL_NAME, pcCell );
    pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
    if( !i_rcAsset.IsPersistent() )
    {
      pcCell->setBackground( Qt::lightGray );
    }

    // Value
    pcValueCell = new QTableWidgetItem();
  }

  QString strValue;
  Sqrat::Object sqValue{ i_sqValue };
  if( i_rcAsset.IsAssetRef() )
  {
    const rumAsset* pcAsset{ rumAsset::Fetch( sqValue.Cast<int32_t>() ) };
    rumAssert( pcAsset );
    if( pcAsset )
    {
      strValue = pcAsset->GetName().c_str();

      pcValueCell->setData( Qt::UserRole, sqValue.Cast<int32_t>() );
    }
    else
    {
      pcValueCell->setData( Qt::UserRole, 0 );
    }
  }
  else if( i_rcAsset.GetValueType() == PropertyValueType::StringToken )
  {
    const auto eTokenID{ sqValue.Cast<rumTokenID>() };
    strValue = rumStringTable::GetTokenName( eTokenID ).c_str();
    pcValueCell->setData( Qt::UserRole, eTokenID );
  }
  else if( !i_rcAsset.GetEnumName().empty() )
  {
    const int32_t iValue{ sqValue.Cast<int32_t>() };
    strValue = rumScript::EnumToString( i_rcAsset.GetEnumName(), iValue,
                                        i_rcAsset.GetValueType() == PropertyValueType::Bitfield ).c_str();
    pcValueCell->setData( Qt::UserRole, iValue );
    if( i_rcAsset.GetValueType() == PropertyValueType::Bitfield )
    {
      pcValueCell->setToolTip( rumStringUtils::ToHexString( iValue ) );
    }
    else
    {
      pcValueCell->setToolTip( rumStringUtils::ToString( iValue ) );
    }
  }
  else
  {
    switch( i_rcAsset.GetValueType() )
    {
      case PropertyValueType::Integer:  strValue = rumStringUtils::ToString( sqValue.Cast<int32_t>() ); break;
      case PropertyValueType::Float:    strValue = rumStringUtils::ToFloatString( sqValue.Cast<float>() ); break;
      case PropertyValueType::Bool:     strValue = sqValue.Cast<bool>() ? "True" : "False"; break;
      case PropertyValueType::String:   strValue = sqValue.Cast<char*>(); break;
      case PropertyValueType::Bitfield: strValue = rumStringUtils::ToHexString( sqValue.Cast<int32_t>() ); break;
    }
  }

  pcValueCell->setData( Qt::DisplayRole, strValue );

  if( bCreate )
  {
    m_pcUI->tableWidget_3->setItem( i_iRow, PROP_COL_VALUE, pcValueCell );
  }
  else
  {
    pcValueCell->setBackground( Qt::darkYellow );
  }

  if( !i_rcAsset.IsPersistent() )
  {
    pcValueCell->setFlags( pcValueCell->flags() ^ Qt::ItemIsEditable );
    pcValueCell->setBackground( Qt::lightGray );
  }
  else if( i_rcAsset.IsAssetRef() || ( i_rcAsset.GetValueType() == PropertyValueType::Bool ) ||
           !i_rcAsset.GetEnumName().empty() )
  {
    pcValueCell->setFlags( pcValueCell->flags() ^ Qt::ItemIsEditable );
  }

  return bCreate;
}


void MapEditor::CopyPawnProperties( rumPawn* i_pcPawn )
{
  if( i_pcPawn )
  {
    m_cPropertyClipboard = i_pcPawn->GetInstanceProperties();
  }
}


void MapEditor::EditPawn( rumPawn* i_pcPawn )
{
  m_pcCurrentPawn = i_pcPawn;

  disconnect( m_pcUI->tableWidget_3, &QTableWidget::itemChanged, this, &MapEditor::onPropertyTableItemChanged );

  m_pcUI->tableWidget->clearContents();
  m_pcUI->tableWidget->setRowCount( 0 );

  m_pcUI->tableWidget_3->clearContents();
  m_pcUI->tableWidget_3->setRowCount( 0 );

  m_cAssetFilterLabel.setEnabled( false );
  m_cAssetFilterEdit.clear();
  m_cAssetFilterEdit.setEnabled( false );

  m_cPropertyFilterLabel.setEnabled( false );
  m_cPropertyFilterEdit.clear();
  m_cPropertyFilterEdit.setEnabled( false );

  if( nullptr == i_pcPawn )
  {
    m_pcUI->tableWidget->setEnabled( false );
    m_pcUI->tableWidget_3->setEnabled( false );
    return;
  }

  const rumPawnAsset* pcPawnAsset{ rumPawnAsset::Fetch( i_pcPawn->GetAssetID() ) };
  rumAssert( pcPawnAsset );
  if( !pcPawnAsset )
  {
    return;
  }

  int32_t iNumRows{ 0 };
  const QString strNull;

  // Add asset properties
#ifdef _DEBUG
  AddAssetProperty( iNumRows++, "ID", rumStringUtils::ToHexString( pcPawnAsset->GetAssetID() ), strNull );
#endif

  AddAssetProperty( iNumRows++, "Name", pcPawnAsset->GetName().c_str(), strNull );
  AddAssetProperty( iNumRows++, "Class", pcPawnAsset->GetBaseClassOverride().c_str(), strNull );

  const rumGraphicAsset* pcGraphicAsset{ rumGraphicAsset::Fetch( pcPawnAsset->GetGraphicID() ) };
  if( pcGraphicAsset )
  {
    AddAssetProperty( iNumRows++, "Graphic", pcGraphicAsset->GetName().c_str(),
                      rumStringUtils::ToHexString( pcGraphicAsset->GetAssetID() ) );
  }

  AddAssetProperty( iNumRows++, "Move Flags",
                    rumScript::EnumToString( "MoveType", pcPawnAsset->GetMoveType(), true ).c_str(),
                    rumStringUtils::ToHexString( pcPawnAsset->GetMoveType() ) );

  AddAssetProperty( iNumRows++, "Blocks LOS", pcPawnAsset->GetBlocksLOS() ? "True" : "False", strNull );

  AddAssetProperty( iNumRows++, "Collision Flags",
                    rumScript::EnumToString( "MoveType", pcPawnAsset->GetCollisionFlags(), true ).c_str(),
                    rumStringUtils::ToHexString( pcPawnAsset->GetCollisionFlags() ) );

  AddAssetProperty( iNumRows++, "Light Range", rumStringUtils::ToString( pcPawnAsset->GetLightRange() ), strNull );

  AddAssetProperty( iNumRows++, "Draw Priority", rumStringUtils::ToString( pcPawnAsset->GetDrawOrder() ), strNull );

  QString strServiceType;
  switch( pcPawnAsset->GetServiceType() )
  {
    case Shared_ServiceType: strServiceType = "Shared"; break;
    case Server_ServiceType: strServiceType = "Server"; break;
    case Client_ServiceType: strServiceType = "Client"; break;
  }

  AddAssetProperty( iNumRows++, "Service Type", strServiceType,
                    rumStringUtils::ToString( pcPawnAsset->GetServiceType() ) );

  // Add instance properties
  m_pcUI->tableWidget_3->setSortingEnabled( false );

  iNumRows = 0;

  const PropertyContainer& rcAssetProperties{ pcPawnAsset->GetProperties() };
  const PropertyContainer& rcProperties{ i_pcPawn->GetInstanceProperties() };

  for( const auto& iter : rcAssetProperties )
  {
    const rumPropertyAsset* pcProperty{ rumPropertyAsset::Fetch( iter.first ) };
    if( AddInstanceProperty( iNumRows, *pcProperty, iter.second ) )
    {
      ++iNumRows;
    }
  }

  for( const auto& iter : rcProperties )
  {
    const rumPropertyAsset* pcProperty{ rumPropertyAsset::Fetch( iter.first ) };
    if( AddInstanceProperty( iNumRows, *pcProperty, iter.second ) )
    {
      ++iNumRows;
    }
  }

  m_pcUI->tableWidget_3->setRowCount( iNumRows );

  m_pcUI->tableWidget_3->setSortingEnabled( true );
  m_pcUI->tableWidget_3->sortItems( PROP_COL_NAME );

  UpdateAssetFilter();
  UpdatePropertyFilter();

  m_pcUI->actionNew_Property->setEnabled( i_pcPawn != nullptr );

  m_pcUI->tableWidget->setEnabled( true );
  m_pcUI->tableWidget_3->setEnabled( i_pcPawn != nullptr );

  m_cAssetFilterLabel.setEnabled( i_pcPawn != nullptr );
  m_cAssetFilterEdit.setEnabled( i_pcPawn != nullptr );

  m_cPropertyFilterLabel.setEnabled( i_pcPawn != nullptr );
  m_cPropertyFilterEdit.setEnabled( i_pcPawn != nullptr );

  connect( m_pcUI->tableWidget_3, &QTableWidget::itemChanged, this, &MapEditor::onPropertyTableItemChanged );
}


void MapEditor::FilterListWidget( QListWidget* i_pcListWidget, const QString& i_strFilter ) const
{
  rumAssert( i_pcListWidget );
  if( !i_pcListWidget )
  {
    return;
  }

  // Visit each item in the table and hide anything that doesn't match the filter settings
  const int32_t iRow{ i_pcListWidget->count() };
  for( int32_t i{ 0 }; i < iRow; ++i )
  {
    bool bShow{ true };

    if( !i_strFilter.isEmpty() )
    {
      // Check the item for a match
      const QListWidgetItem* pcItem{ i_pcListWidget->item( i ) };
      bShow = pcItem->text().contains( i_strFilter, Qt::CaseInsensitive );
    }

    i_pcListWidget->setRowHidden( i, !bShow );
  }
}


uint32_t MapEditor::GetMapHeight() const
{
  return m_pcUI->mapWidget->height();
}


const smMapWidget* MapEditor::GetMapWidget() const
{
  return m_pcUI->mapWidget;
}


uint32_t MapEditor::GetMapWidth() const
{
  return m_pcUI->mapWidget->width();
}


rumAssetID MapEditor::GetPropertyID( int32_t i_iRow ) const
{
  QTableWidgetItem* pcItem{ nullptr };

  if( i_iRow <= m_pcUI->tableWidget_3->rowCount() )
  {
    pcItem = m_pcUI->tableWidget_3->item( i_iRow, PROP_COL_NAME );
  }

  return pcItem ? (rumAssetID)pcItem->data( Qt::UserRole ).toInt() : INVALID_ASSET_ID;
}


smTileWidget* MapEditor::GetTileWidget() const
{
  return m_pcUI->tileWidget;
}


void MapEditor::GotoMapPosition( const QPoint& i_rcPos )
{
  m_pcUI->mapWidget->CenterOnPosition( i_rcPos );
}


void MapEditor::Init()
{
  m_pcUI->toolbar->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );

  const QIcon cGotoIcon( ":/ui/resources/goto.png" );
  m_pcUI->actionGoto_Pos->setIcon( cGotoIcon );

  const QIcon cSaveIcon( ":/ui/resources/save.png" );
  m_pcUI->actionSave->setIcon( cSaveIcon );

  const QIcon cSettingsIcon( ":/ui/resources/settings.png" );
  m_pcUI->actionSettings->setIcon( cSettingsIcon );

  m_pcUI->mapWidget->setContextMenuPolicy( Qt::CustomContextMenu );

  InitListWidget<rumTileAsset>( m_pcUI->listWidget );
  InitListWidget<rumCreatureAsset>( m_pcUI->listWidget_2 );
  InitListWidget<rumWidgetAsset>( m_pcUI->listWidget_3 );
  InitListWidget<rumPortalAsset>( m_pcUI->listWidget_4 );

  connect( m_pcUI->listWidget, SIGNAL( itemClicked( QListWidgetItem* ) ),
           this, SLOT( OnListWidgetItemClicked( QListWidgetItem* ) ) );
  connect( m_pcUI->listWidget_2, SIGNAL( itemClicked( QListWidgetItem* ) ),
           this, SLOT( OnListWidgetItemClicked( QListWidgetItem* ) ) );
  connect( m_pcUI->listWidget_3, SIGNAL( itemClicked( QListWidgetItem* ) ),
           this, SLOT( OnListWidgetItemClicked( QListWidgetItem* ) ) );
  connect( m_pcUI->listWidget_4, SIGNAL( itemClicked( QListWidgetItem* ) ),
           this, SLOT( OnListWidgetItemClicked( QListWidgetItem* ) ) );

  // Asset Table --------------------------------------------------------------

  QStringList cLabelList;
  cLabelList << "Name" << "Value";

  m_pcUI->tableWidget->setMinimumWidth( 200 );
  m_pcUI->tableWidget->setEnabled( false );
  m_pcUI->tableWidget->setColumnCount( cLabelList.size() );
  m_pcUI->tableWidget->setRowCount( 0 );
  m_pcUI->tableWidget->setHorizontalHeaderLabels( cLabelList );

  QHeaderView* pcHorizontalHeader{ m_pcUI->tableWidget->horizontalHeader() };
  pcHorizontalHeader->setStretchLastSection( true );

  // Add a line edit on the toolbar for filtering
  m_cAssetFilterLabel.setText( "Filter: " );
  m_pcUI->toolBar3->addWidget( &m_cAssetFilterLabel );
  m_pcAssetFilterAction = m_pcUI->toolBar3->addWidget( &m_cAssetFilterEdit );
  m_cAssetFilterLabel.setEnabled( false );
  m_cAssetFilterEdit.setEnabled( false );
  connect( &m_cAssetFilterEdit, SIGNAL( editingFinished() ), this, SLOT( OnAssetFilterChanged() ) );

  // Property Table -----------------------------------------------------------

  cLabelList.clear();
  cLabelList << "Name" << "Value";

  m_pcUI->tableWidget_3->setMinimumWidth( 200 );
  m_pcUI->tableWidget_3->setEnabled( false );
  m_pcUI->tableWidget_3->setColumnCount( cLabelList.size() );
  m_pcUI->tableWidget_3->setRowCount( 0 );
  m_pcUI->tableWidget_3->setHorizontalHeaderLabels( cLabelList );

  pcHorizontalHeader = m_pcUI->tableWidget_3->horizontalHeader();
  pcHorizontalHeader->setStretchLastSection( true );

  m_pcUI->actionNew_Property->setEnabled( false );
  m_pcUI->actionRemove_Property->setEnabled( false );

  // Add a line edit on the toolbar for filtering
  m_pcUI->toolBar2->addSeparator();
  m_cPropertyFilterLabel.setText( "Filter: " );
  m_pcUI->toolBar2->addWidget( &m_cPropertyFilterLabel );
  m_pcPropertyFilterAction = m_pcUI->toolBar2->addWidget( &m_cPropertyFilterEdit );
  m_cPropertyFilterLabel.setEnabled( false );
  m_cPropertyFilterEdit.setEnabled( false );
  connect( &m_cPropertyFilterEdit, SIGNAL( editingFinished() ), this, SLOT( OnPropertyFilterChanged() ) );

  connect( m_pcUI->tableWidget_3->selectionModel(),
           SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
           this, SLOT( selectionChanged_Property( const QItemSelection&, const QItemSelection& ) ) );

  //connect( m_pcUI->tableWidget_3, SIGNAL( cellActivated( int32_t, int32_t ) ),
  //         this, SLOT( OnCellStartPropertyEdit( int32_t, int32_t ) ) );
  // Seems like cellActivated works for DoubleClicked as well?
  connect( m_pcUI->tableWidget_3, SIGNAL( cellDoubleClicked( int32_t, int32_t ) ),
           this, SLOT( OnCellStartPropertyEdit( int32_t, int32_t ) ) );
}


void MapEditor::InspectTile( rumAssetID i_eTileID )
{
  disconnect( m_pcUI->tableWidget_3, &QTableWidget::itemChanged, this, &MapEditor::onPropertyTableItemChanged );

  m_pcUI->tableWidget->clearContents();
  m_pcUI->tableWidget->setRowCount( 0 );

  m_pcUI->tableWidget_3->clearContents();
  m_pcUI->tableWidget_3->setRowCount( 0 );

  m_cAssetFilterLabel.setEnabled( false );
  m_cAssetFilterEdit.clear();
  m_cAssetFilterEdit.setEnabled( false );

  m_cPropertyFilterLabel.setEnabled( false );
  m_cPropertyFilterEdit.clear();
  m_cPropertyFilterEdit.setEnabled( false );

  if( INVALID_ASSET_ID == i_eTileID )
  {
    m_pcUI->tableWidget->setEnabled( false );
    m_pcUI->tableWidget_3->setEnabled( false );
    return;
  }

  const rumTileAsset* pcTileAsset{ rumTileAsset::Fetch( i_eTileID ) };
  rumAssert( pcTileAsset );
  if( !pcTileAsset )
  {
    return;
  }

  int32_t iRow{ 0 };
  const QString strNull;

  // Add asset properties
#ifdef _DEBUG
  AddAssetProperty( iRow++, "ID", rumStringUtils::ToHexString( i_eTileID ), strNull );
#endif

  AddAssetProperty( iRow++, "Name", pcTileAsset->GetName().c_str(), strNull );
  AddAssetProperty( iRow++, "Class", pcTileAsset->GetBaseClassOverride().c_str(), strNull );

  const rumGraphicAsset* pcGraphicAsset{ rumGraphicAsset::Fetch( pcTileAsset->GetGraphicID() ) };
  if( pcGraphicAsset )
  {
    AddAssetProperty( iRow++, "Graphic", pcGraphicAsset->GetName().c_str(),
                      rumStringUtils::ToHexString( pcGraphicAsset->GetAssetID() ) );
  }

  AddAssetProperty( iRow++, "Weight", rumStringUtils::ToString( pcTileAsset->GetWeight() ), strNull );
  AddAssetProperty( iRow++, "Blocks LOS", pcTileAsset->GetBlocksLOS() ? "True" : "False", strNull );

  AddAssetProperty( iRow++, "Collision Flags",
                    rumScript::EnumToString( "MoveType", pcTileAsset->GetCollisionFlags(), true ).c_str(),
                    rumStringUtils::ToHexString( pcTileAsset->GetCollisionFlags() ) );

  // Add instance properties
  const PropertyContainer& rcAssetProperties{ pcTileAsset->GetProperties() };
  iRow = 0;

  m_pcUI->tableWidget_3->setRowCount( (int32_t)rcAssetProperties.size() );

  for( const auto& iter : rcAssetProperties )
  {
    const rumPropertyAsset* pcProperty{ rumPropertyAsset::Fetch( iter.first ) };
    if( AddInstanceProperty( iRow, *pcProperty, iter.second ) )
    {
      ++iRow;
    }
  }

  UpdateAssetFilter();
  UpdatePropertyFilter();

  m_pcUI->tableWidget->setEnabled( true );
  m_pcUI->tableWidget_3->setEnabled( true );

  m_cAssetFilterLabel.setEnabled( true );
  m_cAssetFilterEdit.setEnabled( true );

  m_cPropertyFilterLabel.setEnabled( true );
  m_cPropertyFilterEdit.setEnabled( true );

  connect( m_pcUI->tableWidget_3, &QTableWidget::itemChanged, this, &MapEditor::onPropertyTableItemChanged );
}


bool MapEditor::IsDirty() const
{
  return m_pcUI->mapWidget->IsDirty();
}


// slot
void MapEditor::ItemComboPropertyChanged( const QString& i_strText )
{
  const int32_t iRow{ m_pcUI->tableWidget_3->currentRow() };

  // Fetch the asset from the affected row
  const rumPropertyAsset* pcProperty{ rumPropertyAsset::Fetch( GetPropertyID( iRow ) ) };
  rumAssert( pcProperty );
  if( !pcProperty )
  {
    return;
  }

  const int32_t iCol{ m_pcUI->tableWidget_3->currentColumn() };
  QVariant cVariant;

  // Access user data if it is available
  QWidget* pcWidget{ m_pcUI->tableWidget_3->cellWidget( iRow, iCol ) };
  if( pcWidget )
  {
    QComboBox* pcComboBox{ qobject_cast<QComboBox*>( pcWidget ) };
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

  if( pcProperty->IsAssetRef() || ( pcProperty->GetValueType() == PropertyValueType::Bool ) )
  {
    pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
  }

  m_pcUI->mapWidget->MarkDirty();
}


void MapEditor::keyPressEvent( QKeyEvent* i_pcEvent )
{
  //if( !m_bHasFocus )
  //{
  //  super::keyPressEvent( pcEvent );
  //  return;
  //}

  switch( i_pcEvent->key() )
  {
    case Qt::Key_C:
    {
      rumPawn* pcPawn{ m_pcUI->mapWidget->GetPawnAtCurrentTile() };
      CopyPawnProperties( pcPawn );
      break;
    }

    case Qt::Key_E:
    {
      rumPawn* pcPawn{ m_pcUI->mapWidget->GetPawnAtCurrentTile() };
      EditPawn( pcPawn );
      break;
    }

    case Qt::Key_I:
    {
      const QPoint cPoint{ m_pcUI->mapWidget->GetSelectedPos() };
      const rumAssetID eTileID{ m_pcUI->mapWidget->GetTile( cPoint ) };
      InspectTile( eTileID );
      break;
    }

    case Qt::Key_V:
    {
      rumPawn* pcPawn{ m_pcUI->mapWidget->GetPawnAtCurrentTile() };
      PastePawnProperties( pcPawn );
      break;
    }
  }
}


bool MapEditor::Load( const QString& i_strPath )
{
  m_strFilePath = i_strPath;
  return m_pcUI->mapWidget->LoadMap( i_strPath );
}


// slot
void MapEditor::OnAnimTimer()
{
  m_pcUI->mapWidget->OnAnimTimer();
  m_pcUI->tileWidget->OnAnimTimer();
}


// slot
void MapEditor::OnAssetFilterChanged()
{
  UpdateAssetFilter();
}


void MapEditor::OnBrushChanged( const QListWidgetItem* i_pcItem )
{
  const QVariant cVariant{ i_pcItem->data( Qt::UserRole ) };
  if( cVariant.isValid() )
  {
    m_pcUI->mapWidget->SetBrush( (rumAssetID)cVariant.toInt() );
  }
}


void MapEditor::OnCellStartPropertyEdit( int32_t i_iRow, int32_t i_iCol )
{
  if( !m_pcCurrentPawn )
  {
    return;
  }

  const rumAssetID ePropertyID{ GetPropertyID( i_iRow ) };

  Sqrat::Object sqProperty{ m_pcCurrentPawn->GetProperty( ePropertyID ) };

  const rumPropertyAsset* pcProperty{ rumPropertyAsset::Fetch( ePropertyID ) };
  rumAssert( pcProperty );
  if( !pcProperty )
  {
    return;
  }

  if( !pcProperty->IsPersistent() )
  {
    // Pawns can only edit persistent properties
    return;
  }

  if( pcProperty->GetValueType() == PropertyValueType::Bool )
  {
    QComboBox* pcCombo{ new QComboBox };
    pcCombo->addItem( "False", false );
    pcCombo->addItem( "True", true );
    pcCombo->setCurrentIndex( sqProperty.Cast<bool>() ? 1 : 0 );
    pcCombo->view()->setMinimumWidth( pcCombo->minimumSizeHint().width() );
    connect( pcCombo, SIGNAL( currentIndexChanged( const QString& ) ),
             this, SLOT( ItemComboPropertyChanged( const QString& ) ) );
    m_pcUI->tableWidget_3->setCellWidget( i_iRow, i_iCol, pcCombo );
  }
  else if( ( pcProperty->GetValueType() == PropertyValueType::Bitfield ||
             pcProperty->GetValueType() == PropertyValueType::Integer ) &&
           !pcProperty->GetEnumName().empty() )
  {
    int32_t iValue{ 0 };

    // Get the current value
    QTableWidgetItem* pcCell{ m_pcUI->tableWidget_3->item( i_iRow, i_iCol ) };
    const QVariant cValue{ pcCell->data( Qt::UserRole ) };
    if( cValue.isValid() )
    {
      iValue = cValue.toInt();
    }
    else
    {
      iValue = rumStringUtils::ToInt( qPrintable( pcCell->text() ) );
    }

    Sqrat::Object sqValue;
    rumScript::SetValue( sqValue, iValue );

    // The enum field to use as selection data
    Sqrat::Object sqEnumObj{ Sqrat::ConstTable().GetSlot( pcProperty->GetEnumName().c_str() ) };

    Sqrat::Object sqMultiSelection;
    rumScript::SetValue( sqMultiSelection, pcProperty->GetValueType() == PropertyValueType::Bitfield );

    HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

    Sqrat::Array sqOuterArray( pcVM, 1 );

    Sqrat::Array sqInnerArray( pcVM, 4 );
    sqInnerArray.SetValue( 0, pcProperty->GetEnumName().c_str() );
    sqInnerArray.SetValue( 1, sqValue );
    sqInnerArray.SetValue( 2, sqEnumObj );
    sqInnerArray.SetValue( 3, sqMultiSelection );

    sqOuterArray.SetValue( 0, sqInnerArray );

    Sqrat::Array sqResultArray{ MainWindow::ScriptModalDialog( sqOuterArray ) };
    if( sqResultArray.GetType() == OT_ARRAY )
    {
      sqValue = *( sqResultArray.GetValue<Sqrat::Object>( 0 ) );
      if( sqValue.GetType() == OT_INTEGER )
      {
        const int32_t iNewValue{ sqValue.Cast<int32_t>() };
        std::string strEnum{ rumScript::EnumToString( pcProperty->GetEnumName(), iNewValue,
                                                      pcProperty->GetValueType() == PropertyValueType::Bitfield ) };

        pcCell = new QTableWidgetItem();
        pcCell->setData( Qt::DisplayRole, strEnum.c_str() );
        pcCell->setData( Qt::UserRole, iNewValue );
        if( pcProperty->GetValueType() == PropertyValueType::Bitfield )
        {
          pcCell->setToolTip( rumStringUtils::ToHexString( iNewValue ) );
        }
        else
        {
          pcCell->setToolTip( rumStringUtils::ToString( iNewValue ) );
        }
        m_pcUI->tableWidget_3->setItem( i_iRow, i_iCol, pcCell );
        pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );

        m_pcCurrentPawn->SetProperty( ePropertyID, sqValue );

        m_pcUI->mapWidget->MarkDirty();
      }
    }
  }
  else if( pcProperty->GetValueType() == PropertyValueType::StringToken )
  {
    int32_t iValue{ 0 };

    // Get the current value
    QTableWidgetItem* pcCell{ m_pcUI->tableWidget_3->item( i_iRow, i_iCol ) };
    const QVariant cVariant{ pcCell->data( Qt::UserRole ) };
    if( cVariant.isValid() )
    {
      iValue = cVariant.toInt();
    }
    else
    {
      iValue = rumStringUtils::ToInt( qPrintable( pcCell->text() ) );
    }

    const rumTokenID eTokenID{ static_cast<rumTokenID>( iValue ) };

    // Use the string token picker dialog for the new selection
    StringTokenPicker* pcDialog{ new StringTokenPicker( eTokenID, this ) };

    connect( pcDialog, SIGNAL( NewStringTokenSelected( rumTokenID ) ),
             this, SLOT( StringTokenPropertyChanged( rumTokenID ) ) );

    pcDialog->setModal( true );
    pcDialog->show();
  }
  else if( pcProperty->IsAssetRef() )
  {
    QComboBox* pcCombo{ new QComboBox };

    if( ( pcProperty->GetValueType() == PropertyValueType::AssetRef ) ||
        ( pcProperty->GetValueType() == PropertyValueType::BroadcastRef ) )
    {
      // Add every broadcast entry by name and id
      const auto& rcBroadcastHash{ rumBroadcastAsset::GetAssetHash() };
      for( auto& iter : rcBroadcastHash )
      {
        pcCombo->addItem( iter.second->GetName().c_str(), iter.first );
      }
    }

    if( ( pcProperty->GetValueType() == PropertyValueType::AssetRef ) ||
        ( pcProperty->GetValueType() == PropertyValueType::CreatureRef ) )
    {
      // Add every creature entry by name and id
      const auto& rcCreatureHash{ rumCreatureAsset::GetAssetHash() };
      for( const auto& iter : rcCreatureHash )
      {
        pcCombo->addItem( iter.second->GetName().c_str(), iter.first );
      }
    }

    if( ( pcProperty->GetValueType() == PropertyValueType::AssetRef ) ||
        ( pcProperty->GetValueType() == PropertyValueType::CustomRef ) )
    {
      // Add every custom entry by name and id
      const auto& rcCustomHash{ rumCustomAsset::GetAssetHash() };
      for( const auto& iter : rcCustomHash )
      {
        pcCombo->addItem( iter.second->GetName().c_str(), iter.first );
      }
    }

    if( ( pcProperty->GetValueType() == PropertyValueType::AssetRef ) ||
        ( pcProperty->GetValueType() == PropertyValueType::GraphicRef ) )
    {
      // Add every graphic entry by name and id
      const auto& rcGraphicHash{ rumGraphicAsset::GetAssetHash() };
      for( const auto& iter : rcGraphicHash )
      {
        pcCombo->addItem( iter.second->GetName().c_str(), iter.first );
      }
    }

    if( ( pcProperty->GetValueType() == PropertyValueType::AssetRef ) ||
        ( pcProperty->GetValueType() == PropertyValueType::InventoryRef ) )
    {
      // Add every inventory entry by name and id
      const auto& rcInventoryHash{ rumInventoryAsset::GetAssetHash() };
      for( const auto& iter : rcInventoryHash )
      {
        pcCombo->addItem( iter.second->GetName().c_str(), iter.first );
      }
    }

    if( ( pcProperty->GetValueType() == PropertyValueType::AssetRef ) ||
        ( pcProperty->GetValueType() == PropertyValueType::MapRef ) )
    {
      // Add every map entry by name and id
      const auto& rcMapHash{ rumMapAsset::GetAssetHash() };
      for( const auto& iter : rcMapHash )
      {
        pcCombo->addItem( iter.second->GetName().c_str(), iter.first );
      }
    }

    if( ( pcProperty->GetValueType() == PropertyValueType::AssetRef ) ||
        ( pcProperty->GetValueType() == PropertyValueType::PortalRef ) )
    {
      // Add every portal entry by name and id
      const auto& rcPortalHash{ rumPortalAsset::GetAssetHash() };
      for( const auto& iter : rcPortalHash )
      {
        pcCombo->addItem( iter.second->GetName().c_str(), iter.first );
      }
    }

    if( ( pcProperty->GetValueType() == PropertyValueType::AssetRef ) ||
        ( pcProperty->GetValueType() == PropertyValueType::PropertyRef ) )
    {
      // Add every property entry by name and id
      const auto& rcPropertyHash{ rumPropertyAsset::GetAssetHash() };
      for( const auto& iter : rcPropertyHash )
      {
        pcCombo->addItem( iter.second->GetName().c_str(), iter.first );
      }
    }

    if( ( pcProperty->GetValueType() == PropertyValueType::AssetRef ) ||
        ( pcProperty->GetValueType() == PropertyValueType::SoundRef ) )
    {
      // Add every sound entry by name and id
      const auto& rcSoundHash{ rumSoundAsset::GetAssetHash() };
      for( const auto& iter : rcSoundHash )
      {
        pcCombo->addItem( iter.second->GetName().c_str(), iter.first );
      }
    }

    if( ( pcProperty->GetValueType() == PropertyValueType::AssetRef ) ||
        ( pcProperty->GetValueType() == PropertyValueType::TileRef ) )
    {
      // Add every tile entry by name and id
      const auto& rcTileHash{ rumTileAsset::GetAssetHash() };
      for( const auto& iter : rcTileHash )
      {
        pcCombo->addItem( iter.second->GetName().c_str(), iter.first );
      }
    }

    if( ( pcProperty->GetValueType() == PropertyValueType::AssetRef ) ||
        ( pcProperty->GetValueType() == PropertyValueType::WidgetRef ) )
    {
      // Add every widget entry by name and id
      const auto& rcWidgetHash{ rumWidgetAsset::GetAssetHash() };
      for( const auto& iter : rcWidgetHash )
      {
        pcCombo->addItem( iter.second->GetName().c_str(), iter.first );
      }
    }

    // Select the index in the combo box that matches the selection in the table prior to this edit
    const QTableWidgetItem* pcCell{ m_pcUI->tableWidget_3->item( i_iRow, i_iCol ) };
    const QVariant cVariant{ pcCell->data( Qt::UserRole ) };
    const int32_t iIndex{ pcCombo->findData( cVariant ) };
    if( iIndex != -1 )
    {
      pcCombo->setCurrentIndex( iIndex );
    }

    // Make it easier to find things
    pcCombo->model()->sort( 0, Qt::AscendingOrder );
    pcCombo->view()->setMinimumWidth( pcCombo->minimumSizeHint().width() );

    connect( pcCombo, SIGNAL( currentIndexChanged( const QString& ) ),
             this, SLOT( ItemComboPropertyChanged( const QString& ) ) );
    m_pcUI->tableWidget_3->setCellWidget( i_iRow, i_iCol, pcCombo );
  }
}


// slot
void MapEditor::OnCopyPawnProperties()
{
  rumPawn* pcPawn{ m_pcUI->mapWidget->GetPawnAtCurrentTile() };
  CopyPawnProperties( pcPawn );
}


// slot
void MapEditor::OnEditPawn()
{
  rumPawn* pcPawn{ m_pcUI->mapWidget->GetPawnAtCurrentTile() };
  EditPawn( pcPawn );
}


// slot
void MapEditor::OnInspectTile()
{
  const QPoint cPoint{ m_pcUI->mapWidget->GetSelectedPos() };
  const rumAssetID eTileID{ m_pcUI->mapWidget->GetTile( cPoint ) };
  InspectTile( eTileID );
}


// slot
void MapEditor::OnListWidgetItemClicked( QListWidgetItem* i_pcItem )
{
  OnBrushChanged( i_pcItem );
}


// slot
void MapEditor::OnPastePawnProperties()
{
  rumPawn* pcPawn{ m_pcUI->mapWidget->GetPawnAtCurrentTile() };
  PastePawnProperties( pcPawn );
}


void MapEditor::OnPawnAdded( rumPawn* i_pcPawn )
{
  PastePawnProperties( i_pcPawn );
  EditPawn( i_pcPawn );
}


void MapEditor::OnPawnPicked( rumPawn* i_pcPawn )
{
  CopyPawnProperties( i_pcPawn );
  EditPawn( i_pcPawn );
}


void MapEditor::OnPawnRemoved( const rumPawn* i_pcPawn )
{
  m_pcCurrentPawn = nullptr;
  m_pcUI->tableWidget_3->clear();
  m_pcUI->tableWidget_3->setRowCount( 0 );

  m_pcUI->actionNew_Property->setEnabled( false );
  m_pcUI->tableWidget_3->setEnabled( false );

  disconnect( m_pcUI->tableWidget_3, &QTableWidget::itemChanged, this, &MapEditor::onPropertyTableItemChanged );

  m_cAssetFilterLabel.setEnabled( false );
  m_cAssetFilterEdit.setEnabled( false );

  m_cPropertyFilterLabel.setEnabled( false );
  m_cPropertyFilterEdit.setEnabled( false );
}


void MapEditor::OnTilePainted( const QPoint& i_rcPos )
{
  const rumAssetID eTileID{ m_pcUI->mapWidget->GetTile( i_rcPos ) };
  InspectTile( eTileID );
}


void MapEditor::OnTilePicked( rumAssetID i_eTileID )
{
  InspectTile( i_eTileID );
}


// slot
void MapEditor::OnPropertyAdded( rumAssetID i_ePropertyID )
{
  const rumPropertyAsset* pcProperty{ rumPropertyAsset::Fetch( i_ePropertyID ) };
  rumAssert( pcProperty );
  if( !pcProperty )
  {
    return;
  }

  // Unsubscribe from change notifications
  disconnect( m_pcUI->tableWidget_3, &QTableWidget::itemChanged, this, &MapEditor::onPropertyTableItemChanged );

  // Temporarily disable sorting
  m_pcUI->tableWidget_3->setSortingEnabled( false );

  // Default value
  Sqrat::Object sqValue{ pcProperty->GetDefaultValue() };

  if( sqValue.GetType() == OT_NULL )
  {
    if( pcProperty->IsAssetRef() )
    {
      rumScript::SetValue( sqValue, 0 );
    }
    else
    {
      switch( pcProperty->GetValueType() )
      {
        case PropertyValueType::Bitfield:
        case PropertyValueType::Integer:     rumScript::SetValue( sqValue, 0 ); break;
        case PropertyValueType::Float:       rumScript::SetValue( sqValue, 0.f ); break;
        case PropertyValueType::Bool:        rumScript::SetValue( sqValue, false ); break;
        case PropertyValueType::String:      rumScript::SetValue( sqValue, "" ); break;
        case PropertyValueType::StringToken: rumScript::SetValue( sqValue, rumStringTable::INVALID_TOKEN_ID ); break;
      }
    }
  }

  const int32_t iRow{ m_pcUI->tableWidget_3->rowCount() };
  AddInstanceProperty( iRow, *pcProperty, sqValue );

  m_pcUI->tableWidget_3->selectRow( iRow );

  // Subscribe from change notifications
  connect( m_pcUI->tableWidget_3, &QTableWidget::itemChanged, this, &MapEditor::onPropertyTableItemChanged );

  // Re-enable sorting
  m_pcUI->tableWidget_3->setSortingEnabled( true );

  // Scroll to the newly added row
  // NOTE: The row id might have just changed because of sorting, so use currentRow
  const QTableWidgetItem* pcItem{ m_pcUI->tableWidget_3->item( m_pcUI->tableWidget_3->currentRow(), PROP_COL_NAME ) };
  if( pcItem )
  {
    m_pcUI->tableWidget_3->scrollToItem( pcItem );
  }

  // Add the new property to the pawn
  m_pcCurrentPawn->SetProperty( i_ePropertyID, sqValue );

  m_pcUI->mapWidget->MarkDirty();
}


// slot
void MapEditor::OnPropertyFilterChanged()
{
  UpdatePropertyFilter();
}


// slot
void MapEditor::onPropertyTableItemChanged( QTableWidgetItem* i_pcItem )
{
  const rumAssetID ePropertyID{ GetPropertyID( i_pcItem->row() ) };
  if( INVALID_ASSET_ID == ePropertyID )
  {
    return;
  }

  rumPropertyAsset* pcProperty{ rumPropertyAsset::Fetch( ePropertyID ) };
  rumAssert( pcProperty );
  if( !pcProperty )
  {
    return;
  }

  const QString strValue{ i_pcItem->text() };

  Sqrat::Object sqValue{ pcProperty->GetProperty( ePropertyID ) };

  if( pcProperty->IsAssetRef() )
  {
    const QVariant cVariant{ i_pcItem->data( Qt::UserRole ) };
    rumScript::SetValue( sqValue, cVariant.toInt() );
  }
  else
  {
    switch( pcProperty->GetValueType() )
    {
      case PropertyValueType::Bitfield:
      case PropertyValueType::Integer:
      case PropertyValueType::StringToken:
        rumScript::SetValue( sqValue, rumStringUtils::ToInt( qPrintable( strValue ) ) );
        break;

      case PropertyValueType::Bool:
        rumScript::SetValue( sqValue, rumStringUtils::ToBool( qPrintable( strValue ) ) );
        break;

      case PropertyValueType::Float:
        rumScript::SetValue( sqValue, strValue.toFloat() );
        break;

      case PropertyValueType::String:
        rumScript::SetValue( sqValue, qPrintable( strValue ) );
        break;

      default:
        rumAssertMsg( false, "Unsupported value type" );
        return;
    }
  }

  m_pcCurrentPawn->SetProperty( ePropertyID, sqValue );

  m_pcUI->mapWidget->MarkDirty();
}


// slot
void MapEditor::on_lineEdit_textChanged( const QString& i_strFilter )
{
  FilterListWidget( m_pcUI->listWidget, i_strFilter );
  FilterListWidget( m_pcUI->listWidget_2, i_strFilter );
  FilterListWidget( m_pcUI->listWidget_3, i_strFilter );
  FilterListWidget( m_pcUI->listWidget_4, i_strFilter );
}


// slot
void MapEditor::on_toolButton_clicked()
{
  m_pcUI->lineEdit->clear();
}


// slot
void MapEditor::on_actionSettings_triggered()
{
  NewMap* pcDialog{ new NewMap( this, m_pcUI->mapWidget->GetExitMapID(), m_pcUI->mapWidget->GetExitMapPos(), true ) };

  connect( pcDialog, SIGNAL( mapResized( uint32_t, uint32_t, int32_t, rumAssetID ) ),
           m_pcUI->mapWidget, SLOT( ResizeMap( uint32_t, uint32_t, int32_t, rumAssetID ) ) );

  connect( pcDialog, SIGNAL( mapBorderChanged( rumAssetID ) ),
           m_pcUI->mapWidget, SLOT( SetBorderTile( rumAssetID ) ) );

  connect( pcDialog, SIGNAL( mapExitChanged( rumAssetID, const QPoint& ) ),
           m_pcUI->mapWidget, SLOT( OnMapExitChanged( rumAssetID, const QPoint& ) ) );

  pcDialog->SetNumColumns( m_pcUI->mapWidget->GetNumMapColumns() );
  pcDialog->SetNumRows( m_pcUI->mapWidget->GetNumMapRows() );
  pcDialog->SetBorderTile( m_pcUI->mapWidget->GetBorderTile() );
  pcDialog->setModal( true );
  pcDialog->show();
}


// slot
void MapEditor::on_actionGoto_Pos_triggered()
{
  const QPoint cMapSize( m_pcUI->mapWidget->GetNumMapColumns(), m_pcUI->mapWidget->GetNumMapRows() );

  MapGoto* pcDialog{ new MapGoto( cMapSize, this ) };

  connect( pcDialog, SIGNAL( GotoMapPosition( const QPoint& ) ),
           m_pcUI->mapWidget, SLOT( CenterOnPosition( const QPoint& ) ) );

  pcDialog->setModal( true );
  pcDialog->show();
}


// slot
void MapEditor::on_actionNew_Property_triggered()
{
  // Clear the filter
  m_cPropertyFilterEdit.clear();

  QSet<rumAssetID> cSetAssets;

  // Any asset type already in the table will be used as an ignore set for the modal dialog
  for( int32_t i{ 0 }; i < m_pcUI->tableWidget_3->rowCount(); ++i )
  {
    cSetAssets.insert( GetPropertyID( i ) );
  }

  // Open a modal dialog listbox of persistent properties for selection
  bool bPersistenOnly{ true };
  AssetPicker* pcDialog{ new AssetPicker( AssetType::Property_AssetType, bPersistenOnly, cSetAssets, this ) };

  connect( pcDialog, SIGNAL( NewPropertySelected( rumAssetID ) ),
           this, SLOT( OnPropertyAdded( rumAssetID ) ) );

  pcDialog->setModal( true );
  pcDialog->show();
}


// slot
void MapEditor::on_actionRemove_Property_triggered()
{
  const int32_t iRow{ m_pcUI->tableWidget_3->currentRow() };
  if( -1 == iRow )
  {
    return;
  }

  if( m_pcCurrentPawn )
  {
    m_pcCurrentPawn->RemoveProperty( GetPropertyID( iRow ) );
  }

  m_pcUI->tableWidget_3->removeRow( iRow );
}


// slot
void MapEditor::on_actionSave_triggered()
{
  Save();
}


// slot
void MapEditor::on_mapWidget_customContextMenuRequested( const QPoint& i_rcPos )
{
  // Only show the context menu if the map is in edit mode
  if( m_pcUI->mapWidget->GetMapMode() != smMapWidget::EditMode )
  {
    return;
  }

  QMenu* pcMenu{ new QMenu( this ) };

  QAction* pcPickTileAction{ new QAction( "Pick Tile", pcMenu ) };
  const QKeySequence cPickKeySequence( Qt::Key_T );
  pcPickTileAction->setShortcut( cPickKeySequence );
  pcMenu->addAction( pcPickTileAction );
  connect( pcPickTileAction, SIGNAL( triggered() ),
           m_pcUI->mapWidget, SLOT( onPickTile() ) );

  QAction* pcInspectTileAction{ new QAction( "Inspect Tile", pcMenu ) };
  const QKeySequence cInspectKeySequence( Qt::Key_I );
  pcInspectTileAction->setShortcut( cInspectKeySequence );
  pcMenu->addAction( pcInspectTileAction );
  connect( pcInspectTileAction, SIGNAL( triggered() ),
           this, SLOT( OnInspectTile() ) );

  int32_t iGraphicPacks{ 0 };

  pcMenu->addSeparator();

  if( m_pcUI->mapWidget->HasPawnAtCurrentTile() )
  {
    const rumPawn* pcPawn{ m_pcUI->mapWidget->GetPawnAtCurrentTile() };
    rumAssert( pcPawn );
    if( pcPawn )
    {
      if( pcPawn->GetPawnType() == rumPawn::Portal_PawnType )
      {
        QAction* pcOpenMapAction{ new QAction( "Open Destination Map", pcMenu ) };
        const QKeySequence cOpenMapSequence( Qt::Key_Enter );
        pcOpenMapAction->setShortcut( cOpenMapSequence );
        pcMenu->addAction( pcOpenMapAction );
        connect( pcOpenMapAction, SIGNAL( triggered() ),
                 m_pcUI->mapWidget, SLOT( onPortalOpen() ) );

        pcMenu->addSeparator();
      }

      // Note that picking a pawn also copies that pawn's properties
      QAction* pcPickPawnAction{ new QAction( "Pick Pawn", pcMenu ) };
      const QKeySequence cPickPawnSequence( Qt::Key_P );
      pcPickPawnAction->setShortcut( cPickPawnSequence );
      pcMenu->addAction( pcPickPawnAction );
      connect( pcPickPawnAction, SIGNAL( triggered() ),
               m_pcUI->mapWidget, SLOT( onPickPawn() ) );

      //QAction *pcClonePawnAction{ new QAction("Clone Pawn", this) };
      //pcMenu->addAction(pcClonePawnAction);

      QAction* pcEditPawnAction{ new QAction( "Edit Pawn", pcMenu ) };
      const QKeySequence cEditPawnSequence( Qt::Key_E );
      pcEditPawnAction->setShortcut( cEditPawnSequence );
      pcMenu->addAction( pcEditPawnAction );
      connect( pcEditPawnAction, SIGNAL( triggered() ),
               this, SLOT( OnEditPawn() ) );

      QAction* pcMovePawnAction{ new QAction( "Move Pawn", pcMenu ) };
      const QKeySequence cMovePawnSequence( Qt::Key_M );
      pcMovePawnAction->setShortcut( cMovePawnSequence );
      pcMenu->addAction( pcMovePawnAction );
      connect( pcMovePawnAction, SIGNAL( triggered() ),
               m_pcUI->mapWidget, SLOT( onMovePawn() ) );

      QAction* pcDeletePawnAction{ new QAction( "Delete Pawn", pcMenu ) };
      const QKeySequence cDeletePawnSequence( Qt::Key_Delete );
      pcDeletePawnAction->setShortcut( cDeletePawnSequence );
      pcMenu->addAction( pcDeletePawnAction );
      connect( pcDeletePawnAction, SIGNAL( triggered() ),
               m_pcUI->mapWidget, SLOT( onDeleteTopPawn() ) );

      pcMenu->addSeparator();

      QAction* pcCopyPropertiesAction{ new QAction( "Copy Pawn Properties", pcMenu ) };
      const QKeySequence cCopyPropertiesSequence( Qt::Key_C );
      pcCopyPropertiesAction->setShortcut( cCopyPropertiesSequence );
      pcMenu->addAction( pcCopyPropertiesAction );
      connect( pcCopyPropertiesAction, SIGNAL( triggered() ),
               this, SLOT( OnCopyPawnProperties() ) );

      QAction* pcPastePropertiesAction{ new QAction( "Paste Pawn Properties", pcMenu ) };
      const QKeySequence cPastePropertiesSequence( Qt::Key_V );
      pcPastePropertiesAction->setShortcut( cPastePropertiesSequence );
      pcMenu->addAction( pcPastePropertiesAction );
      pcPastePropertiesAction->setEnabled( !m_cPropertyClipboard.empty() );
      connect( pcPastePropertiesAction, SIGNAL( triggered() ),
               this, SLOT( OnPastePawnProperties() ) );

      pcMenu->addSeparator();

      QAction* pcShiftPawnUpAction{ new QAction( "Shift Pawns Up", pcMenu ) };
      const QKeySequence cShiftPawnUpSequence( Qt::Key_Minus );
      pcShiftPawnUpAction->setShortcut( cShiftPawnUpSequence );
      pcMenu->addAction( pcShiftPawnUpAction );
      connect( pcShiftPawnUpAction, SIGNAL( triggered() ),
               m_pcUI->mapWidget, SLOT( onShiftPawnsUp() ) );

      QAction* pcShiftPawnDownAction{ new QAction( "Shift Pawns Down", pcMenu ) };
      const QKeySequence cShiftPawnDownSequence( Qt::Key_Plus );
      pcShiftPawnDownAction->setShortcut( cShiftPawnDownSequence );
      pcMenu->addAction( pcShiftPawnDownAction );
      connect( pcShiftPawnDownAction, SIGNAL( triggered() ),
               m_pcUI->mapWidget, SLOT( onShiftPawnsDown() ) );

      pcMenu->addSeparator();
    }
  }

  pcMenu->addAction( m_pcUI->actionSave );
  pcMenu->addAction( m_pcUI->actionSettings );

  pcMenu->exec( mapToGlobal( i_rcPos ) );
}


bool MapEditor::Open( const QString& i_strFilePath )
{
  // Open script file
  QFile cFile( i_strFilePath );
  if( !cFile.open( QIODevice::ReadWrite | QIODevice::Text ) )
  {
    QMessageBox::information( this, tr( "Unable to open file" ), cFile.errorString() );
    return false;
  }

  return Load( i_strFilePath );
}


void MapEditor::PastePawnProperties( rumPawn* i_pcPawn )
{
  if( i_pcPawn )
  {
    for( const auto& iter : m_cPropertyClipboard )
    {
      const rumAssetID ePropertyID{ iter.first };
      Sqrat::Object sqProperty{ iter.second };
      i_pcPawn->SetProperty( ePropertyID, sqProperty );
    }
  }
}


bool MapEditor::RequestClose()
{
  bool bClose{ true };

  if( IsDirty() )
  {
    const QMessageBox::StandardButton cButton
    {
      QMessageBox::question( this, "Save Changes?", "Save changes before closing?",
                             QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                             QMessageBox::Yes )
    };

    if( cButton == QMessageBox::Yes )
    {
      Save();
    }
    else if( cButton == QMessageBox::Cancel )
    {
      bClose = false;
    }
  }

  return bClose;
}


void MapEditor::Save()
{
  m_pcUI->mapWidget->SaveMap( m_strFilePath );
}


// slot
void MapEditor::selectionChanged_Property( const QItemSelection& i_rcSelected, const QItemSelection& i_rcDeselected )
{
  const bool bPropertySelected{ m_pcUI->tableWidget_3->rowCount() > 0 && m_pcUI->tableWidget_3->currentRow() >= 0 };
  m_pcUI->actionRemove_Property->setEnabled( bPropertySelected );
}


// slot
void MapEditor::StringTokenPropertyChanged( rumTokenID i_eTokenID )
{
  const int32_t iRow{ m_pcUI->tableWidget_3->currentRow() };
  const int32_t iCol{ m_pcUI->tableWidget_3->currentColumn() };

  // Fetch the asset from the affected row
  const rumPropertyAsset* pcProperty{ rumPropertyAsset::Fetch( GetPropertyID( iRow ) ) };
  rumAssert( pcProperty );
  if( !pcProperty )
  {
    return;
  }

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

  Sqrat::Object sqValue;
  rumScript::SetValue( sqValue, i_eTokenID );

  m_pcCurrentPawn->SetProperty( pcProperty->GetAssetID(), sqValue );

  m_pcUI->mapWidget->MarkDirty();
}


void MapEditor::UpdateAssetFilter()
{
  // Hide anything that doesn't pass the filter
  const QString& strFilter{ m_cAssetFilterEdit.text() };

  // Visit each item in the table and hide anything that doesn't match the filter settings
  const int32_t iRow{ m_pcUI->tableWidget->rowCount() };
  for( int32_t i{ 0 }; i < iRow; ++i )
  {
    bool bShow{ true };

    if( !strFilter.isEmpty() )
    {
      const QTableWidgetItem* pcItem{ m_pcUI->tableWidget->item( i, ASSET_COL_NAME ) };
      bShow = pcItem->text().contains( strFilter, Qt::CaseInsensitive );

      if( !bShow )
      {
        pcItem = m_pcUI->tableWidget->item( i, ASSET_COL_VALUE );
        bShow = pcItem->text().contains( strFilter, Qt::CaseInsensitive );
      }
    }

    bShow ? m_pcUI->tableWidget->showRow( i ) : m_pcUI->tableWidget->hideRow( i );
  }

  m_pcUI->tableWidget->resizeColumnToContents( ASSET_COL_NAME );
  m_pcUI->tableWidget->resizeRowsToContents();
}


void MapEditor::UpdatePropertyFilter()
{
  // Hide anything that doesn't pass the filter
  const QString& strFilter{ m_cPropertyFilterEdit.text() };

  // Visit each item in the table and hide anything that doesn't match the filter settings
  const int32_t iRow{ m_pcUI->tableWidget_3->rowCount() };
  for( int32_t i{ 0 }; i < iRow; ++i )
  {
    bool bShow{ true };

    if( !strFilter.isEmpty() )
    {
      const QTableWidgetItem* pcItem{ m_pcUI->tableWidget_3->item( i, PROP_COL_NAME ) };
      bShow = pcItem->text().contains( strFilter, Qt::CaseInsensitive );

      if( !bShow )
      {
        pcItem = m_pcUI->tableWidget_3->item( i, PROP_COL_VALUE );
        bShow = pcItem->text().contains( strFilter, Qt::CaseInsensitive );
      }
    }

    bShow ? m_pcUI->tableWidget_3->showRow( i ) : m_pcUI->tableWidget_3->hideRow( i );
  }

  m_pcUI->tableWidget_3->resizeColumnToContents( PROP_COL_NAME );
  m_pcUI->tableWidget_3->resizeRowsToContents();
}
