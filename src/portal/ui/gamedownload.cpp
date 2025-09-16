#include <gamedownload.h>
#include <ui_gamedownload.h>

#include <game.h>
#include <mainwindow.h>

#include <network/u_patcher.h>
#include <platform.h>
#include <u_db.h>

#include <md5.h>

#include <QCryptographicHash>
#include <QDir>
#include <QNetworkRequest>
#include <QPushButton>
#include <QSqlDatabase>
#include <QSqlQuery>

#include <QCoreApplication>
#include <QEvent>


class FileSkippedEvent : public QEvent
{
public:
  FileSkippedEvent() : QEvent( ( QEvent::Type )QEvent::User )
  {}

  ~FileSkippedEvent() = default;

  static const QEvent::Type m_eType = static_cast<QEvent::Type>( QEvent::User );
};


GameDownload::GameDownload( const Game& i_rcGame, DownloadType i_eType, bool i_bAutoClose, QWidget* i_pcParent )
  : QDialog( i_pcParent )
  , m_pcUI( new Ui::GameDownload )
  , m_pcBuffer( new QByteArray() )
  , m_pcReply( nullptr )
  , m_pcConfig( nullptr )
  , m_rcGame( i_rcGame )
  , m_iCurrentRow( 0 )
  , m_ePhase( DL_CLIENT )
  , m_eType( Download_Full )
  , m_bAutoClose( i_bAutoClose )
{
  MainWindow* pcMain{ MainWindow::GetMainWindow( i_pcParent ) };
  if( pcMain )
  {
    m_pcConfig = &( pcMain->GetConfig() );
    const rumConfig& rcConfig{ *m_pcConfig };
    m_cBaseUrl = QUrl( QString( rcConfig.m_strWebAddress.c_str() ) );
  }

  m_pcUI->setupUi( this );

  // The OK button is disabled until all downloads are complete
  m_pcUI->buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );

  setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );

  QDir cDir;
  cDir.mkpath( i_rcGame.m_strUuid );

  setWindowTitle( "Downloading " + i_rcGame.m_strTitle );

  if( Download_Full == m_eType || Download_Repair == i_eType )
  {
    m_pcUI->listWidget->insertItem( m_iCurrentRow, "Gathering client download info..." );

    QSqlDatabase cDatabase{ MainWindow::GetSqlDatabase( "GAME_LIBRARY" ) };
    Q_ASSERT( cDatabase.isValid() );

    // Which version does the game expect?
    QSqlQuery cQuery( cDatabase );
    if( cQuery.exec( "SELECT rum_major_version,rum_minor_version FROM games WHERE uuid='" + i_rcGame.m_strUuid +
                     "' LIMIT 1" ) )
    {
      if( cQuery.next() )
      {
        const uint32_t uiMajor{ static_cast<uint32_t>( cQuery.value( 0 ).toInt() ) };
        const uint32_t uiMinor{ static_cast<uint32_t>( cQuery.value( 1 ).toInt() ) };

        // Build the client string based on platform and version info
        const QString strClient
        {
          QString( "client_%1_%2_%3" ).arg( GetPlatformString( PLATFORM_TYPE ) ).arg( uiMajor ).arg( uiMinor )
        };

        // Get the files required for the client
        QSqlQuery cQuery2( cDatabase );
        if( cQuery2.exec( "SELECT filepath,crc,size FROM clients WHERE name='" + strClient + "'" ) )
        {
          enum{ DB_FILEPATH, DB_CRC, DB_SIZE };

          while( cQuery2.next() )
          {
            DownloadInfo cInfo;
            cInfo.m_strFilePath = cQuery2.value( DB_FILEPATH ).toString();
            cInfo.m_strSource = m_cBaseUrl.toString() + "/" + cInfo.m_strFilePath;
            cInfo.m_strChecksum = cQuery2.value( DB_CRC ).toString();
            cInfo.m_ui64Size = cQuery2.value( DB_SIZE ).toULongLong();

            // For the target filename, strip everything up to strClient
            const int32_t iOffset{ cInfo.m_strFilePath.indexOf( strClient ) };
            cInfo.m_strDestination = m_rcGame.m_strUuid + "/" +
              cInfo.m_strFilePath.remove( 0, iOffset + strClient.length() + 1 );

            m_cDownloadQueue.push_back( cInfo );
          }
        }
      }
    }
  }

  DownloadNext();
}


GameDownload::~GameDownload()
{
  delete m_pcUI;

  m_ePhase = DL_DONE;
  m_cDownloadQueue.clear();

  if( m_pcBuffer )
  {
    delete m_pcBuffer;
  }

  if( m_pcReply )
  {
    delete m_pcReply;
  }
}


void GameDownload::customEvent( QEvent* i_pcEvent )
{
  if( i_pcEvent->type() == FileSkippedEvent::m_eType )
  {
    // Pop the entry and begin downloading the next file
    m_cDownloadQueue.dequeue();
    DownloadNext();
  }
}


