#pragma once
#include <QWidget>

class QLabel;

class FullscreenMessage : public QWidget
{
    Q_OBJECT
public:
    explicit FullscreenMessage(const QString& text, QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *e) override;

private:
    QLabel *lbl;
};
