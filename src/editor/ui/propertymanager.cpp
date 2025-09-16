#include <propertymanager.h>
#include <ui_propertymanager.h>

#include <mainwindow.h>

#include <QCloseEvent>
#include <QComboBox>
#include <QItemSelection>
#include <QMessageBox>

#include <u_property_asset.h>

enum PropertyColumns
{
  PROPERTY_NAME,
  PROPERTY_BASE_CLASS,
  PROPERTY_VALUE_TYPE,
  PROPERTY_DEFAULT_VALUE,
  PROPERTY_ENUM_NAME,
  PROPERTY_CONSTRAIN,
  PROPERTY_MIN_VALUE,
  PROPERTY_MAX_VALUE,
  PROPERTY_USER_FLAGS,
  PROPERTY_SERVICE_TYPE,
  PROPERTY_REPLICATION_TYPE,
  PROPERTY_PERSISTENT,
  PROPERTY_PRIORITY,
  PROPERTY_NUM_COL
};


PropertyManager::PropertyManager( QWidget* i_pcParent )
  : QMainWindow( i_pcParent )
  , m_pcFilterAction( nullptr )
  , m_pcUI( new Ui::PropertyManager )
{
  m_pcUI->setupUi( this );
  setCentralWidget( m_pcUI->splitter );

  const QIcon cPropertyIcon( ":/ui/resources/property.png" );
  setWindowIcon( cPropertyIcon );

  setMinimumWidth( 1024 );
  setMinimumHeight( 768 );

  // Property table -----------------------------------------------------------

  QStringList cLabelList;
  cLabelList << "Name" << "Base Class" << "Type" << "Default" << "Enum" << "Constrain" << "Min Value" << "Max Value"
             << "User Flags" << "Service Type" << "Replication Type" << "Persistent" << "Priority";

  m_pcUI->tableWidget_3->setMinimumWidth( 200 );
  m_pcUI->tableWidget_3->setEnabled( true );
  m_pcUI->tableWidget_3->setColumnCount( cLabelList.size() );
  m_pcUI->tableWidget_3->setRowCount( 0 );
  m_pcUI->tableWidget_3->setHorizontalHeaderLabels( cLabelList );

  m_pcUI->tableWidget_3->setContextMenuPolicy( Qt::CustomContextMenu );

  // Add a line edit on the toolbar for filtering. Seems you can't do this from designer for some stupid reason.
  m_cFilterLabel.setText( "Filter: " );
  m_pcUI->toolBar2->addWidget( &m_cFilterLabel );
  m_pcFilterAction = m_pcUI->toolBar2->addWidget( &m_cFilterEdit );

  m_cFilterLabel.setEnabled( true );
  m_cFilterEdit.setEnabled( true );

  m_pcUI->toolBar->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );

  const QIcon cAddIcon( ":/ui/resources/add.png" );
  m_pcUI->actionNew_Property->setIcon( cAddIcon );
  m_pcUI->actionNew_Property->setEnabled( true );

  const QIcon cRemoveIcon( ":/ui/resources/remove.png" );
  m_pcUI->actionRemove_Property->setIcon( cRemoveIcon );
  m_pcUI->actionRemove_Property->setEnabled( false );

  const QIcon cSaveIcon( ":/ui/resources/save.png" );
  m_pcUI->actionSave->setIcon( cSaveIcon );
  m_pcUI->actionSave->setEnabled( m_bDirty );

  // Final setup ------------------------------------------------------------

  m_pcUI->tableWidget_3->resizeColumnsToContents();

  connect( &m_cFilterEdit, SIGNAL( editingFinished() ), this, SLOT( onFilterChanged() ) );

  connect( m_pcUI->tableWidget_3->selectionModel(),
           SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
           this, SLOT( selectionChanged_Property( const QItemSelection&, const QItemSelection& ) ) );

  connect( m_pcUI->tableWidget_3, SIGNAL( cellActivated( int32_t, int32_t ) ),
           this, SLOT( onCellStartEdit( int32_t, int32_t ) ) );
  connect( m_pcUI->tableWidget_3, SIGNAL( cellDoubleClicked( int32_t, int32_t ) ),
           this, SLOT( onCellStartEdit( int32_t, int32_t ) ) );

  RefreshTable();
}


PropertyManager::~PropertyManager()
{
  delete m_pcUI;
}


void PropertyManager::AddProperty( int32_t i_iRow, rumAssetID i_eAssetID )
{
  const rumPropertyAsset* pcAsset{ rumPropertyAsset::Fetch( i_eAssetID ) };
  if( pcAsset )
  {
    AddProperty( i_iRow, *pcAsset );
  }
}


