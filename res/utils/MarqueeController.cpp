#include "MarqueeController.h"

MarqueeController::MarqueeController(QLabel* label, const int interval)
    : QObject(label), lbl(label)
{
    timer.setInterval(interval);
    connect(&timer, &QTimer::timeout, this, &MarqueeController::tick);
    timer.start();
}

void MarqueeController::setText(const QString& t)
{
    fullText = t;
    pos = 0;
}

void MarqueeController::tick()
{
    if (!lbl) return;

    QFontMetrics fm(lbl->font());
    if (fm.horizontalAdvance(fullText) <= lbl->width()) {
        lbl->setText(fullText);
        return;
    }

    QString view = fullText.mid(pos) + "   " + fullText.left(pos);
    lbl->setText(view);
    pos = (pos + 1) % fullText.size();
}
