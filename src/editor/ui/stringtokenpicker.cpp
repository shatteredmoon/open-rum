#include <stringtokenpicker.h>
#include <ui_stringtokenpicker.h>


StringTokenPicker::StringTokenPicker( rumTokenID i_eTokenID, QWidget* i_pcParent )
  : QDialog( i_pcParent )
  , m_pcUI( new Ui::StringTokenPicker )
{
  m_pcUI->setupUi( this );

  setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );

  m_pcUI->listWidget->setSortingEnabled( false );

  rumStringTableID uiStringTableID{ 0 };
  const auto& rcStringTablesVector{ rumStringTable::GetStringTableIDs() };
  if( !rcStringTablesVector.empty() )
  {
    // Pick the first available string table
    uiStringTableID = rcStringTablesVector.at( 0 );

    // Populate the string table combo box
    for( const auto uiID : rcStringTablesVector )
    {
      const auto& pcStringTable{ rumStringTable::GetStringTable( uiID ) };
      m_pcUI->comboBox->addItem( pcStringTable.GetName().c_str(), uiID );
    }

    connect( m_pcUI->comboBox, SIGNAL( currentIndexChanged( int32_t ) ),
             this, SLOT( ItemComboChanged( int32_t ) ) );
  }

  if( i_eTokenID != rumStringTable::INVALID_TOKEN_ID )
  {
    // Since a token was provided, assume the user wants to pull from the same string table
    uiStringTableID = rumStringTable::GetStringTableIDFromToken( i_eTokenID );

    // Select the matching string table in the combo box
    m_pcUI->comboBox->setCurrentIndex( m_pcUI->comboBox->findData( uiStringTableID ) );
  }

  RefreshStringTable( uiStringTableID, i_eTokenID );
}


StringTokenPicker::~StringTokenPicker()
{
  delete m_pcUI;
}


// slot
void StringTokenPicker::ItemComboChanged( int32_t i_iIndex )
{
  const QVariant cVariant{ m_pcUI->comboBox->itemData( i_iIndex ) };
  rumStringTableID uiStringTableID( static_cast<rumStringTableID>( cVariant.toInt() ) );
  RefreshStringTable( uiStringTableID, rumStringTable::INVALID_TOKEN_ID );
}


// slot
void StringTokenPicker::on_buttonBox_accepted()
{
  const QListWidgetItem* pcItem{ m_pcUI->listWidget->currentItem() };
  if( pcItem )
  {
    const QVariant cVariant{ pcItem->data( Qt::UserRole ) };
    emit( NewStringTokenSelected( (rumTokenID)cVariant.toInt() ) );
  }

  accept();
}


// slot
void StringTokenPicker::on_lineEdit_Filter_textEdited( const QString &strText )
{
  UpdateFilter();
}


// slot
void StringTokenPicker::on_pushButton_Clear_clicked()
{
  m_pcUI->lineEdit_Filter->clear();
  UpdateFilter();
}


void StringTokenPicker::RefreshStringTable( rumStringTableID uiStringTableID, rumTokenID i_eTokenID )
{
  m_pcUI->listWidget->clear();

  QListWidgetItem* pcSelectedItem{ nullptr };

  const auto& pcStringTable{ rumStringTable::GetStringTable( uiStringTableID ) };

  // Populate the dialog with token entries
  const auto& rcTokenHash{ pcStringTable.GetTokenHash() };
  for( const auto& token : rcTokenHash )
  {
    QListWidgetItem* pcItem{ new QListWidgetItem };
    pcItem->setData( Qt::DisplayRole, token.second.c_str() );
    pcItem->setData( Qt::UserRole, token.first );

    m_pcUI->listWidget->addItem( pcItem );

    if( i_eTokenID != rumStringTable::INVALID_TOKEN_ID && ( token.first == i_eTokenID ) )
    {
      pcSelectedItem = pcItem;
    }
  }

  m_pcUI->listWidget->setSortingEnabled( true );
  m_pcUI->listWidget->sortItems();

  if( pcSelectedItem != nullptr )
  {
    pcSelectedItem->setSelected( true );
    m_pcUI->listWidget->scrollToItem( pcSelectedItem );
  }
}


void StringTokenPicker::UpdateFilter()
{
  // Hide anything that doesn't pass the filter
  const QString& strFilter{ m_pcUI->lineEdit_Filter->text() };

  // Visit each item in the table and hide anything that doesn't match the filter settings
  const int32_t iRow{ m_pcUI->listWidget->count() };
  for( int32_t i{ 0 }; i < iRow; ++i )
  {
    bool bShow{ true };

    if( !strFilter.isEmpty() )
    {
      const QListWidgetItem* pcItem{ m_pcUI->listWidget->item( i ) };
      bShow = pcItem->text().contains( strFilter, Qt::CaseInsensitive );
    }

    m_pcUI->listWidget->setRowHidden( i, !bShow );
  }
}