void PropertyManager::AddProperty( int32_t i_iRow, const rumPropertyAsset& i_rcAsset )
{
  if( m_pcUI->tableWidget_3->rowCount() < i_iRow + 1 )
  {
    m_pcUI->tableWidget_3->setRowCount( i_iRow + 1 );
  }

  // Property name
  QTableWidgetItem* pcCell{ new QTableWidgetItem };
  pcCell->setText( i_rcAsset.GetName().c_str() );
  pcCell->setData( Qt::UserRole, QVariant::fromValue( (int32_t)i_rcAsset.GetAssetID() ) );
  pcCell->setToolTip( rumStringUtils::ToHexString( i_rcAsset.GetAssetID() ) );
  m_pcUI->tableWidget_3->setItem( i_iRow, PROPERTY_NAME, pcCell );
  if( i_rcAsset.IsEngineProperty() )
  {
    pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
    pcCell->setBackground( Qt::lightGray );
  }

  // Base class
  pcCell = new QTableWidgetItem( i_rcAsset.GetAssetClassName().c_str() );
  m_pcUI->tableWidget_3->setItem( i_iRow, PROPERTY_BASE_CLASS, pcCell );
  if( i_rcAsset.IsEngineProperty() )
  {
    pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
    pcCell->setBackground( Qt::lightGray );
  }

  // Value type
  QString strValueType{ GetPropertyValueTypeString( i_rcAsset.GetValueType() ) };

  pcCell = new QTableWidgetItem();
  pcCell->setData( Qt::DisplayRole, strValueType );
  pcCell->setData( Qt::UserRole, rumUtility::ToUnderlyingType( i_rcAsset.GetValueType() ) );
  pcCell->setToolTip( rumStringUtils::ToHexString( rumUtility::ToUnderlyingType( i_rcAsset.GetValueType() ) ) );
  m_pcUI->tableWidget_3->setItem( i_iRow, PROPERTY_VALUE_TYPE, pcCell );
  pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
  if( i_rcAsset.IsEngineProperty() )
  {
    pcCell->setBackground( Qt::lightGray );
  }

  // Enum name
  pcCell = new QTableWidgetItem( i_rcAsset.GetEnumName().c_str() );
  m_pcUI->tableWidget_3->setItem( i_iRow, PROPERTY_ENUM_NAME, pcCell );
  if( i_rcAsset.IsEngineProperty() )
  {
    pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
    pcCell->setBackground( Qt::lightGray );
  }

  // Default value
  QString strDefaultValue;
  Sqrat::Object sqValue = i_rcAsset.GetDefaultValue();

  if( sqValue.GetType() == OT_NULL )
  {
    if( i_rcAsset.IsAssetRef() )
    {
      rumScript::SetValue( sqValue, 0 );
    }
    else
    {
      switch( i_rcAsset.GetValueType() )
      {
        case PropertyValueType::Bitfield:
        case PropertyValueType::Integer:  rumScript::SetValue( sqValue, 0 ); break;
        case PropertyValueType::Float:    rumScript::SetValue( sqValue, 0.f ); break;
        case PropertyValueType::Bool:     rumScript::SetValue( sqValue, false ); break;
        case PropertyValueType::String:   rumScript::SetValue( sqValue, "" ); break;
      }
    }
  }

  if( i_rcAsset.IsAssetRef() )
  {
    strDefaultValue = rumStringUtils::ToString( sqValue.Cast<int32_t>() );
  }
  else
  {
    switch( i_rcAsset.GetValueType() )
    {
      case PropertyValueType::Bitfield:
      case PropertyValueType::Integer: strDefaultValue = rumStringUtils::ToString( sqValue.Cast<int32_t>() ); break;
      case PropertyValueType::Float:   strDefaultValue = rumStringUtils::ToFloatString( sqValue.Cast<float>() ); break;
      case PropertyValueType::Bool:    strDefaultValue = sqValue.Cast<bool>() ? "True" : "False"; break;
      case PropertyValueType::String:  strDefaultValue = sqValue.Cast<char*>();  break;
    }
  }

  pcCell = new QTableWidgetItem( strDefaultValue );
  m_pcUI->tableWidget_3->setItem( i_iRow, PROPERTY_DEFAULT_VALUE, pcCell );
  if( i_rcAsset.IsEngineProperty() )
  {
    pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
    pcCell->setBackground( Qt::lightGray );
  }

  // Uses constraints?
  pcCell = new QTableWidgetItem( i_rcAsset.GetUsesConstraints() ? "True" : "False" );
  m_pcUI->tableWidget_3->setItem( i_iRow, PROPERTY_CONSTRAIN, pcCell );
  pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
  if( i_rcAsset.IsEngineProperty() )
  {
    pcCell->setBackground( Qt::lightGray );
  }

  // Min value
  QString strMinValue;
  sqValue = i_rcAsset.GetMinValue();

  if( sqValue.GetType() == OT_NULL )
  {
    switch( i_rcAsset.GetValueType() )
    {
      case PropertyValueType::Integer: rumScript::SetValue( sqValue, 0 ); break;
      case PropertyValueType::Float:   rumScript::SetValue( sqValue, 0.f ); break;
    }
  }

  switch( i_rcAsset.GetValueType() )
  {
    case PropertyValueType::Integer: strMinValue = rumStringUtils::ToString( sqValue.Cast<int32_t>() ); break;
    case PropertyValueType::Float:   strMinValue = rumStringUtils::ToFloatString( sqValue.Cast<float>() ); break;
  }
  pcCell = new QTableWidgetItem( strMinValue );
  m_pcUI->tableWidget_3->setItem( i_iRow, PROPERTY_MIN_VALUE, pcCell );
  if( i_rcAsset.IsEngineProperty() )
  {
    pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
    pcCell->setBackground( Qt::lightGray );
  }

  // Max value
  QString strMaxValue;
  sqValue = i_rcAsset.GetMaxValue();

  if( sqValue.GetType() == OT_NULL )
  {
    switch( i_rcAsset.GetValueType() )
    {
      case PropertyValueType::Integer: rumScript::SetValue( sqValue, 0 ); break;
      case PropertyValueType::Float:   rumScript::SetValue( sqValue, 0.f ); break;
    }
  }

  switch( i_rcAsset.GetValueType() )
  {
    case PropertyValueType::Integer: strMaxValue = rumStringUtils::ToString( sqValue.Cast<int32_t>() ); break;
    case PropertyValueType::Float:   strMaxValue = rumStringUtils::ToFloatString( sqValue.Cast<float>() ); break;
  }
  pcCell = new QTableWidgetItem( strMaxValue );
  m_pcUI->tableWidget_3->setItem( i_iRow, PROPERTY_MAX_VALUE, pcCell );
  if( i_rcAsset.IsEngineProperty() )
  {
    pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
    pcCell->setBackground( Qt::lightGray );
  }

  // User flags
  const uint32_t uiUserFlags{ i_rcAsset.GetUserFlags() };
  pcCell = new QTableWidgetItem( rumStringUtils::ToHexString( uiUserFlags ) );
  m_pcUI->tableWidget_3->setItem( i_iRow, PROPERTY_USER_FLAGS, pcCell );
  if( i_rcAsset.IsEngineProperty() )
  {
    pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
    pcCell->setBackground( Qt::lightGray );
  }

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
  m_pcUI->tableWidget_3->setItem( i_iRow, PROPERTY_SERVICE_TYPE, pcCell );
  pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
  if( i_rcAsset.IsEngineProperty() )
  {
    pcCell->setBackground( Qt::lightGray );
  }

  // Replication
  QString strReplicationType;
  switch( i_rcAsset.GetClientReplicationType() )
  {
    case None_ClientReplicationType:     strReplicationType = "None";     break;
    case Private_ClientReplicationType:  strReplicationType = "Private";  break;
    case Regional_ClientReplicationType: strReplicationType = "Regional"; break;
    case Global_ClientReplicationType:   strReplicationType = "Global";   break;
  }
  pcCell = new QTableWidgetItem();
  pcCell->setData( Qt::DisplayRole, strReplicationType );
  pcCell->setData( Qt::UserRole, i_rcAsset.GetClientReplicationType() );
  pcCell->setToolTip( rumStringUtils::ToString( i_rcAsset.GetClientReplicationType() ) );
  m_pcUI->tableWidget_3->setItem( i_iRow, PROPERTY_REPLICATION_TYPE, pcCell );
  pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
  if( i_rcAsset.IsEngineProperty() )
  {
    pcCell->setBackground( Qt::lightGray );
  }

  // Persistence
  pcCell = new QTableWidgetItem( i_rcAsset.IsPersistent() ? "True" : "False" );
  m_pcUI->tableWidget_3->setItem( i_iRow, PROPERTY_PERSISTENT, pcCell );
  pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
  if( i_rcAsset.IsEngineProperty() )
  {
    pcCell->setBackground( Qt::lightGray );
  }

  // Priority
  pcCell = new QTableWidgetItem( rumStringUtils::ToString( i_rcAsset.GetPriority() ) );
  m_pcUI->tableWidget_3->setItem( i_iRow, PROPERTY_PRIORITY, pcCell );
  if( i_rcAsset.IsEngineProperty() )
  {
    pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
    pcCell->setBackground( Qt::lightGray );
  }
}


