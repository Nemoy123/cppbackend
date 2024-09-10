#include "request_handler.h"

namespace http_handler {

std::optional<std::string> RequestHandler::EncodeURL (std::string_view in) const {
        std::string result{};
        std::string sym {};
        bool start = false;
        for (const auto& ch : in) {
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
                return std::nullopt;
            }
        }
        if (!sym.empty() && sym.size() == 2) {
            if (sym.size() != 2) return std::nullopt;
            int dec = std::stoi (sym,0,16);
            result += char (dec);
        }
        return result;
}

bool RequestHandler::IsSubPath(const fs::path& p) const {
    // Приводим пути к каноничному виду (без . и ..)
    fs::path path = fs::weakly_canonical(p);

    // Проверяем, что все компоненты files_path_ содержатся внутри path
    for (auto b = files_path_.begin(), p = path.begin(); b != files_path_.end(); ++b, ++p) {
        if (p == path.end() || *p != *b) {
            return false;
        }
    }
    return true;
}

std::string RequestHandler::TypeIdentity (const std::string_view in) const {
        auto dot = in.find_last_of('.');
        if (dot == std::string::npos) { return "application/octet-stream"; }
        std::string ft {};
        for (const auto& ch:in.substr(dot)) {
            ft += std::tolower(ch);
        }
        if (ft == ".htm" || ft == ".html") { return "text/html";}
        else if (ft == ".css") { return "text/css";}
        else if (ft == ".txt") { return "text/plain";}
        else if (ft == ".js" ) { return "text/javascript";}
        else if (ft == ".json" ) { return "application/json"; }
        else if (ft == ".xml" ) { return "application/xml"; }
        else if (ft == ".png" ) { return "image/png";}
        else if (ft == ".jpg" || ft == ".jpe"  || ft == ".jpeg" ) { return "image/jpeg";}
        else if (ft == ".gif" ) {  return "image/gif";}
        else if (ft == ".bmp" ) {  return "image/bmp";}
        else if (ft == ".ico" ) {  return "image/vnd.microsoft.icon";}
        else if (ft == ".tiff" || ft == ".tif" ) {  return "image/tiff";}
        else if (ft == ".svg" || ft == ".svgz") {  return "image/svg+xml";}
        else if (ft == ".mp3") {  return "audio/mpeg";}
        else { return "application/octet-stream";  }
}

StringResponse RequestHandler::MakeStringResponse(http::status status, std::string_view body, unsigned http_version,
                                    bool keep_alive,
                                    std::string_view content_type, bool nocache, std::string_view allow_method) {
        
        StringResponse response(status, http_version);
       
        response.set(http::field::content_type, content_type);
        response.body() = body;
        response.content_length(body.size());
        if (keep_alive) { response.keep_alive(keep_alive); }
        if (nocache)    { response.insert (http::field::cache_control,"no-cache");}
        if (allow_method.size() > 0) {response.insert (http::field::allow, allow_method);}

        return response;
}

} //end namespace