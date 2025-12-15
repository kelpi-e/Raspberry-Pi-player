#include "ui/PlayerWindow.h"
#include "audio/PlayerAudio.h"
#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsProxyWidget>
#include <QMainWindow>
#include "utils/wifichecker.h"

int main(int argc, char *argv[]) {

    qreal rotation;
    if (argc <= 1) {
        qWarning() << "No rotation angle was passed using default 0";
        rotation = 0.0;
    }
    else if (argc <= 2) {
        bool ok = false;
        rotation = QString::fromLatin1(argv[1]).toDouble(&ok);
        if (!ok) {
            qFatal() << "Invalid rotation argument must be float or integer";
            return 1;
        }
    }
    else {
        qFatal() << "Too many arguments";
        return 1;
    }

    QApplication app(argc, argv);

    if(Wifichecker::hasNetworkAccess())
        qInfo() << "[INFO] Network adapter found";

    Wifichecker checker;
    checker.checkSiteAsync("https://spotify.com", [](const bool ok){
        if(ok) qInfo() << "[INFO] spotify available";
        else qInfo() << "[INFO] spotify NOT available";
    }, 5);
    checker.checkSiteAsync("https://youtube.com", [](const bool ok){
        if(ok) qInfo() << "[INFO] youtube available";
        else qInfo() << "[INFO] youtube NOT available";
    }, 5);

    const QString fileName = QCoreApplication::applicationDirPath() + "/../res/music/2.mp3";
    const QString url = "https://youtu.be/mzfizVNCNbc?si=9kk4jxifKGRWeL9N";

    PlayerAudio audio;

    auto* player = new PlayerWindow(nullptr, &audio);
    player->getAudio()->setCurrentlyPlaying(fileName);
    player->getAudio()->setCurrentMediaType(MediaType::Mp3);

    // Сцена
    QGraphicsScene scene;
    QGraphicsProxyWidget* proxy = scene.addWidget(player);
    proxy->setRotation(rotation);  // Поворот

    // После поворота сцена подгоняется под размер прокси
    scene.setSceneRect(proxy->sceneBoundingRect());

    // Вид
    QGraphicsView view(&scene);
    view.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view.setAlignment(Qt::AlignCenter);

    // Окно без рамки
    QMainWindow win;
    win.setWindowFlags(Qt::FramelessWindowHint);
    win.setCentralWidget(&view);
    QObject::connect(player, &PlayerWindow::requestClose, &win, &QMainWindow::close);
    // Подгоняем размер окна под содержимое
    win.resize(proxy->sceneBoundingRect().size().toSize());
    win.show();

    return QApplication::exec();
}