#include "xraycontroller.h"

    void xraycontroller::start() {
        if (proc.state() == QProcess::Running) return;
        proc.start(xrayBinary, QStringList() << "-c" << configPath);
    }

    void xraycontroller::stop() {
        if (proc.state() == QProcess::Running) {
            proc.kill();
            proc.waitForFinished();
        }
    }

    void xraycontroller::setConfigPath(const QString &path) {
        configPath = path;
    }

    void xraycontroller::setBinaryPath(const QString &path) {xrayBinary = path;}
