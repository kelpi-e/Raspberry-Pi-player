#include "ThemeLoader.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

QString ThemeLoader::findThemeJsonPath()
{
    QDir dir(QCoreApplication::applicationDirPath());
    dir.cdUp();
    dir.cdUp();
    return dir.absoluteFilePath("theme_colors.json");
}

bool ThemeLoader::applyPlaylistWindow(QWidget* w)
{
    QFile f(findThemeJsonPath());
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    const auto doc = QJsonDocument::fromJson(f.readAll());
    f.close();
    if (!doc.isObject()) return false;
    const QJsonObject root = doc.object();
    const QJsonObject cpp = root.value("cpp").toObject();
    const QJsonObject pw = cpp.value("playlist_window").toObject();
    if (pw.isEmpty()) return false;

    const auto s = [&pw](const char* key, const char* def) {
        return pw.value(QString::fromUtf8(key)).toString(QString::fromUtf8(def));
    };
    QString ss;
    ss += "QWidget { background-color: " + s("bg_primary", "#121212") + "; color: " + s("color", "#ffffff") + "; font-family: Arial; font-size: 12px; }\n";
    ss += "QLabel { background: transparent; color: " + s("label_color", "#eaeaea") + "; }\n";
    ss += "QLabel.trackTitle { font-size: 12px; font-weight: bold; }\n";
    ss += "QLabel.trackMeta { font-size: 10px; color: " + s("track_meta_color", "#b3b3b3") + "; }\n";
    ss += "QLabel.cover { background-color: " + s("cover_bg", "#1e1e1e") + "; border: 1px solid " + s("cover_border", "#2a2a2a") + "; border-radius: 6px; }\n";
    ss += "QPushButton { background-color: " + s("button_bg", "#1e1e1e") + "; border: 1px solid " + s("button_border", "#333") + "; border-radius: 4px; color: " + s("color", "#ffffff") + "; }\n";
    ss += "QPushButton:pressed { background-color: " + s("button_pressed_bg", "#333") + "; }\n";
    ss += "#btnBack { font-size: 16px; font-weight: bold; min-width: 36px; max-width: 36px; min-height: 28px; max-height: 28px; }\n";
    ss += "#lblPlaylistTitle { font-size: 14px; font-weight: bold; }\n";
    ss += "#btnUp, #btnDown { min-height: 28px; max-height: 28px; font-size: 14px; }\n";
    w->setStyleSheet(ss);
    return true;
}

bool ThemeLoader::applyPlayerWindow(QWidget* w)
{
    QFile f(findThemeJsonPath());
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    const auto doc = QJsonDocument::fromJson(f.readAll());
    f.close();
    if (!doc.isObject()) return false;
    const QJsonObject root = doc.object();
    const QJsonObject cpp = root.value("cpp").toObject();
    const QJsonObject pw = cpp.value("player_window").toObject();
    if (pw.isEmpty()) return false;

    const auto s = [&pw](const char* key, const char* def) {
        return pw.value(QString::fromUtf8(key)).toString(QString::fromUtf8(def));
    };
    QString ss;
    ss += "QWidget#PlayerWindow { background-color: " + s("bg_primary", "#121212") + "; border-radius: 10px; }\n";
    ss += "QLabel { color: " + s("color", "#ffffff") + "; background-color: transparent; }\n";
    ss += "QLabel#lblCover { background-color: " + s("cover_bg", "#1e1e1e") + "; border-radius: 8px; border: 2px solid " + s("cover_border", "#2a2a2a") + "; }\n";
    ss += "QLabel#lblTitle { font-size: 14px; font-weight: 600; }\n";
    ss += "QLabel#lblArtist { color: " + s("artist_color", "#b3b3b3") + "; font-size: 12px; }\n";
    ss += "QProgressBar { background-color: " + s("progress_bg", "#2a2a2a") + "; border: none; border-radius: 4px; height: 6px; }\n";
    ss += "QProgressBar::chunk { background-color: " + s("progress_chunk", "#1db954") + "; border-radius: 4px; }\n";
    ss += "QPushButton { background-color: " + s("button_bg", "#1e1e1e") + "; color: " + s("color", "#ffffff") + "; border: 1px solid " + s("button_border", "#2a2a2a") + "; border-radius: 6px; padding: 3px 4px; font-size: 9px; min-width: 22px; min-height: 20px; }\n";
    ss += "QPushButton#btnPlay { background-color: " + s("btn_play_bg", "#1db954") + "; border-color: " + s("btn_play_border", "#1db954") + "; font-weight: 600; min-width: 32px; min-height: 22px; }\n";
    ss += "QLabel#lblTime { font-size: 12px; color: " + s("time_color", "#ffffff") + "; }\n";
    ss += "QLabel#lblBattery { font-size: 12px; color: " + s("battery_color", "#ffffff") + "; }\n";
    w->setStyleSheet(ss);
    return true;
}
