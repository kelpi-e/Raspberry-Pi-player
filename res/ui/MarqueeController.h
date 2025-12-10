#pragma once
#include "../pch.h"
#include <QObject>
#include <QLabel>
#include <QTimer>
#include <QString>
#include <QFontMetrics>

class MarqueeController : public QObject {
    Q_OBJECT
public:
    explicit MarqueeController(QLabel* label, int interval = 150);
    void setText(const QString& t);

private slots:
    void tick();

private:
    QLabel* lbl;
    QString fullText;
    int pos = 0;
    QTimer timer;
};
