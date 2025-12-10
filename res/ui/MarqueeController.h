#pragma once

#include <QLabel>
#include <QTimer>
#include <QString>

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
