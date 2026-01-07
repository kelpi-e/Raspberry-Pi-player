#pragma once

#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsProxyWidget>
#include <QMainWindow>

#include "PlayerWindow.h"
#include "PlaylistScene.h"
#include "../audio/PlayerAudio.h"

class PlayerScene : public QObject {
    Q_OBJECT

public:
    explicit PlayerScene(qreal rot, QObject* parent = nullptr);
    void show();

private:
    PlayerAudio audio{};
    PlayerWindow* player{};

    QGraphicsScene scene;
    QGraphicsView view;
    QGraphicsProxyWidget* proxy;

    QMainWindow window;

    PlaylistScene* playlistScene{};

};
