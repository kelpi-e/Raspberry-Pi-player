#pragma once
#include <QWidget>
#include <QLabel>
#include "ui_PlaylistWindow.h"

class QWidget;
class QString;

class PlaylistWindow : public QWidget
{
    Q_OBJECT
public:
    explicit PlaylistWindow(QWidget* parent = nullptr);

    void setPlaylist(const QList<QPair<QString, QString>>& tracks,
                     const QString& name);

    signals:
        void requestBack();

private:
    Ui::PlaylistWindow ui;

    struct Item {
        QWidget* box{};
        QLabel* cover{};
        QLabel* title{};
        QLabel* meta{};
    };

    QList<Item> items;
    QList<QPair<QString, QString>> allTracks;
    int startIndex = 0;

    void createItems();
    void updateView();

private slots:
    void scrollUp();
    void scrollDown();
};