// static
QString PropertyManager::GetPropertyValueTypeString( PropertyValueType i_eValueType )
{
  switch( i_eValueType )
  {
    case PropertyValueType::Integer:      return "Integer";
    case PropertyValueType::Bool:         return "Bool";
    case PropertyValueType::Float:        return "Float";
    case PropertyValueType::String:       return "String";
    case PropertyValueType::Bitfield:     return "Bitfield";
    case PropertyValueType::AssetRef:     return "Asset";
    case PropertyValueType::CreatureRef:  return "Creature";
    case PropertyValueType::PortalRef:    return "Portal";
    case PropertyValueType::WidgetRef:    return "Widget";
    case PropertyValueType::BroadcastRef: return "Broadcast";
    case PropertyValueType::TileRef:      return "Tile";
    case PropertyValueType::MapRef:       return "Map";
    case PropertyValueType::GraphicRef:   return "Graphic";
    case PropertyValueType::SoundRef:     return "Sound";
    case PropertyValueType::PropertyRef:  return "Property";
    case PropertyValueType::InventoryRef: return "Inventory";
    case PropertyValueType::CustomRef:    return "Custom";
    case PropertyValueType::StringToken:  return "StringToken";
    default:
      rumAssertMsg( false, "Unsupported value type" );
      break;
  }

  return "<Unknown>";
}


