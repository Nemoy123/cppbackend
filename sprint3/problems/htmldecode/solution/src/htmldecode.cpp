#include "htmldecode.h"
#include <sstream>
#include <map>

/*
 * Декодирует основные HTML-мнемоники:
 * - &lt - <
 * - &gt - >
 * - &amp - &
 * - &pos - '
 * - &quot - "
 *
 * Мнемоника может быть записана целиком либо строчными, либо заглавными буквами:
 * - &lt и &LT декодируются как <
 * - &Lt и &lT не мнемоники
 *
 * После мнемоники может стоять опциональный символ ;
 * - M&amp;M&APOSs декодируется в M&M's
 * - &amp;lt; декодируется в &lt;
 */

std::string HtmlDecode(std::string_view str) {
    std::stringstream out;
    std::map <std::string, std::string> mnem; // ключ мнемоники
    mnem["lt"] = "<";
    mnem["LT"] = "<";
    mnem["gt"] = ">";
    mnem["GT"] = ">";
    mnem["amp"] = "&";
    mnem["AMP"] = "&";
    mnem["pos"] = "'";
    mnem["POS"] = "'";
    mnem["quot"] = "\"";
    mnem["QUOT"] = "\"";
    bool found = false;
    bool test_end = false;
    std::string stroke {}; 
    for (const auto& ch : str) {
        if (!found && test_end){
            test_end = false;
            if (ch == ';') { continue; }
            else { out << ch; }
        }
        else if (ch != '&' && !found) {out << ch;}
        else if (ch == '&' && !found){
            found = true;

        }
        else if (found) {
            stroke += ch;
            if (stroke.size() > 4) {
                out << "&" << stroke; 
                stroke.clear(); 
                found = false;
            }
            if (mnem.contains(stroke)) {
                out << mnem.at(stroke);
                stroke.clear();
                found = false;
                test_end = true;
            }
        }
        
    }
    if (!stroke.empty()) {
        out << stroke;
    }
    std::string temp_debug {out.str()};
    return out.str();
}
