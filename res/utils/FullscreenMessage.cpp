#include "FullscreenMessage.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QKeyEvent>

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
