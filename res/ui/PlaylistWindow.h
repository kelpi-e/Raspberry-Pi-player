#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include "ui_PlaylistWindow.h"

class PlaylistWindow : public QWidget {
    Q_OBJECT

signals:
    void requestBack();

public:
    explicit PlaylistWindow(QWidget* parent = nullptr);

    void setPlaylist(const QList<QPair<QString, QString>>& tracks);

private:
    Ui::PlaylistWindow ui{};
};
