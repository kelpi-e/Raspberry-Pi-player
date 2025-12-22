#pragma once
#include <QProcess>
#include <QString>

class xraycontroller {
public:
    void start();
    void stop();
    void setConfigPath(const QString &path);
    void setBinaryPath(const QString &path);
private:
    QProcess proc;
    QString xrayBinary = "/home/pathetic/Downloads/Xray-core/xray";
    QString configPath = "/home/pathetic/.xray/config.json";
};
