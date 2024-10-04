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
    double play_time;
};