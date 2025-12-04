#pragma once
#include <QString>

class MediaStrategy {
public:
    virtual ~MediaStrategy() {}
    virtual QString resolve(const QString &input) = 0;
};
