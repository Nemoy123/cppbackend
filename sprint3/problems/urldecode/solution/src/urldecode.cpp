#include "urldecode.h"

#include <charconv>
#include <stdexcept>

std::string UrlDecode(std::string_view str) {
    
        std::string result{};
        std::string sym {};
        bool start = false;
        for (const auto& ch : str) {
            if (!start && ch != '%') {
                result += ch;
            }
            else if (!start && ch == '%') {
                start = true;
            }
            else if (start) {
                sym += ch;
                if (sym.size () == 2) {
                    //  шестнадцатеричное в строке преобразовать в десятичное
                    int dec = std::stoi (sym,0,16);
                    result += char (dec);
                    sym.clear();
                    start = false;
                }
            }
            else {
                throw std::invalid_argument("invalid input");
            }
        }
        if (!sym.empty() && sym.size() == 2) {
            if (sym.size() != 2) throw std::invalid_argument("invalid input");
            int dec = std::stoi (sym,0,16);
            result += char (dec);
        }
        return result;

}
