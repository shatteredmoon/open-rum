#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <platform.h>

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSqlDatabase>

class GameEntry;
class QFileInfo;
class QLayout;
class QVBoxLayout;

struct rumConfig;
struct Game;

namespace Ui
{
  class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:

  explicit MainWindow( rumConfig& i_rcConfig, QWidget* i_pcParent = 0 );
  ~MainWindow();

  void CheckGames();

  const rumConfig& GetConfig() const
  {
    return m_rcConfig;
  }

  // Recurses object parents until it gets to the main window, or returns NULL if no main window was found
  static MainWindow* GetMainWindow( QObject* i_pcObject );

  // Access a sql database via connection name
  static QSqlDatabase GetSqlDatabase( const QString& i_strConnection )
  {
    return QSqlDatabase::database( i_strConnection );
  }

  void Init( int32_t i_iArgc, char* i_pcArgv[] );

  static bool RemoveRecursively( const QString& i_strDir );

private slots:

  void slotError( QNetworkReply::NetworkError i_eError );
  void slotFinished();
  void slotReadyRead();

private:

  void AddGame( const Game& i_rcGame, const QString& i_strButtonText, QVBoxLayout& i_rcWidget );
  void AddAvailableGame( const Game& i_rcGame );
  void AddInstalledGame( const Game& i_rcGame );

  void DatabaseAdd( QFileInfo& i_rcFileInfo, const QString& i_strName );
  void DatabaseInit();
  void DatabaseShutdown();

  void RemoveGames( QLayout& i_rcLayout );

  Ui::MainWindow* m_pcUI;

  QNetworkAccessManager m_cNetworkMgr;
  QByteArray* m_pcBuffer;
  QNetworkReply* m_pcReply;

  QList<GameEntry*> m_listGameEntries;

  rumConfig& m_rcConfig;
};

#endif // MAINWINDOW_H
