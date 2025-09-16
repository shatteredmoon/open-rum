#include <assetmanager.h>
#include <ui_assetmanager.h>

#include <mainwindow.h>

#include <QCloseEvent>
#include <QComboBox>
#include <QDirIterator>
#include <QFileDialog>
#include <QItemSelection>
#include <QLineEdit>
#include <QMessageBox>

#include <assetpicker.h>
#include <stringtokenpicker.h>

#include <e_map.h>

#include <u_broadcast_asset.h>
#include <u_creature_asset.h>
#include <u_custom_asset.h>
#include <u_enum.h>
#include <u_graphic_asset.h>
#include <u_inventory_asset.h>
#include <u_map_asset.h>
#include <u_portal_asset.h>
#include <u_sound_asset.h>
#include <u_tile_asset.h>
#include <u_widget_asset.h>

enum AssetColumns
{
  ASSET_COL_NAME = 0
};

enum AssetTypeColumnsRoles
{
  ROLE_ASSET_TYPE = Qt::UserRole,
  ROLE_ID         = Qt::UserRole + 1
};

enum AssetTableColumns
{
  S_COL_NAME             = 0,
  S_COL_CLASS            = 1,
                         
  S_COL_FILENAME         = 2,
                         
  S_COL_GRAPHIC_FRAMES   = 3,
  S_COL_GRAPHIC_STATES   = 4,
  S_COL_GRAPHIC_ANIMTYPE = 5,
  S_COL_GRAPHIC_INTERVAL = 6,
  S_COL_GRAPHIC_RENDER   = 7,
                         
  S_COL_INV_REPLICATION  = 2,
  S_COL_INV_PERSISTENT   = 3,
                         
  S_COL_PAWN_GRAPHIC     = 2,
  S_COL_PAWN_MOVE_TYPE   = 3,
  S_COL_PAWN_LOS         = 4,
  S_COL_PAWN_COL_FLAGS   = 5,
  S_COL_PAWN_LIGHT_RANGE = 6,
  S_COL_PAWN_DRAW_ORDER  = 7,
  S_COL_PAWN_SERVICE     = 8,

  S_COL_TILE_GRAPHIC     = 2,
  S_COL_TILE_WEIGHT      = 3,
  S_COL_TILE_LOS         = 4,
  S_COL_TILE_COL_FLAGS   = 5
};

enum PropertyTableColumns
{
  PROP_COL_NAME   = 0,
  PROP_COL_VALUE  = 1
};


AssetManager::AssetManager( QWidget* i_pcParent )
  : QMainWindow( i_pcParent )
  , m_pcFilterAction( nullptr )
  , m_pcUI( new Ui::AssetManager )
{
  m_pcUI->setupUi( this );
  setCentralWidget( m_pcUI->splitter );

  QIcon cAssetIcon( ":/ui/resources/asset.png" );
  setWindowIcon( cAssetIcon );

  setMinimumWidth( 1024 );
  setMinimumHeight( 768 );

  // Asset table ---------------------------------------------------------

  // Set up the string database widget
  QStringList cLabelList;
  cLabelList << "Asset Table";

  m_pcUI->tableWidget->setMinimumWidth( 150 );
  m_pcUI->tableWidget->setColumnCount( cLabelList.size() );
  m_pcUI->tableWidget->setRowCount( 0 );
  m_pcUI->tableWidget->setHorizontalHeaderLabels( cLabelList );

  m_pcUI->tableWidget->setContextMenuPolicy( Qt::CustomContextMenu );

  // Add all asset types
  AddTable( Broadcast_AssetType, "Broadcast" );
  AddTable( Creature_AssetType,  "Creature" );
  AddTable( Custom_AssetType,    "Custom" );
  AddTable( Graphic_AssetType,   "Graphic" );
  AddTable( Inventory_AssetType, "Inventory" );
  AddTable( Map_AssetType,       "Map" );
  AddTable( Portal_AssetType,    "Portal" );
  AddTable( Sound_AssetType,     "Sound" );
  AddTable( Tile_AssetType,      "Tile" );
  AddTable( Widget_AssetType,    "Widget" );

  m_pcUI->tableWidget->resizeColumnsToContents();

  // Single selections only
  m_pcUI->tableWidget->setSelectionBehavior( QAbstractItemView::SelectRows );
  m_pcUI->tableWidget->setSelectionMode( QAbstractItemView::SingleSelection );

  connect( m_pcUI->tableWidget->selectionModel(),
           SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
           this, SLOT( selectionChanged_AssetType( const QItemSelection&, const QItemSelection& ) ) );

  // Asset table -----------------------------------------------------------

  cLabelList.clear();
  cLabelList << "Name" << "Base Class";

  m_pcUI->tableWidget_3->setMinimumWidth( 200 );
  m_pcUI->tableWidget_3->setEnabled( false );
  m_pcUI->tableWidget_3->setColumnCount( cLabelList.size() );
  m_pcUI->tableWidget_3->setRowCount( 0 );
  m_pcUI->tableWidget_3->setHorizontalHeaderLabels( cLabelList );

  m_pcUI->tableWidget_3->setContextMenuPolicy( Qt::CustomContextMenu );

  m_pcUI->toolBar->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );

  QIcon cAddIcon( ":/ui/resources/add.png" );
  m_pcUI->actionNew_Asset->setIcon( cAddIcon );
  m_pcUI->actionNew_Asset->setEnabled( false );

  QIcon cRemoveIcon( ":/ui/resources/remove.png" );
  m_pcUI->actionRemove_Asset->setIcon( cRemoveIcon );
  m_pcUI->actionRemove_Asset->setEnabled( false );

  // Add a line edit on the toolbar for filtering
  m_cFilterLabel.setText( "Filter: " );
  m_pcUI->toolBar2->addWidget( &m_cFilterLabel );
  m_pcFilterAction = m_pcUI->toolBar2->addWidget( &m_cFilterEdit );
  m_cFilterLabel.setEnabled( false );
  m_cFilterEdit.setEnabled( false );
  connect( &m_cFilterEdit, SIGNAL( editingFinished() ), this, SLOT( onFilterChanged() ) );

  connect( m_pcUI->tableWidget_3->selectionModel(),
           SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
           this, SLOT( selectionChanged_Asset( const QItemSelection&, const QItemSelection& ) ) );

  connect( m_pcUI->tableWidget_3, SIGNAL( cellActivated( int32_t, int32_t ) ),
           this, SLOT( OnCellStartEdit( int32_t, int32_t ) ) );

  // Seems like cellActivated works for DoubleClicked as well?
  //connect( ui->tableWidget_3, SIGNAL( cellDoubleClicked( int32_t, int32_t ) ),
  //         this, SLOT( OnCellStartEdit( int32_t, int32_t ) ) );

  // Property Table --------------------------------------------------------

  cLabelList.clear();
  cLabelList << "Name" << "Value";

  m_pcUI->tableWidget_4->setMinimumWidth( 200 );
  m_pcUI->tableWidget_4->setEnabled( false );
  m_pcUI->tableWidget_4->setColumnCount( cLabelList.size() );
  m_pcUI->tableWidget_4->setRowCount( 0 );
  m_pcUI->tableWidget_4->setHorizontalHeaderLabels( cLabelList );

  m_pcUI->tableWidget_4->setContextMenuPolicy( Qt::CustomContextMenu );

  m_pcUI->actionNew_Property->setIcon( cAddIcon );
  m_pcUI->actionNew_Property->setEnabled( false );

  m_pcUI->actionRemove_Property->setIcon( cRemoveIcon );
  m_pcUI->actionRemove_Property->setEnabled( false );

  // Add a line edit on the toolbar for filtering
  m_cPropertyFilterLabel.setText( "Filter: " );
  m_pcUI->toolBar3->addWidget( &m_cPropertyFilterLabel );
  m_pcPropertyFilterAction = m_pcUI->toolBar3->addWidget( &m_cPropertyFilterEdit );
  m_cPropertyFilterLabel.setEnabled( false );
  m_cPropertyFilterEdit.setEnabled( false );
  connect( &m_cPropertyFilterEdit, SIGNAL( editingFinished() ), this, SLOT( onPropertyFilterChanged() ) );

  connect( m_pcUI->tableWidget_4->selectionModel(),
           SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
           this, SLOT( selectionChanged_Property( const QItemSelection&, const QItemSelection& ) ) );
  
  //connect( m_pcUI->tableWidget_4, SIGNAL( cellActivated( int32_t, int32_t ) ),
  //         this, SLOT( OnCellStartPropertyEdit( int32_t, int32_t ) ) );
  // Seems like cellActivated works for DoubleClicked as well?
  connect( m_pcUI->tableWidget_4, SIGNAL( cellDoubleClicked( int32_t, int32_t ) ),
           this, SLOT( OnCellStartPropertyEdit( int32_t, int32_t ) ) );

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

  QIcon cSaveIcon( ":/ui/resources/save.png" );
  m_pcUI->actionSave->setIcon( cSaveIcon );
  m_pcUI->actionSave->setEnabled( false );
}


AssetManager::~AssetManager()
{
  delete m_pcUI;
}


void AssetManager::AddAsset( int32_t i_iRow, const rumAsset& i_rcAsset )
{
  if( m_pcUI->tableWidget_3->rowCount() < i_iRow + 1 )
  {
    m_pcUI->tableWidget_3->setRowCount( i_iRow + 1 );
  }

  // Name
  QTableWidgetItem* pcCell{ new QTableWidgetItem() };
  pcCell->setText( i_rcAsset.GetName().c_str() );
  pcCell->setData( ROLE_ID, QVariant::fromValue( i_rcAsset.GetAssetID() ) );
  pcCell->setToolTip( rumStringUtils::ToHexString( i_rcAsset.GetAssetID() ) );
  m_pcUI->tableWidget_3->setItem( i_iRow, S_COL_NAME, pcCell );

  // Base class
  pcCell = new QTableWidgetItem( i_rcAsset.GetBaseClassOverride().c_str() );
  m_pcUI->tableWidget_3->setItem( i_iRow, S_COL_CLASS, pcCell );
}


void AssetManager::AddFileAsset( int32_t i_iRow, const rumFileAsset& i_rcAsset )
{
  AddAsset( i_iRow, i_rcAsset );

  // Filename
  QTableWidgetItem* pcCell{ new QTableWidgetItem( i_rcAsset.GetFilename().c_str() ) };
  m_pcUI->tableWidget_3->setItem( i_iRow, S_COL_FILENAME, pcCell );
  pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
}


void AssetManager::AddGraphicAsset( int32_t i_iRow, const rumGraphicAsset& i_rcAsset )
{
  AddFileAsset( i_iRow, i_rcAsset );

  // Num animation frames
  QTableWidgetItem* pcCell{ new QTableWidgetItem( rumStringUtils::ToString( i_rcAsset.GetNumAnimFrames() ) ) };
  m_pcUI->tableWidget_3->setItem( i_iRow, S_COL_GRAPHIC_FRAMES, pcCell );
  pcCell->setFlags( pcCell->flags() );

  // Num animation states
  pcCell = new QTableWidgetItem( rumStringUtils::ToString( i_rcAsset.GetNumAnimStates() ) );
  m_pcUI->tableWidget_3->setItem( i_iRow, S_COL_GRAPHIC_STATES, pcCell );
  pcCell->setFlags( pcCell->flags() );

  // Animation type
  QString strAnimType;
  switch( i_rcAsset.GetAnimType() )
  {
    case rumAnimationType::StandardLooping_AnimationType: strAnimType = "Standard Looping"; break;
    case rumAnimationType::StandardOnce_AnimationType: strAnimType = "Standard Once"; break;
    case rumAnimationType::Random_AnimationType: strAnimType = "Random Frame"; break;
    case rumAnimationType::Custom_AnimationType: strAnimType = "Custom"; break;
    default: strAnimType = "Unknown"; break;
  }
  pcCell = new QTableWidgetItem( strAnimType );
  m_pcUI->tableWidget_3->setItem( i_iRow, S_COL_GRAPHIC_ANIMTYPE, pcCell );
  pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
  pcCell->setData( ROLE_ID, QVariant::fromValue( static_cast<uint32_t>( i_rcAsset.GetAnimType() ) ) );

  // Animation interval
  pcCell = new QTableWidgetItem( rumStringUtils::ToFloatString( i_rcAsset.GetAnimInterval() ) );
  m_pcUI->tableWidget_3->setItem( i_iRow, S_COL_GRAPHIC_INTERVAL, pcCell );
  pcCell->setFlags( pcCell->flags() );

  // Client rendered
  pcCell = new QTableWidgetItem( i_rcAsset.IsClientRendered() ? "True" : "False" );
  m_pcUI->tableWidget_3->setItem( i_iRow, S_COL_GRAPHIC_RENDER, pcCell );
  pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
}


