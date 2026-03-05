#pragma once
#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include "PlaylistWindow.h"

struct PlaylistInfo {
    QString id;
    QString name;
    int trackCount = 0;
};

class PlaylistScene : public QObject
{
    Q_OBJECT
public:
    explicit PlaylistScene(qreal rot, QObject* parent = nullptr);

    QGraphicsScene* scene();
    PlaylistWindow* window();

signals:
    void requestBack();
    void requestPlayTrack(const QString& path, const QString& type);

private slots:
    void handleItemClicked(int index);
    void handleBackRequested();
    void handleShuffleToggle();
    void handlePlayPlaylist();

private:
    QGraphicsScene sc;
    QGraphicsProxyWidget* proxy{};
    PlaylistWindow* wnd{};

    enum class Mode {
        PlaylistsList,
        TracksView
    };

    Mode currentMode = Mode::PlaylistsList;
    QList<PlaylistInfo> playlists;
    QList<TrackInfo> currentTracks;
    QString currentPlaylistName;

    void loadPlaylistsFromDisk();
    void showPlaylistsList();
    void openPlaylistByIndex(int index);
};
