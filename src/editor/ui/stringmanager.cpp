#include <stringmanager.h>
#include <ui_stringmanager.h>

#include <mainwindow.h>
#include <languagetranslator.h>

#include <QCloseEvent>
#include <QComboBox>
#include <QItemSelection>
#include <QLineEdit>
#include <QMessageBox>

// Used for translation functionality:
//#include <QFileDialog>
//#include <QFileInfo>
//#include <QNetworkAccessManager>
//#include <QNetworkRequest>
//#include <QNetworkReply>
//#include <QTextCodec>
//#include <QUrlQuery>

#include <u_utility.h>
#include <u_zlib.h>

enum TableColumns
{
  D_COL_NAME         = 0,
  D_COL_SERVICE_TYPE = 1
};

enum LanguageColumns
{
  L_COL_NAME = 0,
  L_COL_CODE = 1,
};

enum TokenColumns
{
  S_COL_TOKEN       = 0,
  S_COL_TRANSLATION = 1
};


StringManager::StringManager( QWidget* i_pcParent )
  : QMainWindow( i_pcParent )
  , m_pcFilterAction( nullptr )
  , m_pcUI( new Ui::StringManager )
{
  m_pcUI->setupUi( this );
  setCentralWidget( m_pcUI->splitter );

  const QIcon cStringIcon( ":/ui/resources/string.png" );
  setWindowIcon( cStringIcon );

  setMinimumWidth( 1024 );
  setMinimumHeight( 768 );

  const QIcon cSaveIcon( ":/ui/resources/save.png" );
  m_pcUI->actionSave->setIcon( cSaveIcon );

  m_pcUI->toolBar->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );

  // String table -----------------------------------------------------------

  InitStringTables();

  // Single selections only
  m_pcUI->tableWidget->setSelectionBehavior( QAbstractItemView::SelectRows );
  m_pcUI->tableWidget->setSelectionMode( QAbstractItemView::SingleSelection );

  m_pcUI->tableWidget->setContextMenuPolicy( Qt::CustomContextMenu );

  connect( m_pcUI->tableWidget, SIGNAL( cellActivated( int32_t, int32_t ) ),
           this, SLOT( OnStringTableItemStartEdit( int32_t, int32_t ) ) );

  m_pcUI->actionNew_String_Table->setEnabled( true );

  const QIcon cAddIcon( ":/ui/resources/add.png" );
  m_pcUI->actionNew_String_Table->setIcon( cAddIcon );

  const QIcon cRemoveIcon( ":/ui/resources/remove.png" );
  m_pcUI->actionRemove_String_Table->setIcon( cRemoveIcon );

  // Language table ---------------------------------------------------------

  InitLanguages();

  // Single selections only
  m_pcUI->tableWidget_2->setSelectionBehavior( QAbstractItemView::SelectRows );
  m_pcUI->tableWidget_2->setSelectionMode( QAbstractItemView::SingleSelection );

  m_pcUI->tableWidget_2->setContextMenuPolicy( Qt::CustomContextMenu );

  m_pcUI->actionNew_Language->setEnabled( true );
  m_pcUI->actionRemove_Language->setEnabled( false );

  m_pcUI->actionNew_Language->setIcon( cAddIcon );
  m_pcUI->actionRemove_Language->setIcon( cRemoveIcon );

  connect( m_pcUI->tableWidget_2->selectionModel(),
           SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
           this, SLOT( selectionChanged_Language( const QItemSelection&, const QItemSelection& ) ) );

  // Translation table --------------------------------------------------------

  m_pcUI->actionNew_String->setEnabled( false );
  m_pcUI->actionNew_String->setIcon( cAddIcon );

  m_pcUI->actionRemove_String->setEnabled( false );
  m_pcUI->actionRemove_String->setIcon( cRemoveIcon );

  m_pcUI->actionTranslate_All->setEnabled( false );

  // Add a line edit on the string's toolbar for filtering. Seems you can't
  // do this from designer for some stupid reason
  m_pcUI->toolBar3->addSeparator();
  m_cFilterLabel.setText( "Filter: " );
  m_pcUI->toolBar3->addWidget( &m_cFilterLabel );
  m_pcFilterAction = m_pcUI->toolBar3->addWidget( &m_cFilterEdit );

  m_pcUI->tableWidget_3->setContextMenuPolicy( Qt::CustomContextMenu );

  m_cFilterLabel.setEnabled( false );
  m_cFilterEdit.setEnabled( false );

  connect( &m_cFilterEdit, SIGNAL( editingFinished() ), this, SLOT( onFilterChanged() ) );

  connect( m_pcUI->tableWidget_3->selectionModel(),
           SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
           this, SLOT( selectionChanged_String( const QItemSelection&, const QItemSelection& ) ) );

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

  // Microsoft Translator API Support ---------------------------------------

  /*QSqlDatabase& rcDatabase{ MainWindow::GetSqlDatabase( "USER" ) };
  Q_ASSERT( rcDatabase.isValid() );
  QSqlQuery cQuery( rcDatabase );

  // Fetch translator info from the game database
  if( cQuery.exec( "SELECT mstranslate_client_id, mstranslate_client_secret FROM settings LIMIT 1" ) )
  {
    while( cQuery.next() )
    {
      m_cMSTranslateInfo.m_strClientID = sqlQuery.value( 0 ).toString();
      m_cMSTranslateInfo.m_strClientSecret = sqlQuery.value( 1 ).toString();
    }
  }

  m_pcUI->actionTranslate_All->setEnabled( false );*/
}


StringManager::~StringManager()
{
  delete m_pcUI;
}


void StringManager::AddLanguage( const QString& i_strName, const QString& i_strCode,
                                 rumLanguageID i_iLanguageID ) const
{
  const int32_t iRow{ m_pcUI->tableWidget_2->rowCount() };
  m_pcUI->tableWidget_2->setRowCount( iRow + 1 );

  // Language name column
  QTableWidgetItem* pcCell{ new QTableWidgetItem() };
  pcCell->setData( Qt::DisplayRole, i_strName );
  pcCell->setData( Qt::UserRole, QVariant::fromValue( i_iLanguageID ) );
  m_pcUI->tableWidget_2->setItem( iRow, L_COL_NAME, pcCell );

  // Language code column
  pcCell = new QTableWidgetItem( i_strCode );
  m_pcUI->tableWidget_2->setItem( iRow, L_COL_CODE, pcCell );
}


