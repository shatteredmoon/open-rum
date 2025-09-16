#include <scripteditor.h>
#include <ui_scripteditor.h>

#include <mainwindow.h>
#include <scriptgoto.h>
#include <qtFindReplace/finddialog.h>
#include <qtFindReplace/findreplacedialog.h>

#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QSyntaxHighlighter>
#include <QTextStream>

static QStringList g_cKeywordList(
  QStringList() << "base" << "break" << "case" << "catch" << "class" << "clone" << "continue"
                << "const" << "default" << "delete" << "enum" << "extends" << "for" << "foreach"
                << "function" << "if" << "in" << "local" << "null" << "resume" << "return"
                << "switch" << "this" << "throw" << "try" << "typeof" << "while" << "yield"
                << "constructor" << "instanceof" << "true" << "false" << "static" );


class ScriptSyntaxHighlighter : public QSyntaxHighlighter
{
public:

  ScriptSyntaxHighlighter( QTextEdit* i_pcParent )
    : QSyntaxHighlighter( i_pcParent )
  {
    HighlightingRule cHighlightRule;

    m_cFunctionFormat.setFontItalic( true );
    m_cFunctionFormat.setForeground( Qt::darkBlue );
    cHighlightRule.m_cPatternRegExp = QRegExp( "\\b[A-Za-z0-9_]+(?=\\()" );
    cHighlightRule.m_cFormat = m_cFunctionFormat;
    m_cHighlightingRulesVector.append( cHighlightRule );

    m_cKeywordFormat.setForeground( Qt::blue );
    foreach( const QString& strPattern, g_cKeywordList )
    {
      cHighlightRule.m_cPatternRegExp = QRegExp( "\\b" + strPattern + "\\b" );
      cHighlightRule.m_cFormat = m_cKeywordFormat;
      m_cHighlightingRulesVector.append( cHighlightRule );
    }

    QColor VisualStudioRed( 163, 21, 21 );
    m_cQuotationFormat.setForeground( VisualStudioRed );
    cHighlightRule.m_cPatternRegExp = QRegExp( "\".*\"" );
    cHighlightRule.m_cFormat = m_cQuotationFormat;
    m_cHighlightingRulesVector.append( cHighlightRule );

    m_cSingleLineCommentFormat.setForeground( Qt::darkGreen );
    cHighlightRule.m_cPatternRegExp = QRegExp( "//[^\n]*" );
    cHighlightRule.m_cFormat = m_cSingleLineCommentFormat;
    m_cHighlightingRulesVector.append( cHighlightRule );

    m_cMultiLineCommentFormat.setForeground( Qt::darkGreen );

    m_cCommentStartRegExp = QRegExp( "/\\*" );
    m_cCommentEndRegExp = QRegExp( "\\*/" );
  }

protected:

  void highlightBlock( const QString& i_strText )
  {
    foreach( const HighlightingRule& rcRule, m_cHighlightingRulesVector )
    {
      const QRegExp cRegExp( rcRule.m_cPatternRegExp );
      int32_t iIndex{ cRegExp.indexIn( i_strText ) };
      while( iIndex >= 0 )
      {
        const int32_t iLength{ cRegExp.matchedLength() };
        setFormat( iIndex, iLength, rcRule.m_cFormat );
        iIndex = cRegExp.indexIn( i_strText, iIndex + iLength );
      }
    }

    setCurrentBlockState( 0 );

    int32_t iStartIndex{ 0 };
    if( previousBlockState() != 1 )
    {
      iStartIndex = m_cCommentStartRegExp.indexIn( i_strText );
    }

    while( iStartIndex >= 0 )
    {
      int32_t iCommentLength;
      const int32_t iEndIndex{ m_cCommentEndRegExp.indexIn( i_strText, iStartIndex ) };
      if( iEndIndex == -1 )
      {
        setCurrentBlockState( 1 );
        iCommentLength = i_strText.length() - iStartIndex;
      }
      else
      {
        iCommentLength = iEndIndex - iStartIndex + m_cCommentEndRegExp.matchedLength();
      }

      setFormat( iStartIndex, iCommentLength, m_cMultiLineCommentFormat );
      iStartIndex = m_cCommentStartRegExp.indexIn( i_strText, iStartIndex + iCommentLength );
    }
  }

private:

  struct HighlightingRule
  {
    QRegExp m_cPatternRegExp;
    QTextCharFormat m_cFormat;
  };

  QVector<HighlightingRule> m_cHighlightingRulesVector;

  QRegExp m_cCommentStartRegExp;
  QRegExp m_cCommentEndRegExp;

  QTextCharFormat m_cKeywordFormat;
  QTextCharFormat m_cSingleLineCommentFormat;
  QTextCharFormat m_cMultiLineCommentFormat;
  QTextCharFormat m_cQuotationFormat;
  QTextCharFormat m_cFunctionFormat;
};


