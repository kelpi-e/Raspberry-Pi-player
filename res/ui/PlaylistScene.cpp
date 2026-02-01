#include "PlaylistScene.h"

PlaylistScene::PlaylistScene(qreal rot, QObject* parent)
    : QObject(parent)
{
    wnd = new PlaylistWindow;
    proxy = sc.addWidget(wnd);
    proxy->setRotation(rot);
    sc.setSceneRect(proxy->sceneBoundingRect());

    connect(wnd, &PlaylistWindow::requestBack,
            this, &PlaylistScene::requestBack);
}

QGraphicsScene* PlaylistScene::scene()
{
    return &sc;
}

PlaylistWindow* PlaylistScene::window()
{
    return wnd;
}
