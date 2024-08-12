#pragma once

#include <random>
//#include <tagged.h>
#include <sstream>
#include <ios>
#include "tagged.h"
#include <memory>

namespace detail {
struct Token {
    Token (const uint64_t& dog, const std::string& token_string)
        : id_dog_(dog)
        , token_string_(token_string)
    {}
    uint64_t id_dog_{};
    std::string token_string_{};
    bool operator== (const Token& b) const  {
        return id_dog_ == b.id_dog_ && token_string_==b.token_string_;
    }
};
}  // namespace detail




class PlayerTokens {
    public:

    std::string GenerateToken ();
    
private:
    std::random_device random_device_;
    std::mt19937_64 generator1_{[this] {
        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
        return dist(random_device_);
    }()};
    std::mt19937_64 generator2_{[this] {
        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
        return dist(random_device_);
    }()};
    // Чтобы сгенерировать токен, получите из generator1_ и generator2_
    // два 64-разрядных числа и, переведя их в hex-строки, склейте в одну.
    // Вы можете поэкспериментировать с алгоритмом генерирования токенов,
    // чтобы сделать их подбор ещё более затруднительным
    uint64_t GenerateFunction ();
}; 