void GameDownload::DownloadDone()
{
  m_pcUI->listWidget->insertItem( ++m_iCurrentRow, "Download finished." );
  m_pcUI->listWidget->scrollToBottom();

  if( m_bAutoClose )
  {
    accept();
  }
  else
  {
    // Show the okay button
    m_pcUI->buttonBox->button( QDialogButtonBox::Ok )->setEnabled( true );
    m_pcUI->buttonBox->button( QDialogButtonBox::Cancel )->setEnabled( false );
  }
}


void GameDownload::DownloadNext()
{
  if( !m_cDownloadQueue.isEmpty() && m_eType != Download_None )
  {
    const DownloadInfo& rcInfo{ m_cDownloadQueue.head() };
    bool bDownload{ true };

    QFileInfo fileInfo( rcInfo.m_strDestination );
    if( fileInfo.exists() )
    {
      if( rcInfo.m_bEditable )
      {
        bDownload = false;
      }
      else
      {
        const QString strChecksum{ GetFileChecksum( rcInfo.m_strDestination ) };
        if( strChecksum.compare( rcInfo.m_strChecksum, Qt::CaseInsensitive ) == 0 )
        {
          bDownload = false;
        }
      }
    }

    if( bDownload )
    {
      // Start downloading files
      QNetworkRequest cRequest;
      QUrl cUrl( rcInfo.m_strSource );
      cUrl.setPort( m_pcConfig->m_iWebPort );
      cRequest.setUrl( cUrl );
      cRequest.setRawHeader( "User-Agent", "RUM Portal" );

      m_pcReply = m_cNetworkMgr.get( cRequest );
      connect( m_pcReply, SIGNAL( readyRead() ), this, SLOT( slotReadyRead() ) );
      connect( m_pcReply, SIGNAL( error( QNetworkReply::NetworkError ) ),
               this, SLOT( slotError( QNetworkReply::NetworkError ) ) );
      connect( m_pcReply, SIGNAL( finished() ), this, SLOT( slotFinished() ) );

      //connect(reply, SIGNAL(sslErrors(QList<QSslError>)),
      //        this, SLOT(slotSslErrors(QList<QSslError>)));

      m_pcUI->listWidget->insertItem( ++m_iCurrentRow, "Downloading " + rcInfo.m_strFilePath );
      m_pcUI->listWidget->scrollToBottom();
    }
    else
    {
      m_pcUI->listWidget->insertItem( ++m_iCurrentRow, "Verified " + rcInfo.m_strFilePath );
      m_pcUI->listWidget->scrollToBottom();

      QCoreApplication::postEvent( this, new FileSkippedEvent );
    }
  }
  else
  {
    // Next download phase
    m_ePhase = (DownloadPhase)( (int32_t)(m_ePhase)+1 );

    switch( m_ePhase )
    {
      case DL_GAME_PATCH: DownloadGamePatchDatabase(); break;
      case DL_GAME_FILES: DownloadGameFiles(); break;
      case DL_DONE:
      default:
        DownloadDone(); break;
    }
  }
}


void GameDownload::DownloadGameFiles()
{
  if( m_eType != Download_None )
  {
    // Open the patch database and determine which files to download
    m_pcUI->listWidget->insertItem( ++m_iCurrentRow, "Parsing game download info..." );
    m_pcUI->listWidget->scrollToBottom();

    const QString strDatabase( m_rcGame.m_strUuid + "/" + m_rcGame.m_strNickname + "/" +
                               rumPatcher::GetPatchDatabaseName().data() );

    // Scope for db - required by removeDatabase call
    {
      // Connection doesn't exist, create it
      QSqlDatabase cDatabase{ QSqlDatabase::addDatabase( "QSQLITE", rumPatcher::GetPatchDatabaseName().data() ) };
      cDatabase.setDatabaseName( strDatabase );
      if( cDatabase.open() )
      {
        QSet<QString> cEditableFilesSet;

        if( Download_Patch == m_eType )
        {
          const QString strQuery{ QString( "SELECT filepath FROM conditional" ) };
          // WHERE platform = % 1" ).arg( PLATFORM_TYPE );
          QSqlQuery cQuery( cDatabase );
          if( cQuery.exec( strQuery ) )
          {
            enum{ DB_FILEPATH };

            while( cQuery.next() )
            {
              cEditableFilesSet.insert( cQuery.value( DB_FILEPATH ).toString() );
            }
          }

          cQuery.finish();
        }

        {
          const QString strQuery{ QString( "SELECT filepath,crc,size FROM standard" ) };
          // WHERE platform = % 1" ).arg( PLATFORM_TYPE );
          QSqlQuery cQuery( cDatabase );
          if( cQuery.exec( strQuery ) )
          {
            enum { DB_FILEPATH, DB_CRC, DB_SIZE };

            while( cQuery.next() )
            {
              QString strFilePath( cQuery.value( DB_FILEPATH ).toString() );

              DownloadInfo cInfo;
              cInfo.m_strFilePath = m_rcGame.m_strNickname + "/" + strFilePath;
              cInfo.m_strDestination = m_rcGame.m_strUuid + "/" + cInfo.m_strFilePath;
              cInfo.m_strSource = m_cBaseUrl.toString() + "/" + cInfo.m_strFilePath;
              cInfo.m_strChecksum = cQuery.value( DB_CRC ).toString();
              cInfo.m_ui64Size = cQuery.value( DB_SIZE ).toULongLong();
              cInfo.m_bEditable = cEditableFilesSet.contains( strFilePath );
              m_cDownloadQueue.push_back( cInfo );
            }
          }

          cQuery.finish();
        }

        cDatabase.close();
      }
    }

    QSqlDatabase::removeDatabase( rumPatcher::GetPatchDatabaseName().data() );
  }

  DownloadNext();
}


