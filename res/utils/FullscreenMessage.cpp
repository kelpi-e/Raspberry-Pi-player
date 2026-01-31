#include "FullscreenMessage.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <qpainter.h>

#include "../pch.h"
FullscreenMessage::FullscreenMessage(const QString& text, QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setFocusPolicy(Qt::StrongFocus);

    QLabel *lbl = new QLabel(text, this);
    lbl->setWordWrap(true);
    lbl->setAlignment(Qt::AlignCenter);
    lbl->setStyleSheet("color:white; font-size:18px;");

    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->addWidget(lbl);
    lay->setContentsMargins(10, 10, 10, 10);

    setStyleSheet("background:black;");

    showFullScreen();
}

void FullscreenMessage::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape)
        close();
}

void FullscreenMessage::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.translate(width()/2, height()/2);
    p.rotate(ROTATION);
    p.translate(-width()/2, -height()/2);
    QWidget::paintEvent(event);
}