void StringManager::AddString( int32_t i_iRow,
                               rumTokenID i_iTokenID,
                               const QString& i_strToken,
                               const QString& i_strTranslation ) const
{
  if( m_pcUI->tableWidget_3->rowCount() < i_iRow + 1 )
  {
    m_pcUI->tableWidget_3->setRowCount( i_iRow + 1 );
  }

  // Token name
  QTableWidgetItem* pcCell{ new QTableWidgetItem() };
  pcCell->setData( Qt::DisplayRole, i_strToken );
  pcCell->setData( Qt::UserRole, i_iTokenID );
  pcCell->setToolTip( QString( rumStringUtils::ToHexString( i_iTokenID ) ) );
  m_pcUI->tableWidget_3->setItem( i_iRow, S_COL_TOKEN, pcCell );

  // Localized string
  pcCell = new QTableWidgetItem( i_strTranslation );
  m_pcUI->tableWidget_3->setItem( i_iRow, S_COL_TRANSLATION, pcCell );
}


void StringManager::AddStringTable( const rumStringTable& i_rcStringTable ) const
{
  const int32_t iRow{ m_pcUI->tableWidget->rowCount() };
  m_pcUI->tableWidget->setRowCount( iRow + 1 );

  // Name
  QTableWidgetItem* pcCell{ new QTableWidgetItem };
  pcCell->setData( Qt::DisplayRole, i_rcStringTable.GetName().c_str() );
  pcCell->setData( Qt::UserRole, QVariant::fromValue( (int32_t)i_rcStringTable.GetID() ) );
  m_pcUI->tableWidget->setItem( iRow, D_COL_NAME, pcCell );

  // Service type
  QString strServiceType;
  switch( i_rcStringTable.GetServiceType() )
  {
    case Shared_ServiceType: strServiceType = "Shared"; break;
    case Server_ServiceType: strServiceType = "Server"; break;
    case Client_ServiceType: strServiceType = "Client"; break;
  }
  pcCell = new QTableWidgetItem();
  pcCell->setData( Qt::DisplayRole, strServiceType );
  pcCell->setData( Qt::UserRole, QVariant::fromValue( (int32_t)i_rcStringTable.GetServiceType() ) );
  pcCell->setToolTip( rumStringUtils::ToString( i_rcStringTable.GetServiceType() ) );
  m_pcUI->tableWidget->setItem( iRow, D_COL_SERVICE_TYPE, pcCell );
  pcCell->setFlags( pcCell->flags() ^ Qt::ItemIsEditable );
}


void StringManager::closeEvent( QCloseEvent* i_pcEvent )
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


QString StringManager::GetLanguageName( int32_t i_iRow ) const
{
  QTableWidgetItem* pcItem{ nullptr };

  if( i_iRow <= m_pcUI->tableWidget_2->rowCount() )
  {
    pcItem = m_pcUI->tableWidget_2->item( i_iRow, L_COL_NAME );
  }

  return pcItem ? pcItem->text() : QString();
}


rumStringTableID StringManager::GetSelectedStringTableID() const
{
  // Get the database type: Server, Client, or Shared
  const int32_t iRow{ m_pcUI->tableWidget->currentRow() };
  const QTableWidgetItem* pcItem{ m_pcUI->tableWidget->item( iRow, D_COL_NAME ) };
  return pcItem ? (rumStringTableID)pcItem->data( Qt::UserRole ).toInt() : rumStringTable::INVALID_STRINGTABLE_ID;
}


QString StringManager::GetLanguageCode( int32_t i_iRow ) const
{
  QTableWidgetItem* pcItem{ nullptr };

  if( i_iRow <= m_pcUI->tableWidget_2->rowCount() )
  {
    pcItem = m_pcUI->tableWidget_2->item( i_iRow, L_COL_CODE );
  }

  return pcItem ? pcItem->text() : QString();
}


rumLanguageID StringManager::GetSelectedLanguageID() const
{
  const int32_t iRow{ m_pcUI->tableWidget_2->currentRow() };
  const QTableWidgetItem* pcItem{ m_pcUI->tableWidget_2->item( iRow, L_COL_NAME ) };
  return pcItem ? (rumLanguageID)pcItem->data( Qt::UserRole ).toInt() : rumLanguage::INVALID_LANGUAGE_ID;
}


rumTokenID StringManager::GetTokenID( int32_t i_iRow ) const
{
  QTableWidgetItem* pcItem{ nullptr };

  if( i_iRow <= m_pcUI->tableWidget_3->rowCount() )
  {
    pcItem = m_pcUI->tableWidget_3->item( i_iRow, S_COL_TOKEN );
  }

  return pcItem ? (rumTokenID)pcItem->data( Qt::UserRole ).toInt() : rumStringTable::INVALID_TOKEN_ID;
}


QString StringManager::GetTableName( int32_t i_iRow ) const
{
  QTableWidgetItem* pcItem{ nullptr };

  if( i_iRow <= m_pcUI->tableWidget->rowCount() )
  {
    pcItem = m_pcUI->tableWidget->item( i_iRow, D_COL_NAME );
  }

  return pcItem ? pcItem->text() : QString();
}


QString StringManager::GetToken( int32_t i_iRow ) const
{
  QTableWidgetItem* pcItem{ nullptr };

  if( i_iRow <= m_pcUI->tableWidget_3->rowCount() )
  {
    pcItem = m_pcUI->tableWidget_3->item( i_iRow, S_COL_TOKEN );
  }

  return pcItem ? pcItem->text() : QString();
}


void StringManager::InitLanguages()
{
  disconnect( m_pcUI->tableWidget_2->selectionModel(),
              SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
              this, SLOT( selectionChanged_Language( const QItemSelection&, const QItemSelection& ) ) );

  disconnect( m_pcUI->tableWidget_2, &QTableWidget::itemChanged, this, &StringManager::onLanguageItemChanged );

  // Set up the language table
  QStringList cLabelList;
  cLabelList << "Language" << "Code";

  m_pcUI->tableWidget_2->setMinimumWidth( 200 );
  m_pcUI->tableWidget_2->setColumnCount( cLabelList.size() );
  m_pcUI->tableWidget_2->setRowCount( 0 );
  m_pcUI->tableWidget_2->setHorizontalHeaderLabels( cLabelList );

  QHeaderView* pcHorizontalHeader{ m_pcUI->tableWidget_2->horizontalHeader() };
  pcHorizontalHeader->setStretchLastSection( true );

  // Fetch all loaded string tables
  const std::vector<rumLanguageID> cLanguageIDs{ rumLanguage::GetLanguageIDs() };
  for( const auto iter : cLanguageIDs )
  {
    const rumLanguage& rcLanguage{ rumLanguage::GetLanguageInfo( iter ) };
    AddLanguage( rcLanguage.GetName().c_str(), rcLanguage.GetCode().c_str(), rcLanguage.GetID() );
  }

  if( m_pcUI->tableWidget_2->rowCount() == 0 )
  {
    // Add the default language
    AddLanguage( "English", "en", 1 );
  }

  m_pcUI->tableWidget_2->resizeColumnsToContents();

  connect( m_pcUI->tableWidget_2->selectionModel(),
           SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
           this, SLOT( selectionChanged_Language( const QItemSelection&, const QItemSelection& ) ) );

  connect( m_pcUI->tableWidget_2, &QTableWidget::itemChanged, this, &StringManager::onLanguageItemChanged );

  m_pcUI->actionRemove_Language->setEnabled( false );

  m_pcUI->tableWidget_2->selectRow( 0 );
}