void PropertyManager::onCellStartEdit( int32_t i_iRow, int32_t i_iCol )
{
  const rumPropertyAsset* pcAsset{ GetAsset( i_iRow ) };
  if( !pcAsset )
  {
    return;
  }

  if( pcAsset->IsEngineProperty() )
  {
    return;
  }

  if( PROPERTY_VALUE_TYPE == i_iCol )
  {
    QComboBox* pcCombo{ new QComboBox };
    pcCombo->addItem( "Integer",     rumUtility::ToUnderlyingType( PropertyValueType::Integer ) );
    pcCombo->addItem( "Bool",        rumUtility::ToUnderlyingType( PropertyValueType::Bool ) );
    pcCombo->addItem( "Float",       rumUtility::ToUnderlyingType( PropertyValueType::Float ) );
    pcCombo->addItem( "String",      rumUtility::ToUnderlyingType( PropertyValueType::String ) );
    pcCombo->addItem( "Bitfield",    rumUtility::ToUnderlyingType( PropertyValueType::Bitfield ) );
    pcCombo->addItem( "Asset",       rumUtility::ToUnderlyingType( PropertyValueType::AssetRef ) );
    pcCombo->addItem( "Creature",    rumUtility::ToUnderlyingType( PropertyValueType::CreatureRef ) );
    pcCombo->addItem( "Portal",      rumUtility::ToUnderlyingType( PropertyValueType::PortalRef ) );
    pcCombo->addItem( "Widget",      rumUtility::ToUnderlyingType( PropertyValueType::WidgetRef ) );
    pcCombo->addItem( "Broadcast",   rumUtility::ToUnderlyingType( PropertyValueType::BroadcastRef ) );
    pcCombo->addItem( "Tile",        rumUtility::ToUnderlyingType( PropertyValueType::TileRef ) );
    pcCombo->addItem( "Map",         rumUtility::ToUnderlyingType( PropertyValueType::MapRef ) );
    pcCombo->addItem( "Graphic",     rumUtility::ToUnderlyingType( PropertyValueType::GraphicRef ) );
    pcCombo->addItem( "Sound",       rumUtility::ToUnderlyingType( PropertyValueType::SoundRef ) );
    pcCombo->addItem( "Property",    rumUtility::ToUnderlyingType( PropertyValueType::PropertyRef ) );
    pcCombo->addItem( "Inventory",   rumUtility::ToUnderlyingType( PropertyValueType::InventoryRef ) );
    pcCombo->addItem( "Custom",      rumUtility::ToUnderlyingType( PropertyValueType::CustomRef ) );
    pcCombo->addItem( "StringToken", rumUtility::ToUnderlyingType( PropertyValueType::StringToken ) );

    const int32_t iIndex{ pcCombo->findData( rumUtility::ToUnderlyingType( pcAsset->GetValueType() ) ) };
    pcCombo->setCurrentIndex( iIndex );
    pcCombo->view()->setMinimumWidth( pcCombo->minimumSizeHint().width() );

    connect( pcCombo, SIGNAL( currentIndexChanged( const QString& ) ),
             this, SLOT( itemComboChanged_Property( const QString& ) ) );
    m_pcUI->tableWidget_3->setCellWidget( i_iRow, i_iCol, pcCombo );
  }
  else if( PROPERTY_CONSTRAIN == i_iCol )
  {
    QComboBox* pcCombo{ new QComboBox };
    pcCombo->addItem( "False", false );
    pcCombo->addItem( "True", true );
    pcCombo->setCurrentIndex( pcAsset->GetUsesConstraints() ? 1 : 0 );
    pcCombo->view()->setMinimumWidth( pcCombo->minimumSizeHint().width() );
    connect( pcCombo, SIGNAL( currentIndexChanged( const QString& ) ),
              this, SLOT( itemComboChanged_Property( const QString& ) ) );
    m_pcUI->tableWidget_3->setCellWidget( i_iRow, i_iCol, pcCombo );
  }
  else if( PROPERTY_SERVICE_TYPE == i_iCol )
  {
    QComboBox* pcCombo{ new QComboBox };
    pcCombo->addItem( "Shared", ServiceType::Shared_ServiceType );
    pcCombo->addItem( "Server", ServiceType::Server_ServiceType );
    pcCombo->addItem( "Client", ServiceType::Client_ServiceType );

    const int32_t iIndex{ pcCombo->findData( pcAsset->GetServiceType() ) };
    pcCombo->setCurrentIndex( iIndex );
    pcCombo->view()->setMinimumWidth( pcCombo->minimumSizeHint().width() );

    connect( pcCombo, SIGNAL( currentIndexChanged( const QString& ) ),
             this, SLOT( itemComboChanged_Property( const QString& ) ) );
    m_pcUI->tableWidget_3->setCellWidget( i_iRow, i_iCol, pcCombo );
  }
  else if( PROPERTY_REPLICATION_TYPE == i_iCol )
  {
    QComboBox* pcCombo{ new QComboBox };
    pcCombo->addItem( "None", ClientReplicationType::None_ClientReplicationType );
    pcCombo->addItem( "Private", ClientReplicationType::Private_ClientReplicationType );
    pcCombo->addItem( "Regional", ClientReplicationType::Regional_ClientReplicationType );
    pcCombo->addItem( "Global", ClientReplicationType::Global_ClientReplicationType );

    const int32_t iIndex{ pcCombo->findData( pcAsset->GetClientReplicationType() ) };
    pcCombo->setCurrentIndex( iIndex );
    pcCombo->view()->setMinimumWidth( pcCombo->minimumSizeHint().width() );

    connect( pcCombo, SIGNAL( currentIndexChanged( const QString& ) ),
             this, SLOT( itemComboChanged_Property( const QString& ) ) );
    m_pcUI->tableWidget_3->setCellWidget( i_iRow, i_iCol, pcCombo );
  }
  else if( PROPERTY_PERSISTENT == i_iCol )
  {
    QComboBox* pcCombo{ new QComboBox };
    pcCombo->addItem( "False", false );
    pcCombo->addItem( "True", true );
    pcCombo->setCurrentIndex( pcAsset->IsPersistent() ? 1 : 0 );
    pcCombo->view()->setMinimumWidth( pcCombo->minimumSizeHint().width() );
    connect( pcCombo, SIGNAL( currentIndexChanged( const QString& ) ),
             this, SLOT( itemComboChanged_Property( const QString& ) ) );
    m_pcUI->tableWidget_3->setCellWidget( i_iRow, i_iCol, pcCombo );
  }
}


