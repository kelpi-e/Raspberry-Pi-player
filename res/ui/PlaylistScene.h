#pragma once
#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QWidget>
#include "PlaylistWindow.h"

class PlaylistScene : public QObject
{
    Q_OBJECT
public:

    explicit PlaylistScene(const qreal rot, QObject* parent = nullptr);

    QGraphicsScene* getScene();                // возвращает указатель на сцену
    QGraphicsProxyWidget* getProxy() const;          // возвращает прокси виджет
    QWidget* getWidget() const;                      // возвращает сам виджет плейлиста

private:
    QGraphicsScene scene_;                     // сама сцена
    QGraphicsProxyWidget* proxy_{};           // прокси виджет
    PlaylistWindow* playlist_{};              // виджет UI плейлиста
};
