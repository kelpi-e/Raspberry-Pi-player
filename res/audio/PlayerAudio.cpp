#include "PlayerAudio.h"

PlayerAudio::PlayerAudio(QObject *parent)
    : QObject(parent)
{
    out = new QAudioOutput(this);
    player = new QMediaPlayer(this);
    player->setAudioOutput(out);
}

void PlayerAudio::play(const QString &path) {
    player->setSource(QUrl::fromLocalFile(path));
    out->setVolume(VOLUME);
    player->setPosition(currentPosition);
    setCurrentlyPlaying(path);
    player->play();
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

void PlayerAudio::setCurrentlyPlaying(const QString &path) {currentlyPlaying = path;}

QString PlayerAudio::getCurrentlyPlaying() const {return currentlyPlaying;}