void PropertyManager::closeEvent( QCloseEvent* i_pcEvent )
{
  if( m_bDirty )
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


rumPropertyAsset* PropertyManager::GetAsset( int32_t i_iRow ) const
{
  return rumPropertyAsset::Fetch( GetAssetID( i_iRow ) );
}


rumAssetID PropertyManager::GetAssetID( int32_t i_iRow ) const
{
  QTableWidgetItem* pcItem{ nullptr };

  if( i_iRow <= m_pcUI->tableWidget_3->rowCount() )
  {
    pcItem = m_pcUI->tableWidget_3->item( i_iRow, PROPERTY_NAME );
  }

  return pcItem ? (rumAssetID)pcItem->data( Qt::UserRole ).toInt() : INVALID_ASSET_ID;
}


QString PropertyManager::GetName( int32_t i_iRow ) const
{
  QTableWidgetItem* pcItem{ nullptr };

  if( i_iRow <= m_pcUI->tableWidget_3->rowCount() )
  {
    pcItem = m_pcUI->tableWidget_3->item( i_iRow, PROPERTY_NAME );
  }

  return pcItem ? pcItem->text() : QString();
}


rumPropertyAsset* PropertyManager::GetSelectedProperty() const
{
  const int32_t iRow{ m_pcUI->tableWidget_3->currentRow() };
  return GetAsset( iRow );
}


// slot
void PropertyManager::itemComboChanged_Property( const QString& i_strText )
{
  const int32_t iRow{ m_pcUI->tableWidget_3->currentRow() };

  // Fetch the asset from the affected row
  const rumPropertyAsset* pcAsset{ GetAsset( iRow ) };
  rumAssert( pcAsset );
  if( !pcAsset )
  {
    return;
  }

  const int32_t iCol{ m_pcUI->tableWidget_3->currentColumn() };

  switch( iCol )
  {
    case PROPERTY_VALUE_TYPE:
    case PROPERTY_CONSTRAIN:
    case PROPERTY_PERSISTENT:
    case PROPERTY_SERVICE_TYPE:
    case PROPERTY_REPLICATION_TYPE:
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
      break;
    }

    default:
      rumAssertMsg( false, "Unexpected column modified" );
      return;
  }

  SetDirty( true );
}


