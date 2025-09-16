#include <smTextEdit.h>

#include <mainwindow.h>

#include <QIcon>


smTextEdit::smTextEdit( QWidget* i_pcParent )
  : QTextEdit( i_pcParent )
  , m_bDirty( false )
{
  Init();
  setPlainText( "" );
}


smTextEdit::smTextEdit( const QString& i_strText, QWidget* i_pcParent )
  : QTextEdit( i_pcParent )
  , m_bDirty( false )
{
  Init();
  setPlainText( i_strText );
}


void smTextEdit::Init()
{
  setLineWrapMode( QTextEdit::NoWrap );

  const QFont cFont( "Courier New", 10 );
  setCurrentFont( cFont );

  connect( this, SIGNAL( cursorPositionChanged() ),
           this, SLOT( onCursorPositionChanged() ) );

  connect( document(), SIGNAL( modificationChanged( bool ) ),
           this, SLOT( onModificationChanged( bool ) ) );
}


void smTextEdit::onCursorPositionChanged()
{
  const QTextCursor& rcCursor{ textCursor() };

  //qDebug << "Cursor index: " << arg(cursor.position();

  MainWindow* pcMain{ MainWindow::GetMainWindow() };
  pcMain->SetStatusBarText( QString( " Ln %1 Col %2 " )
                            .arg( rcCursor.blockNumber() + 1 )
                            .arg( rcCursor.positionInBlock() + 1 ) );
}


void smTextEdit::onGotoCursorPosition( int32_t i_iLine, int32_t i_iColumn )
{
  QTextCursor& rcCursor{ textCursor() };

  rcCursor.setPosition( 0, QTextCursor::MoveAnchor );
  rcCursor.movePosition( QTextCursor::Down, QTextCursor::MoveAnchor, i_iLine - 1 );
  rcCursor.movePosition( QTextCursor::NextCharacter, QTextCursor::MoveAnchor, i_iColumn );

  setTextCursor( rcCursor );
}


void smTextEdit::onModificationChanged( bool i_bChanged )
{
  const QIcon cSaveIcon( ":/ui/resources/save.png" );
  MainWindow* pcMain{ MainWindow::GetMainWindow() };

  m_bDirty = i_bChanged;

  if( m_bDirty )
  {
    pcMain->SetTabIcon( pcMain->GetCurrentTabIndex(), cSaveIcon );
  }
  else
  {
    pcMain->SetTabIcon( pcMain->GetCurrentTabIndex(), QIcon() );
  }
}
