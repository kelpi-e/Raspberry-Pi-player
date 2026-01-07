#include "SpotifyScene.h"

#include <QWebEngineSettings>
#include <QUrl>

SpotifyScene::SpotifyScene(const qreal rot, QObject* parent)
    : QObject(parent)
{
    web = new QWebEngineView;

    web->setUrl(QUrl("https://open.spotify.com"));
    web->setFixedSize(2560, 1600);

    auto* s = web->settings();
    s->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    s->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    s->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);

    proxy = sc.addWidget(web);
    proxy->setRotation(rot);

    sc.setSceneRect(proxy->sceneBoundingRect());
}

QGraphicsScene* SpotifyScene::scene() {
    return &sc;
}

QSize SpotifyScene::size() const {
    return sc.sceneRect().size().toSize();
}
