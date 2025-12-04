#pragma once
#include <QProcess>
#include "mediastategy.h"

class YoutubeStrategy : public MediaStrategy {
public:
    QString resolve(const QString &url) override {
        QProcess p;
        p.start("yt-dlp", {"-f", "bestaudio", "-g", url});
        p.waitForFinished(-1);
        return QString(p.readAllStandardOutput()).trimmed();
    }
};
