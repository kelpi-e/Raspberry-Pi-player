#include "utils/wifichecker.h"
#include "utils/xraycontroller.h"
#include "ui/PlayerScene.h"
#include <QApplication>
#include "ui/PlaylistScene.h"
#include "pch.h"
#include <QResource>
int main(int argc, char *argv[]) {

    // Проверка существования файлов в файловой системе
    qDebug() << "Checking files in filesystem:";
    qDebug() << "play.svg exists:" << QFile::exists("res/ui/icons/play.svg");
    qDebug() << "menu.svg exists:" << QFile::exists("res/ui/icons/menu.svg");
    qDebug() << "vocab.txt exists:" << QFile::exists("res/vocab.txt");

    // Проверка ресурсов
    qDebug() << "\nChecking resources:";
    qDebug() << ":/res/ui/icons/play.svg:" << QResource(":/res/ui/icons/play.svg").isValid();
    qDebug() << ":/res/vocab.txt:" << QResource(":/res/vocab.txt").isValid();


    qreal rotation;
    if (argc <= 1) {
        qWarning() << "[WARNING] No rotation angle was passed using default 0";
        rotation = 0.0;
    }
    else if (argc <= 2) {
        bool ok = false;
        rotation = QString::fromLatin1(argv[1]).toDouble(&ok);
        if (!ok) {
            qFatal() << "[FATAL] Invalid rotation argument must be float or integer";
            return 1;
        }
    }
    else {
        qFatal() << "[FATAL] Too many arguments";
        return 1;
    }
    ROTATION = rotation;
    qputenv("QTWEBENGINE_DISABLE_SANDBOX", "1");
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--no-sandbox --disable-gpu");
    qputenv("QT_OPENGL", "software");

    QApplication app(argc, argv);

    if(Wifichecker::hasNetworkAccess()) {
        qInfo() << "[INFO] Network adapter found";
    }
    Wifichecker checker;
    checker.checkSiteAsync("https://spotify.com", [](const bool ok){
        if(ok) qInfo() << "[INFO] spotify available";
        else qInfo() << "[INFO] spotify NOT available";
    }, 5);
    checker.checkSiteAsync("https://youtube.com", [](const bool ok){
        if(ok) qInfo() << "[INFO] youtube available";
        else qInfo() << "[INFO] youtube NOT available";
    }, 5);

    xraycontroller xray;
    xray.start();

    PlayerScene scene(ROTATION);
    scene.show();


    return QApplication::exec();
}