void StringManager::InitStringTables()
{
  disconnect( m_pcUI->tableWidget->selectionModel(),
              SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
              this, SLOT( selectionChanged_StringTable( const QItemSelection&, const QItemSelection& ) ) );

  disconnect( m_pcUI->tableWidget, &QTableWidget::itemChanged, this, &StringManager::onStringTableItemChanged );

  QStringList cLabelList;
  cLabelList << "String Table" << "Service";

  m_pcUI->tableWidget->setMinimumWidth( 200 );
  m_pcUI->tableWidget->setColumnCount( cLabelList.size() );
  m_pcUI->tableWidget->setRowCount( 0 );
  m_pcUI->tableWidget->setHorizontalHeaderLabels( cLabelList );

  // Fetch all loaded string tables
  const std::vector<rumStringTableID> cStringTableIDs{ rumStringTable::GetStringTableIDs() };
  for( const auto& iter : cStringTableIDs )
  {
    const rumStringTableID iStringTableID{ iter };
    const rumStringTable& rcStringTable{ rumStringTable::GetStringTable( iter ) };
    AddStringTable( rcStringTable );
  }

  m_pcUI->tableWidget->resizeColumnsToContents();

  connect( m_pcUI->tableWidget->selectionModel(),
           SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
           this, SLOT( selectionChanged_StringTable( const QItemSelection&, const QItemSelection& ) ) );

  connect( m_pcUI->tableWidget, &QTableWidget::itemChanged, this, &StringManager::onStringTableItemChanged );

  m_pcUI->actionRemove_String_Table->setEnabled( false );
}


bool StringManager::IsLanguageCodeUnique( const QString& i_strCode ) const
{
  bool bUnique{ true };

  const int32_t iSelectedRow{ m_pcUI->tableWidget_2->currentRow() };

  // Visit each table row and compare names
  const int32_t iNumRows{ m_pcUI->tableWidget_2->rowCount() };
  for( int32_t iRow{ 0 }; iRow < iNumRows && bUnique; ++iRow )
  {
    if( iRow != iSelectedRow )
    {
      const QString& strName{ GetLanguageCode( iRow ) };
      if( strName.compare( i_strCode, Qt::CaseInsensitive ) == 0 )
      {
        bUnique = false;
      }
    }
  }

  return bUnique;
}


bool StringManager::IsLanguageUnique( const QString& i_strName ) const
{
  bool bUnique{ true };

  const int32_t iSelectedRow{ m_pcUI->tableWidget_2->currentRow() };

  // Visit each table row and compare names
  const int32_t iNumRows{ m_pcUI->tableWidget_2->rowCount() };
  for( int32_t iRow{ 0 }; iRow < iNumRows && bUnique; ++iRow )
  {
    if( iRow != iSelectedRow )
    {
      const QString& strName{ GetLanguageName( iRow ) };
      if( strName.compare( i_strName, Qt::CaseInsensitive ) == 0 )
      {
        bUnique = false;
      }
    }
  }

  return bUnique;
}


bool StringManager::IsTableNameUnique( const QString& i_strName ) const
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


bool StringManager::IsTokenUnique( const QString& i_strToken ) const
{
  bool bUnique{ true };

  const int32_t iSelectedRow{ m_pcUI->tableWidget_3->currentRow() };

  // Visit each table row and compare names
  const int32_t iNumRows{ m_pcUI->tableWidget_3->rowCount() };
  for( int32_t iRow{ 0 }; iRow < iNumRows && bUnique; ++iRow )
  {
    if( iRow != iSelectedRow )
    {
      const QString& strToken{ GetToken( iRow ) };
      if( strToken.compare( i_strToken, Qt::CaseInsensitive ) == 0 )
      {
        bUnique = false;
      }
    }
  }

  return bUnique;
}


void StringManager::LanguageLineEditFinished()
{
  const int32_t iCol{ m_pcUI->tableWidget_2->currentColumn() };
  const int32_t iRow{ m_pcUI->tableWidget_2->currentRow() };

  if( iCol == L_COL_NAME )
  {
    const QWidget* pcWidget{ m_pcUI->tableWidget_2->cellWidget( iRow, iCol ) };
    if( pcWidget )
    {
      const QLineEdit* pcLineEdit{ qobject_cast<const QLineEdit*>( pcWidget ) };
      if( pcLineEdit )
      {
        disconnect( pcLineEdit, SIGNAL( editingFinished() ), this, SLOT( LanguageLineEditFinished() ) );

        const QString& strName{ pcLineEdit->text() };
        if( IsLanguageUnique( strName ) )
        {
          QTableWidgetItem* pcItem{ m_pcUI->tableWidget_2->item( iRow, iCol ) };
          if( pcItem )
          {
            pcItem->setText( strName );
            SetDirty( true );
          }
        }
        else
        {
          const QString strError{ "Language names must be unique" };
          QMessageBox::critical( this, tr( "Error" ), strError );
        }
      }

      m_pcUI->tableWidget_2->removeCellWidget( iRow, iCol );
    }
  }
  else if( L_COL_CODE == iCol )
  {
    const QWidget* pcWidget{ m_pcUI->tableWidget_2->cellWidget( iRow, iCol ) };
    if( pcWidget )
    {
      const QLineEdit* pcLineEdit{ qobject_cast<const QLineEdit*>( pcWidget ) };
      if( pcLineEdit )
      {
        disconnect( pcLineEdit, SIGNAL( editingFinished() ), this, SLOT( LanguageLineEditFinished() ) );

        const QString& strName{ pcLineEdit->text() };
        if( IsLanguageCodeUnique( strName ) )
        {
          QTableWidgetItem* pcItem{ m_pcUI->tableWidget_2->item( iRow, iCol ) };
          if( pcItem )
          {
            pcItem->setText( strName );
            SetDirty( true );
          }
        }
        else
        {
          const QString strError{ "Language codes must be unique" };
          QMessageBox::critical( this, tr( "Error" ), strError );
        }
      }

      m_pcUI->tableWidget_2->removeCellWidget( iRow, iCol );
    }
  }
}


void StringManager::TableLineEditFinished()
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
          const QString strError{ "String Table names must be unique" };
          QMessageBox::critical( this, tr( "Error" ), strError );
        }
      }

      m_pcUI->tableWidget->removeCellWidget( iRow, iCol );
    }
  }
}


