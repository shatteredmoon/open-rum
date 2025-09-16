#include <gameentry.h>
#include <ui_gameentry.h>

#include <gamedownload.h>
#include <platform.h>
#include <mainwindow.h>

#include <QDesktopServices>
#include <QDir>
#include <QIcon>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPixMap>
#include <QProcess>

#define SCRIPT_FOLDER_NAME "scripts"

GameEntry::GameEntry( const Game& i_rcGame, const QString& i_strButtonText, QWidget* i_pcParent )
  : QWidget( i_pcParent )
  , m_pcUI( new Ui::GameEntry )
{
  m_pcUI->setupUi( this );

  m_cGame = i_rcGame;

  m_pcUI->pushButton->setToolTip( i_strButtonText + " " + i_rcGame.m_strTitle );

  m_pcUI->labelTitle->setText( i_rcGame.m_strTitle );
  m_pcUI->labelTitle->setToolTip( "<p style='white-space:pre'>" + i_rcGame.m_strDescription );

  QPixmap cPixmap;
  cPixmap.loadFromData( i_rcGame.m_cPixMap );

  QIcon cIcon;
  cIcon.addPixmap( cPixmap );

  m_pcUI->pushButton->setIcon( cIcon );
  m_pcUI->pushButton->setContextMenuPolicy( Qt::CustomContextMenu );

  connect( m_pcUI->pushButton, SIGNAL( customContextMenuRequested( const QPoint& ) ),
           this, SLOT( ShowContextMenu( const QPoint& ) ) );

  connect( m_pcUI->pushButton, SIGNAL( released() ), this, SLOT( HandleButton() ) );
}


GameEntry::~GameEntry()
{
  delete m_pcUI;
}


void GameEntry::actionDownload_triggered()
{
  DownloadGame( Download_Full, true, false );
}


void GameEntry::actionExplore_triggered()
{
  const QString strFilePath( QDir::toNativeSeparators( QApplication::applicationDirPath() + "/" +
                                                       m_cGame.m_strUuid + "/" + m_cGame.m_strNickname ) );
  QDesktopServices::openUrl( QUrl::fromLocalFile( strFilePath ) );
}


void GameEntry::actionLaunch_triggered()
{
  LaunchGame();
}


void GameEntry::actionRepair_triggered()
{
  MainWindow::RemoveRecursively( m_cGame.m_strUuid + "/" + m_cGame.m_strNickname + "/" + SCRIPT_FOLDER_NAME );
  DownloadGame( Download_Repair, true, false );
}


void GameEntry::actionUninstall_triggered()
{
  UninstallGame();
}


void GameEntry::DownloadGame( DownloadType i_eType, bool i_bPrompt, bool i_bAutostart )
{
  bool bDownload = false;

  QString strPrompt;

  switch( i_eType )
  {
    case Download_Full:   strPrompt = "Download"; break;
    case Download_Patch:  strPrompt = "Patch"; break;
    case Download_Repair: strPrompt = "Repair"; break;
  }

  if( i_bPrompt )
  {
    const QMessageBox::StandardButton cButton =
      QMessageBox::question( this, "Verify " + strPrompt, strPrompt + " " + m_cGame.m_strTitle + "?",
                             QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel );

    bDownload = ( cButton == QMessageBox::Yes );
  }
  else
  {
    bDownload = true;
  }

  if( bDownload )
  {
    // Modal download dialog
    GameDownload* pcGameDownload{ new GameDownload( m_cGame, i_eType, i_bAutostart, this ) };
    pcGameDownload->setModal( true );
    pcGameDownload->exec();

    MainWindow* pcMain{ MainWindow::GetMainWindow( parent() ) };
    if( pcMain )
    {
      pcMain->CheckGames();
    }
  }
}


void GameEntry::HandleButton()
{
  if( m_cGame.bInstalled )
  {
    LaunchGame();
  }
  else
  {
    DownloadGame( Download_Full, true, false );
  }
}


void GameEntry::LaunchGame()
{
  QProcess cProcess;
  const QString strFilePath{ m_cGame.m_strUuid + "/" + "rum_client" + DEFAULT_BIN_EXT };
  QStringList strList;
  if( cProcess.startDetached( strFilePath, strList, m_cGame.m_strUuid ) )
  {
    // Close self
    QApplication::quit();
  }
}


void GameEntry::ShowContextMenu( const QPoint& i_rcPos )
{
  const QPoint cGlobalPos{ m_pcUI->pushButton->mapToGlobal( i_rcPos ) };

  QMenu cMenu;

  if( m_cGame.bInstalled )
  {
    cMenu.addAction( "Launch", this, SLOT( actionLaunch_triggered() ) );
    cMenu.addAction( "Show in Explorer", this, SLOT( actionExplore_triggered() ) );
    cMenu.addAction( "Repair", this, SLOT( actionRepair_triggered() ) );
    cMenu.addAction( "Uninstall", this, SLOT( actionUninstall_triggered() ) );
  }
  else
  {
    cMenu.addAction( "Download", this, SLOT( actionDownload_triggered() ) );
  }

  cMenu.exec( cGlobalPos );
}


void GameEntry::UninstallGame()
{
  const QMessageBox::StandardButton cButton =
    QMessageBox::question( this, "Verify Uninstall", "This will completely remove " + m_cGame.m_strTitle +
                           ". Continue Uninstall?",
                           QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel );
  if( cButton == QMessageBox::Yes )
  {
    MainWindow::RemoveRecursively( m_cGame.m_strUuid );

    MainWindow* pcMain{ MainWindow::GetMainWindow( parent() ) };
    if( pcMain )
    {
      pcMain->CheckGames();
    }
  }
}
