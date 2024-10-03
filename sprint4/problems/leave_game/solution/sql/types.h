#pragma once
#include "../src/tagged.h"

using namespace util;

enum class TYPE {
    SHOWRECORDS,
    SAVERECORD
};

struct Record {
    UUIDType id;
    std::string name;
    int score;
    size_t play_time;
};