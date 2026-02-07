#include "PlayerScene.h"
#include <QCoreApplication>
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

    QList<QPair<QString, QString>> tracks = {
        {"Track 1", "Artist 1 — 3:45"},
        {"Track 2", "Artist 2 — 4:12"},
        {"Track 3", "Artist 3 — 2:58"},
        {"Track 4", "Artist 4 — 5:01"}
    };

    playlistScene->window()->setPlaylist(tracks, "Мой плейлист");

    connect(player->getUI().btnMenu, &QPushButton::clicked, this, [this]() {
        view.setScene(playlistScene->scene());
    });

    connect(playlistScene, &PlaylistScene::requestBack, this, [this]() {
        view.setScene(&scene);
    });

}


void PlayerScene::show() {
    window.show();
}