void StringManager::TokenLineEditFinished()
{
  const int32_t iCol{ m_pcUI->tableWidget_3->currentColumn() };

  if( iCol == S_COL_TOKEN )
  {
    const int32_t iRow{ m_pcUI->tableWidget_3->currentRow() };

    const QWidget* pcWidget{ m_pcUI->tableWidget_3->cellWidget( iRow, iCol ) };
    if( pcWidget )
    {
      const QLineEdit* pcLineEdit{ qobject_cast<const QLineEdit*>( pcWidget ) };
      if( pcLineEdit )
      {
        const QString& strToken{ pcLineEdit->text() };
        if( IsTokenUnique( strToken ) )
        {
          QTableWidgetItem* pcItem{ m_pcUI->tableWidget_3->item( iRow, iCol ) };
          if( pcItem )
          {
            pcItem->setText( strToken );
            SetDirty( true );
            m_pcUI->tableWidget_3->scrollToItem( pcItem );
          }
        }
        else
        {
          const QString strError{ "Token names must be unique" };
          QMessageBox::critical( this, tr( "Error" ), strError );
        }
      }

      m_pcUI->tableWidget_3->removeCellWidget( iRow, iCol );
    }
  }
}


// slot
void StringManager::onLanguageItemChanged( QTableWidgetItem* i_pcItem )
{
  if( !i_pcItem )
  {
    return;
  }

  const int32_t iCol{ i_pcItem->column() };

  const rumLanguageID iLanguageID{ GetSelectedLanguageID() };
  rumLanguage& rcLanguage{ rumLanguage::GetLanguageInfo( iLanguageID ) };

  const std::string strValue{ qPrintable( i_pcItem->data( Qt::DisplayRole ).toString() ) };

  if( iCol == L_COL_NAME )
  {
    rcLanguage.SetName( strValue );
    InitLanguages();
  }
  else if( iCol == L_COL_CODE )
  {
    rcLanguage.SetCode( strValue );
    InitLanguages();
  }

  SetDirty( true );
}


