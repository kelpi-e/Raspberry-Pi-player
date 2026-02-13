#include "PlaylistScene.h"
#include "PlaylistWindow.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

PlaylistScene::PlaylistScene(qreal rot, QObject* parent)
    : QObject(parent)
{
    wnd = new PlaylistWindow;
    proxy = sc.addWidget(wnd);
    proxy->setRotation(rot);
    sc.setSceneRect(proxy->sceneBoundingRect());

    connect(wnd, &PlaylistWindow::requestBack,
            this, &PlaylistScene::handleBackRequested);
    connect(wnd, &PlaylistWindow::itemClicked,
            this, &PlaylistScene::handleItemClicked);

    loadPlaylistsFromDisk();
    showPlaylistsList();
}

QGraphicsScene* PlaylistScene::scene()
{
    return &sc;
}

PlaylistWindow* PlaylistScene::window()
{
    return wnd;
}

void PlaylistScene::handleItemClicked(const int index)
{
    if (currentMode == Mode::PlaylistsList) {
        openPlaylistByIndex(index);
    } else {
        // In track view we do nothing on click for now
    }
}

void PlaylistScene::handleBackRequested()
{
    if (currentMode == Mode::TracksView) {
        showPlaylistsList();
    } else {
        emit requestBack();
    }
}

void PlaylistScene::loadPlaylistsFromDisk()
{
    playlists.clear();

    QDir dir(QCoreApplication::applicationDirPath());
    // Go up to project root and then to web-interface/data/playlists
    dir.cdUp();  // from build dir to project (e.g. main)
    dir.cdUp();  // to repo root (e.g. rspl)
    if (!dir.cd("web-interface/data/playlists")) {
        qWarning() << "[PlaylistScene] Could not find playlists directory at web-interface/data/playlists";
        return;
    }

    const QFileInfoList files = dir.entryInfoList(QStringList() << "*.json",
                                                  QDir::Files | QDir::NoDotAndDotDot,
                                                  QDir::Name);

    for (const QFileInfo& fi : files) {
        QFile f(fi.absoluteFilePath());
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            continue;
        }
        const auto data = f.readAll();
        f.close();

        const auto doc = QJsonDocument::fromJson(data);
        if (!doc.isObject()) {
            continue;
        }
        const QJsonObject obj = doc.object();
        PlaylistInfo info;
        info.id = fi.completeBaseName();
        info.name = obj.value(QStringLiteral("name")).toString(info.id);

        const QJsonArray tracks = obj.value(QStringLiteral("tracks")).toArray();
        info.trackCount = tracks.size();

        playlists.append(info);
    }
}

void PlaylistScene::showPlaylistsList()
{
    currentMode = Mode::PlaylistsList;

    QList<TrackInfo> rows;
    rows.reserve(playlists.size());
    for (const auto& pl : playlists) {
        TrackInfo info;
        info.title = pl.name;
        info.meta = QString::number(pl.trackCount) + QStringLiteral(" трек(ов)");
        info.coverPath = QString(); // У плейлистов нет обложек
        rows.append(info);
    }

    wnd->setPlaylist(rows, QStringLiteral("Плейлисты"));
}

void PlaylistScene::openPlaylistByIndex(const int index)
{
    if (index < 0 || index >= playlists.size()) {
        return;
    }

    const PlaylistInfo& pl = playlists.at(index);

    QDir dir(QCoreApplication::applicationDirPath());
    dir.cdUp();
    dir.cdUp();
    if (!dir.cd("web-interface/data/playlists")) {
        qWarning() << "[PlaylistScene] Could not find playlists directory when opening playlist";
        return;
    }

    const QString filePath = dir.filePath(pl.id + QStringLiteral(".json"));
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "[PlaylistScene] Failed to open playlist" << filePath;
        return;
    }
    const auto data = f.readAll();
    f.close();

    const auto doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        qWarning() << "[PlaylistScene] Invalid JSON in playlist" << filePath;
        return;
    }

    const QJsonObject obj = doc.object();
    const QJsonArray tracksArr = obj.value(QStringLiteral("tracks")).toArray();

    QList<TrackInfo> tracks;
    tracks.reserve(tracksArr.size());
    for (const QJsonValue& v : tracksArr) {
        if (!v.isObject()) continue;
        const QJsonObject t = v.toObject();
        TrackInfo info;
        info.title = t.value(QStringLiteral("name")).toString();
        const QString artist = t.value(QStringLiteral("artist")).toString();
        const QString duration = t.value(QStringLiteral("duration")).toString();
        if (!artist.isEmpty() && !duration.isEmpty()) {
            info.meta = artist + QStringLiteral(" — ") + duration;
        } else if (!artist.isEmpty()) {
            info.meta = artist;
        } else {
            info.meta = duration;
        }
        info.coverPath = t.value(QStringLiteral("cover")).toString();
        tracks.append(info);
    }

    currentMode = Mode::TracksView;
    wnd->setPlaylist(tracks, pl.name);
}
