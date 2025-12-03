#include "ui/PlayerWindow.h"
#include "audio/PlayerAudio.h"
#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsProxyWidget>
#include <QMainWindow>

int main(int argc, char *argv[]) {
    //setlocale(LC_ALL, "en_US.UTF-8");
    QApplication app(argc, argv);

    const QString fileName = QCoreApplication::applicationDirPath() + "/../res/music/1.mp3";
    PlayerAudio audio;

    // Create the PlayerWindow (assuming it's a QWidget-derived class)
    PlayerWindow* playerWidget = new PlayerWindow(nullptr, &audio);
    playerWidget->setCurrentFile(fileName);

    // Embed into QGraphicsScene with proxy
    QGraphicsScene scene;
    QGraphicsProxyWidget* proxy = scene.addWidget(playerWidget);
    proxy->setRotation(90);  // Rotate by 90 degrees (clockwise); use -90 for counterclockwise

    // Create a view to display the scene
    QGraphicsView view(&scene);
    view.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Optional: Adjust view to center and fit the rotated content
    view.setAlignment(Qt::AlignCenter);
    view.ensureVisible(proxy);  // Ensure the proxy is visible
    // If needed, resize the view based on original widget size (swap width/height after rotation)
    QSize originalSize = playerWidget->sizeHint();  // Or use fixed size if set
    view.resize(originalSize.height(), originalSize.width());  // Swap for 90-degree rotation

    // Use a QMainWindow to host the view (or just show the view directly if no menus/toolbars needed)
    QMainWindow mainWindow;
    mainWindow.setCentralWidget(&view);
    mainWindow.resize(view.size());  // Match window to view size
    mainWindow.show();

    return QApplication::exec();
}