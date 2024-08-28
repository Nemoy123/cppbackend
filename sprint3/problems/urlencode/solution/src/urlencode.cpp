#include "urlencode.h"
#include <sstream>

std::string UrlEncode(std::string_view str) {
    std::stringstream out;
   
    for (const auto& ch :str) {
        if (ch == ' ') {
            out << "+";
        }
        // !#$&  '()*+ ,/:;=?@[]
        else if (ch < 32 || ch > 127 || ch == '!' || ch == '#'|| ch == '$'|| ch == '&'
                || ch == 39 || ch == '('|| ch == ')'|| ch == '*'|| ch == '+' || ch == ','
                || ch == '/'|| ch == ':'|| ch == ';'|| ch == '='|| ch == '?'|| ch == '@' || ch == '['|| ch == ']') {
            out << "%" <<std::hex << static_cast<int>(ch);
        }
        else {
            out << ch;
        }
    }
    std::string temp {out.str()};
    return out.str();
}


// std::optional<std::string> RequestHandler::EncodeURL (std::string_view in) const {
//         std::string result{};
//         std::string sym {};
//         bool start = false;
//         for (const auto& ch : in) {
//             if (!start && ch != '%') {
//                 result += ch;
//             }
//             else if (!start && ch == '%') {
//                 start = true;
//             }
//             else if (start) {
//                 sym += ch;
//                 if (sym.size () == 2) {
//                     //  шестнадцатеричное в строке преобразовать в десятичное
//                     int dec = std::stoi (sym,0,16);
//                     result += char (dec);
//                     sym.clear();
//                     start = false;
//                 }
//             }
//             else {
//                 return std::nullopt;
//             }
//         }
//         if (!sym.empty() && sym.size() == 2) {
//             if (sym.size() != 2) return std::nullopt;
//             int dec = std::stoi (sym,0,16);
//             result += char (dec);
//         }
//         return result;
// }