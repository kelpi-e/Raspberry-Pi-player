#include "PlayerWindow.h"
#include "../utils/ThemeLoader.h"
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
    ThemeLoader::applyPlayerWindow(this);
    setWindowFlags(Qt::FramelessWindowHint);
    setMinimumSize(240, 320);
    resize(WIDTH, HEIGHT); // начальный размер, но окно адаптивное

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
            if(title.isEmpty()) title = "no title";
            titleMarquee->setText(title);

            QString artist = audio->getArtist(audio->getCurrentlyPlaying());
            if(artist.isEmpty()) artist = "unknown";
            artistMarquee->setText("by " + artist);
        }
    });

    connect(ui.btnPlay, &QPushButton::clicked, this, [this, audio]() {
        if (!this->audio) return;
        if (this->audio->isPlaying()) {
            this->audio->pause();
            ui.btnPlay->setIcon(QIcon(":/res/ui/icons/play.svg"));
        } else {
            this->audio->play(audio->getCurrentMediaType(), audio->getCurrentlyPlaying());
            ui.btnPlay->setIcon(QIcon(":/res/ui/icons/pause.svg"));
        }
    });

    connect(ui.btnPrev, &QPushButton::clicked, this, [this]() { backwardRewind(5); });
    connect(ui.btnNext, &QPushButton::clicked, this, [this]() { forwardRewind(5); });

    connect(audio->getPlayer(), &QMediaPlayer::positionChanged, this, &PlayerWindow::updateProgressBar);
    connect(audio->getPlayer(), &QMediaPlayer::durationChanged, this, &PlayerWindow::updateProgressRange);
    connect(audio->getPlayer(), &QMediaPlayer::metaDataChanged, this, &PlayerWindow::updateCover);

    connect(ui.btnLoop, &QPushButton::clicked, this, [this]() {
        isrepeat = !isrepeat;
        ui.btnLoop->setIcon(QIcon(isrepeat ? ":/res/ui/icons/repeatenabled.svg" : ":/res/ui/icons/repeat.svg"));
    });

    connect(ui.btnShuffle, &QPushButton::clicked, this, [this]() {
        isrepeat = !isrepeat;
        ui.btnShuffle->setIcon(QIcon(isrepeat ? ":/res/ui/icons/shuffleenabled.svg" : ":/res/ui/icons/shuffle.svg"));
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
    if (!audio) return;

    switch (event->key()) {
        case Qt::Key_Escape: emit requestClose(); close(); break;
        case Qt::Key_Space:
            if (audio->isPlaying()) { audio->pause(); ui.btnPlay->setIcon(QIcon(":/res/ui/icons/play.svg")); }
            else { audio->play(audio->getCurrentMediaType(), audio->getCurrentlyPlaying()); ui.btnPlay->setIcon(QIcon(":/res/ui/icons/pause.svg")); }
            break;
        case Qt::Key_Left: backwardRewind(5); break;
        case Qt::Key_Right: forwardRewind(5); break;
        case Qt::Key_Up: audio->setVolume(audio->getVolume() + 0.1); break;
        case Qt::Key_Down: audio->setVolume(audio->getVolume() - 0.1); break;
        case Qt::Key_F7:
            if (!msg) {
                god g(":/res/vocab.txt");
                msg = new FullscreenMessage(QString::fromStdString(g.speak()));
                connect(msg, &QWidget::destroyed, this, [this]() { msg = nullptr; });
                msg->show();
            }
            break;
    }
}

void PlayerWindow::forwardRewind(const qint64 dt) {
    const qint64 value = progressBar->value();
    const qint64 maximum = progressBar->maximum();
    progressBar->setValue(qMin(value + dt, maximum));
    audio->forwardRewind(dt);
}

void PlayerWindow::backwardRewind(const qint64 dt) {
    const qint64 value = progressBar->value();
    const qint64 minimum = progressBar->minimum();
    progressBar->setValue(qMax(value - dt, minimum));
    audio->backwardRewind(dt);
}

void PlayerWindow::updateCover()
{
    const QMediaMetaData md = audio->getPlayer()->metaData();
    QImage img;

    if (!md.value(QMediaMetaData::ThumbnailImage).isNull())
        img = md.value(QMediaMetaData::ThumbnailImage).value<QImage>();
    else if (!md.value(QMediaMetaData::CoverArtImage).isNull())
        img = md.value(QMediaMetaData::CoverArtImage).value<QImage>();
    else
        img = QImage(":/res/ui/icons/default.svg");

    if (!img.isNull()) {
        QPixmap pix = QPixmap::fromImage(img).scaled(
            ui.lblCover->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        );
        ui.lblCover->setPixmap(pix);
    } else {
        ui.lblCover->setPixmap(QPixmap());
        ui.lblCover->setText("Нет обложки");
    }

    ui.lblCover->setAlignment(Qt::AlignCenter);
}

PlayerAudio* PlayerWindow::getAudio() { return audio; }
Ui::PlayerWindow PlayerWindow::getUI() { return ui; }

void PlayerWindow::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    int w = this->width();
    int h = this->height();

    // ---- Квадратная обложка ----
    int maxW = ui.lblTitle->width(); // ограничение по ширине названия
    int usedH = ui.hLayStatus->sizeHint().height()
              + ui.lblTitle->sizeHint().height()
              + ui.lblArtist->sizeHint().height()
              + ui.hLayProgress->sizeHint().height()
              + ui.hLayButtons->sizeHint().height()
              + 8*5; // суммарные spacing и отступы

    int maxH = qMax(0, h - usedH);
    int side = qMin(maxW, maxH);

    ui.lblCover->setFixedWidth(side);
    ui.lblCover->setFixedHeight(side);
    updateCover();

    // ---- Масштабирование шрифтов ----
    // Базовое окно: WIDTH=240, HEIGHT=320
    float scaleFactor = h / 320.0f;

    QFont titleFont = ui.lblTitle->font();
    titleFont.setPointSizeF(14 * scaleFactor);
    ui.lblTitle->setFont(titleFont);

    QFont artistFont = ui.lblArtist->font();
    artistFont.setPointSizeF(12 * scaleFactor);
    ui.lblArtist->setFont(artistFont);

    QFont timeFont = ui.lblTime->font();
    timeFont.setPointSizeF(12 * scaleFactor);
    ui.lblTime->setFont(timeFont);

    QFont batteryFont = ui.lblBattery->font();
    batteryFont.setPointSizeF(12 * scaleFactor);
    ui.lblBattery->setFont(batteryFont);

    // Кнопки можно масштабировать, если нужно
    QList<QPushButton*> buttons = { ui.btnPlay, ui.btnPrev, ui.btnNext, ui.btnLoop, ui.btnShuffle, ui.btnMenu };
    for (auto btn : buttons) {
        QFont f = btn->font();
        f.setPointSizeF(9 * scaleFactor); // базовый размер 9
        btn->setFont(f);
    }
}