#include "PlaylistScene.h"

PlaylistScene::PlaylistScene(const qreal rot, QObject* parent)
    : QObject(parent)
{
    playlist_ = new PlaylistWindow;
    playlist_->setFixedSize(320, 240);

    proxy_ = scene_.addWidget(playlist_);
    proxy_->setRotation(rot);
    scene_.setSceneRect(proxy_->sceneBoundingRect());
}

QGraphicsScene* PlaylistScene::getScene() {
    return &scene_;
}

QGraphicsProxyWidget* PlaylistScene::getProxy() {
    return proxy_;
}

QWidget* PlaylistScene::getWidget() {
    return playlist_;
}