// slot
void StringManager::OnStringTableComboChanged( const QString& i_strText )
{
  const int32_t iRow{ m_pcUI->tableWidget->currentRow() };
  const int32_t iCol{ m_pcUI->tableWidget->currentColumn() };

  if( iCol == D_COL_SERVICE_TYPE )
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
void StringManager::OnStringTableItemStartEdit( int32_t i_iRow, int32_t i_iCol )
{
  if( D_COL_SERVICE_TYPE == i_iCol )
  {
    QComboBox* pcCombo{ new QComboBox };
    pcCombo->addItem( "Shared", ServiceType::Shared_ServiceType );
    pcCombo->addItem( "Server", ServiceType::Server_ServiceType );
    pcCombo->addItem( "Client", ServiceType::Client_ServiceType );

    const rumStringTableID iStringTableID{ GetSelectedStringTableID() };
    const rumStringTable& rcStringTable{ rumStringTable::GetStringTable( iStringTableID ) };

    const int32_t iIndex{ pcCombo->findData( rcStringTable.GetServiceType() ) };
    pcCombo->setCurrentIndex( iIndex );
    pcCombo->view()->setMinimumWidth( pcCombo->minimumSizeHint().width() );

    connect( pcCombo, SIGNAL( currentIndexChanged( const QString& ) ),
             this, SLOT( OnStringTableComboChanged( const QString& ) ) );

    m_pcUI->tableWidget->setCellWidget( i_iRow, i_iCol, pcCombo );
  }
}


// slot
void StringManager::onStringTableItemChanged( QTableWidgetItem* i_pcItem )
{
  // Mark language or table dirty, depending on the edited column
  const int32_t iCol{ i_pcItem->column() };

  const rumStringTableID iStringTableID{ GetSelectedStringTableID() };
  rumStringTable& rcStringTable{ rumStringTable::GetStringTable( iStringTableID ) };

  if( iCol == D_COL_NAME )
  {
    rcStringTable.SetName( qPrintable( i_pcItem->text() ) );
  }
  else if( iCol == D_COL_SERVICE_TYPE )
  {
    const ServiceType eServiceType{ (ServiceType)i_pcItem->data( Qt::UserRole ).toInt() };
    rcStringTable.SetServiceType( eServiceType );
  }

  SetDirty( true );
}


// slot
void StringManager::onTranslationTableItemChanged( QTableWidgetItem* i_pcItem )
{
  const rumStringTableID iStringTableID{ GetSelectedStringTableID() };
  rumStringTable& rcStringTable{ rumStringTable::GetStringTable( iStringTableID ) };
  const rumTokenID iTokenID{ GetTokenID( i_pcItem->row() ) };

  const int32_t iCol{ i_pcItem->column() };

  if( S_COL_TOKEN == iCol )
  {
    rcStringTable.SetTokenName( iTokenID, qPrintable( i_pcItem->text() ) );
  }
  else if( S_COL_TRANSLATION == iCol )
  {
    const rumLanguageID iLanguageID{ GetSelectedLanguageID() };
    rcStringTable.SetTranslation( iLanguageID, iTokenID, qPrintable( i_pcItem->text() ) );
  }

  SetDirty( true );
}


// slot
void StringManager::on_actionNew_Language_triggered()
{
  const rumLanguageID iLanguageID{ rumLanguage::GetNextAvailableLanguageID() };

  const std::string strID{ rumStringUtils::ToString( iLanguageID ) };
  const std::string strName{ "Language" + strID };
  const std::string strCode{ "lang" + strID };

  if( rumLanguage::AddLanguage( iLanguageID, strName, strCode ) )
  {
    // Add the language token hash to all string tables
    const std::vector<rumStringTableID> cStringTableIDs{ rumStringTable::GetStringTableIDs() };
    for( const auto& iter : cStringTableIDs )
    {
      const rumStringTableID iStringTableID{ iter };
      rumStringTable& rcStringTable{ rumStringTable::GetStringTable( iStringTableID ) };
      rcStringTable.AddTokenHash( iLanguageID );
    }

    InitLanguages();

    // Refresh the current table
    RefreshTranslationTable();
  }
}


// slot
void StringManager::on_actionNew_String_Table_triggered()
{
  const rumStringTableID iStringTableID{ rumStringTable::GetNextStringTableID() };
  std::string strName{ "table" };
  strName += rumStringUtils::ToString( iStringTableID );

  if( rumStringTable::AddStringTable( iStringTableID, strName, ServiceType::Shared_ServiceType ) )
  {
    InitStringTables();
    RefreshTranslationTable();
  }
}


// slot
void StringManager::on_actionNew_String_triggered()
{
  // Clear the filter
  m_cFilterEdit.clear();

  UpdateFilter();

  const rumLanguageID iLanguageID{ GetSelectedLanguageID() };
  const rumStringTableID iStringTableID{ GetSelectedStringTableID() };
  rumStringTable& rcStringTable{ rumStringTable::GetStringTable( iStringTableID ) };
  const rumTokenID iTokenID{ rcStringTable.GetNextTokenID() };

  disconnect( m_pcUI->tableWidget_3, &QTableWidget::itemChanged, this, &StringManager::onTranslationTableItemChanged );

  // Disable sorting
  m_pcUI->tableWidget_3->setSortingEnabled( false );

  QString strNewToken{ "token_" };
  strNewToken += rumStringUtils::ToHexString( iTokenID );

  rcStringTable.AddToken( iTokenID, qPrintable( strNewToken ) );

  // Add a new row to the table widget
  const int32_t iRow{ m_pcUI->tableWidget_3->rowCount() };

  AddString( iRow, iTokenID, strNewToken, "" );

  // Subscribe to change notifications
  connect( m_pcUI->tableWidget_3, &QTableWidget::itemChanged, this, &StringManager::onTranslationTableItemChanged );

  m_pcUI->tableWidget_3->selectRow( iRow );

  // Re-enable sorting
  m_pcUI->tableWidget_3->setSortingEnabled( true );

  // Scroll to the newly added row
  // NOTE: The row id might have just changed because of sorting, so use currentRow
  const QTableWidgetItem* pcItem{ m_pcUI->tableWidget_3->item( m_pcUI->tableWidget_3->currentRow(), S_COL_TOKEN ) };
  if( pcItem )
  {
    m_pcUI->tableWidget_3->scrollToItem( pcItem );
  }

  SetDirty( true );
}


// slot
void StringManager::on_actionRemove_Language_triggered()
{
  const int32_t iRow{ m_pcUI->tableWidget_2->currentRow() };

  const QString strQuestion
  {
    "Are you sure you want to remove language " + GetLanguageName( iRow ) +
    "?\n\nWARNING: This will remove all strings associated with this language!\n"
    "Be sure to backup your database files before removing a language."
  };

  // Verify delete
  if( QMessageBox::Yes == QMessageBox::question( this, "Verify", strQuestion,
                                                 QMessageBox::Yes | QMessageBox::No,
                                                 QMessageBox::No ) )
  {
    const rumLanguageID iLanguageID{ GetSelectedLanguageID() };
    if( rumLanguage::RemoveLanguage( iLanguageID ) )
    {
      // Remove the table widget row
      m_pcUI->tableWidget_2->removeRow( iRow );

      // Remove the matching language token hash from all string tables
      const std::vector<rumStringTableID> cStringTableIDs{ rumStringTable::GetStringTableIDs() };
      for( const auto& iter : cStringTableIDs )
      {
        const rumStringTableID iStringTableID{ iter };
        rumStringTable& rcStringTable{ rumStringTable::GetStringTable( iStringTableID ) };
        rcStringTable.RemoveTokenHash( iLanguageID );
      }

      SetDirty( true );
    }
  }
}


// slot
void StringManager::on_actionRemove_String_Table_triggered()
{
  const rumStringTableID iStringTableID{ GetSelectedStringTableID() };
  rumStringTable& rcStringTable{ rumStringTable::GetStringTable( iStringTableID ) };

  QString strQuestion{ "Are you sure you want to remove string table " };
  strQuestion += rcStringTable.GetName().c_str();
  strQuestion += "?\n\nWARNING: This will remove all strings associated with this table!\n"
                 "Be sure to backup your files before removing a string table.";

  // Verify delete
  if( QMessageBox::Yes == QMessageBox::question( this, "Verify", strQuestion,
                                                 QMessageBox::Yes | QMessageBox::No,
                                                 QMessageBox::No ) )
  {
    if( rcStringTable.RemoveStringTable( iStringTableID ) )
    {
      // Remove the table widget row
      const int32_t iRow{ m_pcUI->tableWidget->currentRow() };
      m_pcUI->tableWidget->removeRow( iRow );
    }
  }
}


void StringManager::on_actionRemove_String_triggered()
{
  const int32_t iRow{ m_pcUI->tableWidget_3->currentRow() };

  const QString strQuestion{ "Are you sure you want to remove string token " + GetToken( iRow ) + "?" };

  // Verify delete
  if( QMessageBox::Yes == QMessageBox::question( this, "Verify", strQuestion,
                                                 QMessageBox::Yes | QMessageBox::No,
                                                 QMessageBox::No ) )
  {
    const rumTokenID iTokenID{ GetTokenID( iRow) };
    rumStringTable& rcStringTable{ rumStringTable::GetStringTable( GetSelectedStringTableID() ) };
    rcStringTable.RemoveToken( iTokenID );

    SetDirty( true );

    // Remove the table widget entry
    m_pcUI->tableWidget_3->removeRow( iRow );
  }
}


// slot
void StringManager::on_actionSave_triggered()
{
  const std::vector<rumStringTableID> cStringTableIDs{ rumStringTable::GetStringTableIDs() };
  for( const auto& iter : cStringTableIDs )
  {
    rumStringTableID iStringTableID{ iter };
    SaveTable( iStringTableID );
  }
}


// slot
void StringManager::on_actionTranslate_All_triggered()
{
  // Create the translator dialog
  /*LanguageTranslator* pcEdit{new LanguageTranslator(this)};

  // For communicating finished and accepted translations
  connect( pcEdit, SIGNAL( MSTranslationsAdded( const TranslationMap& ) ),
           this, SLOT( onMSTranslationsAdded( const TranslationMap& ) ) );

  pcEdit->setModal( true );
  pcEdit->show();*/
}


// slot
void StringManager::onFilterChanged()
{
  UpdateFilter();
}


void StringManager::on_tableWidget_cellDoubleClicked( int32_t i_iRow, int32_t i_iCol )
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
}


void StringManager::on_tableWidget_customContextMenuRequested( const QPoint& i_rcPos )
{
  if( i_rcPos.isNull() )
  {
    return;
  }

  QMenu* pcMenu{ new QMenu( m_pcUI->tableWidget ) };
  pcMenu->addAction( m_pcUI->actionNew_String_Table );
  pcMenu->addAction( m_pcUI->actionRemove_String_Table );

  pcMenu->exec( m_pcUI->tableWidget->mapToGlobal( i_rcPos ) );
}


