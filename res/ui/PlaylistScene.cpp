#include "PlaylistScene.h"

PlaylistScene::PlaylistScene(const qreal rot, QObject* parent)
    : QObject(parent)
{
    playlist_ = new PlaylistWindow;
    playlist_->setFixedSize(240, 320);

    proxy_ = scene_.addWidget(playlist_);
    proxy_->setRotation(rot);
    scene_.setSceneRect(proxy_->sceneBoundingRect());
}

QGraphicsScene* PlaylistScene::getScene() {
    return &scene_;
}

QGraphicsProxyWidget* PlaylistScene::getProxy() const {
    return proxy_;
}

QWidget* PlaylistScene::getWidget() const {
    return playlist_;
}
