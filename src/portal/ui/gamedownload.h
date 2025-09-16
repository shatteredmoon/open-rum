#ifndef GAMEDOWNLOAD_H
#define GAMEDOWNLOAD_H

#include <p_rum.h>

#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QQueue>
#include <QUrl>

class QEvent;

struct rumConfig;
struct Game;

namespace Ui
{
  class GameDownload;
}

class GameDownload : public QDialog
{
  Q_OBJECT

public:

  explicit GameDownload( const Game& i_rcGame, DownloadType i_eType, bool i_bAutostart, QWidget* i_pcParent = 0 );
  ~GameDownload();

private slots:

  void on_buttonBox_accepted();
  void on_buttonBox_rejected();
  void slotError( QNetworkReply::NetworkError i_eError );
  void slotFinished();
  void slotReadyRead();

private:

  void customEvent( QEvent* i_pcEvent );

  void DownloadDone();
  void DownloadNext();
  void DownloadGameFiles();
  void DownloadGamePatchDatabase();

  static QString GetFileChecksum( const QString& i_strFilePath );

  struct DownloadInfo
  {
    DownloadInfo() : m_ui64Size( 0 ), m_bEditable( false )
    {}
    QString m_strFilePath;
    QString m_strSource;
    QString m_strDestination;
    QString m_strChecksum;
    quint64 m_ui64Size;

    // TODO: currently unused, but could possibly be used to distinguish between a user's request for a full
    // download versus a repair
    bool m_bEditable;
  };

  enum DownloadPhase
  {
    DL_CLIENT, DL_GAME_PATCH, DL_GAME_FILES, DL_DONE
  };

  Ui::GameDownload* m_pcUI;

  QNetworkAccessManager m_cNetworkMgr;

  QQueue<DownloadInfo> m_cDownloadQueue;

  QUrl m_cBaseUrl;

  QByteArray* m_pcBuffer;
  QNetworkReply* m_pcReply;

  const rumConfig* m_pcConfig;
  const Game& m_rcGame;

  int32_t m_iCurrentRow;

  DownloadPhase m_ePhase;

  DownloadType m_eType;

  // When true, the download dialog closes automatically
  bool m_bAutoClose;
};

#endif // GAMEDOWNLOAD_H
