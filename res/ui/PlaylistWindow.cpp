#include "PlaylistWindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPixmap>

PlaylistWindow::PlaylistWindow(QWidget* parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    connect(ui.btnBack, &QPushButton::clicked,
            this, &PlaylistWindow::requestBack);
    connect(ui.btnUp, &QPushButton::clicked,
            this, &PlaylistWindow::scrollUp);
    connect(ui.btnDown, &QPushButton::clicked,
            this, &PlaylistWindow::scrollDown);

    createItems();
}

void PlaylistWindow::createItems()
{
    for (int i = 0; i < 3; ++i) {
        Item it;
        it.box = new QWidget;
        auto* h = new QHBoxLayout(it.box);

        it.cover = new QLabel;
        it.cover->setFixedSize(48,48);

        auto* v = new QVBoxLayout;
        it.title = new QLabel;
        it.meta = new QLabel;

        v->addWidget(it.title);
        v->addWidget(it.meta);

        h->addWidget(it.cover);
        h->addLayout(v);

        ui.vLayTracks->addWidget(it.box);
        items.append(it);
    }
}

void PlaylistWindow::setPlaylist(const QList<QPair<QString, QString>>& tracks,
                                 const QString& name)
{
    allTracks = tracks;
    startIndex = 0;
    ui.lblPlaylistTitle->setText(name);
    updateView();
}

void PlaylistWindow::updateView()
{
    for (int i = 0; i < items.size(); ++i) {
        int idx = startIndex + i;
        if (idx < allTracks.size()) {
            items[i].box->setVisible(true);
            items[i].title->setText(allTracks[idx].first);
            items[i].meta->setText(allTracks[idx].second);
            items[i].cover->setPixmap(
                QPixmap(":/res/ui/icons/default.svg").scaled(48,48)
            );
        } else {
            items[i].box->setVisible(false);
        }
    }
}

void PlaylistWindow::scrollUp()
{
    if (startIndex > 0) {
        startIndex--;
        updateView();
    }
}

void PlaylistWindow::scrollDown()
{
    if (startIndex + 3 < allTracks.size()) {
        startIndex++;
        updateView();
    }
}
