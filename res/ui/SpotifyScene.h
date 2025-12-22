#pragma once

#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QWebEngineView>

class SpotifyScene : public QObject {
    Q_OBJECT

public:
    explicit SpotifyScene(qreal rot, QObject* parent = nullptr);

    QGraphicsScene* scene();
    QSize size() const;

private:
    QGraphicsScene sc;
    QWebEngineView* web;
    QGraphicsProxyWidget* proxy;
};
