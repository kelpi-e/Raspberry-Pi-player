#include "godSays.h"


god::god(const std::string& path) : _path(path) {
    seed = std::chrono::steady_clock::now().time_since_epoch().count();
    a = const_A;
    c = const_C;
    state = seed;

    readFile();
}

void god::readFile() {
    QFile file(QString::fromStdString(_path));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "[WARNING] Cannot open dictionary file at" << QString::fromStdString(_path);
        return;
    }

    QTextStream in(&file);
    lines.clear();

    while (!in.atEnd()) {
        QString word = in.readLine().trimmed();
        if (!word.isEmpty()) {
            lines.push_back(word.toStdString());
        }
    }

    if (lines.empty()) {
        qWarning() << "[WARNING] Dictionary file is empty or contains no valid words.";
    } else {
        qInfo() << "[INFO] Loaded" << lines.size() << "words from dictionary.";
    }

    file.close();
}

unsigned int god::generate() {
    state = state * a + c;
    return state >> 16;
}

std::string god::speak() {
    if (lines.empty()) {
        return "[ERROR] Dictionary not loaded or empty!";
    }

    std::string words;
    for (int i = 0; i < 32; ++i) {
        words += lines[generate() % lines.size()];
        if (i < 31) words += " ";
    }

    return words;
}