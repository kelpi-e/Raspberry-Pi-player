#include "ui/PlayerWindow.h"
#include "audio/PlayerAudio.h"
#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsProxyWidget>
#include <QMainWindow>

int main(int argc, char *argv[]) {

    QApplication app(argc, argv);

    const QString fileName = QCoreApplication::applicationDirPath() + "/../res/music/2.mp3";
    const QString url = "https://youtu.be/mzfizVNCNbc?si=9kk4jxifKGRWeL9N";

    PlayerAudio audio;

    // Создаём PlayerWindow
    auto* player = new PlayerWindow(nullptr, &audio);
    player->getAudio()->setCurrentlyPlaying(url);
    player->getAudio()->setCurrentMediaType(MediaType::Youtube);

    // Сцена
    QGraphicsScene scene;
    QGraphicsProxyWidget* proxy = scene.addWidget(player);
    proxy->setRotation(90);  // Поворот

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