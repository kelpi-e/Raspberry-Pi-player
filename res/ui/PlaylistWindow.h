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
    QString path;
    QString type;  // "mp3", "youtube", "spotify"
};

class PlaylistWindow : public QWidget
{
    Q_OBJECT
public:
    explicit PlaylistWindow(QWidget* parent = nullptr);
    ~PlaylistWindow();

    void setPlaylist(const QList<TrackInfo>& tracks,
                     const QString& name,
                     bool showCovers = true);

signals:
    void requestBack();
    void itemClicked(int index);
    void requestShuffleToggle();
    void requestPlayPlaylist();

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
    int visibleRows = 4;
    bool showCovers = true;
    QPushButton* btnShuffle = nullptr;
    QPushButton* btnPlay = nullptr;
    bool shuffleEnabled = false;

    void createItems(int rowCount);
    void updateView();
    QPixmap loadCover(const QString& coverPath);

private slots:
    void scrollUp();
    void scrollDown();
};
