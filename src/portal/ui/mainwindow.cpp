#include <mainwindow.h>
#include <ui_mainwindow.h>

#include <gameentry.h>
#include <ui_gameentry.h>

#include <u_log.h>

#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QNetworkRequest>
#include <QSqlQuery>


MainWindow::MainWindow( rumConfig& i_rcConfig, QWidget* i_pcParent )
  : QMainWindow( i_pcParent )
  , m_pcUI( new Ui::MainWindow )
  , m_rcConfig( i_rcConfig )
  , m_pcBuffer( new QByteArray() )
  , m_pcReply( NULL )
{
  m_pcUI->setupUi( this );
  m_pcUI->verticalLayout->setEnabled( false );
}


MainWindow::~MainWindow()
{
  if( m_pcBuffer )
  {
    delete m_pcBuffer;
  }

  if( m_pcReply )
  {
    delete m_pcReply;
  }

  delete m_pcUI;
  DatabaseShutdown();
}


void MainWindow::AddGame( const Game& i_rcGame, const QString &i_strButtonText, QVBoxLayout& i_rcLayout )
{
  GameEntry* pcGameEntry{ new GameEntry( i_rcGame, i_strButtonText, this ) };

  i_rcLayout.setAlignment( Qt::AlignTop );

  i_rcLayout.addWidget( pcGameEntry );

  QFrame* pcFrame{ new QFrame() };
  pcFrame->setFrameShape( QFrame::HLine );
  pcFrame->setFrameShadow( QFrame::Sunken );

  i_rcLayout.addWidget( pcFrame );
}


void MainWindow::AddAvailableGame( const Game& i_rcGame )
{
  AddGame( i_rcGame, "Download", *( m_pcUI->verticalLayout_Available ) );
}



void MainWindow::AddInstalledGame( const Game& i_rcGame )
{
  AddGame( i_rcGame, "Launch", *( m_pcUI->verticalLayout_Installed ) );
}


void MainWindow::CheckGames()
{
  RemoveGames( *( m_pcUI->verticalLayout_Available ) );
  RemoveGames( *( m_pcUI->verticalLayout_Installed ) );

  GameMap cGamesMap;

  QSqlDatabase cLibraryDatabase{ MainWindow::GetSqlDatabase( "GAME_LIBRARY" ) };
  Q_ASSERT( cLibraryDatabase.isValid() );

  QSqlQuery cQuery( cLibraryDatabase );
  if( cQuery.exec( "SELECT title,uuid,nickname,icon,description FROM games" ) )
  {
    enum{ DB_TITLE, DB_UUID, DB_NICKNAME, DB_ICON, DB_DESC };
    while( cQuery.next() )
    {
      Game cGame;
      cGame.m_strTitle = cQuery.value( DB_TITLE ).toString();
      cGame.m_strUuid = cQuery.value( DB_UUID ).toString();
      cGame.m_strNickname = cQuery.value( DB_NICKNAME ).toString();
      cGame.m_cPixMap = cQuery.value( DB_ICON ).toByteArray();
      cGame.m_strDescription = cQuery.value( DB_DESC ).toByteArray();
      cGamesMap.insert( cGame.m_strUuid, cGame );
    }
  }

  // Determine Installed projects
  QMap<QString, Game>::iterator iter( cGamesMap.begin() );
  QMap<QString, Game>::iterator end( cGamesMap.end() );
  while( iter != end )
  {
    Game& rcGame{ iter.value() };

    const QString strFilePath( iter.key() + "/" + iter.value().m_strNickname );
    const QDir cDir( strFilePath );
    if( cDir.exists() )
    {
      const QFileInfo cFileInfo( cDir, "game.rum" );
      if( cFileInfo.exists() )
      {
        // Open the database and verify uuids agree
        QSqlDatabase cGameDatabase{ QSqlDatabase::addDatabase( "QSQLITE", "GAME" ) };
        cGameDatabase.setDatabaseName( cFileInfo.absoluteFilePath() );
        if( cGameDatabase.isValid() && cGameDatabase.open() )
        {
          {
            QSqlQuery cGameQuery( cGameDatabase );
            if( cGameQuery.exec( "SELECT uuid FROM settings" ) )
            {
              if( cGameQuery.next() )
              {
                const QString strUuid{ cGameQuery.value( 0 ).toString() };
                if( strUuid.compare( iter.key(), Qt::CaseInsensitive ) == 0 )
                {
                  rcGame.bInstalled = true;
                  AddInstalledGame( rcGame );
                }
              }
            }
          }

          cGameDatabase.close();
        }

        QSqlDatabase::removeDatabase( "GAME" );
      }
    }

    if( !rcGame.bInstalled )
    {
      AddAvailableGame( rcGame );
    }

    ++iter;
  }

  QString strUuid( m_rcConfig.m_strUuid.c_str() );

  bool bDownloaded = false;

  // Download or patch any of the games?
  QList<GameEntry*>::iterator gameIter( m_listGameEntries.begin() );
  QList<GameEntry*>::iterator gameEnd( m_listGameEntries.end() );
  while( !bDownloaded && gameIter != gameEnd )
  {
    GameEntry* pcGame{ *gameIter };
    if( pcGame->GetUuid().compare( strUuid, Qt::CaseInsensitive ) == 0 )
    {
      if( m_rcConfig.m_eDownloadType == Download_Full )
      {
        pcGame->DownloadGame( Download_Full, false, m_rcConfig.m_bAutoStart );
        bDownloaded = true;
      }
      else if( m_rcConfig.m_eDownloadType == Download_Patch )
      {
        pcGame->DownloadGame( Download_Patch, false, m_rcConfig.m_bAutoStart );
        bDownloaded = true;
      }
      else if( m_rcConfig.m_eDownloadType == Download_Repair )
      {
        pcGame->DownloadGame( Download_Repair, false, m_rcConfig.m_bAutoStart );
        bDownloaded = true;
      }

      if( bDownloaded && m_rcConfig.m_bAutoStart )
      {
        pcGame->LaunchGame();
      }
    }

    ++gameIter;
  }
}


