#pragma once
#include <QWidget>
#include "../pch.h"

class FullscreenMessage : public QWidget {
    Q_OBJECT
public:
    explicit FullscreenMessage(const QString& text, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent*) override;
    void keyPressEvent(QKeyEvent*) override;

private:
    QString msg;
};