void AssetManager::AddInventoryAsset( int32_t i_iRow, const rumInventoryAsset& i_rcAsset )
{
  AddAsset( i_iRow, i_rcAsset );

  // Replication type
  QString strReplicationType;
  switch( i_rcAsset.GetClientReplicationType() )
  {
    case None_ClientReplicationType:     strReplicationType = "None";     break;
    case Private_ClientReplicationType:  strReplicationType = "Private";  break;
    case Regional_ClientReplicationType: strReplicationType = "Regional"; break;
    case Global_ClientReplicationType:   strReplicationType = "Global";   break;
  }
  QTableWidgetItem* pcCell{ new QTableWidgetItem() };
  pcCell->setData( Qt::DisplayRole, strReplicationType );
  pcCell->setData( Qt::UserRole, i_rcAsset.GetClientReplicationType() );
  pcCell->setToolTip( rumStringUtils::ToString( i_rcAsset.GetClientReplicationType() ) );
  m_pcUI->tableWidget_3->setItem( i_iRow, S_COL_INV_REPLICATION, pcCell );
  pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );

  // Persistent
  pcCell = new QTableWidgetItem( i_rcAsset.IsPersistent() ? "True" : "False" );
  m_pcUI->tableWidget_3->setItem( i_iRow, S_COL_INV_PERSISTENT, pcCell );
  pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
}


void AssetManager::AddMapAsset( int32_t i_iRow, const rumMapAsset& i_rcAsset )
{
  AddFileAsset( i_iRow, i_rcAsset );
}


void AssetManager::AddPawnAsset( int32_t i_iRow, const rumPawnAsset& i_rcAsset )
{
  AddAsset( i_iRow, i_rcAsset );

  QString strName;

  // Note that newly created assets will not have a valid graphic ID - and that's okay!
  rumAsset* pcGraphic{ rumGraphicAsset::Fetch( i_rcAsset.GetGraphicID() ) };
  if( pcGraphic != nullptr )
  {
    strName = pcGraphic->GetName().c_str();
  }

  // Graphic ID
  QTableWidgetItem* pcCell{ new QTableWidgetItem() };
  pcCell->setData( Qt::DisplayRole, strName );
  pcCell->setData( Qt::UserRole, i_rcAsset.GetGraphicID() );
  pcCell->setToolTip( rumStringUtils::ToHexString( i_rcAsset.GetGraphicID() ) );
  m_pcUI->tableWidget_3->setItem( i_iRow, S_COL_PAWN_GRAPHIC, pcCell );
  pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );

  // Move flags
  std::string strFlags{ rumScript::EnumToString( "MoveType", i_rcAsset.GetMoveType(), true /* bitfield */ ) };
  pcCell = new QTableWidgetItem();
  pcCell->setData( Qt::DisplayRole, strFlags.c_str() );
  pcCell->setData( Qt::UserRole, i_rcAsset.GetMoveType() );
  pcCell->setToolTip( rumStringUtils::ToHexString( i_rcAsset.GetMoveType() ) );
  m_pcUI->tableWidget_3->setItem( i_iRow, S_COL_PAWN_MOVE_TYPE, pcCell );
  pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );

  // Blocks LOS
  pcCell = new QTableWidgetItem( i_rcAsset.GetBlocksLOS() ? "True" : "False" );
  m_pcUI->tableWidget_3->setItem( i_iRow, S_COL_PAWN_LOS, pcCell );
  pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );

  // Collision flags
  strFlags = rumScript::EnumToString( "MoveType", i_rcAsset.GetCollisionFlags(), true /* bitfield */ );
  pcCell = new QTableWidgetItem();
  pcCell->setData( Qt::DisplayRole, strFlags.c_str() );
  pcCell->setData( Qt::UserRole, i_rcAsset.GetCollisionFlags() );
  pcCell->setToolTip( rumStringUtils::ToHexString( i_rcAsset.GetCollisionFlags() ) );
  m_pcUI->tableWidget_3->setItem( i_iRow, S_COL_PAWN_COL_FLAGS, pcCell );
  pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );

  // Light range
  pcCell = new QTableWidgetItem( rumStringUtils::ToString( i_rcAsset.GetLightRange() ) );
  m_pcUI->tableWidget_3->setItem( i_iRow, S_COL_PAWN_LIGHT_RANGE, pcCell );

  // Render priority
  pcCell = new QTableWidgetItem( rumStringUtils::ToString( i_rcAsset.GetDrawOrder() ) );
  m_pcUI->tableWidget_3->setItem( i_iRow, S_COL_PAWN_DRAW_ORDER, pcCell );

  // Service type
  QString strServiceType;
  switch( i_rcAsset.GetServiceType() )
  {
    case Shared_ServiceType: strServiceType = "Shared"; break;
    case Server_ServiceType: strServiceType = "Server"; break;
    case Client_ServiceType: strServiceType = "Client"; break;
  }
  pcCell = new QTableWidgetItem();
  pcCell->setData( Qt::DisplayRole, strServiceType );
  pcCell->setData( Qt::UserRole, i_rcAsset.GetServiceType() );
  pcCell->setToolTip( rumStringUtils::ToString( i_rcAsset.GetServiceType() ) );
  m_pcUI->tableWidget_3->setItem( i_iRow, S_COL_PAWN_SERVICE, pcCell );
  pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
}


void AssetManager::AddPropertyAsset( int32_t i_iRow, const rumPropertyAsset& i_rcAsset, Sqrat::Object i_sqValue )
{
  if( m_pcUI->tableWidget_4->rowCount() < i_iRow + 1 )
  {
    m_pcUI->tableWidget_4->setRowCount( i_iRow + 1 );
  }

  // Name
  QTableWidgetItem* pcCell{ new QTableWidgetItem };
  pcCell->setText( i_rcAsset.GetName().c_str() );
  pcCell->setData( ROLE_ID, QVariant::fromValue( i_rcAsset.GetAssetID() ) );
  pcCell->setToolTip( rumStringUtils::ToHexString( i_rcAsset.GetAssetID() ) );
  m_pcUI->tableWidget_4->setItem( i_iRow, PROP_COL_NAME, pcCell );
  pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );

  // Value
  pcCell = CreateTableWidgetItemFromProperty( i_rcAsset, i_sqValue );
  if( pcCell )
  {
    m_pcUI->tableWidget_4->setItem( i_iRow, PROP_COL_VALUE, pcCell );
  }
}


void AssetManager::AddTable( AssetType i_eAssetType, const QString& i_strName )
{
  const int32_t iRow{ m_pcUI->tableWidget->rowCount() };
  m_pcUI->tableWidget->setRowCount( iRow + 1 );

  // Name column
  QTableWidgetItem* pcCell{ new QTableWidgetItem() };
  pcCell->setData( Qt::DisplayRole, i_strName );
  pcCell->setData( ROLE_ASSET_TYPE, QVariant::fromValue( (int32_t)i_eAssetType ) );
  m_pcUI->tableWidget->setItem( iRow, ASSET_COL_NAME, pcCell );
  pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
}


void AssetManager::AddTileAsset( int32_t i_iRow, const rumTileAsset& i_rcAsset )
{
  AddAsset( i_iRow, i_rcAsset );

  QString strName;

  // Note that newly created assets will not have a valid graphic ID - and that's okay!
  const rumAsset* pcGraphic{ rumGraphicAsset::Fetch( i_rcAsset.GetGraphicID() ) };
  if( pcGraphic != nullptr )
  {
    strName = pcGraphic->GetName().c_str();
  }

  // Graphic ID
  QTableWidgetItem* pcCell{ new QTableWidgetItem() };
  pcCell->setData( Qt::DisplayRole, strName );
  pcCell->setData( Qt::UserRole, i_rcAsset.GetGraphicID() );
  pcCell->setToolTip( rumStringUtils::ToHexString( i_rcAsset.GetGraphicID() ) );
  m_pcUI->tableWidget_3->setItem( i_iRow, S_COL_TILE_GRAPHIC, pcCell );
  pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );

  // Weight
  pcCell = new QTableWidgetItem( rumStringUtils::ToFloatString( i_rcAsset.GetWeight() ) );
  m_pcUI->tableWidget_3->setItem( i_iRow, S_COL_TILE_WEIGHT, pcCell );

  // Blocks LOS
  pcCell = new QTableWidgetItem( i_rcAsset.GetBlocksLOS() ? "True" : "False" );
  m_pcUI->tableWidget_3->setItem( i_iRow, S_COL_TILE_LOS, pcCell );
  pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );

  // Collision flags
  const std::string strFlags{ rumScript::EnumToString( "MoveType", i_rcAsset.GetCollisionFlags(),
                              true /* bitfield */ ) };
  pcCell = new QTableWidgetItem();
  pcCell->setData( Qt::DisplayRole, strFlags.c_str() );
  pcCell->setData( Qt::UserRole, i_rcAsset.GetCollisionFlags() );
  pcCell->setToolTip( rumStringUtils::ToHexString( i_rcAsset.GetCollisionFlags() ) );
  m_pcUI->tableWidget_3->setItem( i_iRow, S_COL_TILE_COL_FLAGS, pcCell );
  pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
}


void AssetManager::AssetLineEditFinished()
{
  const int32_t iCol{ m_pcUI->tableWidget_3->currentColumn() };

  if( iCol == ASSET_COL_NAME )
  {
    const int32_t iRow{ m_pcUI->tableWidget_3->currentRow() };

    const QWidget* pcWidget{ m_pcUI->tableWidget_3->cellWidget( iRow, iCol ) };
    if( pcWidget )
    {
      const QLineEdit* pcLineEdit{ qobject_cast<const QLineEdit*>( pcWidget ) };
      if( pcLineEdit )
      {
        disconnect( pcLineEdit, SIGNAL( editingFinished() ),
                    this, SLOT( AssetLineEditFinished() ) );

        const QString& strName{ pcLineEdit->text() };
        if( IsAssetNameUnique( strName ) )
        {
          QTableWidgetItem* pcItem{ m_pcUI->tableWidget_3->item( iRow, iCol ) };
          if( pcItem )
          {
            pcItem->setText( strName );
            SetDirty( true );
          }
        }
        else
        {
          const QString strError{ "Asset names must be unique" };
          QMessageBox::critical( this, tr( "Error" ), strError );
        }
      }

      m_pcUI->tableWidget_3->removeCellWidget( iRow, iCol );
    }
  }
}