void MainWindow::DatabaseAdd( QFileInfo& i_rcFileInfo, const QString& i_strName )
{
  // Connection doesn't exist, create it
  QSqlDatabase cDatabase{ QSqlDatabase::addDatabase( "QSQLITE", i_strName ) };
  cDatabase.setDatabaseName( i_rcFileInfo.absoluteFilePath() );
  if( !cDatabase.open() )
  {
    QMessageBox::critical( this, tr( "Error" ), "Failed to open database " + i_rcFileInfo.fileName() );
    return;
  }
}


void MainWindow::DatabaseInit()
{
  // All database files are known, do not perform a recursive check
  QFileInfo cFileInfo( DEFAULT_LIBRARY_DB );
  DatabaseAdd( cFileInfo, "GAME_LIBRARY" );
}


void MainWindow::DatabaseShutdown()
{
  const QStringList strConnections{ QSqlDatabase::connectionNames() };
  for( int32_t i = 0; i < strConnections.size(); ++i )
  {
    const QString strName{ strConnections.at( i ) };
    {
      QSqlDatabase cDatabase{ MainWindow::GetSqlDatabase( strName ) };
      if( cDatabase.isValid() )
      {
        cDatabase.close();
      }
    }

    QSqlDatabase::removeDatabase( strName );
  }
}


// static
MainWindow* MainWindow::GetMainWindow( QObject* i_pcObject )
{
  QObject* pcObject{ i_pcObject };
  MainWindow* pcMainWindow{ nullptr };
  while( pcObject != nullptr && ( nullptr == pcMainWindow ) )
  {
    pcMainWindow = qobject_cast<MainWindow *>( pcObject );
    pcObject = pcObject->parent();
  }

  return pcMainWindow;
}


void MainWindow::Init( int32_t i_iArgc, char* i_pcArgv[] )
{
  RUM_COUT( "Using Qt version" << qVersion() << "\n" );

  // Start downloading files
  QNetworkRequest cRequest;
  QUrl cUrl( QString( m_rcConfig.m_strWebAddress.c_str() ) + "/" + DEFAULT_LIBRARY_DB );
  cUrl.setPort( m_rcConfig.m_iWebPort );
  cRequest.setUrl( cUrl );
  cRequest.setRawHeader( "User-Agent", "RUM Portal" );

  m_pcReply = m_cNetworkMgr.get( cRequest );
  connect( m_pcReply, SIGNAL( readyRead() ), this, SLOT( slotReadyRead() ) );
  connect( m_pcReply, SIGNAL( error( QNetworkReply::NetworkError ) ),
           this, SLOT( slotError( QNetworkReply::NetworkError ) ) );
  connect( m_pcReply, SIGNAL( finished() ), this, SLOT( slotFinished() ) );
}


void MainWindow::RemoveGames( QLayout& i_rcLayout )
{
  QLayoutItem* pcItem{ nullptr };

  while( i_rcLayout.count() != 0 )
  {
    pcItem = i_rcLayout.takeAt( 0 );
    if( pcItem->layout() != 0 )
    {
      RemoveGames( *( pcItem->layout() ) );
    }
    else if( pcItem->widget() != 0 )
    {
      delete pcItem->widget();
    }

    delete pcItem;
  }
}


// static
bool MainWindow::RemoveRecursively( const QString& i_strDir )
{
  bool bResult{ true };

  const QDir cDir( i_strDir );
  if( cDir.exists() )
  {
    Q_FOREACH( QFileInfo cInfo, cDir.entryInfoList( QDir::NoDotAndDotDot | QDir::System | QDir::Hidden |
                                                    QDir::AllDirs | QDir::Files, QDir::DirsFirst ) )
    {
      if( cInfo.isDir() )
      {
        bResult = RemoveRecursively( cInfo.absoluteFilePath() );
      }
      else
      {
        bResult = QFile::remove( cInfo.absoluteFilePath() );
      }

      if( !bResult )
      {
        return bResult;
      }
    }

    bResult = cDir.rmdir( i_strDir );
  }

  return bResult;
}


void MainWindow::slotError( QNetworkReply::NetworkError i_eError )
{
  const QString strError{ QString( "Network Error: %1" ).arg( i_eError ) };
  Logger::LogStandard( strError.toLocal8Bit().constData(), Logger::LOG_ERROR );
}


void MainWindow::slotFinished()
{
  // Create a file for storing the downloaded contents
  QFile cFile( DEFAULT_LIBRARY_DB );
  if( cFile.open( QIODevice::WriteOnly ) )
  {
    cFile.write( *m_pcBuffer );
    cFile.close();
  }
  else
  {
    const QString strError{ QString( "Failed to save file " ) + DEFAULT_LIBRARY_DB };
    Logger::LogStandard( strError.toLocal8Bit().constData(), Logger::LOG_ERROR );
  }

  m_pcReply->deleteLater();
  m_pcReply = nullptr;

  m_pcBuffer->clear();

  DatabaseInit();
  CheckGames();
}


void MainWindow::slotReadyRead()
{
  // Append data to QByteArray buffer
  *m_pcBuffer += m_pcReply->readAll();
}
