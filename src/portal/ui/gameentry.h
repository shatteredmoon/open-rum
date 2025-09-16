#ifndef GAMEENTRY_H
#define GAMEENTRY_H

#include <QWidget>

#include <game.h>
#include <p_rum.h>

namespace Ui
{
  class GameEntry;
}

class GameEntry : public QWidget
{
  Q_OBJECT

public:

  explicit GameEntry( const Game& i_rcGame, const QString& i_strButtonText, QWidget* i_pcParent = 0 );
  ~GameEntry();

  const QString& GetNickName() const
  {
    return m_cGame.m_strNickname;
  }
  const QString& GetUuid() const
  {
    return m_cGame.m_strUuid;
  }

  void DownloadGame( DownloadType i_eType, bool i_bPrompt, bool i_bAutostart );
  void LaunchGame();

private slots:

  void actionDownload_triggered();
  void actionExplore_triggered();
  void actionLaunch_triggered();
  void actionRepair_triggered();
  void actionUninstall_triggered();

  void HandleButton();
  void ShowContextMenu( const QPoint& i_rcPos );

private:

  void UninstallGame();

  Ui::GameEntry* m_pcUI;

  Game m_cGame;
};

#endif // GAMEENTRY_H