void AssetManager::closeEvent( QCloseEvent* i_pcEvent )
{
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


QTableWidgetItem* AssetManager::CreateTableWidgetItemFromProperty( const rumPropertyAsset& i_rcAsset,
                                                                   Sqrat::Object i_sqValue ) const
{
  // Value
  QTableWidgetItem* pcCell{ new QTableWidgetItem() };
  QString strValue;
  Sqrat::Object sqValue{ i_sqValue };
  if( i_rcAsset.IsAssetRef() )
  {
    const rumAsset* pcAsset{ rumAsset::Fetch( sqValue.Cast<int32_t>() ) };
    rumAssert( pcAsset );
    if( pcAsset )
    {
      strValue = pcAsset->GetName().c_str();

      pcCell->setData( Qt::UserRole, sqValue.Cast<int32_t>() );
    }
    else
    {
      pcCell->setData( Qt::UserRole, 0 );
    }
  }
  else if( i_rcAsset.GetValueType() == PropertyValueType::StringToken )
  {
    const auto eTokenID{ sqValue.GetType() == OT_NULL ? rumStringTable::INVALID_TOKEN_ID : sqValue.Cast<rumTokenID>() };
    strValue = rumStringTable::GetTokenName( eTokenID ).c_str();
    pcCell->setData( Qt::UserRole, eTokenID );
  }
  else if( !i_rcAsset.GetEnumName().empty() )
  {
    const int32_t iValue{ sqValue.Cast<int32_t>() };
    strValue = rumScript::EnumToString( i_rcAsset.GetEnumName(), iValue,
                                        i_rcAsset.GetValueType() == PropertyValueType::Bitfield ).c_str();
    pcCell->setData( Qt::UserRole, iValue );
    if( i_rcAsset.GetValueType() == PropertyValueType::Bitfield )
    {
      pcCell->setToolTip( rumStringUtils::ToHexString( iValue ) );
    }
    else
    {
      pcCell->setToolTip( rumStringUtils::ToString( iValue ) );
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

  pcCell->setData( Qt::DisplayRole, strValue );

  if( i_rcAsset.IsAssetRef() || ( i_rcAsset.GetValueType() == PropertyValueType::Bool ) ||
      !i_rcAsset.GetEnumName().empty() )
  {
    pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
  }

  return pcCell;
}


rumAsset* AssetManager::GetAsset( int32_t i_iRow ) const
{
  return rumAsset::Fetch( GetAssetID( i_iRow ) );
}


rumAssetID AssetManager::GetAssetID( int32_t i_iRow ) const
{
  if( -1 == i_iRow )
  {
    return INVALID_ASSET_ID;
  }

  const QTableWidgetItem* pcItem{ m_pcUI->tableWidget_3->item( i_iRow, S_COL_NAME ) };
  Q_ASSERT( pcItem );
  return pcItem ? (rumAssetID)pcItem->data( ROLE_ID ).toInt() : INVALID_ASSET_ID;
}


QString AssetManager::GetAssetName( int32_t i_iRow ) const
{
  QTableWidgetItem* pcItem{ nullptr };

  if( i_iRow <= m_pcUI->tableWidget_3->rowCount() )
  {
    pcItem = m_pcUI->tableWidget_3->item( i_iRow, S_COL_NAME );
  }

  return pcItem ? pcItem->text() : QString();
}


AssetType AssetManager::GetAssetType( int32_t i_iRow ) const
{
  QTableWidgetItem* pcItem{ m_pcUI->tableWidget->item( i_iRow, ASSET_COL_NAME ) };
  Q_ASSERT( pcItem );
  return (AssetType)pcItem->data( ROLE_ASSET_TYPE ).toInt();
}


rumAssetID AssetManager::GetPropertyID( int32_t i_iRow ) const
{
  QTableWidgetItem* pcItem{ nullptr };

  if( i_iRow <= m_pcUI->tableWidget_4->rowCount() )
  {
    pcItem = m_pcUI->tableWidget_4->item( i_iRow, PROP_COL_NAME );
  }

  return pcItem ? (rumAssetID)pcItem->data( ROLE_ID ).toInt() : INVALID_ASSET_ID;
}


rumAsset* AssetManager::GetSelectedAsset() const
{
  const int32_t iRow{ m_pcUI->tableWidget_3->currentRow() };
  return GetAsset( iRow );
}


AssetType AssetManager::GetSelectedAssetType() const
{
  return GetAssetType( m_pcUI->tableWidget->currentRow() );
}


rumPropertyContainer::PropertyContainer AssetManager::GetSelectedProperties() const
{
  PropertyContainer cProperties;

  const auto& rcAsset{ GetSelectedAsset() };

  const auto& rcList{ m_pcUI->tableWidget_4->selectedItems() };
  for( const auto& iter : rcList )
  {
    const auto uiPropertyID{ GetPropertyID( iter->row() ) };
    cProperties.insert( { uiPropertyID, rcAsset->GetProperty( uiPropertyID ) } );
  }

  return cProperties;
}


Sqrat::Object AssetManager::GetSelectedProperty() const
{
  rumAsset* pcAsset{ GetSelectedAsset() };
  return pcAsset ? pcAsset->GetProperty( GetSelectedPropertyID() ) : Sqrat::Object();
}


rumAssetID AssetManager::GetSelectedPropertyID() const
{
  return GetPropertyID( m_pcUI->tableWidget_4->currentRow() );
}


void AssetManager::HandleAssetChange( rumAsset* i_pcAsset, QTableWidgetItem* i_pcItem )
{
  rumAssert( i_pcAsset );
  if( !i_pcAsset )
  {
    return;
  }

  const int32_t iCol{ m_pcUI->tableWidget_3->currentColumn() };
  switch( iCol )
  {
    case S_COL_NAME:
      i_pcAsset->SetName( qPrintable( i_pcItem->text() ) );
      break;

    case S_COL_CLASS:
      i_pcAsset->SetBaseClassOverride( qPrintable( i_pcItem->text() ) );
      break;

    default:
      rumAssertMsg( false, "Unexpected column modified" );
      break;
  }
}


void AssetManager::HandleFileAssetChange( rumFileAsset* i_pcAsset, QTableWidgetItem* i_pcItem )
{
  rumAssert( i_pcAsset );
  if( !i_pcAsset )
  {
    return;
  }

  const int32_t iCol{ m_pcUI->tableWidget_3->currentColumn() };
  switch( iCol )
  {
    case S_COL_NAME:
    case S_COL_CLASS:
      HandleAssetChange( i_pcAsset, i_pcItem );
      break;

    case S_COL_FILENAME:
      i_pcAsset->SetFilename( qPrintable( i_pcItem->text() ) );
      break;

    default:
      rumAssertMsg( false, "Unexpected column modified" );
  }
}


void AssetManager::HandleGraphicAssetChange( rumGraphicAsset* i_pcAsset, QTableWidgetItem* i_pcItem )
{
  rumAssert( i_pcAsset );
  if( !i_pcAsset )
  {
    return;
  }

  const int32_t iCol{ m_pcUI->tableWidget_3->currentColumn() };
  switch( iCol )
  {
    case S_COL_NAME:
    case S_COL_CLASS:
      HandleAssetChange( i_pcAsset, i_pcItem );
      break;

    case S_COL_FILENAME:
      HandleFileAssetChange( i_pcAsset, i_pcItem );
      break;

    case S_COL_GRAPHIC_FRAMES:
      i_pcAsset->SetNumAnimFrames( rumStringUtils::ToUInt( qPrintable( i_pcItem->text() ) ) );
      break;

    case S_COL_GRAPHIC_STATES:
      i_pcAsset->SetNumAnimStates( rumStringUtils::ToUInt( qPrintable( i_pcItem->text() ) ) );
      break;

    case S_COL_GRAPHIC_ANIMTYPE:
    {
      const QVariant cVariant{ i_pcItem->data( Qt::UserRole ) };
      i_pcAsset->SetAnimType( static_cast<rumAnimationType>( cVariant.toUInt() ) );
      break;
    }

    case S_COL_GRAPHIC_INTERVAL:
      i_pcAsset->SetAnimInterval( rumStringUtils::ToFloat( qPrintable( i_pcItem->text() ) ) );
      break;

    case S_COL_GRAPHIC_RENDER:
      i_pcAsset->SetClientRendered( rumStringUtils::ToBool( qPrintable( i_pcItem->text() ) ) );
      break;

    default:
      rumAssertMsg( false, "Unexpected column modified" );
  }
}


void AssetManager::HandleInventoryAssetChange( rumInventoryAsset* i_pcAsset, QTableWidgetItem* i_pcItem )
{
  rumAssert( i_pcAsset );
  if( !i_pcAsset )
  {
    return;
  }

  const int32_t iCol{ m_pcUI->tableWidget_3->currentColumn() };
  switch( iCol )
  {
    case S_COL_NAME:
    case S_COL_CLASS:
      HandleAssetChange( i_pcAsset, i_pcItem );
      break;

    case S_COL_INV_REPLICATION:
    {
      const QVariant cVariant{ i_pcItem->data( Qt::UserRole ) };
      i_pcAsset->SetClientReplicationType( ClientReplicationType( cVariant.toInt() ) );
      break;
    }

    case S_COL_INV_PERSISTENT:
      i_pcAsset->SetPersistence( rumStringUtils::ToBool( qPrintable( i_pcItem->text() ) ) );
      break;

    default:
      rumAssertMsg( false, "Unexpected column modified" );
  }
}


void AssetManager::HandleMapAssetChange( rumMapAsset* i_pcAsset, QTableWidgetItem* i_pcItem )
{
  rumAssert( i_pcAsset );
  if( !i_pcAsset )
  {
    return;
  }

  const int32_t iCol{ m_pcUI->tableWidget_3->currentColumn() };
  switch( iCol )
  {
    case S_COL_NAME:
    case S_COL_CLASS:
      HandleAssetChange( i_pcAsset, i_pcItem );
      break;

    case S_COL_FILENAME:
      HandleFileAssetChange( i_pcAsset, i_pcItem );
      break;

    default:
      rumAssertMsg( false, "Unexpected column modified" );
  }
}


void AssetManager::HandlePawnAssetChange( rumPawnAsset* i_pcAsset, QTableWidgetItem* i_pcItem )
{
  rumAssert( i_pcAsset );
  if( !i_pcAsset )
  {
    return;
  }

  const int32_t iCol{ m_pcUI->tableWidget_3->currentColumn() };
  switch( iCol )
  {
    case S_COL_NAME:
    case S_COL_CLASS:
      HandleAssetChange( i_pcAsset, i_pcItem );
      break;

    case S_COL_PAWN_GRAPHIC:
    {
      const QVariant cVariant{ i_pcItem->data( Qt::UserRole ) };
      i_pcAsset->SetGraphicID( (rumAssetID)cVariant.toInt() );
      break;
    }

    case S_COL_PAWN_MOVE_TYPE:
    {
      const QVariant cVariant{ i_pcItem->data( Qt::UserRole ) };
      i_pcAsset->SetMoveType( static_cast<rumMoveFlags>( cVariant.toInt() ) );
      break;
    }

    case S_COL_PAWN_LOS:
      i_pcAsset->SetBlocksLOS( rumStringUtils::ToBool( qPrintable( i_pcItem->text() ) ) );
      break;

    case S_COL_PAWN_COL_FLAGS:
    {
      const QVariant cVariant{ i_pcItem->data( Qt::UserRole ) };
      i_pcAsset->SetCollisionFlags( (uint32_t)cVariant.toInt() );
      break;
    }

    case S_COL_PAWN_LIGHT_RANGE:
      i_pcAsset->SetLightRange( i_pcItem->text().toInt() );
      break;

    case S_COL_PAWN_DRAW_ORDER:
      i_pcAsset->SetDrawOrder( i_pcItem->text().toFloat() );
      break;

    case S_COL_PAWN_SERVICE:
    {
      const QVariant cVariant{ i_pcItem->data( Qt::UserRole ) };
      i_pcAsset->SetServiceType( ServiceType( cVariant.toInt() ) );
      break;
    }

    default:
      rumAssertMsg( false, "Unexpected column modified" );
  }
}


void AssetManager::HandleTileAssetChange( rumTileAsset* i_pcAsset, QTableWidgetItem* i_pcItem )
{
  rumAssert( i_pcAsset );
  if( !i_pcAsset )
  {
    return;
  }

  const int32_t iCol{ m_pcUI->tableWidget_3->currentColumn() };
  switch( iCol )
  {
    case S_COL_NAME:
    case S_COL_CLASS:
      HandleAssetChange( i_pcAsset, i_pcItem );
      break;

    case S_COL_TILE_GRAPHIC:
    {
      const QVariant cVariant{ i_pcItem->data( Qt::UserRole ) };
      i_pcAsset->SetGraphicID( (rumAssetID)cVariant.toInt() );
      break;
    }

    case S_COL_TILE_WEIGHT:
      i_pcAsset->SetWeight( i_pcItem->text().toFloat() );
      break;

    case S_COL_TILE_LOS:
      i_pcAsset->SetBlocksLOS( rumStringUtils::ToBool( qPrintable( i_pcItem->text() ) ) );
      break;

    case S_COL_TILE_COL_FLAGS:
    {
      const QVariant cVariant{ i_pcItem->data( Qt::UserRole ) };
      i_pcAsset->SetCollisionFlags( (uint32_t)cVariant.toInt() );
      break;
    }


    default:
      rumAssertMsg( false, "Unexpected column modified" );
  }
}


void AssetManager::InitBroadcasts()
{
  QStringList cLabelList;
  cLabelList << "Name" << "Base Class";
  m_pcUI->tableWidget_3->setColumnCount( cLabelList.size() );
  m_pcUI->tableWidget_3->setHorizontalHeaderLabels( cLabelList );

  const auto& i_rcAssetHash{ rumBroadcastAsset::GetAssetHash() };

  m_pcUI->tableWidget_3->setRowCount( (int32_t)i_rcAssetHash.size() );

  int32_t iIndex{ 0 };

  for( const auto& iter : i_rcAssetHash )
  {
    const rumBroadcastAsset* pcAsset{ iter.second };
    rumAssert( pcAsset );
    AddAsset( iIndex++, *pcAsset );
  }
}


void AssetManager::InitCustom()
{
  QStringList cLabelList;
  cLabelList << "Name" << "Base Class";
  m_pcUI->tableWidget_3->setColumnCount( cLabelList.size() );
  m_pcUI->tableWidget_3->setHorizontalHeaderLabels( cLabelList );

  const auto& i_rcAssetHash{ rumCustomAsset::GetAssetHash() };

  m_pcUI->tableWidget_3->setRowCount( (int32_t)i_rcAssetHash.size() );

  int32_t iIndex{ 0 };

  for( const auto& iter : i_rcAssetHash )
  {
    const rumCustomAsset* pcAsset{ iter.second };
    rumAssert( pcAsset );
    AddAsset( iIndex++, *pcAsset );
  }
}


void AssetManager::InitGraphics()
{
  QStringList cLabelList;
  cLabelList << "Name" << "Base Class" << "Filename" << "Anim Frames" << "Anim States" << "Anim Type" << "Interval" << "Render";
  m_pcUI->tableWidget_3->setColumnCount( cLabelList.size() );
  m_pcUI->tableWidget_3->setHorizontalHeaderLabels( cLabelList );

  const auto& i_rcAssetHash{ rumGraphicAsset::GetAssetHash() };

  m_pcUI->tableWidget_3->setRowCount( (int32_t)i_rcAssetHash.size() );

  int32_t iIndex{ 0 };

  for( const auto& iter : i_rcAssetHash )
  {
    const rumGraphicAsset* pcAsset{ iter.second };
    rumAssert( pcAsset );
    AddGraphicAsset( iIndex++, *pcAsset );
  }
}


void AssetManager::InitInventory()
{
  QStringList cLabelList;
  cLabelList << "Name" << "Base Class" << "Replication" << "Persistent";
  m_pcUI->tableWidget_3->setColumnCount( cLabelList.size() );
  m_pcUI->tableWidget_3->setHorizontalHeaderLabels( cLabelList );

  const auto& i_rcAssetHash{ rumInventoryAsset::GetAssetHash() };

  m_pcUI->tableWidget_3->setRowCount( (int32_t)i_rcAssetHash.size() );

  int32_t iIndex{ 0 };

  for( const auto& iter : i_rcAssetHash )
  {
    const rumInventoryAsset* pcAsset{ iter.second };
    rumAssert( pcAsset );
    AddInventoryAsset( iIndex++, *pcAsset );
  }
}


void AssetManager::InitMaps()
{
  QStringList cLabelList;
  cLabelList << "Name" << "Base Class" << "Filename";
  m_pcUI->tableWidget_3->setColumnCount( cLabelList.size() );
  m_pcUI->tableWidget_3->setHorizontalHeaderLabels( cLabelList );

  const auto& i_rcAssetHash{ rumMapAsset::GetAssetHash() };

  m_pcUI->tableWidget_3->setRowCount( (int32_t)i_rcAssetHash.size() );

  int32_t iIndex{ 0 };

  for( const auto& iter : i_rcAssetHash )
  {
    const rumMapAsset* pcAsset{ iter.second };
    rumAssert( pcAsset );
    AddMapAsset( iIndex++, *pcAsset );
  }
}


void AssetManager::InitPawns( rumPawn::PawnType i_ePawnType )
{
  QStringList cLabelList;
  cLabelList << "Name" << "Base Class" << "Graphic ID" << "Move Flags" << "Block LOS" << "Collision Flags"
         << "Light Range" << "Priority" << "Service Type";
  m_pcUI->tableWidget_3->setColumnCount( cLabelList.size() );
  m_pcUI->tableWidget_3->setHorizontalHeaderLabels( cLabelList );

  int32_t iIndex{ 0 };

  if( rumPawn::Creature_PawnType == i_ePawnType )
  {
    const auto& i_rcAssetHash{ rumCreatureAsset::GetAssetHash() };

    m_pcUI->tableWidget_3->setRowCount( (int32_t)i_rcAssetHash.size() );

    for( const auto& iter : i_rcAssetHash )
    {
      const rumPawnAsset* pcAsset{ iter.second };
      rumAssert( pcAsset );
      AddPawnAsset( iIndex++, *pcAsset );
    }
  }
  else if( rumPawn::Portal_PawnType == i_ePawnType )
  {
    const auto& i_rcAssetHash{ rumPortalAsset::GetAssetHash() };

    m_pcUI->tableWidget_3->setRowCount( (int32_t)i_rcAssetHash.size() );

    for( const auto& iter : i_rcAssetHash )
    {
      const rumPawnAsset* pcAsset{ iter.second };
      rumAssert( pcAsset );
      AddPawnAsset( iIndex++, *pcAsset );
    }
  }
  else if( rumPawn::Widget_PawnType == i_ePawnType )
  {
    const auto& i_rcAssetHash{ rumWidgetAsset::GetAssetHash() };

    m_pcUI->tableWidget_3->setRowCount( (int32_t)i_rcAssetHash.size() );

    for( const auto& iter : i_rcAssetHash )
    {
      const rumPawnAsset* pcAsset{ iter.second };
      rumAssert( pcAsset );
      AddPawnAsset( iIndex++, *pcAsset );
    }
  }
}


void AssetManager::InitSounds()
{
  QStringList cLabelList;
  cLabelList << "Name" << "Base Class" << "Filename";
  m_pcUI->tableWidget_3->setColumnCount( cLabelList.size() );
  m_pcUI->tableWidget_3->setHorizontalHeaderLabels( cLabelList );

  const auto& i_rcAssetHash{ rumSoundAsset::GetAssetHash() };

  m_pcUI->tableWidget_3->setRowCount( (int32_t)i_rcAssetHash.size() );

  int32_t iIndex{ 0 };

  for( const auto& iter : i_rcAssetHash )
  {
    const rumSoundAsset* pcAsset{ iter.second };
    rumAssert( pcAsset );
    AddFileAsset( iIndex++, *pcAsset );
  }
}


void AssetManager::InitTiles()
{
  QStringList cLabelList;
  cLabelList << "Name" << "Base Class" << "Graphic ID" << "Weight" << "Block LOS" << "Collision Flags";
  m_pcUI->tableWidget_3->setColumnCount( cLabelList.size() );
  m_pcUI->tableWidget_3->setHorizontalHeaderLabels( cLabelList );

  const auto& i_rcAssetHash{ rumTileAsset::GetAssetHash() };

  m_pcUI->tableWidget_3->setRowCount( (int32_t)i_rcAssetHash.size() );

  int32_t iIndex{ 0 };

  for( const auto& iter : i_rcAssetHash )
  {
    const rumTileAsset* pcAsset{ iter.second };
    rumAssert( pcAsset );
    AddTileAsset( iIndex++, *pcAsset );
  }
}


bool AssetManager::IsAssetNameUnique( const QString& i_strName ) const
{
  bool bUnique{ true };

  const int32_t iSelectedRow{ m_pcUI->tableWidget_3->currentRow() };

  // Visit each table row and compare names
  const int32_t iNumRows{ m_pcUI->tableWidget_3->rowCount() };
  for( int32_t iRow{ 0 }; iRow < iNumRows && bUnique; ++iRow )
  {
    if( iRow != iSelectedRow )
    {
      const QString& strName{ GetAssetName( iRow ) };
      if( strName.compare( i_strName, Qt::CaseInsensitive ) == 0 )
      {
        bUnique = false;
      }
    }
  }

  return bUnique;
}


// slot
void AssetManager::ItemComboChanged( const QString& i_strText )
{
  // Fetch the asset from the affected row
  const rumAsset* pcAsset{ GetSelectedAsset() };
  rumAssert( pcAsset );
  if( !pcAsset )
  {
    return;
  }

  const int32_t iRow{ m_pcUI->tableWidget_3->currentRow() };
  const int32_t iCol{ m_pcUI->tableWidget_3->currentColumn() };
  bool bAcceptChange{ false };

  if( pcAsset->GetAssetType() == Graphic_AssetType )
  {
    bAcceptChange = ( S_COL_GRAPHIC_ANIMTYPE == iCol ) || ( S_COL_GRAPHIC_RENDER == iCol );
  }
  else if( pcAsset->GetAssetType() == Inventory_AssetType )
  {
    bAcceptChange = ( S_COL_INV_REPLICATION == iCol ) || ( S_COL_INV_PERSISTENT == iCol );
  }
  else if( ( pcAsset->GetAssetType() == Creature_AssetType ) ||
           ( pcAsset->GetAssetType() == Portal_AssetType ) ||
           ( pcAsset->GetAssetType() == Widget_AssetType ) )
  {
    bAcceptChange = (S_COL_PAWN_GRAPHIC == iCol ) || ( S_COL_PAWN_LOS == iCol ) || ( S_COL_PAWN_SERVICE == iCol );
  }
  else if( pcAsset->GetAssetType() == Tile_AssetType )
  {
    bAcceptChange = ( S_COL_TILE_GRAPHIC == iCol ) || ( S_COL_TILE_LOS == iCol );
  }

  if( bAcceptChange )
  {
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
  else
  {
    rumAssertMsg( false, "Unexpected column modified" );
  }

  SetDirty( true );
}


// slot
void AssetManager::ItemComboPropertyChanged( const QString& i_strText )
{
  // Fetch the asset from the affected row
  const rumPropertyAsset* pcProperty{ rumPropertyAsset::Fetch( GetSelectedPropertyID() ) };
  rumAssert( pcProperty );
  if( !pcProperty )
  {
    return;
  }

  const int32_t iRow{ m_pcUI->tableWidget_4->currentRow() };
  const int32_t iCol{ m_pcUI->tableWidget_4->currentColumn() };

  QVariant cVariant;

  // Access user data if it is available
  const QWidget* pcWidget{ m_pcUI->tableWidget_4->cellWidget( iRow, iCol ) };
  if( pcWidget )
  {
    const QComboBox* pcComboBox{ qobject_cast<const QComboBox*>( pcWidget ) };
    if( pcComboBox )
    {
      cVariant = pcComboBox->currentData();
    }

    m_pcUI->tableWidget_4->removeCellWidget( iRow, iCol );
  }

  QTableWidgetItem* pcCell{ new QTableWidgetItem() };
  pcCell->setData( Qt::DisplayRole, i_strText );

  if( cVariant.isValid() )
  {
    pcCell->setData( Qt::UserRole, cVariant );
  }

  m_pcUI->tableWidget_4->setItem( iRow, iCol, pcCell );

  if( pcProperty->IsAssetRef() || ( pcProperty->GetValueType() == PropertyValueType::Bool ) )
  {
    pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
  }

  SetDirty( true );
}


// slot
void AssetManager::on_actionCopy_All_Properties_triggered()
{
  const rumAsset* pcAsset{ GetSelectedAsset() };
  rumAssert( pcAsset );
  if( pcAsset )
  {
    // Copy all of the asset's properties
    const PropertyContainer cPropertyContainer{ pcAsset->GetProperties() };
    MainWindow::SetPropertyClipboard( std::move( cPropertyContainer ) );
  }
}


// slot
void AssetManager::on_actionCopy_Property_triggered()
{
  Sqrat::Object sqProperty{ GetSelectedProperty() };

  PropertyContainer cPropertyContainer;
  cPropertyContainer.insert( { GetSelectedPropertyID(), sqProperty } );

  MainWindow::SetPropertyClipboard( std::move( cPropertyContainer ) );
}


// slot
void AssetManager::on_actionCopy_Selected_Properties_triggered()
{
  PropertyContainer rcProperties{ GetSelectedProperties() };
  MainWindow::SetPropertyClipboard( std::move( rcProperties ) );
}


// slot
void AssetManager::on_actionFind_References_triggered()
{
  const auto pcAsset{ GetSelectedAsset() };
  if( pcAsset == nullptr )
  {
    return;
  }

  MainWindow::FindAssetReferences( *pcAsset, true );
}


// slot
void AssetManager::on_actionNew_Asset_triggered()
{
  // Clear the filter
  m_cFilterEdit.clear();

  UpdateFilter();
  UpdatePropertyFilter();

  disconnect( m_pcUI->tableWidget_3, &QTableWidget::itemChanged, this, &AssetManager::onAssetTableItemChanged );

  m_pcUI->tableWidget_3->setSortingEnabled( false );

  const int32_t iRow{ m_pcUI->tableWidget_3->rowCount() };
  rumAsset* pcAsset{ nullptr };

  switch( GetSelectedAssetType() )
  {
    case Broadcast_AssetType:
    {
      rumBroadcastAsset* pcBroadcastAsset{ CreateAsset<rumBroadcastAsset>() };
      if( pcBroadcastAsset )
      {
        AddAsset( iRow, *pcBroadcastAsset );
        pcAsset = pcBroadcastAsset;
      }
      break;
    }

    case Creature_AssetType:
    {
      rumCreatureAsset* pcCreatureAsset{ CreateAsset<rumCreatureAsset>() };
      if( pcCreatureAsset )
      {
        AddPawnAsset( iRow, *pcCreatureAsset );
        pcAsset = pcCreatureAsset;
      }
      break;
    }

    case Custom_AssetType:
    {
      rumCustomAsset* pcCustomAsset{ CreateAsset<rumCustomAsset>() };
      if( pcCustomAsset )
      {
        AddAsset( iRow, *pcCustomAsset );
        pcAsset = pcCustomAsset;
      }
      break;
    }

    case Graphic_AssetType:
    {
      rumGraphicAsset* pcGraphicAsset{ CreateAsset<rumGraphicAsset>() };
      if( pcGraphicAsset )
      {
        AddGraphicAsset( iRow, *pcGraphicAsset );
        pcAsset = pcGraphicAsset;
      }
      break;
    }

    case Inventory_AssetType:
    {
      rumInventoryAsset* pcInventoryAsset{ CreateAsset<rumInventoryAsset>() };
      if( pcInventoryAsset )
      {
        AddInventoryAsset( iRow, *pcInventoryAsset );
        pcAsset = pcInventoryAsset;
      }
      break;
    }

    case Map_AssetType:
    {
      rumMapAsset* pcMapAsset{ CreateAsset<rumMapAsset>() };
      if( pcMapAsset )
      {
        AddMapAsset( iRow, *pcMapAsset );
        pcAsset = pcMapAsset;
      }
      break;
    }

    case Portal_AssetType:
    {
      rumPortalAsset* pcPortalAsset{ CreateAsset<rumPortalAsset>() };
      if( pcPortalAsset )
      {
        AddPawnAsset( iRow, *pcPortalAsset );
        pcAsset = pcPortalAsset;
      }
      break;
    }

    case Sound_AssetType:
    {
      rumSoundAsset* pcSoundAsset{ CreateAsset<rumSoundAsset>() };
      if( pcSoundAsset )
      {
        AddFileAsset( iRow, *pcSoundAsset );
        pcAsset = pcSoundAsset;
      }
      break;
    }

    case Tile_AssetType:
    {
      rumTileAsset* pcTileAsset{ CreateAsset<rumTileAsset>() };
      if( pcTileAsset )
      {
        AddTileAsset( iRow, *pcTileAsset );
        pcAsset = pcTileAsset;
      }
      break;
    }

    case Widget_AssetType:
    {
      rumWidgetAsset* pcWidgetAsset{ CreateAsset<rumWidgetAsset>() };
      if( pcWidgetAsset )
      {
        AddPawnAsset( iRow, *pcWidgetAsset );
        pcAsset = pcWidgetAsset;
      }
      break;
    }

    default:
      rumAssertMsg( false, "Unsupported asset type" );
      break;
  }

  if( pcAsset )
  {
    // Set the selected row prior to sorting
    m_pcUI->tableWidget_3->selectRow( iRow );
  }

  // Subscribe to change notifications
  connect( m_pcUI->tableWidget_3, &QTableWidget::itemChanged, this, &AssetManager::onAssetTableItemChanged );

  m_pcUI->tableWidget_3->setSortingEnabled( true );

  if( pcAsset )
  {
    // Scroll to the newly added row
    // NOTE: The row id might have just changed because of sorting, so use currentRow
    const QTableWidgetItem* pcItem{ m_pcUI->tableWidget_3->item( m_pcUI->tableWidget_3->currentRow(), S_COL_NAME ) };
    if( pcItem )
    {
      m_pcUI->tableWidget_3->scrollToItem( pcItem );
    }
  }

  SetDirty( true );
}


void AssetManager::on_actionNew_Property_triggered()
{
  QSet<rumAssetID> cAssetsSet;

  // Any asset type already in the table will be used as an ignore set for the modal dialog
  for( int32_t i{ 0 }; i < m_pcUI->tableWidget_4->rowCount(); ++i )
  {
    const QTableWidgetItem* pcItem{ m_pcUI->tableWidget_4->item( i, PROP_COL_NAME ) };
    if( pcItem )
    {
      cAssetsSet.insert( (rumAssetID)pcItem->data( ROLE_ID ).toInt() );
    }
  }

  // Open a modal dialog listbox of all properties for selection
  bool bPersistenOnly{ false };
  AssetPicker* pcDialog{ new AssetPicker( AssetType::Property_AssetType, bPersistenOnly, cAssetsSet, this ) };

  connect( pcDialog, SIGNAL( NewPropertySelected( rumAssetID ) ),
           this, SLOT( OnPropertyAdded( rumAssetID ) ) );

  pcDialog->setModal( true );
  pcDialog->show();
}


void AssetManager::on_actionPaste_Properties_Merge_triggered()
{
  m_cPropertyFilterEdit.clear();

  rumAsset* pcAsset{ GetSelectedAsset() };
  rumAssert( pcAsset );
  if( !pcAsset )
  {
    return;
  }

  const PropertyContainer& rcProperties{ MainWindow::GetPropertyClipboard() };
  for( const auto& iter : rcProperties )
  {
    const rumAssetID ePropertyID{ iter.first };
    Sqrat::Object sqValue{ iter.second };

    // Add the new property if it doesn't already exist
    if( !pcAsset->HasProperty( ePropertyID ) )
    {
      pcAsset->SetProperty( ePropertyID, sqValue );
    }
  }

  RefreshPropertyTable();
  SetDirty( true );
}


void AssetManager::on_actionPaste_Properties_Overwrite_triggered()
{
  m_cPropertyFilterEdit.clear();

  rumAsset* pcAsset{ GetSelectedAsset() };
  rumAssert( pcAsset );
  if( !pcAsset )
  {
    return;
  }

  const PropertyContainer& rcProperties{ MainWindow::GetPropertyClipboard() };
  for( const auto& iter : rcProperties )
  {
    const rumAssetID ePropertyID{ iter.first };
    Sqrat::Object sqValue{ iter.second };

    // Overwrite the existing property value
    pcAsset->SetProperty( ePropertyID, sqValue );
  }

  RefreshPropertyTable();
  SetDirty( true );
}


void AssetManager::on_actionRemove_Asset_triggered()
{
  const auto* pcAsset{ GetSelectedAsset() };
  if( nullptr == pcAsset )
  {
    return;
  }

  const auto& rcAsset{ *pcAsset };

  QString strQuestion{ "Are you sure you want to remove asset " };
  strQuestion += rcAsset.GetName().c_str();
  strQuestion += '?';

  // Verify delete
  if( QMessageBox::Yes == QMessageBox::question( this, "Verify", strQuestion,
                                                 QMessageBox::Yes | QMessageBox::No,
                                                 QMessageBox::No ) )
  {
    // Audit the asset's references and disallow removal if anything is found
    const uint32_t uiNumRefs{ MainWindow::FindAssetReferences( rcAsset, false /* no logging */ ) };
    if( uiNumRefs > 0 )
    {
      // TODO - list the references
      QMessageBox::critical( this, "Remove failed",
                             "You cannot remove the asset because it has existing data references",
                             QMessageBox::Ok );
    }
    else
    {
      // Remove the table widget row
      const int32_t iRow{ m_pcUI->tableWidget_3->currentRow() };
      m_pcUI->tableWidget_3->removeRow( iRow );

      switch( rcAsset.GetAssetType() )
      {
        case Broadcast_AssetType: rumBroadcastAsset::Remove( rcAsset.GetAssetID() ); break;
        case Creature_AssetType:  rumCreatureAsset::Remove( rcAsset.GetAssetID() );  break;
        case Custom_AssetType:    rumCustomAsset::Remove( rcAsset.GetAssetID() );    break;
        case Graphic_AssetType:   rumGraphicAsset::Remove( rcAsset.GetAssetID() );   break;
        case Inventory_AssetType: rumInventoryAsset::Remove( rcAsset.GetAssetID() ); break;
        case Map_AssetType:       rumMapAsset::Remove( rcAsset.GetAssetID() );       break;
        case Portal_AssetType:    rumPortalAsset::Remove( rcAsset.GetAssetID() );    break;
        case Sound_AssetType:     rumSoundAsset::Remove( rcAsset.GetAssetID() );     break;
        case Tile_AssetType:      rumTileAsset::Remove( rcAsset.GetAssetID() );      break;
        case Widget_AssetType:    rumWidgetAsset::Remove( rcAsset.GetAssetID() );    break;
        default:
          rumAssertMsg( false, "Unsupported asset type" );
          break;
      }

      SetDirty( true );
    }
  }
}


void AssetManager::on_actionRemove_Property_triggered()
{
  rumAsset* pcAsset{ GetSelectedAsset() };
  rumAssert( pcAsset );
  if( pcAsset )
  {
    pcAsset->RemoveProperty( GetSelectedPropertyID() );
    m_pcUI->tableWidget_4->removeRow( m_pcUI->tableWidget_4->currentRow() );
    SetDirty( true );
  }
}


// slot
void AssetManager::on_actionSave_triggered()
{
  if( IsDirty() )
  {
    const int32_t iRows{ m_pcUI->tableWidget->rowCount() };
    for( int32_t i{ 0 }; i < iRows; ++i )
    {
      SaveTable( i );
    }

    SetDirty( false );
  }
}


void AssetManager::on_tableWidget_3_cellDoubleClicked( int32_t i_iRow, int32_t i_iCol )
{
  if( S_COL_NAME == i_iCol )
  {
    // Only allow numbers, letters, dash, and underscore up to 64 characters
    const QRegExp cRegExp( "[\\w-]{1,64}" );
    const QRegExpValidator* pcValidator{ new QRegExpValidator( cRegExp, this ) };

    QLineEdit* pcLineEdit{ new QLineEdit };
    pcLineEdit->setText( GetAssetName( i_iRow ) );
    pcLineEdit->setValidator( pcValidator );

    connect( pcLineEdit, SIGNAL( editingFinished() ),
             this, SLOT( AssetLineEditFinished() ) );

    m_pcUI->tableWidget_3->setCellWidget( i_iRow, i_iCol, pcLineEdit );
  }
}


void AssetManager::on_tableWidget_3_customContextMenuRequested( const QPoint& i_rcPos )
{
  if( i_rcPos.isNull() )
  {
    return;
  }

  QMenu* pcMenu{ new QMenu( m_pcUI->tableWidget_3 ) };
  pcMenu->addAction( m_pcUI->actionNew_Asset );
  pcMenu->addAction( m_pcUI->actionRemove_Asset );
  pcMenu->addSeparator();
  pcMenu->addAction( m_pcUI->actionFind_References );

  pcMenu->exec( m_pcUI->tableWidget_3->mapToGlobal( i_rcPos ) );
}


void AssetManager::on_tableWidget_4_customContextMenuRequested( const QPoint& i_rcPos )
{
  if( i_rcPos.isNull() )
  {
    return;
  }

  QMenu* pcMenu{ new QMenu( m_pcUI->tableWidget_4 ) };
  pcMenu->addAction( m_pcUI->actionNew_Property );
  pcMenu->addAction( m_pcUI->actionRemove_Property );
  pcMenu->addSeparator();
  pcMenu->addAction( m_pcUI->actionCopy_Property );
  pcMenu->addAction( m_pcUI->actionCopy_All_Properties );
  pcMenu->addAction( m_pcUI->actionCopy_Selected_Properties );
  pcMenu->addSeparator();
  pcMenu->addAction( m_pcUI->actionPaste_Properties_Overwrite );
  pcMenu->addAction( m_pcUI->actionPaste_Properties_Merge );

  pcMenu->exec( m_pcUI->tableWidget_4->mapToGlobal( i_rcPos ) );
}


void AssetManager::on_tableWidget_customContextMenuRequested( const QPoint& i_rcPos )
{
  if( i_rcPos.isNull() )
  {
    return;
  }

  QMenu* pcMenu{ new QMenu( m_pcUI->tableWidget ) };
  pcMenu->addAction( m_pcUI->actionSave );

  pcMenu->exec( m_pcUI->tableWidget->mapToGlobal( i_rcPos ) );
}


// slot
void AssetManager::onAssetTableItemChanged( QTableWidgetItem* i_pcItem )
{
  const auto* pcAsset( GetAsset( i_pcItem->row() ) );
  rumAssert( pcAsset != nullptr );
  if( !pcAsset )
  {
    return;
  }

  const auto eAssetID{ pcAsset->GetAssetID() };

  switch( pcAsset->GetAssetType() )
  {
    case Broadcast_AssetType:
    case Custom_AssetType:
      HandleAssetChange( rumAsset::Fetch( eAssetID ), i_pcItem );
      break;

    case Creature_AssetType:
    case Portal_AssetType:
    case Widget_AssetType:
      HandlePawnAssetChange( rumPawnAsset::Fetch( eAssetID ), i_pcItem );
      break;

    case Graphic_AssetType:
      HandleGraphicAssetChange( rumGraphicAsset::Fetch( eAssetID ), i_pcItem );
      break;

    case Inventory_AssetType:
      HandleInventoryAssetChange( rumInventoryAsset::Fetch( eAssetID ), i_pcItem );
      break;

    case Map_AssetType:
      HandleMapAssetChange( rumMapAsset::Fetch( eAssetID ), i_pcItem );
      break;

    case Sound_AssetType:
      HandleFileAssetChange( rumSoundAsset::Fetch( eAssetID ), i_pcItem );
      break;

    case Tile_AssetType:
      HandleTileAssetChange( rumTileAsset::Fetch( eAssetID ), i_pcItem );
      break;

    default:
      rumAssertMsg( false, "Unsupported asset type" );
  }

  SetDirty( true );
}


void AssetManager::OnCellStartEdit( int32_t i_iRow, int32_t i_iCol )
{
  const rumAsset* pcAsset{ GetSelectedAsset() };
  if( !pcAsset )
  {
    return;
  }

  if( ( ( pcAsset->GetAssetType() == Graphic_AssetType ) ||
        ( pcAsset->GetAssetType() == Map_AssetType )     ||
        ( pcAsset->GetAssetType() == Sound_AssetType ) ) &&
      ( S_COL_FILENAME == i_iCol ) )
  {
    QFileDialog cDialog( this );
    cDialog.setFileMode( QFileDialog::ExistingFile );
    cDialog.setViewMode( QFileDialog::Detail );

    if( pcAsset->GetAssetType() == Graphic_AssetType )
    {
      cDialog.setNameFilter( tr( "Graphic (*.png *.bmp *.jpg *.pcx)" ) );
      cDialog.setDirectory( MainWindow::GetProjectGraphicDir() );
    }
    else if( pcAsset->GetAssetType() == Map_AssetType )
    {
      cDialog.setNameFilter( tr( "Map (*.map)" ) );
      cDialog.setDirectory( MainWindow::GetProjectMapDir() );
    }
    else if( pcAsset->GetAssetType() == Sound_AssetType )
    {
      cDialog.setNameFilter( tr( "Sound (*.snd *.wav *.ogg *.mid *.mp3)" ) );
      cDialog.setDirectory( MainWindow::GetProjectAudioDir() );
    }

    QStringList cFileList;
    if( cDialog.exec() )
    {
      cFileList = cDialog.selectedFiles();
    }

    if( !cFileList.isEmpty() )
    {
      QFileInfo cFileInfo{ cFileList.front() };

      QTableWidgetItem* pcCell{ new QTableWidgetItem( cFileInfo.fileName() ) };
      m_pcUI->tableWidget_3->setItem( i_iRow, i_iCol, pcCell );
      pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
    }
  }
  else if( pcAsset->GetAssetType() == Graphic_AssetType )
  {
    const rumGraphicAsset* pcGraphic{ rumGraphicAsset::Fetch( pcAsset->GetAssetID() ) };

    if( S_COL_GRAPHIC_ANIMTYPE == i_iCol )
    {
      QComboBox* pcCombo{ new QComboBox };
      pcCombo->addItem( "Standard Looping", rumAnimationType::StandardLooping_AnimationType );
      pcCombo->addItem( "Standard Once", rumAnimationType::StandardOnce_AnimationType );
      pcCombo->addItem( "Random Frame", rumAnimationType::Random_AnimationType );
      pcCombo->addItem( "Custom", rumAnimationType::Custom_AnimationType );
      pcCombo->setCurrentIndex( pcGraphic->GetAnimType() );
      pcCombo->view()->setMinimumWidth( pcCombo->minimumSizeHint().width() );
      connect( pcCombo, SIGNAL( currentIndexChanged( const QString& ) ),
               this, SLOT( ItemComboChanged( const QString& ) ) );
      m_pcUI->tableWidget_3->setCellWidget( i_iRow, i_iCol, pcCombo );
    }
    else if( S_COL_GRAPHIC_RENDER == i_iCol )
    {
      QComboBox* pcCombo{ new QComboBox };
      pcCombo->addItem( "False", false );
      pcCombo->addItem( "True", true );
      pcCombo->setCurrentIndex( pcGraphic->IsClientRendered() ? 1 : 0 );
      pcCombo->view()->setMinimumWidth( pcCombo->minimumSizeHint().width() );
      connect( pcCombo, SIGNAL( currentIndexChanged( const QString& ) ),
               this, SLOT( ItemComboChanged( const QString& ) ) );
      m_pcUI->tableWidget_3->setCellWidget( i_iRow, i_iCol, pcCombo );
    }
  }
  else if( pcAsset->GetAssetType() == Inventory_AssetType )
  {
    const rumInventoryAsset* pcInventory{ rumInventoryAsset::Fetch( pcAsset->GetAssetID() ) };

    if( S_COL_INV_REPLICATION == i_iCol )
    {
      QComboBox* pcCombo{ new QComboBox };
      pcCombo->addItem( "None", ClientReplicationType::None_ClientReplicationType );
      pcCombo->addItem( "Private", ClientReplicationType::Private_ClientReplicationType );
      pcCombo->addItem( "Regional", ClientReplicationType::Regional_ClientReplicationType );
      pcCombo->addItem( "Global", ClientReplicationType::Global_ClientReplicationType );

      const int32_t iIndex{ pcCombo->findData( pcInventory->GetClientReplicationType() ) };
      pcCombo->setCurrentIndex( iIndex );
      pcCombo->view()->setMinimumWidth( pcCombo->minimumSizeHint().width() );

      connect( pcCombo, SIGNAL( currentIndexChanged( const QString& ) ),
               this, SLOT( ItemComboChanged( const QString& ) ) );
      m_pcUI->tableWidget_3->setCellWidget( i_iRow, i_iCol, pcCombo );
    }
    else if( S_COL_INV_PERSISTENT == i_iCol )
    {
      QComboBox* pcCombo{ new QComboBox };
      pcCombo->addItem( "False", false );
      pcCombo->addItem( "True", true );
      pcCombo->setCurrentIndex( pcInventory->IsPersistent() ? 1 : 0 );
      pcCombo->view()->setMinimumWidth( pcCombo->minimumSizeHint().width() );
      connect( pcCombo, SIGNAL( currentIndexChanged( const QString& ) ),
               this, SLOT( ItemComboChanged( const QString& ) ) );
      m_pcUI->tableWidget_3->setCellWidget( i_iRow, i_iCol, pcCombo );
    }
  }
  else if( ( pcAsset->GetAssetType() == Creature_AssetType ) ||
           ( pcAsset->GetAssetType() == Portal_AssetType )   ||
           ( pcAsset->GetAssetType() == Widget_AssetType ) )
  {
    const rumPawnAsset* pcPawn{ rumPawnAsset::Fetch( pcAsset->GetAssetID() ) };

    if( S_COL_PAWN_GRAPHIC == i_iCol )
    {
      QComboBox* pcCombo{ new QComboBox };

      // Add every graphic entry by name and id
      const auto& rcHash{ rumGraphicAsset::GetAssetHash() };
      for( auto& iter : rcHash )
      {
        pcCombo->addItem( iter.second->GetName().c_str(), iter.first );
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
               this, SLOT( ItemComboChanged( const QString& ) ) );
      m_pcUI->tableWidget_3->setCellWidget( i_iRow, i_iCol, pcCombo );
    }
    else if( ( S_COL_PAWN_MOVE_TYPE == i_iCol ) || ( S_COL_PAWN_COL_FLAGS == i_iCol ) )
    {
      // TODO: Guarantee that MoveType is defined natively if not found in script?

      // Get the current value
      const QTableWidgetItem* pcMoveFlags{ m_pcUI->tableWidget_3->item( i_iRow, i_iCol ) };
      const QVariant cVariant{ pcMoveFlags->data( Qt::UserRole ) };
      const int32_t eMoveFlags{ cVariant.toInt() };
      Sqrat::Object sqValue;
      rumScript::SetValue( sqValue, eMoveFlags );

      // The enum field to use as selection data
      Sqrat::Object sqEnumObj{ Sqrat::ConstTable().GetSlot( "MoveType" ) };

      // Since this is a bitfield, allow multiple selections
      Sqrat::Object sqMultiSelection;
      rumScript::SetValue( sqMultiSelection, true );

      HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

      Sqrat::Array sqObjectArray( pcVM, 1 );

      Sqrat::Array sqMoveFlagArray( pcVM, 4 );
      sqMoveFlagArray.SetValue( 0, "Flags" );
      sqMoveFlagArray.SetValue( 1, sqValue );
      sqMoveFlagArray.SetValue( 2, sqEnumObj );
      sqMoveFlagArray.SetValue( 3, sqMultiSelection );

      sqObjectArray.SetValue( 0, sqMoveFlagArray );

      Sqrat::Array sqResultArray{ MainWindow::ScriptModalDialog( sqObjectArray ) };
      if( sqResultArray.GetType() == OT_ARRAY )
      {
        sqValue = *( sqResultArray.GetValue<Sqrat::Object>( 0 ) );
        if( sqValue.GetType() == OT_INTEGER )
        {
          const int32_t eFlags{ sqValue.Cast<int32_t>() };
          const std::string strFlags{ rumScript::EnumToString( "MoveType", eFlags, true /* bitfield */ ) };

          QTableWidgetItem* pcCell{ new QTableWidgetItem() };
          pcCell->setData( Qt::DisplayRole, strFlags.c_str() );
          pcCell->setData( Qt::UserRole, eFlags );
          pcCell->setToolTip( rumStringUtils::ToHexString( eFlags ) );
          m_pcUI->tableWidget_3->setItem( i_iRow, i_iCol, pcCell );
          pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
        }
      }
    }
    else if( S_COL_PAWN_LOS == i_iCol )
    {
      QComboBox* pcCombo{ new QComboBox };
      pcCombo->addItem( "False", false );
      pcCombo->addItem( "True", true );
      pcCombo->setCurrentIndex( pcPawn->GetBlocksLOS() ? 1 : 0 );
      pcCombo->view()->setMinimumWidth( pcCombo->minimumSizeHint().width() );
      connect( pcCombo, SIGNAL( currentIndexChanged( const QString& ) ),
               this, SLOT( ItemComboChanged( const QString& ) ) );
      m_pcUI->tableWidget_3->setCellWidget( i_iRow, i_iCol, pcCombo );
    }
    else if( S_COL_PAWN_SERVICE == i_iCol )
    {
      QComboBox* pcCombo{ new QComboBox };
      pcCombo->addItem( "Shared", ServiceType::Shared_ServiceType );
      pcCombo->addItem( "Server", ServiceType::Server_ServiceType );
      pcCombo->addItem( "Client", ServiceType::Client_ServiceType );

      const int32_t iIndex{ pcCombo->findData( pcPawn->GetServiceType() ) };
      pcCombo->setCurrentIndex( iIndex );
      pcCombo->view()->setMinimumWidth( pcCombo->minimumSizeHint().width() );

      connect( pcCombo, SIGNAL( currentIndexChanged( const QString& ) ),
               this, SLOT( ItemComboChanged( const QString& ) ) );
      m_pcUI->tableWidget_3->setCellWidget( i_iRow, i_iCol, pcCombo );
    }
  }
  else if( pcAsset->GetAssetType() == Tile_AssetType )
  {
    const rumTileAsset* pcTile{ rumTileAsset::Fetch( pcAsset->GetAssetID() ) };

    if( S_COL_TILE_GRAPHIC == i_iCol )
    {
      QComboBox* pcCombo{ new QComboBox };

      // Add every graphic entry by name and id
      const auto& rcHash{ rumGraphicAsset::GetAssetHash() };
      for( auto& iter : rcHash )
      {
        pcCombo->addItem( iter.second->GetName().c_str(), iter.first );
      }

      // Select the index in the combo box that matches the selection in the table prior to this edit
      const QTableWidgetItem* pcCell{ m_pcUI->tableWidget_3->item( m_pcUI->tableWidget_3->currentRow(),
                                                                   S_COL_TILE_GRAPHIC ) };
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
               this, SLOT( ItemComboChanged( const QString& ) ) );
      m_pcUI->tableWidget_3->setCellWidget( i_iRow, i_iCol, pcCombo );
    }
    else if( S_COL_TILE_LOS == i_iCol )
    {
      QComboBox* pcCombo{ new QComboBox };
      pcCombo->addItem( "False", false );
      pcCombo->addItem( "True", true );
      pcCombo->setCurrentIndex( pcTile->GetBlocksLOS() ? 1 : 0 );
      pcCombo->view()->setMinimumWidth( pcCombo->minimumSizeHint().width() );
      connect( pcCombo, SIGNAL( currentIndexChanged( const QString& ) ),
               this, SLOT( ItemComboChanged( const QString& ) ) );
      m_pcUI->tableWidget_3->setCellWidget( i_iRow, i_iCol, pcCombo );
    }
    else if( S_COL_TILE_COL_FLAGS == i_iCol )
    {
      // TODO: Guarantee that MoveType is defined natively if not found in script?

      // Get the current value
      const QTableWidgetItem* pcMoveFlags{ m_pcUI->tableWidget_3->item( i_iRow, i_iCol ) };
      const QVariant cVariant{ pcMoveFlags->data( Qt::UserRole ) };
      const int32_t eMoveFlags{ cVariant.toInt() };
      Sqrat::Object sqValue;
      rumScript::SetValue( sqValue, eMoveFlags );

      // The enum field to use as selection data
      Sqrat::Object sqEnumObj{ Sqrat::ConstTable().GetSlot( "MoveType" ) };

      // Since this is a bitfield, allow multiple selections
      Sqrat::Object sqMultiSelection;
      rumScript::SetValue( sqMultiSelection, true );

      HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

      Sqrat::Array sqObjectArray( pcVM, 1 );

      Sqrat::Array sqMoveFlagArray( pcVM, 4 );
      sqMoveFlagArray.SetValue( 0, "Flags" );
      sqMoveFlagArray.SetValue( 1, sqValue );
      sqMoveFlagArray.SetValue( 2, sqEnumObj );
      sqMoveFlagArray.SetValue( 3, sqMultiSelection );

      sqObjectArray.SetValue( 0, sqMoveFlagArray );

      Sqrat::Array sqResultArray{ MainWindow::ScriptModalDialog( sqObjectArray ) };
      if( sqResultArray.GetType() == OT_ARRAY )
      {
        sqValue = *( sqResultArray.GetValue<Sqrat::Object>( 0 ) );
        if( sqValue.GetType() == OT_INTEGER )
        {
          const int32_t eFlags{ sqValue.Cast<int32_t>() };
          const std::string strFlags{ rumScript::EnumToString( "MoveType", eFlags, true /* bitfield */ ) };

          QTableWidgetItem* pcCell{ new QTableWidgetItem() };
          pcCell->setData( Qt::DisplayRole, strFlags.c_str() );
          pcCell->setData( Qt::UserRole, eFlags );
          pcCell->setToolTip( rumStringUtils::ToHexString( eFlags ) );
          m_pcUI->tableWidget_3->setItem( i_iRow, i_iCol, pcCell );
          pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
        }
      }
    }
  }
}


void AssetManager::OnCellStartPropertyEdit( int32_t i_iRow, int32_t i_iCol )
{
  if( PROP_COL_NAME == i_iCol )
  {
    return;
  }

  rumAsset* pcAsset{ GetSelectedAsset() };
  if( !pcAsset )
  {
    return;
  }

  Sqrat::Object sqProperty{ GetSelectedProperty() };

  const rumPropertyAsset* pcProperty{ rumPropertyAsset::Fetch( GetSelectedPropertyID() ) };
  if( pcProperty->GetValueType() == PropertyValueType::Bool )
  {
    QComboBox* pcCombo{ new QComboBox };
    pcCombo->addItem( "False", false );
    pcCombo->addItem( "True", true );
    pcCombo->setCurrentIndex( sqProperty.Cast<bool>() ? 1 : 0 );
    pcCombo->view()->setMinimumWidth( pcCombo->minimumSizeHint().width() );
    connect( pcCombo, SIGNAL( currentIndexChanged( const QString& ) ),
             this, SLOT( ItemComboPropertyChanged( const QString& ) ) );
    m_pcUI->tableWidget_4->setCellWidget( i_iRow, i_iCol, pcCombo );
  }
  else if( ( pcProperty->GetValueType() == PropertyValueType::Bitfield ||
             pcProperty->GetValueType() == PropertyValueType::Integer ) &&
           !pcProperty->GetEnumName().empty() )
  {
    int32_t iValue{ 0 };

    // Get the current value
    QTableWidgetItem* pcCell{ m_pcUI->tableWidget_4->item( i_iRow, i_iCol ) };
    const QVariant cVariant{ pcCell->data( Qt::UserRole ) };
    if( cVariant.isValid() )
    {
      iValue = cVariant.toInt();
    }
    else
    {
      iValue = rumStringUtils::ToInt( qPrintable( pcCell->text() ) );
    }

    Sqrat::Object sqValue;
    rumScript::SetValue( sqValue, iValue );

    // The enum field to use as selection data
    Sqrat::Object sqEnumObj{ Sqrat::ConstTable().GetSlot( pcProperty->GetEnumName().c_str() ) };

    // TODO - Determine programmatically if this is a bitfield?
    Sqrat::Object sqMultiSelection;
    rumScript::SetValue( sqMultiSelection, false );

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
        m_pcUI->tableWidget_4->setItem( i_iRow, i_iCol, pcCell );
        pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );

        pcAsset->SetProperty( pcProperty->GetAssetID(), sqValue );

        SetDirty( true );
      }
    }
  }
  else if( pcProperty->GetValueType() == PropertyValueType::StringToken )
  {
    int32_t iValue{ 0 };

    // Get the current value
    QTableWidgetItem* pcCell{ m_pcUI->tableWidget_4->item( i_iRow, i_iCol ) };
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
      for( const auto& iter : rcBroadcastHash )
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
    const QTableWidgetItem* pcCell{ m_pcUI->tableWidget_4->item( i_iRow, i_iCol ) };
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
    m_pcUI->tableWidget_4->setCellWidget( i_iRow, i_iCol, pcCombo );
  }
}


