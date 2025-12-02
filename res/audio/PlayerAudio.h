#pragma once
#include "../pch.h"

class PlayerAudio : public QObject {
    Q_OBJECT
public:
    explicit PlayerAudio(QObject *parent = nullptr);
    void play(const QString &path);
    void setCurrentPosition(qint64 position);
    void pause();
    void forwardRewind(qint64 dt) const;
    void backwardRewind(qint64 dt) const;
    [[nodiscard]] bool isPlaying() const;
    [[nodiscard]] qint64 position() const;
    [[nodiscard]] qint64 duration() const;

    QString getTitle(const QString &path);
    QString getArtist(const QString &path);


    void setCurrentlyPlaying(const QString &path);

    QString getCurrentlyPlaying() const;

    QMediaPlayer* getPlayer () const;

private:
    QMediaPlayer *player;
    QAudioOutput *out{};
    qint64 currentPosition = 0;
    QString currentlyPlaying = "";

};
