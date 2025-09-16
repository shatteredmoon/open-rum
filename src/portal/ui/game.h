#ifndef GAME_H
#define GAME_H

#include <QByteArray>
#include <QMap>
#include <QString>

struct Game
{
    Game() : bInstalled(false) {}
    QString m_strTitle;
    QString m_strDescription;
    QString m_strNickname;
    QString m_strUuid;
    QByteArray m_cPixMap;
    bool bInstalled;
};

typedef QMap<QString, Game> GameMap;

#endif // GAME_H