// slot
void AssetManager::onFilterChanged()
{
  UpdateFilter();
}


// slot
void AssetManager::OnPropertyAdded( rumAssetID i_ePropertyID )
{
  const rumPropertyAsset* pcProperty{ rumPropertyAsset::Fetch( i_ePropertyID ) };
  rumAssert( pcProperty );
  if( !pcProperty )
  {
    return;
  }

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
        case PropertyValueType::Integer: rumScript::SetValue( sqValue, 0 ); break;
        case PropertyValueType::Float:   rumScript::SetValue( sqValue, 0.f ); break;
        case PropertyValueType::Bool:    rumScript::SetValue( sqValue, false ); break;
        case PropertyValueType::String:  rumScript::SetValue( sqValue, "" ); break;
      }
    }
  }

  // Clear the filter
  m_cFilterEdit.clear();

  UpdatePropertyFilter();

  // Unsubscribe from change notifications
  disconnect( m_pcUI->tableWidget_4, &QTableWidget::itemChanged, this, &AssetManager::onPropertyTableItemChanged );

  // Disable sorting so that the new property appears last
  m_pcUI->tableWidget_4->setSortingEnabled( false );

  const int32_t iRow{ m_pcUI->tableWidget_4->rowCount() };
  AddPropertyAsset( iRow, *pcProperty, sqValue );

  rumAsset* pcAsset{ GetSelectedAsset() };
  rumAssert( pcAsset );
  if( pcAsset )
  {
    pcAsset->SetProperty( i_ePropertyID, sqValue );
    m_pcUI->tableWidget_4->selectRow( iRow );
  }

  // Subscribe to change notifications
  connect( m_pcUI->tableWidget_4, &QTableWidget::itemChanged, this, &AssetManager::onPropertyTableItemChanged );

  m_pcUI->tableWidget_4->setSortingEnabled( true );

  if( pcAsset )
  {
    // Scroll to the newly added row
    // NOTE: The row id might have just changed because of sorting, so use currentRow
    const QTableWidgetItem* pcItem{ m_pcUI->tableWidget_4->item( m_pcUI->tableWidget_4->currentRow(),
                                                                 PROP_COL_NAME ) };
    if( pcItem )
    {
      m_pcUI->tableWidget_4->scrollToItem( pcItem );
    }
  }

  SetDirty( true );
}