void StringManager::on_tableWidget_2_cellDoubleClicked( int32_t i_iRow, int32_t i_iCol )
{
  if( L_COL_NAME == i_iCol )
  {
    // Only allow numbers, letters, dash, and underscore up to 32 characters
    const QRegExp cRegExp( "[\\w-]{1,32}" );
    const QRegExpValidator* pcValidator{ new QRegExpValidator( cRegExp, this ) };

    QLineEdit* pcLineEdit{ new QLineEdit };
    pcLineEdit->setText( GetLanguageName( i_iRow ) );
    pcLineEdit->setValidator( pcValidator );

    connect( pcLineEdit, SIGNAL( editingFinished() ), this, SLOT( LanguageLineEditFinished() ) );

    m_pcUI->tableWidget_2->setCellWidget( i_iRow, i_iCol, pcLineEdit );
  }
  else if( L_COL_CODE == i_iCol )
  {
    // Only allow lower case letters, dash, and underscore up to 7 characters
    const QRegExp cRegExp( "[a-z-_]{1,7}" );
    const QRegExpValidator* pcValidator{ new QRegExpValidator( cRegExp, this ) };

    QLineEdit* pcLineEdit{ new QLineEdit };
    pcLineEdit->setText( GetLanguageCode( i_iRow ) );
    pcLineEdit->setValidator( pcValidator );

    connect( pcLineEdit, SIGNAL( editingFinished() ), this, SLOT( LanguageLineEditFinished() ) );

    m_pcUI->tableWidget_2->setCellWidget( i_iRow, i_iCol, pcLineEdit );
  }
}


void StringManager::on_tableWidget_2_customContextMenuRequested( const QPoint& i_rcPos )
{
  if( i_rcPos.isNull() )
  {
    return;
  }

  QMenu* pcMenu{ new QMenu( m_pcUI->tableWidget_2 ) };
  pcMenu->addAction( m_pcUI->actionNew_Language );
  pcMenu->addAction( m_pcUI->actionRemove_Language );

  pcMenu->exec( m_pcUI->tableWidget_2->mapToGlobal( i_rcPos ) );
}


void StringManager::on_tableWidget_3_cellDoubleClicked( int32_t i_iRow, int32_t i_iCol )
{
  if( S_COL_TOKEN == i_iCol )
  {
    // Only allow numbers, letters, dash, and underscore up to 64 characters
    const QRegExp cRegExp( "[\\w-]{1,64}" );
    const QRegExpValidator* pcValidator{ new QRegExpValidator( cRegExp, this ) };

    QLineEdit* pcLineEdit{ new QLineEdit };
    pcLineEdit->setText( GetToken( i_iRow ) );
    pcLineEdit->setValidator( pcValidator );

    connect( pcLineEdit, SIGNAL( editingFinished() ), this, SLOT( TokenLineEditFinished() ) );

    m_pcUI->tableWidget_3->setCellWidget( i_iRow, i_iCol, pcLineEdit );
  }
}


void StringManager::on_tableWidget_3_customContextMenuRequested( const QPoint& i_rcPos )
{
  if( i_rcPos.isNull() )
  {
    return;
  }

  QMenu* pcMenu{ new QMenu( m_pcUI->tableWidget_3 ) };
  pcMenu->addAction( m_pcUI->actionNew_String );
  pcMenu->addAction( m_pcUI->actionRemove_String );

  pcMenu->exec( m_pcUI->tableWidget_3->mapToGlobal( i_rcPos ) );
}


#if 0
// slot
void StringManager::onMSTranslateLanguagesResponse( QNetworkReply* i_pcReply )
{
  Q_ASSERT( i_pcReply );

  const quint64 uiSize{ i_pcReply->size() };
  const QByteArray cData{ i_pcReply->read( uiSize ) };

  // Verify the UTF-8 header
  Q_ASSERT( (unsigned char)cData[0] == 0xEF );
  Q_ASSERT( (unsigned char)cData[1] == 0xBB );
  Q_ASSERT( (unsigned char)cData[2] == 0xBF );

  // Convert the encoded data to UTF-8
  const QTextCodec* pcCodec{ QTextCodec::codecForName( "UTF-8" ) };
  QString strData( pcCodec->toUnicode( cData ) );

  //qDebug() << strData;

  // Remove the byte order marks and surrounding quotation marks
  strData.remove( 0, strData.indexOf( "\"" ) );
  strData.truncate( strData.lastIndexOf( "\"" ) + 1 );

  bool bUpdated{ false };

  const QStringList strSplitList{ strData.split( "," ) };
  for( int32_t i = 0; i < strSplitList.size(); ++i )
  {
    // Break each entry into a key/value pair
    QString strCode{ strSplitList.at( i ) };

    // Remove the first and last characters (quotation marks)
    strCode.remove( 0, 1 );
    strCode.chop( 1 );

    if( strCode.compare( "\"error_description\"" ) == 0 )
    {
      QMessageBox::critical(this, tr("Error"),
                            "Failed to obtain an access token from Microsoft Translator\n" +
                            strValue);
    }
    else
    {
      //m_LanguageSet.insert( strCode );
    }
  }
}


// slot
void StringManager::onMSTranslateTextResponse( QNetworkReply* i_pcReply )
{
  Q_ASSERT( i_pcReply );

  const quint64 uiSize{ i_pcReply->size() };
  const QByteArray cData{ i_pcReply->read( uiSize ) };

  // Verify the UTF-8 header
  Q_ASSERT( (unsigned char)cData[0] == 0xEF );
  Q_ASSERT( (unsigned char)cData[1] == 0xBB );
  Q_ASSERT( (unsigned char)cData[2] == 0xBF );

  // Convert the encoded data to UTF-8
  const QTextCodec* pcCodec{ QTextCodec::codecForName( "UTF-8" ) };
  QString strData( pcCodec->toUnicode( cData ) );

  //qDebug() << strData;

  // Remove the byte order marks and surrounding quotation marks
  strData.remove( 0, strData.indexOf( "\"" ) + 1 );
  strData.truncate( strData.lastIndexOf( "\"" ) );

  m_cMSTranslateInfo.m_strLastTranslatedText = strData;

  emit MSTranslateTextDone();
}


// slot
void StringManager::onMSTranslateTokenExpired()
{
  m_cMSTranslateInfo.m_tExpireTimer.stop();
  qDebug() << "Microsoft Translator Token expired";
}


