#pragma once

#include <QString>
#include <QWidget>

class ThemeLoader {
public:
    static bool applyPlaylistWindow(QWidget* w);
    static bool applyPlayerWindow(QWidget* w);
private:
    static QString findThemeJsonPath();
};
