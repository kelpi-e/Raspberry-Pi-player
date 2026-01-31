#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <chrono>
#include <iostream>
#include <cstring>
#include <qlogging.h>

#define const_A 214013    // 0x343FD
#define const_C 2531011   // 0x269EC3

class god {
private:
    unsigned int seed, a, c, state;
    std::string _path;
    std::vector<std::string> lines;

    void readFile();
    unsigned int generate();

public:
    explicit god(const std::string& path);
    std::string speak();
};