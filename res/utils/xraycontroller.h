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
    QString xrayBinary = "xray";
    QString configPath = "~/.xray/config.json";
};
