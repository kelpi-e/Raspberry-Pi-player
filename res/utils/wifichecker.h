#pragma once
#include <QObject>
#include <QUrl>
#include <functional>

class Wifichecker : public QObject {
    Q_OBJECT
public:
    explicit Wifichecker(QObject *parent = nullptr);

    static bool hasNetworkAccess();

    void checkSiteAsync(const QString &url_string, const std::function<void(bool)> &callback, int timeout);
};
