#pragma once
#include "MarqueeController.h"
#include "../audio/PlayerAudio.h"
#include "ui_PlayerWindow.h"
#include "../utils/timemanager.h"

class PlayerWindow : public QWidget {
    Q_OBJECT
    signals:
    void requestClose();

public:
    explicit PlayerWindow(QWidget *parent = nullptr, PlayerAudio *audio = nullptr);
    void forwardRewind(qint64 dt);
    void backwardRewind(qint64 dt);

    void updateCover();

    PlayerAudio *getAudio();

    void updateProgressBar (qint64 pos) const;

    void updateProgressRange(qint64 dur) const;

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    Ui::PlayerWindow ui{};
    QProgressBar *progressBar{};
    QLabel *timeLabel{};
    QTimer *timer{};
    PlayerAudio *audio{};
    MarqueeController* titleMarquee;
    MarqueeController* artistMarquee;
    bool isrepeat = false;
};