// slot
void PropertyManager::on_actionFind_References_triggered()
{
  const auto pcProperty{ GetSelectedProperty() };
  if( pcProperty == nullptr )
  {
    return;
  }

  MainWindow::FindAssetReferences( *pcProperty, true );
}


// slot
void PropertyManager::on_actionNew_Property_triggered()
{
  // Clear the filter
  m_cFilterEdit.clear();

  UpdateFilter();

  disconnect( m_pcUI->tableWidget_3, &QTableWidget::itemChanged, this, &PropertyManager::onTableItemChanged );

  m_pcUI->tableWidget_3->setSortingEnabled( false );

  const int32_t iRow{ m_pcUI->tableWidget_3->rowCount() };
  const rumAssetID eAssetID{ rumPropertyAsset::GetNextFreeID() };

  // Create the new asset
  std::string strName{ "Property_" };
  strName += rumStringUtils::ToHexString( eAssetID );
  const std::string strBaseClass{ rumStringUtils::NullString() };
  const rumAsset* pcAsset{ rumAsset::CreateAsset<rumPropertyAsset>( eAssetID, strName, strBaseClass ) };
  if( pcAsset )
  {
    rumAsset::RegisterClass<rumPropertyAsset>( pcAsset );

    // Add new property to table widget
    AddProperty( iRow, pcAsset->GetAssetID() );

    m_pcUI->tableWidget_3->selectRow( iRow );
  }

  // Subscribe to change notifications
  connect( m_pcUI->tableWidget_3, &QTableWidget::itemChanged, this, &PropertyManager::onTableItemChanged );

  m_pcUI->tableWidget_3->setSortingEnabled( true );

  if( pcAsset )
  {
    // Scroll to the newly added row
    // NOTE: The row id might have just changed because of sorting, so use currentRow
    const QTableWidgetItem* pcItem{ m_pcUI->tableWidget_3->item( m_pcUI->tableWidget_3->currentRow(),
                                                                 PROPERTY_NAME ) };
    if( pcItem )
    {
      m_pcUI->tableWidget_3->scrollToItem( pcItem );
    }
  }

  SetDirty( true );
}


void PropertyManager::on_actionRemove_Property_triggered()
{
  const auto* pcAsset{ GetSelectedProperty() };
  if( nullptr == pcAsset )
  {
    return;
  }

  const auto& rcAsset{ *pcAsset };

  QString strQuestion{ "Are you sure you want to remove property " };
  strQuestion += rcAsset.GetName().c_str();
  strQuestion += '?';

  // Verify delete
  if( QMessageBox::Yes == QMessageBox::question( this, "Verify", strQuestion,
                                                 QMessageBox::Yes | QMessageBox::No,
                                                 QMessageBox::No ) )
  {
    // Audit the property's references and disallow removal if anything is found
    const uint32_t uiNumRefs{ MainWindow::FindAssetReferences( rcAsset, false /* no logging */ ) };
    if( uiNumRefs > 0 )
    {
      // TODO - list the references
      QMessageBox::critical( this, "Remove failed",
                             "You cannot remove the property because it has existing data references",
                             QMessageBox::Ok );
    }
    else
    {
      // Remove the table widget row
      const int32_t iRow{ m_pcUI->tableWidget_3->currentRow() };
      m_pcUI->tableWidget_3->removeRow( iRow );

      rumPropertyAsset::Remove( rcAsset.GetAssetID() );

      SetDirty( true );
    }
  }
}


