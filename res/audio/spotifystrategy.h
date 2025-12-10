#pragma once
#include <QProcess>
#include <QString>
#include "mediastategy.h"

class SpotifyStrategy final : public MediaStrategy {
public:
    QString resolve(const QString &url) override {
        return "";
    }
};
