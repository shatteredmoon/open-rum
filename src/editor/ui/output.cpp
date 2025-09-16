#include <output.h>
#include <ui_output.h>

#include <mainwindow.h>
#include <qtFindReplace/finddialog.h>


OutputDialog::OutputDialog( QWidget* i_pcParent )
  : QDialog( i_pcParent )
  , m_pcUI( new Ui::Output )
{
  m_pcUI->setupUi( this );
  m_pcUI->textEdit->setReadOnly( true );
  setWindowTitle( "Output Log" );
  setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );
  Init();
}


OutputDialog::~OutputDialog()
{
  delete m_pcFindDialog;
  delete m_pcUI;
}


void OutputDialog::Append( const QString& i_strText )
{
  m_pcUI->textEdit->append( i_strText );
  qApp->processEvents();
}


void OutputDialog::Init()
{
  m_pcFindDialog = new FindDialog( this );
  m_pcFindDialog->setModal( false );
  m_pcFindDialog->setTextEdit( m_pcUI->textEdit );
}


void OutputDialog::on_actionClear_triggered()
{
  m_pcUI->textEdit->clear();
}


void OutputDialog::on_actionCopy_To_Clipboard_triggered()
{
  m_pcUI->textEdit->selectAll();
  m_pcUI->textEdit->copy();
}


void OutputDialog::on_actionFind_triggered()
{
  m_pcFindDialog->show();
}
