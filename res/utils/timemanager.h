#include <QWidget>
#include <QTimer>
#include <QLabel>

class timeManager : public QWidget {
    Q_OBJECT
public:
    timeManager(QWidget *p = nullptr);

private:
    QLabel *lbl;
    QTimer *tmr;

private slots:
    void updt();
};