// slot
void AssetManager::onPropertyFilterChanged()
{
  UpdatePropertyFilter();
}


// slot
void AssetManager::onPropertyTableItemChanged( QTableWidgetItem* i_pcItem )
{
  rumAsset* pcAsset{ GetSelectedAsset() };
  rumAssert( pcAsset );
  if( !pcAsset )
  {
    return;
  }

  const rumAssetID ePropertyID{ GetSelectedPropertyID() };
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

  QString strValue{ i_pcItem->text() };

  Sqrat::Object sqValue{ pcProperty->GetProperty( ePropertyID ) };

  if( pcProperty->IsAssetRef() || ( pcProperty->GetValueType() == PropertyValueType::StringToken ) )
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

  pcAsset->SetProperty( ePropertyID, sqValue );

  SetDirty( true );
}


void AssetManager::RefreshPropertyTable()
{
  const rumAsset* pcAsset{ GetSelectedAsset() };
  if( !pcAsset )
  {
    return;
  }

  // Clear the string table
  m_pcUI->tableWidget_4->setRowCount( 0 );
  m_pcUI->tableWidget_4->setSortingEnabled( false );

  disconnect( m_pcUI->tableWidget_4, &QTableWidget::itemChanged, this, &AssetManager::onPropertyTableItemChanged );

  const PropertyContainer& rcPropertyContainer{ pcAsset->GetProperties() };

  int32_t iRow{ 0 };
  m_pcUI->tableWidget_4->setRowCount( (int32_t)rcPropertyContainer.size() );

  for( const auto& iter : rcPropertyContainer )
  {
    const rumPropertyAsset* pcProperty{ rumPropertyAsset::Fetch( iter.first ) };
    AddPropertyAsset( iRow++, *pcProperty, iter.second );
  }

  m_pcUI->tableWidget_4->setSortingEnabled( true );
  m_pcUI->tableWidget_4->sortItems( PROP_COL_NAME );

  UpdatePropertyFilter();

  // Subscribe to change notifications
  connect( m_pcUI->tableWidget_4, &QTableWidget::itemChanged, this, &AssetManager::onPropertyTableItemChanged );
}


