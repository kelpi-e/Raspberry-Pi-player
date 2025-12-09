#include "PlayerWindow.h"

#include <QMainWindow>

PlayerWindow::PlayerWindow(QWidget *parent, PlayerAudio *audio)
    : QWidget(parent), audio(audio)
{
    ui.setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    setFixedSize(WIDTH, HEIGHT);

    progressBar = ui.pb;

    connect(audio->getPlayer(),
            &QMediaPlayer::mediaStatusChanged,
            this,
            [this, audio](QMediaPlayer::MediaStatus s){
        if(s == QMediaPlayer::LoadedMedia || s == QMediaPlayer::BufferedMedia)
        {
            QString title = audio->getTitle(audio->getCurrentlyPlaying());
            if(title.isEmpty())
                title = "no title";
            ui.lblTitle->setText(title);

            QString artist = audio->getArtist(audio->getCurrentlyPlaying());
            if(artist.isEmpty())
                artist = "unknown";
            artist = "by " + artist;
            ui.lblArtist->setText(artist);
        }
    });


    connect(ui.btnPlay, &QPushButton::clicked, this, [this, audio]() {
        if (!this->audio) return;
        if (this->audio->isPlaying()) {
            this->audio->pause();
            ui.btnPlay->setText("Play");
        } else {
            this->audio->play(audio->getCurrentMediaType(),  audio->getCurrentlyPlaying());
            ui.btnPlay->setText("Pause");
        }
    });

    connect(ui.btnPrev, &QPushButton::clicked, this, [this]() {
        backwardRewind(5);
    });

    connect(ui.btnNext, &QPushButton::clicked, this, [this]() {
        forwardRewind(5);
    });
    connect(audio->getPlayer(), &QMediaPlayer::positionChanged,
            this, &PlayerWindow::updateProgressBar);

    connect(audio->getPlayer(), &QMediaPlayer::durationChanged,
            this, &PlayerWindow::updateProgressRange);
    connect(audio->getPlayer(), &QMediaPlayer::metaDataChanged,
        this, &PlayerWindow::updateCover);



}
void PlayerWindow::updateProgressBar(qint64 pos) {
    progressBar->setValue(pos);
}
void PlayerWindow::updateProgressRange(qint64 dur) {
    progressBar->setRange(0, dur);
}


void PlayerWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        emit requestClose();
        close();
    }
}

void PlayerWindow::forwardRewind(const qint64 dt) {
    const qint64 value = progressBar->value();
    const qint64 maximum = progressBar->maximum();
    progressBar->setValue(value + dt < maximum ? value + dt : maximum);
    audio->forwardRewind(dt);
}

void PlayerWindow::backwardRewind(const qint64 dt) {
    const qint64 value = progressBar->value();
    const qint64 minimum = progressBar->minimum();
    progressBar->setValue(value - dt  < minimum ? minimum : value - dt);
    audio->backwardRewind(dt);
}
void PlayerWindow::updateCover() {
    const QMediaMetaData md = audio->getPlayer()->metaData();

    QVariant thumb = md.value(QMediaMetaData::ThumbnailImage);
    if (!thumb.isNull()) {
        auto img = thumb.value<QImage>();
        ui.lblCover->setPixmap(QPixmap::fromImage(img).scaled(
            180, 180, Qt::KeepAspectRatio, Qt::SmoothTransformation
        ));
        return;
    }

    QVariant cover = md.value(QMediaMetaData::CoverArtImage);
    if (!cover.isNull()) {
        QImage img = cover.value<QImage>();
        ui.lblCover->setPixmap(QPixmap::fromImage(img).scaled(
            180, 180, Qt::KeepAspectRatio, Qt::SmoothTransformation
        ));
        return;
    }

    ui.lblCover->setPixmap(QPixmap());
    ui.lblCover->setText("Нет обложки");
}

PlayerAudio* PlayerWindow::getAudio() {
    return audio;
}
