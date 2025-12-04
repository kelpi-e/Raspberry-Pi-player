#include "PlayerAudio.h"
#include <QProcess>

#include "mediastategy.h"

PlayerAudio::PlayerAudio(QObject *parent)
    : QObject(parent)
{
    out = new QAudioOutput(this);
    player = new QMediaPlayer(this);
    player->setAudioOutput(out);
}


void PlayerAudio::play(MediaType type, const QString &input)
{
    qDebug() << "[DEBUG] Play called with type:" << static_cast<int>(type) << ", input:" << input;

    std::unique_ptr<MediaStrategy> s(MediaFactory::create(type));
    if (!s) {
        qDebug() << "[ERROR] Failed to create MediaStrategy!";
        return;
    }

    qDebug() << "[DEBUG] MediaStrategy created:" << typeid(*s).name();

    const QString url = s->resolve(input);
    qDebug() << "[DEBUG] Resolved URL:" << url;

    if (url.isEmpty()) {
        qDebug() << "[ERROR] Resolved URL is empty!";
        return;
    }

    player->setSource(QUrl(url));
    qDebug() << "[DEBUG] QMediaPlayer source set";

    out->setVolume(VOLUME);
    player->setPosition(currentPosition);
    qDebug() << "[DEBUG] Volume set to" << VOLUME << ", position set to" << currentPosition;

    setCurrentlyPlaying(input);
    player->play();
    qDebug() << "[DEBUG] Player started";
}

void PlayerAudio::setCurrentPosition(const qint64 position) { currentPosition = position;}

void PlayerAudio::pause() {
    setCurrentPosition(player->position());
    player->pause();
}

void PlayerAudio::forwardRewind(const qint64 dt) const {
    const qint64 duration = player->duration();
    const qint64 position = player->position();
    player->setPosition(position + dt * 1000 <= duration ? position + dt * 1000 : duration);
}

void PlayerAudio::backwardRewind(const qint64 dt) const {
    const qint64 position = player->position();
    player->setPosition(position - dt * 1000 >= 0 ? position - dt * 1000 : 0);
}
bool PlayerAudio::isPlaying() const {
    return player->playbackState() == QMediaPlayer::PlayingState;
}
qint64 PlayerAudio::position() const {
    return player->position();
}

qint64 PlayerAudio::duration() const {
    return player->duration();
}


QString PlayerAudio::getTitle(const QString &path) {
    TagLib::FileRef f(path.toUtf8().constData());
    if(!f.isNull() && f.tag()) {
        return QString::fromStdWString(f.tag()->title().toWString());
    }
    return QFileInfo(path).baseName();
}

QString PlayerAudio::getArtist(const QString &path) {
    TagLib::FileRef f(path.toUtf8().constData());
    if(!f.isNull() && f.tag()) {
        return QString::fromStdWString(f.tag()->artist().toWString());
    }
    return "unknown";
}


QMediaPlayer* PlayerAudio::getPlayer() const { return player; }

void PlayerAudio::setCurrentlyPlaying (const QString &str) {
    currentlyPlaying = str;
}

QString PlayerAudio::getCurrentlyPlaying() const {
    return currentlyPlaying;
}

void PlayerAudio::setCurrentMediaType(const MediaType type) {
    currentMediaType = type;
}

MediaType PlayerAudio::getCurrentMediaType () {
    return currentMediaType;
}