// slot
void StringManager::onMSTranslateTokenResponse( QNetworkReply* i_pcReply )
{
  Q_ASSERT( i_pcReply );

  const quint64 uiSize{ i_pcReply->size() };
  const QByteArray cData{ i_pcReply->read( uiSize ) };

  QString strData( cData );

  //qDebug() << strData;

  // Remove the first and last characters
  strData.remove( 0, 1 );
  strData.chop( 1 );

  bool bUpdated{ false };

  const QStringList strSplitList{ strData.split( "," ) };
  for( int32_t i = 0; i < strSplitList.size(); ++i )
  {
    // Break each entry into a key/value pair
    const QString& strKeyValues{ strSplitList.at( i ) };
    const QStringList strSplitList2{ strKeyValues.split( ":\"" ) };
    const QString strKey{ strSplitList2.at( 0 ) };
    QString strValue{ strSplitList2.at( 1 ) };

    // Eat the last quotation mark
    strValue.chop( 1 );

    if( strKey.compare( "\"token_type\"" ) == 0 )
    {
      m_cMSTranslateInfo.m_strTokenType = strValue;
      bUpdated = true;
    }
    else if( strKey.compare( "\"access_token\"" ) == 0 )
    {
      m_cMSTranslateInfo.m_strAccessToken = strValue;
      bUpdated = true;
    }
    else if( strKey.compare( "\"expires_in\"" ) == 0 )
    {
      // Start timer with seconds converted to milliseconds
      m_cMSTranslateInfo.m_tExpireTimer.start( strValue.toInt() * 1000 );
      connect( &( m_cMSTranslateInfo.m_tExpireTimer ), SIGNAL( timeout() ),
               this, SLOT( onMSTranslateTokenExpired() ) );
      bUpdated = true;
    }
    else if( strKey.compare( "\"scope\"" ) == 0 )
    {
      m_cMSTranslateInfo.m_strScope = strValue;
      bUpdated = true;
    }
    else if( strKey.compare( "\"error_description\"" ) == 0 )
    {
      QMessageBox::critical( this, tr( "Error" ), "Failed to obtain an access token from Microsoft Translator\n" +
                             strValue );
      bUpdated = false;
    }
  }

  if( bUpdated )
  {
    if( m_LanguageSet.empty() )
    {
      RequestMSTranslateLanguages();
    }

    emit MSTranslateTokenUpdated();
  }
}


// slot
void StringManager::onResumeMSTranslateLanguages()
{
  RequestMSTranslateLanguages();
}


// slot
void StringManager::onResumeMSTranslateText()
{
  RequestMSTranslateText();
}


// slot
void StringManager::onMSTranslationsAdded( const TranslationMap& i_rcTranslationMap )
{
  // Add this string info to the current string table
  QSqlDatabase cDatabase{ MainWindow::GetSqlDatabase( GetSelectedDatabaseName() ) };
  Q_ASSERT( cDatabase.isValid() );

  QSqlQuery sqlQuery( cDatabase );

  QMapIterator<QString, QString> iter( i_rcTranslationMap );
  while( iter.hasNext() )
  {
    iter.next();

    sqlQuery.prepare( "SELECT name,key FROM string WHERE string_id=:string_id" );
    sqlQuery.bindValue( ":string_id", iter.key() );
    if( sqlQuery.exec() )
    {
      while( sqlQuery.next() )
      {
        AddString( sqlQuery.value( 0 ).toString(), iter.value(), iter.key(), sqlQuery.value( 1 ).toString() );
      }
    }
  }

  ui->tableWidget_3->resizeColumnsToContents();
}


