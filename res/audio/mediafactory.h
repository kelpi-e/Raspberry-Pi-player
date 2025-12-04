#pragma once
#include "mp3strategy.h"
#include "youtubestrategy.h"

enum class MediaType {
    Mp3,
    Youtube
};

class MediaFactory {
public:
    static MediaStrategy* create(MediaType type) {
        switch (type) {
            case MediaType::Mp3:      return new Mp3Strategy();
            case MediaType::Youtube:  return new YoutubeStrategy();
        }
        return nullptr;
    }
};
