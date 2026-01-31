#include "PlayerWindow.h"
#include <QMainWindow>
#include <QKeyEvent>
#include <QMediaMetaData>
#include <QTime>
#include <QWidget>

#include "../utils/godSays.h"


PlayerWindow::PlayerWindow(QWidget *parent, PlayerAudio *audio)
    : QWidget(parent), audio(audio)
{

    ui.setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    setFixedSize(WIDTH, HEIGHT);

    ui.lblWifi->setVisible(true);
    ui.lblWifi->setVisible(true);

    titleMarquee = new MarqueeController(ui.lblTitle, 200);
    artistMarquee = new MarqueeController(ui.lblArtist, 200);

    progressBar = ui.pb;
    timeLabel = ui.lblTime;
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        if (timeLabel)
            timeLabel->setText(QTime::currentTime().toString("HH:mm:ss"));
    });

    timer->start(1000);
    connect(audio->getPlayer(),
            &QMediaPlayer::mediaStatusChanged,
            this,
            [this, audio](const QMediaPlayer::MediaStatus s){
        if(s == QMediaPlayer::LoadedMedia || s == QMediaPlayer::BufferedMedia)
        {
            QString title = audio->getTitle(audio->getCurrentlyPlaying());
            if(title.isEmpty())
                title = "no title";
            titleMarquee->setText(title);

            QString artist = audio->getArtist(audio->getCurrentlyPlaying());
            if(artist.isEmpty())
                artist = "unknown";
            artist = "by " + artist;
            artistMarquee->setText(artist);
        }
    });


    connect(ui.btnPlay, &QPushButton::clicked, this, [this, audio]() {
        if (!this->audio) return;
        if (this->audio->isPlaying()) {
            this->audio->pause();
            ui.btnPlay->setIcon(QIcon(":/res/ui/icons/play.svg"));
        } else {
            this->audio->play(audio->getCurrentMediaType(),  audio->getCurrentlyPlaying());
            ui.btnPlay->setIcon(QIcon(":/res/ui/icons/pause.svg"));
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
    connect(ui.btnLoop, &QPushButton::clicked, this, [this]() {
        if (!isrepeat) {
            ui.btnLoop->setIcon(QIcon(":/res/ui/icons/repeatenabled.svg"));
            isrepeat = true;
        }
        else {
            ui.btnLoop->setIcon(QIcon(":/res/ui/icons/repeat.svg"));
            isrepeat = false;
        }
    });
    connect(ui.btnShuffle, &QPushButton::clicked, this, [this]() {
    if (!isrepeat) {
        ui.btnShuffle->setIcon(QIcon(":/res/ui/icons/shuffleenabled.svg"));
        isrepeat = true;
    }
    else {
        ui.btnShuffle->setIcon(QIcon(":/res/ui/icons/shuffle.svg"));
        isrepeat = false;
    }
    });



}
QString msToTime(const qint64 ms) {
    const qint64 totalSeconds = ms / 1000;
    const qint64 hours = totalSeconds / 3600;
    const qint64 minutes = (totalSeconds % 3600) / 60;
    const qint64 seconds = totalSeconds % 60;

    if (hours > 0)
        return QString("%1:%2:%3")
            .arg(hours, 1, 10, QLatin1Char('0'))
            .arg(minutes, 2, 10, QLatin1Char('0'))
            .arg(seconds, 2, 10, QLatin1Char('0'));
    return QString("%1:%2")
            .arg(minutes, 1, 10, QLatin1Char('0'))
            .arg(seconds, 2, 10, QLatin1Char('0'));
}
void PlayerWindow::updateProgressBar(qint64 pos) const {
    progressBar->setValue(pos);
    ui.lblCur->setText(msToTime(pos));
}
void PlayerWindow::updateProgressRange(const qint64 dur) const {
    progressBar->setRange(0, dur);
    ui.lblDur->setText(msToTime(dur));
}


void PlayerWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        emit requestClose();
        close();
    }
    else if (event->key() == Qt::Key_Space) {
        if (audio->isPlaying()) {
            audio->pause();
            ui.btnPlay->setIcon(QIcon(":/res/ui/icons/play.svg"));
        }
        else {
            audio->play(audio->getCurrentMediaType(), audio->getCurrentlyPlaying());
            ui.btnPlay->setIcon(QIcon(":/res/ui/icons/pause.svg"));
        }
    }
    else if (event->key() == Qt::Key_Left) {
        backwardRewind(5);
    }
    else if (event->key() == Qt::Key_Right) {
        forwardRewind(5);
    }
    else if (event->key() == Qt::Key_Up) {
        audio->setVolume(audio->getVolume() + 0.1);
    }
    else if (event->key() == Qt::Key_Down) {
        audio->setVolume(audio->getVolume() - 0.1);
    }
    else if (event->key() == Qt::Key_F7) {
        if (!msg) {
            god g(":/res/vocab.txt");
            msg = new FullscreenMessage(
                 QString::fromStdString(g.speak())
            );
            connect(msg, &QWidget::destroyed, this, [this]() {
                msg = nullptr;
            });
            msg->show();
        }
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
        auto img = cover.value<QImage>();
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

Ui::PlayerWindow PlayerWindow::getUI() { return ui; }

