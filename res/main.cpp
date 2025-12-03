#include "ui/PlayerWindow.h"
#include "audio/PlayerAudio.h"
#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsProxyWidget>
#include <QMainWindow>
void hideCursor() {
    std::printf("\033[?25l");
    std::fflush(stdout);
}
int main(int argc, char *argv[]) {

    hideCursor();
    QApplication app(argc, argv);

    const QString fileName =
        QCoreApplication::applicationDirPath() + "/../res/music/1.mp3";

    PlayerAudio audio;

    // Создаём PlayerWindow
    auto* player = new PlayerWindow(nullptr, &audio);
    player->setCurrentFile(fileName);

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

    //audio.play(fileName);

    return QApplication::exec();
}