// slot
void PropertyManager::on_actionSave_triggered()
{
  rumAsset::ExportCSVFiles<rumPropertyAsset>( qPrintable( MainWindow::GetProjectDir().absolutePath() ) );
  SetDirty( false );
}


// slot
void PropertyManager::on_tableWidget_3_customContextMenuRequested( const QPoint& i_rcPos )
{
  if( i_rcPos.isNull() )
  {
    return;
  }

  QMenu* pcMenu{ new QMenu( m_pcUI->tableWidget_3 ) };
  pcMenu->addAction( m_pcUI->actionNew_Property );
  pcMenu->addAction( m_pcUI->actionRemove_Property );
  pcMenu->addSeparator();
  pcMenu->addAction( m_pcUI->actionFind_References );

  pcMenu->exec( m_pcUI->tableWidget_3->mapToGlobal( i_rcPos ) );
}


// slot
void PropertyManager::onFilterChanged()
{
  UpdateFilter();
}


// slot
void PropertyManager::onTableItemChanged( QTableWidgetItem* i_pcItem )
{
  const int32_t iRow{ i_pcItem->row() };
  rumPropertyAsset* pcAsset{ GetAsset( iRow ) };
  rumAssert( pcAsset );
  if( !pcAsset )
  {
    return;
  }

  const int32_t iCol{ i_pcItem->column() };
  switch( iCol )
  {
    case PROPERTY_NAME:
      pcAsset->SetName( qPrintable( i_pcItem->text() ) );
      break;

    case PROPERTY_BASE_CLASS:
      pcAsset->SetBaseClassOverride( qPrintable( i_pcItem->text() ) );
      break;

    case PROPERTY_VALUE_TYPE:
    {
      const QVariant cVariant{ i_pcItem->data( Qt::UserRole ) };
      pcAsset->SetValueType( PropertyValueType( cVariant.toInt() ) );
      break;
    }

    case PROPERTY_ENUM_NAME:
      pcAsset->SetEnumName( qPrintable( i_pcItem->text() ) );
      break;

    case PROPERTY_DEFAULT_VALUE:
    {
      Sqrat::Object sqObject;

      switch( pcAsset->GetValueType() )
      {
        case PropertyValueType::Bitfield:
        case PropertyValueType::Integer:
          rumScript::SetValue( sqObject, rumStringUtils::ToInt( qPrintable( i_pcItem->text() ) ) );
          break;

        case PropertyValueType::Bool:
          rumScript::SetValue( sqObject, rumStringUtils::ToBool( qPrintable( i_pcItem->text() ) ) );
          break;

        case PropertyValueType::Float:
          rumScript::SetValue( sqObject, i_pcItem->text().toFloat() );
          break;

        case PropertyValueType::String:
          rumScript::SetValue( sqObject, qPrintable( i_pcItem->text() ) );
          break;

        default:
          rumAssertMsg( false, "Unexpected value type" );
          return;
      }

      pcAsset->SetDefaultValue( sqObject );
      break;
    }

    case PROPERTY_CONSTRAIN:
      pcAsset->SetUsesConstraints( rumStringUtils::ToBool( qPrintable( i_pcItem->text() ) ) );
      break;

    case PROPERTY_MIN_VALUE:
    {
      Sqrat::Object sqObject;

      switch( pcAsset->GetValueType() )
      {
        case PropertyValueType::Bitfield:
        case PropertyValueType::Integer:
          rumScript::SetValue( sqObject, rumStringUtils::ToInt( qPrintable( i_pcItem->text() ) ) );
          break;

        case PropertyValueType::Bool:
          rumScript::SetValue( sqObject, rumStringUtils::ToBool( qPrintable( i_pcItem->text() ) ) );
          break;

        case PropertyValueType::Float:
          rumScript::SetValue( sqObject, i_pcItem->text().toFloat() );
          break;

        case PropertyValueType::String:
          rumScript::SetValue( sqObject, qPrintable( i_pcItem->text() ) );
          break;

        default:
          rumAssertMsg( false, "Unexpected value type" );
          return;
      }

      pcAsset->SetMinValue( sqObject );
      break;
    }

    case PROPERTY_MAX_VALUE:
    {
      Sqrat::Object sqObject;

      switch( pcAsset->GetValueType() )
      {
        case PropertyValueType::Bitfield:
        case PropertyValueType::Integer:
          rumScript::SetValue( sqObject, rumStringUtils::ToInt( qPrintable( i_pcItem->text() ) ) );
          break;

        case PropertyValueType::Bool:
          rumScript::SetValue( sqObject, rumStringUtils::ToBool( qPrintable( i_pcItem->text() ) ) );
          break;

        case PropertyValueType::Float:
          rumScript::SetValue( sqObject, i_pcItem->text().toFloat() );
          break;

        case PropertyValueType::String:
          rumScript::SetValue( sqObject, qPrintable( i_pcItem->text() ) );
          break;

        default:
          rumAssertMsg( false, "Unexpected value type" );
          return;
      }

      pcAsset->SetMaxValue( sqObject );
      break;
    }

    case PROPERTY_USER_FLAGS:
    {
      std::string strUserFlags{ qPrintable( i_pcItem->text() ) };
      pcAsset->SetUserFlags( rumStringUtils::ToUInt( strUserFlags ) );
      break;
    }

    case PROPERTY_SERVICE_TYPE:
    {
      const QVariant cVariant{ i_pcItem->data( Qt::UserRole ) };
      pcAsset->SetServiceType( ServiceType( cVariant.toInt() ) );
      break;
    }

    case PROPERTY_REPLICATION_TYPE:
    {
      const QVariant cVariant{ i_pcItem->data( Qt::UserRole ) };
      pcAsset->SetClientReplicationType( ClientReplicationType( cVariant.toInt() ) );
      break;
    }

    case PROPERTY_PERSISTENT:
      pcAsset->SetPersistence( rumStringUtils::ToBool( qPrintable( i_pcItem->text() ) ) );
      break;

    case PROPERTY_PRIORITY:
    {
      const std::string strPriority{ qPrintable( i_pcItem->text() ) };
      pcAsset->SetPriority( rumStringUtils::ToInt( strPriority ) );
      break;
    }

    default:
      rumAssertMsg( false, "Unexpected column modified" );
      return;
  }

  SetDirty( true );
}


