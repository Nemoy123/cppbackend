#include "token.h"


uint64_t PlayerTokens::GenerateFunction () 
{
    uint64_t temp = 0;
    std::random_device rd;
    std::uniform_int_distribution<int> dist(1, 9);
    for (auto i =0; i < 100; ++i) {
            if (i % 2 == 0) {
                temp += generator1_() * 17 * dist(rd);
                temp -= generator2_() / 13 * dist(rd);
            } else {
                temp += generator1_() * 23 * dist(rd);
                temp -= generator2_() / 29 * dist(rd);
            }
    }
    return temp;
}

std::string PlayerTokens::GenerateToken ()  {
    uint64_t number1 = GenerateFunction ();
    uint64_t number2 = GenerateFunction ();
    std::stringstream stream;
    stream << std::hex << number1 << number2;
    std::string result( stream.str() );
    
    return result;
}