void AssetManager::SaveTable( int32_t i_iRow )
{
  AssetType eAssetType{ GetAssetType( i_iRow ) };

  const QString& i_rcPath{ MainWindow::GetProjectDir().absolutePath() };

  switch( eAssetType )
  {
    case Broadcast_AssetType: rumAsset::ExportCSVFiles<rumBroadcastAsset>( qPrintable( i_rcPath ) ); break;
    case Creature_AssetType:  rumAsset::ExportCSVFiles<rumCreatureAsset>( qPrintable( i_rcPath ) );  break;
    case Custom_AssetType:    rumAsset::ExportCSVFiles<rumCustomAsset>( qPrintable( i_rcPath ) );    break;
    case Graphic_AssetType:   rumAsset::ExportCSVFiles<rumGraphicAsset>( qPrintable( i_rcPath ) );   break;
    case Inventory_AssetType: rumAsset::ExportCSVFiles<rumInventoryAsset>( qPrintable( i_rcPath ) ); break;
    case Map_AssetType:       rumAsset::ExportCSVFiles<rumMapAsset>( qPrintable( i_rcPath ) );       break;
    case Portal_AssetType:    rumAsset::ExportCSVFiles<rumPortalAsset>( qPrintable( i_rcPath ) );    break;
    case Sound_AssetType:     rumAsset::ExportCSVFiles<rumSoundAsset>( qPrintable( i_rcPath ) );     break;
    case Tile_AssetType:      rumAsset::ExportCSVFiles<rumTileAsset>( qPrintable( i_rcPath ) );      break;
    case Widget_AssetType:    rumAsset::ExportCSVFiles<rumWidgetAsset>( qPrintable( i_rcPath ) );    break;
    default:
      rumAssertMsg( false, "Unsupported asset type" );
      break;
  }
}