void StringManager::RequestMSTranslateLanguages()
{
  // If we have token info, is it valid?
  if( m_cMSTranslateInfo.m_strAccessToken.isEmpty() || !m_cMSTranslateInfo.m_tExpireTimer.isActive() )
  {
    connect( this, SIGNAL( MSTranslateTokenUpdated() ), this, SLOT( onResumeMSTranslateLanguages() ) );

    // Request a new token
    RequestMSTranslateToken();
    return;
  }

  QNetworkAccessManager* pcManager{ new QNetworkAccessManager( this ) };
  connect( pcManager, SIGNAL( finished( QNetworkReply* ) ),
           this, SLOT( onMSTranslateLanguagesResponse( QNetworkReply* ) ) );

  // For some reason, this was border-line impossible to figure out because of bad documentation/wording on both
  // Microsoft's and Qt's part. The most important thing: the token obtained from the MS Translator must be encoded
  // before passing it back to microsoft, even though it's already partially encoded as it is. Also, do not try to take
  // something from one QUrl and try to insert it into another. QUrl *automatically* encodes entries, and will happily
  // re-encode everything again. Don't use .toString() from QUrls either. It does behind-the-scenes conversion of the
  // internal string to a human-readable version and what-you-see is not at all what-you-get.
  QUrl cServiceUrl( "http://api.microsofttranslator.com/V2/Ajax.svc/GetLanguagesForTranslate" );
  const QString strAppID( "Bearer " + m_cMSTranslateInfo.m_strAccessToken );

  QUrlQuery cQuery;
  cQuery.addQueryItem( "appId", QUrl::toPercentEncoding( strAppID ) );
  cServiceUrl.setQuery( cQuery );

  //qDebug() << serviceUrl.toEncoded();

  QNetworkRequest cRequest( cServiceUrl );
  cRequest.setHeader( QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded" );

  manager->get( cRequest );
}


void StringManager::RequestMSTranslateText( const QString& i_strText,
                                            const QString& i_strFrom,
                                            const QString& i_strTo )
{
  m_cMSTranslateInfo.m_strTranslateText = i_strText;
  m_cMSTranslateInfo.m_strFromLanguage = i_strFrom;
  m_cMSTranslateInfo.m_strToLanguage = i_strTo;

  // Call the internal version
  RequestMSTranslateText();
}


void StringManager::RequestMSTranslateText()
{
  // If we have token info, is it valid?
  if( m_cMSTranslateInfo.m_strAccessToken.isEmpty() || !m_cMSTranslateInfo.m_tExpireTimer.isActive() )
  {
    connect( this, SIGNAL( MSTranslateTokenUpdated() ), this, SLOT( onResumeMSTranslateText() ) );

    // Request a new token
    RequestMSTranslateToken();
    return;
  }

  QNetworkAccessManager* pcManager{ new QNetworkAccessManager( this ) };
  connect( pcManager, SIGNAL( finished( QNetworkReply* ) ),
           this, SLOT( onMSTranslateTextResponse( QNetworkReply* ) ) );

  QUrl cServiceUrl( "http://api.microsofttranslator.com/V2/Ajax.svc/Translate" );
  const QString strAppID( "Bearer " + m_cMSTranslateInfo.m_strAccessToken );

  QUrlQuery cQuery;
  cQuery.addQueryItem( "appId", QUrl::toPercentEncoding( strAppID ) );
  cQuery.addQueryItem( "text", QUrl::toPercentEncoding( m_cMSTranslateInfo.m_strTranslateText ) );
  cQuery.addQueryItem( "from", QUrl::toPercentEncoding( m_cMSTranslateInfo.m_strFromLanguage ) );
  cQuery.addQueryItem( "to", QUrl::toPercentEncoding( m_cMSTranslateInfo.m_strToLanguage ) );
  cServiceUrl.setQuery( cQuery );

  //qDebug() << serviceUrl.toEncoded();

  QNetworkRequest cRequest( cServiceUrl );
  cRequest.setHeader( QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded" );

  manager->get( cRequest );
}


void StringManager::RequestMSTranslateToken()
{
  QNetworkAccessManager* pcManager{ new QNetworkAccessManager( this ) };
  connect( manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT( onMSTranslateTokenResponse( QNetworkReply* ) ) );

  // TODO - add dialog warning for lack of SSL support
  //Q_ASSERT(QSslSocket::supportsSsl());

  QUrl cServiceUrl( "https://datamarket.accesscontrol.windows.net/v2/OAuth2-13" );
  QNetworkRequest cRequest( cServiceUrl );
  cRequest.setHeader( QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded" );

  QUrlQuery cQuery;
  cQuery.addQueryItem( "grant_type", "client_credentials" );
  cQuery.addQueryItem( "client_id", m_cMSTranslateInfo.m_strClientID );
  cQuery.addQueryItem( "client_secret", m_cMSTranslateInfo.m_strClientSecret );
  cQuery.addQueryItem( "scope", "http://api.microsofttranslator.com" );
  cServiceUrl.setQuery( cQuery );

  manager->post( cRequest, cServiceUrl.query( QUrl::FullyEncoded ).toLatin1() );
}

#endif // 0


void StringManager::SaveTable( rumStringTableID i_iStringTableID )
{
  rumStringTable& rcStringTable{ rumStringTable::GetStringTable( i_iStringTableID ) };
  rcStringTable.ExportCSVFiles( qPrintable( MainWindow::GetProjectDir().canonicalPath() ) );
  SetDirty( false );
}


void StringManager::RefreshTranslationTable()
{
  m_pcUI->tableWidget_3->setEnabled( false );

  const rumLanguageID iLanguageID{ GetSelectedLanguageID() };
  const rumStringTableID iStringTableID{ GetSelectedStringTableID() };

  // Early out if one of the tables is not selected
  if( ( iStringTableID == rumStringTable::INVALID_STRINGTABLE_ID ) ||
      ( iLanguageID == rumLanguage::INVALID_LANGUAGE_ID ) )
  {
    m_pcUI->actionNew_String->setEnabled( false );
    //ui->actionTranslate_All->setEnabled( false );
    m_cFilterLabel.setEnabled( false );
    m_cFilterEdit.setEnabled( false );
    return;
  }

  const rumLanguage& rcLanguage{ rumLanguage::GetLanguageInfo( iLanguageID ) };

  QStringList cLabelList;
  cLabelList << "Token" << rcLanguage.GetName().c_str();

  m_pcUI->tableWidget_3->setMinimumWidth( 200 );
  m_pcUI->tableWidget_3->setSortingEnabled( false );
  m_pcUI->tableWidget_3->setColumnCount( cLabelList.size() );
  m_pcUI->tableWidget_3->setRowCount( 0 );
  m_pcUI->tableWidget_3->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
  m_pcUI->tableWidget_3->setHorizontalScrollMode( QAbstractItemView::ScrollPerPixel );
  m_pcUI->tableWidget_3->setHorizontalHeaderLabels( cLabelList );

  QHeaderView* pcHorizontalHeader{ m_pcUI->tableWidget_3->horizontalHeader() };
  pcHorizontalHeader->setMaximumSectionSize( 300 );
  pcHorizontalHeader->setStretchLastSection( true );

  disconnect( m_pcUI->tableWidget_3, &QTableWidget::itemChanged, this, &StringManager::onTranslationTableItemChanged );

  const int32_t iTableRow{ m_pcUI->tableWidget->currentRow() };

  m_pcUI->actionNew_String->setEnabled( true );
  //ui->actionTranslate_All->setEnabled( true );
  m_cFilterLabel.setEnabled( true );
  m_cFilterEdit.setEnabled( true );

  // Get the selected table
  rumStringTable& rcStringTable{ rumStringTable::GetStringTable( iStringTableID ) };
  const rumTranslationHash& rcTranslationHash{ rcStringTable.GetTranslationHash() };
  if( ( rcStringTable.GetNumTokens() == 0 ) || rcTranslationHash.empty() )
  {
    // Load the table for the selected language
    rcStringTable.LoadStringTable( iStringTableID, iLanguageID, qPrintable( MainWindow::GetProjectDir().canonicalPath() ) );
  }

  int32_t iRow{ 0 };
  m_pcUI->tableWidget_3->setRowCount( rcStringTable.GetNumTokens() );

  const rumTokenHash& rcTokenHash{ rcStringTable.GetTokenHash() };
  for( const auto& iter : rcTokenHash )
  {
    rumTokenID iTokenID{ iter.first };
    const std::string& strToken{ iter.second };
    const std::string& strTranslation{ rcStringTable.Fetch( iTokenID, iLanguageID ) };
    AddString( iRow++, iTokenID, strToken.c_str(), strTranslation.c_str() );
  }

  UpdateFilter();

  m_pcUI->tableWidget_3->setSortingEnabled( true );
  m_pcUI->tableWidget_3->sortItems( S_COL_TOKEN );

  m_pcUI->tableWidget_3->setEnabled( true );

  // Subscribe to change notifications
  connect( m_pcUI->tableWidget_3, &QTableWidget::itemChanged, this, &StringManager::onTranslationTableItemChanged );
}


// slot
void StringManager::selectionChanged_Language( const QItemSelection& selected, const QItemSelection& deselected )
{
  m_pcUI->actionRemove_Language->setEnabled( !m_pcUI->tableWidget_2->selectedItems().empty() );
  RefreshTranslationTable();
}


// slot
void StringManager::selectionChanged_String( const QItemSelection& selected, const QItemSelection& deselected )
{
  m_pcUI->actionRemove_String->setEnabled( !m_pcUI->tableWidget_3->selectedItems().empty() );
}


// slot
void StringManager::selectionChanged_StringTable( const QItemSelection& selected, const QItemSelection& deselected )
{
  m_pcUI->actionRemove_String_Table->setEnabled( !m_pcUI->tableWidget->selectedItems().empty() );
  RefreshTranslationTable();
}


void StringManager::SetDirty( bool i_bDirty )
{
  // Only do work if the current dirty state doesn't equal the requested dirty state
  if( m_bDirty != i_bDirty )
  {
    m_bDirty = i_bDirty;
    m_pcUI->actionSave->setEnabled( m_bDirty );
  }
}


void StringManager::UpdateFilter()
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
      const QTableWidgetItem* pcItem{ m_pcUI->tableWidget_3->item( i, S_COL_TOKEN ) };
      bShow = pcItem->text().contains( strFilter, Qt::CaseInsensitive );

      if( !bShow )
      {
        int32_t iIndex{1};

        // Visit each language column
        const std::vector<rumLanguageID> LanguageIDVector{ rumLanguage::GetLanguageIDs() };
        for( const auto iter : LanguageIDVector )
        {
          pcItem = m_pcUI->tableWidget_3->item( i, S_COL_TOKEN + iIndex );
          bShow = pcItem->text().contains( strFilter, Qt::CaseInsensitive );
        }
      }
    }

    bShow ? m_pcUI->tableWidget_3->showRow( i ) : m_pcUI->tableWidget_3->hideRow( i );
  }

  m_pcUI->tableWidget_3->resizeColumnsToContents();
}
