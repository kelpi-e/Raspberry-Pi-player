#pragma once
#include <QProcess>
#include <QString>
#include "mediastategy.h"

class YoutubeStrategy final : public MediaStrategy {
public:
    QString resolve(const QString &url) override {
        QProcess p;

        QStringList args;
        args << "-f" << "bestaudio"    // только аудио
             << "-g"                  // получить прямой URL
             << "--no-check-certificate" // пропустить проверку SSL
             << "--no-warnings"           // убрать предупреждения
             << "--quiet"                 // минимальный вывод
             << url;

        p.start("yt-dlp", args);
        p.waitForFinished(-1);  // ждем завершения процесса

        QString streamUrl = QString(p.readAllStandardOutput()).trimmed();
        return streamUrl;
    }
};
