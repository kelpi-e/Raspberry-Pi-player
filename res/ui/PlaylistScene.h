#pragma once
#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include "PlaylistWindow.h"

class PlaylistScene : public QObject
{
    Q_OBJECT
public:
    explicit PlaylistScene(qreal rot, QObject* parent = nullptr);

    QGraphicsScene* scene();
    PlaylistWindow* window();

    signals:
        void requestBack();

private:
    QGraphicsScene sc;
    QGraphicsProxyWidget* proxy{};
    PlaylistWindow* wnd{};
};
