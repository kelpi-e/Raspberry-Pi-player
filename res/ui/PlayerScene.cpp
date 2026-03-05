#include "PlayerScene.h"
#include "../audio/mediafactory.h"
#include <QCoreApplication>
#include <QLatin1String>
#include <QMainWindow>

#include "PlaylistScene.h"

PlayerScene::PlayerScene(const qreal rot, QObject* parent)
    : QObject(parent),
      view(&scene)
{
    const QString fileName = QCoreApplication::applicationDirPath() + "/../res/music/mp3/2.mp3";
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

    // ---------------- PlaylistScene ----------------
    playlistScene = new PlaylistScene(rot, this);

    connect(player->getUI().btnMenu, &QPushButton::clicked, this, [this]() {
        view.setScene(playlistScene->scene());
    });

    connect(playlistScene, &PlaylistScene::requestBack, this, [this]() {
        view.setScene(&scene);
    });

    connect(playlistScene, &PlaylistScene::requestPlayTrack, this, [this](const QString& path, const QString& typeStr) {
        MediaType mt = MediaType::Mp3;
        if (typeStr == QLatin1String("youtube")) mt = MediaType::Youtube;
        else if (typeStr == QLatin1String("spotify")) mt = MediaType::Spotify;
        audio.play(mt, path);
        view.setScene(&scene);
    });
}


void PlayerScene::show() {
    window.show();
}
