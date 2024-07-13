#pragma once

#include <filesystem>
#include "http_server.h"
#include "model.h"
#include <boost/json.hpp>

namespace http_handler {
namespace beast = boost::beast;
namespace http = beast::http;
namespace json = boost::json;
// Запрос, тело которого представлено в виде строки
using StringRequest = http::request<http::string_body>;
// Ответ, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;
using namespace std::literals;
namespace sys = boost::system;
namespace fs = std::filesystem;

struct ContentType {
    ContentType() = delete;
    constexpr static std::string_view TEXT_HTML = "text/html"sv;
    constexpr static std::string_view JSON = "application/json"sv;
    constexpr static std::string_view PLAIN_TEXT = "text/plain"sv;
};

class RequestHandler {
public:
    explicit RequestHandler(model::Game& game, fs::path&& files_path)
        : game_{game}
        , files_path_(fs::weakly_canonical(std::move(files_path)))
    {
    }

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;


    
        // Создаёт StringResponse с заданными параметрами
    static StringResponse MakeStringResponse(http::status status, std::string_view body, unsigned http_version,
                                    bool keep_alive,
                                    std::string_view content_type = ContentType::JSON) {
        StringResponse response(status, http_version);
       
        response.set(http::field::content_type, content_type);
        response.body() = body;
        response.content_length(body.size());
        response.keep_alive(keep_alive);
        return response;
    }

    

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        
        // Обработать запрос request и отправить ответ, используя send
        if (req.method_string() == "GET") {
            const auto text_response = [&](http::status status, std::string_view text) {
                return MakeStringResponse(status, text, req.version(), req.keep_alive());
            };
            const auto text_response_with_type= [&](http::status status, std::string_view text, std::string_view con_type) {
                return MakeStringResponse(status, text, req.version(), req.keep_alive(), con_type);
            };
            std::string target = std::string(req.target());
            
            if (target == "/api/v1/maps") {
                json::array arr;
                for (const auto& gamemap : game_.GetMaps()) {
                    boost::json::object obj;
                    obj[boost::json::string {"id"}] = boost::json::string {*gamemap.GetId()};
                    obj[boost::json::string {"name"}] = gamemap.GetName();
                    arr.push_back(obj);
                }
                std::string res = json::serialize(arr);
                send(text_response(http::status::ok, res));
                return;
            }
            else if (target.starts_with ("/api/v1/maps/") && target.size() > 13) {
                std::string map_name {target.substr(13)};
                const auto ptr_map  = (game_.FindMap(model::Map::Id{map_name}));
                if (ptr_map != nullptr) {
                    boost::json::object val;
                    val["id"] = *(ptr_map->GetId());
                    val["name"] = ptr_map->GetName();
                    val["roads"] = json_loader::GetJsonRoads(*ptr_map);
                    val["buildings"] = json_loader::GetJsonBuildings(*ptr_map);
                    val["offices"] = json_loader::GetJsonOffices(*ptr_map);
                    send(text_response(http::status::ok, json::serialize(val)));
                    return;
                }
                else {
                    boost::json::object val;
                    val["code"] = "mapNotFound";
                    val["message"] = "Map not found";
                    send(text_response(http::status::not_found, json::serialize(val)));
                    return;
                }

            }
            else if (target.substr(0,4) != "/api") {
                auto temp = EncodeURL(target);
                if (temp.has_value() && !IsSubPath(temp.value())) {
                    send(text_response_with_type(http::status::bad_request, "Bad request", ContentType::PLAIN_TEXT));
                    return;
                }
                if (temp.has_value() && IsSubPath(temp.value())) {
                    using namespace http;
                    std::string req_url = temp.value();
                    response <file_body> res;
                    res.version(11);  // HTTP/1.1
                    res.result(status::ok);
                    auto type = TypeIdentity (req_url);
                    
                    res.insert(field::content_type, type);

                    file_body::value_type file;

                    if (sys::error_code ec; file.open(req_url.data(), beast::file_mode::read, ec), ec) {

                        send(text_response_with_type(http::status::not_found, "File NOT found", ContentType::PLAIN_TEXT));
                        return;
                    }

                    res.body() = std::move(file);
                    // Метод prepare_payload заполняет заголовки Content-Length и Transfer-Encoding
                    // в зависимости от свойств тела сообщения
                    res.prepare_payload();
                    send(res);
                    return;
                }
                
                
            }

            
            boost::json::object val;
            val["code"] = "badRequest";
            val["message"] = "Bad request";
            send(text_response(http::status::bad_request, json::serialize(val)));
            
        }

        
    }

private:
    model::Game& game_;
    fs::path files_path_;

    std::string TypeIdentity (const std::string_view in) const {
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
        else if (ft == ".js" ) { return "text/javascript"; }
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

    std::optional<std::string> EncodeURL (std::string_view in) const {
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

    bool IsSubPath(fs::path path) const {
        // Приводим пути к каноничному виду (без . и ..)
        path = fs::weakly_canonical(path);

        // Проверяем, что все компоненты files_path_ содержатся внутри path
        for (auto b = files_path_.begin(), p = path.begin(); b != files_path_.end(); ++b, ++p) {
            if (p == path.end() || *p != *b) {
                return false;
            }
        }
        return true;
    }

};




}  // namespace http_handler
