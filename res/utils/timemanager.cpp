#include "timemanager.h"
#include <QTime>

timeManager::timeManager(QWidget *p) : QWidget(p) {
    lbl = new QLabel("00:00:00", this);

    tmr = new QTimer(this);
    connect(tmr, &QTimer::timeout, this, &timeManager::updt);
    tmr->start(100);

    update();
}

void timeManager::updt() {
    lbl->setText(QTime::currentTime().toString("HH:mm:ss"));
}