// slot
void AssetManager::selectionChanged_Asset( const QItemSelection& i_rcSelected, const QItemSelection& i_rcDeselected )
{
  const bool bAssetSelected{ m_pcUI->tableWidget_3->rowCount() > 0 && m_pcUI->tableWidget_3->currentRow() >= 0 };

  m_pcUI->actionRemove_Asset->setEnabled( bAssetSelected );
  m_pcUI->actionNew_Property->setEnabled( bAssetSelected );
  m_pcUI->tableWidget_4->setEnabled( bAssetSelected );
  m_cPropertyFilterLabel.setEnabled( bAssetSelected );
  m_cPropertyFilterEdit.setEnabled( bAssetSelected );

  RefreshPropertyTable();
}


// slot
void AssetManager::selectionChanged_AssetType( const QItemSelection& i_rcSelected,
                                               const QItemSelection& i_rcDeselected )
{
  if( m_pcUI->tableWidget->currentRow() < 0 )
  {
    m_pcUI->actionNew_Asset->setEnabled( false );
    m_cFilterLabel.setEnabled( false );
    m_cFilterEdit.setEnabled( false );
    m_cPropertyFilterLabel.setEnabled( false );
    m_cPropertyFilterEdit.setEnabled( false );
    m_pcUI->tableWidget_3->setEnabled( false );
    m_pcUI->tableWidget_4->setEnabled( false );
    return;
  }

  m_pcUI->tableWidget_4->setRowCount( 0 );
  m_pcUI->tableWidget_4->setSortingEnabled( false );
  m_pcUI->tableWidget_4->setEnabled( false );
  m_cPropertyFilterLabel.setEnabled( false );
  m_cPropertyFilterEdit.setEnabled( false );

  disconnect( m_pcUI->tableWidget_3, &QTableWidget::itemChanged, this, &AssetManager::onAssetTableItemChanged );
  disconnect( m_pcUI->tableWidget_4, &QTableWidget::itemChanged, this, &AssetManager::onPropertyTableItemChanged );

  m_pcUI->tableWidget_3->setRowCount( 0 );
  m_pcUI->tableWidget_3->setSortingEnabled( false );
  m_pcUI->tableWidget_3->setEnabled( true );
  m_pcUI->actionNew_Asset->setEnabled( true );
  m_cFilterLabel.setEnabled( true );
  m_cFilterEdit.setEnabled( true );

  switch( GetSelectedAssetType() )
  {
    case AssetType::Broadcast_AssetType: InitBroadcasts(); break;
    case AssetType::Creature_AssetType:  InitPawns( rumPawn::Creature_PawnType ); break;
    case AssetType::Custom_AssetType:    InitCustom(); break;
    case AssetType::Graphic_AssetType:   InitGraphics(); break;
    case AssetType::Inventory_AssetType: InitInventory(); break;
    case AssetType::Map_AssetType:       InitMaps(); break;
    case AssetType::Portal_AssetType:    InitPawns( rumPawn::Portal_PawnType ); break;
    case AssetType::Sound_AssetType:     InitSounds(); break;
    case AssetType::Tile_AssetType:      InitTiles(); break;
    case AssetType::Widget_AssetType:    InitPawns( rumPawn::Widget_PawnType ); break;
    default:
      rumAssertMsg( false, "Unsupported asset type" );
      break;
  }

  m_pcUI->actionSave->setEnabled( IsDirty() );

  UpdateFilter();
  UpdatePropertyFilter();

  m_pcUI->tableWidget_3->setSortingEnabled( true );
  m_pcUI->tableWidget_3->sortItems( S_COL_NAME );

  // Subscribe to change notifications
  connect( m_pcUI->tableWidget_3, &QTableWidget::itemChanged, this, &AssetManager::onAssetTableItemChanged );
  connect( m_pcUI->tableWidget_4, &QTableWidget::itemChanged, this, &AssetManager::onPropertyTableItemChanged );
}