void GameDownload::DownloadGamePatchDatabase()
{
  if( m_eType != Download_None )
  {
    // Download the game file list (patch database)
    QDir cDir;
    cDir.mkpath( m_rcGame.m_strUuid + "/" + m_rcGame.m_strNickname );

    m_pcUI->listWidget->insertItem( ++m_iCurrentRow, "Gathering game download info..." );
    m_pcUI->listWidget->scrollToBottom();

    DownloadInfo cInfo;
    cInfo.m_strFilePath = m_rcGame.m_strNickname + "/" + rumPatcher::GetPatchDatabaseName().data();
    cInfo.m_strDestination = m_rcGame.m_strUuid + "/" + cInfo.m_strFilePath;
    cInfo.m_strSource = m_cBaseUrl.toString() + "/" + cInfo.m_strFilePath;
    cInfo.m_ui64Size = 0;
    m_cDownloadQueue.push_back( cInfo );
  }

  DownloadNext();
}


// static
QString GameDownload::GetFileChecksum( const QString& i_strFilePath )
{
  QString strHash;

  QFile cFile( i_strFilePath );
  if( cFile.open( QIODevice::ReadOnly ) )
  {
    strHash = QCryptographicHash::hash( cFile.readAll(), QCryptographicHash::Md5 ).toHex().constData();
    cFile.close();
  }

  return strHash;
}


void GameDownload::on_buttonBox_accepted()
{
  accept();
}


void GameDownload::on_buttonBox_rejected()
{
  // Prevent anything else from downloading
  m_eType = Download_None;

  reject();
}


void GameDownload::slotError( QNetworkReply::NetworkError i_eError )
{
  const QString strError{ QString( "Network Error: %1" ).arg( i_eError ) };
  m_pcUI->listWidget->insertItem( ++m_iCurrentRow, strError );
  m_pcUI->listWidget->scrollToBottom();
}


void GameDownload::slotFinished()
{
  const DownloadInfo& rcInfo{ m_cDownloadQueue.head() };

  QFileInfo cFileInfo( rcInfo.m_strDestination );

  // Make sure the underlying directory structure exists
  QDir cDir;
  cDir.mkpath( cFileInfo.absolutePath() );

  // Create a file for storing the downloaded contents
  QFile cFile( rcInfo.m_strDestination );
  if( cFile.open( QIODevice::WriteOnly ) )
  {
    cFile.write( *m_pcBuffer );
    cFile.close();
  }
  else
  {
    const QString strError{ QString( "Failed to save file " ) + rcInfo.m_strFilePath };
    m_pcUI->listWidget->insertItem( ++m_iCurrentRow, strError );
    m_pcUI->listWidget->scrollToBottom();
  }

  m_pcReply->deleteLater();
  m_pcReply = nullptr;

  // Pop the entry
  m_cDownloadQueue.dequeue();
  DownloadNext();

  m_pcBuffer->clear();
}


void GameDownload::slotReadyRead()
{
  // Append data to QByteArray buffer
  *m_pcBuffer += m_pcReply->readAll();

  QListWidgetItem* pcItem{ m_pcUI->listWidget->item( m_iCurrentRow ) };
  if( pcItem )
  {
    const DownloadInfo& rcInfo{ m_cDownloadQueue.head() };

    if( rcInfo.m_ui64Size > 0 )
    {
      double dPercent{ m_pcBuffer->size() * 100 / static_cast<double>( rcInfo.m_ui64Size ) };
      if( dPercent > 100 )
      {
        // Just in case the size is stored incorrectly, we don't want this number to go above 100
        dPercent = 100;
      }

      const QString strProgress{ QString( "Downloading %1: %2%" ).arg( rcInfo.m_strFilePath ).arg( dPercent ) };
      pcItem->setText( strProgress );
    }
    else
    {
      const QString strProgress{ QString( "Downloading %1" ).arg( rcInfo.m_strFilePath ) };
      pcItem->setText( strProgress );
    }
  }
}
