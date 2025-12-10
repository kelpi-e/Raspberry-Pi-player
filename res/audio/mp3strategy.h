#pragma once
#include <QUrl>

#include "mediastategy.h"

class Mp3Strategy final : public MediaStrategy {
public:
    QString resolve(const QString &path) override {
        return QUrl::fromLocalFile(path).toString();
    }
};
