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

private slots:
    void handleItemClicked(int index);
    void handleBackRequested();

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

    void loadPlaylistsFromDisk();
    void showPlaylistsList();
    void openPlaylistByIndex(int index);
};
