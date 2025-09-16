#include <newscript.h>
#include <ui_newscript.h>

#include <mainwindow.h>

#include <QDate>
#include <QFileDialog.h>
#include <QMessageBox>
#include <QPushButton>
#include <QTextStream>


NewScript::NewScript( QWidget* i_pcParent )
  : QDialog( i_pcParent )
  , m_pcUI( new Ui::NewScript )
{
  m_pcUI->setupUi( this );

  // Make error labels red
  QPalette cPalette{ m_pcUI->labelFile_Error->palette() };
  cPalette.setColor( m_pcUI->labelFile_Error->foregroundRole(), Qt::red );
  m_pcUI->labelFile_Error->setPalette( cPalette );

  // Hide error labels
  m_pcUI->labelFile_Error->setHidden( true );
}


NewScript::~NewScript()
{
  delete m_pcUI;
}


void NewScript::on_buttonBox_accepted()
{
  if( !ValidateInput() )
  {
    return;
  }

  const QFileInfo cFileInfo( m_strAbsoluteFilePath );

  // Make the path
  QDir cDir;
  if( !cDir.mkpath( cFileInfo.absolutePath() ) )
  {
    QMessageBox::critical( this, "Script Creation Failure", "Failed to create path for " + m_strAbsoluteFilePath );
  }

  // Create the file
  QFile cFile( m_strAbsoluteFilePath );
  if( !cFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
  {
    QMessageBox::critical( this, "Script Creation Failure", "Failed to create " + m_strAbsoluteFilePath );
    reject();
    return;
  }

  const QDate cDate( QDate::currentDate() );

  QTextStream cStream( &cFile );
  cStream << "// Created " << cDate.toString() << Qt::endl;

  cFile.close();

  QMessageBox::information( this, "Script Created", "Successfully created " + m_strAbsoluteFilePath );
  emit scriptCreated( m_strAbsoluteFilePath );

  accept();
}


void NewScript::on_lineEditFile_textEdited( const QString& i_strText )
{
  m_pcUI->labelFile_Error->setHidden( true );
}


void NewScript::on_pushButtonFile_clicked()
{
  m_pcUI->labelFile_Error->setHidden( true );

  // Get the existing file path
  QString strFilePath{ m_pcUI->lineEditFile->text() };
  strFilePath = QDir::fromNativeSeparators( strFilePath );
  const QDir cDirFilePath( strFilePath );

  // Determine the standard path to game maps
  const QDir cDirScriptPath{ MainWindow::GetProjectScriptDir() };
  const QString strScriptPath{ QDir::fromNativeSeparators( cDirScriptPath.canonicalPath() ) };

  // Convert the existing file path to absolute
  QString strAbsFilePath;
  if( cDirFilePath.isRelative() )
  {
    strAbsFilePath = cDirScriptPath.absoluteFilePath( strFilePath );
    strAbsFilePath = QDir::cleanPath( strAbsFilePath );
  }
  else
  {
    strAbsFilePath = strFilePath;
  }

  // Make sure file is under the project maps subfolder
  if( !strAbsFilePath.startsWith( strScriptPath ) )
  {
    // Fall back to default maps folder
    strAbsFilePath = strScriptPath;
  }

  const QString strNewFilePath{ QFileDialog::getSaveFileName( this, tr( "File Location" ), strAbsFilePath ) };
  if( !strNewFilePath.isEmpty() )
  {
    // Create a relative file path to the selected file
    const QString strRelFilePath{ cDirScriptPath.relativeFilePath( strNewFilePath ) };
    m_pcUI->lineEditFile->setText( strRelFilePath );
  }
}


bool NewScript::ValidateInput()
{
  bool bValid = true;

  m_strAbsoluteFilePath.clear();

  // Get the existing file path
  QString strFilePath{ m_pcUI->lineEditFile->text() };
  strFilePath = QDir::fromNativeSeparators( strFilePath );
  const QDir cDirFilePath( strFilePath );

  // Determine the standard path to game maps
  const QDir cDirScriptPath{ MainWindow::GetProjectScriptDir() };
  const QString strScriptPath{ QDir::fromNativeSeparators( cDirScriptPath.canonicalPath() ) };

  // Convert the existing file path to absolute
  QString strAbsFilePath;
  if( cDirFilePath.isRelative() )
  {
    strAbsFilePath = cDirScriptPath.absoluteFilePath( strFilePath );
    strAbsFilePath = QDir::cleanPath( strAbsFilePath );
  }
  else
  {
    strAbsFilePath = strFilePath;
  }

  // Make sure file is provided and that it is under the project maps subfolder
  if( strFilePath.isEmpty() )
  {
    bValid = false;
    m_pcUI->labelFile_Error->setText( "Filename required" );
    m_pcUI->labelFile_Error->setHidden( false );
  }
  else if( !strAbsFilePath.startsWith( strScriptPath ) )
  {
    bValid = false;
    m_pcUI->labelFile_Error->setText( "Invalid Path or Filename" );
    m_pcUI->labelFile_Error->setHidden( false );
  }
  else if( !strAbsFilePath.endsWith( ".nut" ) )
  {
    bValid = false;
    m_pcUI->labelFile_Error->setText( "Filename extension must be \".nut\"" );
    m_pcUI->labelFile_Error->setHidden( false );
  }
  else
  {
    // Cache the final absolute file path to the new map
    m_strAbsoluteFilePath = strAbsFilePath;
  }

  return bValid;
}