// slot
void AssetManager::selectionChanged_Property( const QItemSelection& i_rcSelected,
                                              const QItemSelection& i_rcDeselected )
{
  const bool bAssetSelected{ m_pcUI->tableWidget_4->rowCount() > 0 && m_pcUI->tableWidget_4->currentRow() >= 0 };
  m_pcUI->actionRemove_Property->setEnabled( bAssetSelected );
}


void AssetManager::SetDirty( bool i_bDirty )
{
  if( m_bDirty != i_bDirty )
  {
    m_bDirty = i_bDirty;
    m_pcUI->actionSave->setEnabled( i_bDirty );
  }
}


// slot
void AssetManager::StringTokenPropertyChanged( rumTokenID i_eTokenID )
{
  const int32_t iRow{ m_pcUI->tableWidget_4->currentRow() };
  const int32_t iCol{ m_pcUI->tableWidget_4->currentColumn() };

  const auto& strToken{ rumStringTable::GetTokenName( i_eTokenID ) };

  QTableWidgetItem* pcCell{ new QTableWidgetItem() };
  pcCell->setData( Qt::DisplayRole, strToken.c_str() );

  QVariant cVariant{ i_eTokenID };
  if( cVariant.isValid() )
  {
    pcCell->setData( Qt::UserRole, cVariant );
  }

  m_pcUI->tableWidget_4->setItem( iRow, iCol, pcCell );
  pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
}


void AssetManager::UpdateFilter()
{
  // Hide anything that doesn't pass the filter
  const QString& strFilter{ m_cFilterEdit.text() };

  // Visit each item in the table and hide anything that doesn't match the filter settings
  const int32_t iRow{ m_pcUI->tableWidget_3->rowCount() };
  for( int32_t i{ 0 }; i < iRow; ++i )
  {
    bool bShow{ true };

    if( !strFilter.isEmpty() )
    {
      QTableWidgetItem* pcItem{ m_pcUI->tableWidget_3->item( i, S_COL_NAME ) };
      bShow = pcItem->text().contains( strFilter, Qt::CaseInsensitive );

      if( !bShow )
      {
        pcItem = m_pcUI->tableWidget_3->item( i, S_COL_CLASS );
        bShow = pcItem->text().contains( strFilter, Qt::CaseInsensitive );
      }
    }

    bShow ? m_pcUI->tableWidget_3->showRow( i ) : m_pcUI->tableWidget_3->hideRow( i );
  }

  m_pcUI->tableWidget_3->resizeColumnsToContents();
}


void AssetManager::UpdatePropertyFilter()
{
  // Hide anything that doesn't pass the filter
  const QString& strFilter{ m_cPropertyFilterEdit.text() };

  // Visit each item in the table and hide anything that doesn't match the filter settings
  const int32_t iRow{ m_pcUI->tableWidget_4->rowCount() };
  for( int32_t i{ 0 }; i < iRow; ++i )
  {
    bool bShow{ true };

    if( !strFilter.isEmpty() )
    {
      QTableWidgetItem* pcItem{ m_pcUI->tableWidget_4->item( i, PROP_COL_NAME ) };
      bShow = pcItem->text().contains( strFilter, Qt::CaseInsensitive );

      if( !bShow )
      {
        pcItem = m_pcUI->tableWidget_4->item( i, PROP_COL_VALUE );
        bShow = pcItem->text().contains( strFilter, Qt::CaseInsensitive );
      }
    }

    bShow ? m_pcUI->tableWidget_4->showRow( i ) : m_pcUI->tableWidget_4->hideRow( i );
  }

  m_pcUI->tableWidget_4->resizeColumnsToContents();
}
