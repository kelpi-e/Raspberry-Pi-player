#pragma once
#include "mp3strategy.h"
#include "youtubestrategy.h"

enum class MediaType {
    Mp3,
    Youtube,
    Spotify
};

class MediaFactory {
public:
    static MediaStrategy* create(const MediaType type) {
        switch (type) {
            case MediaType::Mp3: return new Mp3Strategy();
            case MediaType::Youtube: return new YoutubeStrategy();
            default: ;
        }
        return nullptr;
    }
};
