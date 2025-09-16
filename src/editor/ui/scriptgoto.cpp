#include <scriptgoto.h>
#include <ui_scriptgoto.h>

#include <QTextDocument>


ScriptGoto::ScriptGoto( QTextDocument* i_pcDocument, QWidget* i_pcParent )
  : QDialog( i_pcParent )
  , i_pcUI( new Ui::ScriptGoto )
  , m_iLineMax( i_pcDocument->lineCount() )
{
  i_pcUI->setupUi( this );
}


ScriptGoto::~ScriptGoto()
{
  delete i_pcUI;
}


void ScriptGoto::on_buttonBox_accepted()
{
  emit GotoPosition( i_pcUI->spinBoxLine->value(), i_pcUI->spinBoxColumn->value() );
  accept();
}


void ScriptGoto::on_spinBoxLine_valueChanged( int32_t i_iLine )
{
  i_pcUI->spinBoxLine->setValue( qBound( 0, m_iLineMax, i_iLine ) );
}


void ScriptGoto::on_spinBoxColumn_valueChanged( int32_t i_iColumn )
{
  i_pcUI->spinBoxColumn->setValue( i_iColumn );
}