void PropertyManager::RefreshTable()
{
  // Clear the string table
  m_pcUI->tableWidget_3->setRowCount( 0 );
  m_pcUI->tableWidget_3->setSortingEnabled( false );

  disconnect( m_pcUI->tableWidget_3, &QTableWidget::itemChanged, this, &PropertyManager::onTableItemChanged );

  const auto cAssetsHash{ rumPropertyAsset::GetAssetHash() };

  int32_t iRow{ 0 };
  m_pcUI->tableWidget_3->setRowCount( (int32_t)cAssetsHash.size() );

  for( const auto& iter : cAssetsHash )
  {
    rumPropertyAsset* pcAsset{ iter.second };
    if( pcAsset )
    {
      AddProperty( iRow++, *pcAsset );
    }
  }

  m_pcUI->actionSave->setEnabled( m_bDirty );
  UpdateFilter();

  m_pcUI->tableWidget_3->setSortingEnabled( true );
  m_pcUI->tableWidget_3->sortItems( PROPERTY_NAME );

  // Subscribe to change notifications
  connect( m_pcUI->tableWidget_3, &QTableWidget::itemChanged, this, &PropertyManager::onTableItemChanged );
}


// slot
void PropertyManager::selectionChanged_Property( const QItemSelection& i_rcSelected,
                                                 const QItemSelection& i_rcDeselected )
{
  if( m_pcUI->tableWidget_3->selectedItems().empty() )
  {
    m_pcUI->actionRemove_Property->setEnabled( false );
  }
  else
  {
    m_pcUI->actionRemove_Property->setEnabled( true );
  }
}


void PropertyManager::SetDirty( bool i_bDirty )
{
  if( m_bDirty != i_bDirty )
  {
    m_bDirty = i_bDirty;
    m_pcUI->actionSave->setEnabled( m_bDirty );
  }
}


void PropertyManager::UpdateFilter()
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
      const QTableWidgetItem* pcItem{ m_pcUI->tableWidget_3->item( i, PROPERTY_NAME ) };
      bShow = pcItem->text().contains( strFilter, Qt::CaseInsensitive );

      if( !bShow )
      {
        pcItem = m_pcUI->tableWidget_3->item( i, PROPERTY_BASE_CLASS );
        bShow = pcItem->text().contains( strFilter, Qt::CaseInsensitive );
      }

      if( !bShow )
      {
        pcItem = m_pcUI->tableWidget_3->item( i, PROPERTY_ENUM_NAME );
        bShow = pcItem->text().contains( strFilter, Qt::CaseInsensitive );
      }
    }

    bShow ? m_pcUI->tableWidget_3->showRow( i ) : m_pcUI->tableWidget_3->hideRow( i );
  }

  m_pcUI->tableWidget_3->resizeColumnsToContents();
}
