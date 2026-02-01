#include "FullscreenMessage.h"
#include <QPainter>
#include <QKeyEvent>

FullscreenMessage::FullscreenMessage(const QString& text, QWidget *parent)
    : QWidget(parent), msg(text)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setFocusPolicy(Qt::StrongFocus);
    setStyleSheet("background:black;");
    showFullScreen();
}

void FullscreenMessage::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::white);
    p.setFont(QFont("Sans", 18));

    p.translate(width() / 2, height() / 2);
    p.rotate(ROTATION);
    p.translate(-height() / 2, -width() / 2);

    QRect r(0, 0, height(), width());
    p.drawText(r, Qt::AlignCenter | Qt::TextWordWrap, msg);
}

void FullscreenMessage::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape)
        close();
}