ScriptEditor::ScriptEditor( QWidget* i_pcParent )
  : QWidget( i_pcParent )
  , m_pcUI( new Ui::ScriptEditor )
{
  m_pcUI->setupUi( this );
  Init();
}


ScriptEditor::ScriptEditor( const QString& i_strFilePath, QWidget* i_pcParent )
  : QWidget( i_pcParent )
  , m_pcUI( new Ui::ScriptEditor )
{
  m_pcUI->setupUi( this );
  Init();

  const QFileInfo cFileInfo( i_strFilePath );
  if( cFileInfo.exists() )
  {
    Open( i_strFilePath );
  }
  else
  {
    m_pcUI->textEdit->setPlainText( i_strFilePath );
  }
}


ScriptEditor::~ScriptEditor()
{
  delete m_pcFindDialog;
  delete m_pcFindReplaceDialog;
  delete m_pcUI;
}


void ScriptEditor::ForceModification()
{
  m_pcUI->textEdit->document()->setModified( true );
}


void ScriptEditor::Init()
{
  m_pcFindDialog = new FindDialog( this );
  m_pcFindDialog->setModal( false );
  m_pcFindDialog->setTextEdit( m_pcUI->textEdit );

  m_pcFindReplaceDialog = new FindReplaceDialog( this );
  m_pcFindReplaceDialog->setModal( false );
  m_pcFindReplaceDialog->setTextEdit( m_pcUI->textEdit );
}


bool ScriptEditor::IsDirty() const
{
  return m_pcUI->textEdit->IsDirty();
}


void ScriptEditor::on_actionFind_triggered()
{
  m_pcFindDialog->show();
}


void ScriptEditor::on_actionFindReplace_triggered()
{
  m_pcFindReplaceDialog->show();
}


void ScriptEditor::on_actionGoto_Pos_triggered()
{
  ScriptGoto* pcDialog{ new ScriptGoto( m_pcUI->textEdit->document(), this ) };

  connect( pcDialog, SIGNAL( GotoPosition( int32_t, int32_t ) ),
           m_pcUI->textEdit, SLOT( onGotoCursorPosition( int32_t, int32_t ) ) );

  pcDialog->setModal( true );
  pcDialog->show();
}


void ScriptEditor::on_actionSave_Script_triggered()
{
  Save();
}


bool ScriptEditor::Open( const QString& i_strFilePath )
{
  m_strFilePath = i_strFilePath;

  // Open script file
  QFile cFile( i_strFilePath );
  if( !cFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    QMessageBox::critical( this, tr( "Unable to open file" ), cFile.errorString() );
    return false;
  }

  QTextStream cTextStream( &cFile );
  m_pcUI->textEdit->setPlainText( cTextStream.readAll() );

  return true;
}


bool ScriptEditor::RequestClose()
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


void ScriptEditor::Save()
{
  bool bNewSave{ false };

  if( m_strFilePath.isEmpty() )
  {
    const QString strScriptPath{ MainWindow::GetProjectScriptDir().absolutePath() };
    const QString strNewFilePath{ QFileDialog::getSaveFileName( this, tr( "Save Location" ), strScriptPath ) };
    if( !strNewFilePath.isEmpty() )
    {
      bool bValid{ true };
      QString strError;

      // Scripts must end with the .nut extension
      if( !strNewFilePath.endsWith( ".nut" ) )
      {
        bValid = false;
        strError = "Script file extension must be \".nut\"";
      }
      else if( !strNewFilePath.startsWith( strScriptPath ) )
      {
        bValid = false;
        strError = "Scripts can only exist in subfolder " + QDir::toNativeSeparators( strScriptPath );
      }

      if( bValid )
      {
        m_strFilePath = strNewFilePath;
        bNewSave = true;
      }
      else
      {
        QMessageBox::critical( this, tr( "Save Failed" ), strError );
      }
    }
  }

  QFile cFile( m_strFilePath );
  if( cFile.open( QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text ) )
  {
    QTextStream cTextStream( &cFile );
    cTextStream << m_pcUI->textEdit->toPlainText();
    m_pcUI->textEdit->document()->setModified( false );

    if( bNewSave )
    {
      // Refresh the tree widget and update the current tab to reflect the new file name info
      MainWindow* pcWindow{ MainWindow::GetMainWindow() };
      pcWindow->RefreshTreeWidget();
      pcWindow->SetTabInfo( pcWindow->GetCurrentTabIndex(), m_strFilePath );
    }
  }
  else if( !m_strFilePath.isEmpty() )
  {
    QMessageBox::critical( this, tr( "Unable to open file" ), cFile.errorString() );
  }
}
