#include "wifichecker.h"
#include <QNetworkInformation>
#include <QNetworkReply>
#include <QTimer>

Wifichecker::Wifichecker(QObject *parent)
    : QObject(parent)
{}

bool Wifichecker::hasNetworkAccess() {
    const auto *n = QNetworkInformation::instance();
    return n && n->reachability() == QNetworkInformation::Reachability::Online;
}



void Wifichecker::checkSiteAsync(const QString &url_string, const std::function<void(bool)>& callback, int timeout = 5) {
    timeout *= 1000;
    const QUrl url = QUrl::fromUserInput(url_string);
    if(!url.isValid()) {
        callback(false);
        return;
    }

    auto *manager = new QNetworkAccessManager(this);
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply *reply = manager->head(request);

    // Таймаут 5 секунд
    auto *timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [reply, callback]() {
        reply->abort();         // прерываем запрос
        callback(false);        // считаем недоступным
    });
    timer->start(timeout); // 5 секунд

    connect(reply, &QNetworkReply::finished, this, [reply, callback, timer]() {
        if(timer->isActive())
            timer->stop();
        timer->deleteLater();

        const bool ok = reply->error() == QNetworkReply::NoError;
        reply->deleteLater();
        callback(ok);
    });
}

