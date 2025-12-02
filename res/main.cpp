#include "ui/PlayerWindow.h"
#include "audio/PlayerAudio.h"
int main(int argc, char *argv[]) {
    //setlocale(LC_ALL, "en_US.UTF-8");
    QApplication app(argc, argv);
    const QString fileName = QCoreApplication::applicationDirPath() + "/../res/music/1.mp3";

    PlayerAudio audio;
    PlayerWindow window(nullptr, &audio);

    window.setCurrentFile(fileName);
    window.show();

    return QApplication::exec();
}
