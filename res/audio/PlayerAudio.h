#pragma once
#include "mediafactory.h"
#include "../pch.h"
#include <QMediaPlayer>
class PlayerAudio : public QObject {
    Q_OBJECT
public:
    explicit PlayerAudio(QObject *parent = nullptr);

    void play(MediaType type, const QString &input);


    void setCurrentPosition(qint64 position);
    void pause();
    void forwardRewind(qint64 dt) const;
    void backwardRewind(qint64 dt) const;
    [[nodiscard]] bool isPlaying() const;
    [[nodiscard]] qint64 position() const;
    [[nodiscard]] qint64 duration() const;

    QString getTitle(const QString &path);
    QString getArtist(const QString &path);


    void setCurrentlyPlaying(const QString &str);

    QString getCurrentlyPlaying() const;

    void setCurrentMediaType(MediaType type);

    MediaType getCurrentMediaType();

    QMediaPlayer* getPlayer () const;

private:
    QMediaPlayer *player;
    QAudioOutput *out{};
    qint64 currentPosition = 0;
    QString currentlyPlaying = "";
    MediaType currentMediaType{};
};
