#pragma once
#include <QWidget>
#include <QLabel>
#include "ui_PlaylistWindow.h"
#include "../utils/MarqueeController.h"

class QWidget;
class QString;

struct TrackInfo {
    QString title;
    QString meta;
    QString coverPath;
};

class PlaylistWindow : public QWidget
{
    Q_OBJECT
public:
    explicit PlaylistWindow(QWidget* parent = nullptr);
    ~PlaylistWindow();

    void setPlaylist(const QList<TrackInfo>& tracks,
                     const QString& name);

signals:
    void requestBack();
    // Emitted when a visible row is clicked: index in the current list
    void itemClicked(int index);

private:
    Ui::PlaylistWindow ui;

    struct Item {
        QWidget* box{};
        QWidget* coverWrapper{};  // Сохраняем обёртку для правильного удаления
        QLabel* cover{};
        QLabel* title{};
        QLabel* meta{};
        MarqueeController* titleMarquee{};
        MarqueeController* metaMarquee{};
    };

    QList<Item> items;
    QList<TrackInfo> allTracks;
    int startIndex = 0;

    void createItems();
    void updateView();
    QPixmap loadCover(const QString& coverPath);

private slots:
    void scrollUp();
    void scrollDown();
};
