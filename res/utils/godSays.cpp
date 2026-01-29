#include "godSays.h"


god::god(const std::string& path) : _path(path) {
    seed = __rdtsc();
    a = const_A;
    c = const_C;
    state = seed;

    readFile();
}

void god::readFile() {
    std::ifstream fi(_path);

    if (!fi.is_open()) {
        return;
    }

    std::string word;
    while (fi >> word) {
        lines.push_back(word);
    }

    if (lines.empty()) {
        std::cerr << "[WARNING] Dictionary file is empty or contains no valid words." << std::endl;
    } else {
        //std::cout << "[INFO] Loaded " << lines.size() << " words from dictionary." << std::endl;
    }

    fi.close();
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