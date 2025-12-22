#include "PlayerScene.h"
#include <QCoreApplication>

PlayerScene::PlayerScene(const qreal rot, QObject* parent)
    : QObject(parent),
      view(&scene)
{
    const QString fileName = QCoreApplication::applicationDirPath() + "/../res/music/2.mp3";
    player = new PlayerWindow(nullptr, &audio);

    audio.setCurrentlyPlaying(fileName);
    audio.setCurrentMediaType(MediaType::Mp3);

    proxy = scene.addWidget(player);
    proxy->setRotation(rot);
    scene.setSceneRect(proxy->sceneBoundingRect());

    view.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view.setAlignment(Qt::AlignCenter);

    window.setWindowFlags(Qt::FramelessWindowHint);
    window.setCentralWidget(&view);
    window.resize(proxy->sceneBoundingRect().size().toSize());

    connect(player, &PlayerWindow::requestClose,
            &window, &QMainWindow::close);
}

void PlayerScene::show() {
    window.show();
}
