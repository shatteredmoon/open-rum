#include <combopicker.h>
#include <ui_combopicker.h>


ComboPicker::ComboPicker( const QString& i_strLabel,
                          const ComboPickerVector& i_rcEntryVector,
                          QWidget* i_pcParent )
  : QDialog( i_pcParent )
  , m_pcUI( new Ui::ComboPicker )
{
  m_pcUI->setupUi( this );

  setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );

  // Update the label
  m_pcUI->label->setText( i_strLabel );

  // Update the dropdown selections
  for( const auto& rcPair : i_rcEntryVector )
  {
    m_pcUI->comboBox->addItem( rcPair.first, rcPair.second );
  }
}


ComboPicker::~ComboPicker()
{
  delete m_pcUI;
}


// slot
void ComboPicker::on_buttonBox_accepted()
{
  const QVariant cVariant{ m_pcUI->comboBox->currentData() };
  emit( ComboPickerItemSelected( cVariant.toInt() ) );

  accept();
}
