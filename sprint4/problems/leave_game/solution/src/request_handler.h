#pragma once

#include <filesystem>

#include "http_server.h"

#include "game.h"
#include "json_loader.h"
#include <sstream>
#include "application.h"

//#include "model.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace json = boost::json;
namespace sys = boost::system;
namespace fs = std::filesystem;
namespace net = boost::asio;

//using namespace json_map_loader;

namespace http_handler {


// Запрос, тело которого представлено в виде строки
using StringRequest = http::request<http::string_body>;
// Ответ, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;
using namespace std::literals;
using namespace app;

struct ContentType {
    ContentType() = delete;
    constexpr static std::string_view TEXT_HTML = "text/html"sv;
    constexpr static std::string_view JSON = "application/json"sv;
    constexpr static std::string_view PLAIN_TEXT = "text/plain"sv;
};

class RequestHandler : public std::enable_shared_from_this<RequestHandler> {
public:
    using Strand = net::strand<net::io_context::executor_type>;

    explicit RequestHandler(Game& game, const fs::path& files_path, Strand& api_strand)
        : game_{game}
        , files_path_(fs::weakly_canonical(files_path))
        , api_strand_ (api_strand)
        , application(game)
    {
    }

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;


    
        // Создаёт StringResponse с заданными параметрами
    static StringResponse MakeStringResponse(http::status status, std::string_view body, unsigned http_version,
                                    bool keep_alive,
                                    std::string_view content_type = ContentType::JSON, bool nocache = false, std::string_view allow_method = {}) {
        
        StringResponse response(status, http_version);
       
        response.set(http::field::content_type, content_type);
        response.body() = body;
        response.content_length(body.size());
        if (keep_alive) { response.keep_alive(keep_alive); }
        if (nocache)    { response.insert (http::field::cache_control,"no-cache");}
        if (allow_method.size() > 0) {response.insert (http::field::allow, allow_method);}

        return response;
    }

    template <typename FuncMakeResponse, typename FuncSend>
    void FileHandler (std::string_view target, FuncMakeResponse& funcmakestring, FuncSend& send);
    template <typename Request, typename Send>
    void ApiHandler (Request&& req, Send&& send);

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send);


private:
    Game& game_ ;
    fs::path files_path_;
    Strand api_strand_;
    Application application;

    std::string TypeIdentity (const std::string_view in) const;

    std::optional<std::string> EncodeURL (std::string_view in) const;

    bool IsSubPath(const fs::path& p) const;

    template <typename Request, typename Send>
    void ChangeMove (Request&& req, Send&& send);
    
};


template <typename FuncMakeResponse, typename FuncSend>
void RequestHandler::FileHandler (std::string_view target, FuncMakeResponse& funcmakestring, FuncSend& send) {
    auto temp = EncodeURL(target);
    if (!temp.has_value()) {
        send(funcmakestring(http::status::bad_request, "Bad request", ContentType::PLAIN_TEXT));
        return;
    }
    fs::path n_path = files_path_;
    n_path += temp.value();
    bool sub_path = IsSubPath(n_path);
    if (!sub_path) {
        send(funcmakestring(http::status::bad_request, "Bad request", ContentType::PLAIN_TEXT));
        return;
    }
    if (temp.value().back() == '/') {
        n_path += std::string {"index.html"};
    }
    using namespace http;
    std::string req_url = n_path;
    response <file_body> res;
    res.version(11);  // HTTP/1.1
    res.result(status::ok);
    auto type = TypeIdentity (req_url);
    res.insert(field::content_type, type);
    file_body::value_type file;
    if (sys::error_code ec; file.open(req_url.data(), beast::file_mode::read, ec), ec) {

        send(funcmakestring(http::status::not_found, "File NOT found", ContentType::PLAIN_TEXT));
        return;
    }
    res.body() = std::move(file);
    // Метод prepare_payload заполняет заголовки Content-Length и Transfer-Encoding
    // в зависимости от свойств тела сообщения
    res.prepare_payload();
    send(res);
}


template <typename Request, typename Send>
void RequestHandler::ApiHandler (Request&& req, Send&& send) {
        
        if (req.target() == "/api/v1/maps") {
            application.AllMaps(req, send);
            return;
        }
        else if (req.target().starts_with ("/api/v1/maps/") && req.target().size() > 13) {
            application.ThisMap(req, send);
            return;
        }
        
        else if (req.target().starts_with ("/api/v1/game/join") ) {
            application.Join (req, send);
            return;           
        }

        else if (req.target() == ("/api/v1/game/players"))  {
            application.Players(req, send);
            return;
        }
        
        else if (req.target() == ("/api/v1/game/records"))  {
            application.Records(req, send);
            return;
        }

        else if (req.target().starts_with ("/api/v1/game/state") ) { 
            application.State(req, send);
            return;
        }
    
        else if (req.target().starts_with ("/api/v1/game/player/action") ) { 
            application.Action(req, send);
            return;
        }  
                
        else if (req.target().starts_with ("/api/v1/game/tick") ) { 
            application.Tick (req, send);
            return;
        }

        else {
            application.MakeBadRequestError (req, send);
            return;
        }
}




template <typename Body, typename Allocator, typename Send>
void RequestHandler::operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
            const auto text_response = [&](http::status status, std::string_view text) {
                return MakeStringResponse(status, text, req.version(), req.keep_alive());
            };
            const auto text_response_with_type= [&](http::status status, std::string_view text, std::string_view con_type) {
                return MakeStringResponse(status, text, req.version(), req.keep_alive(), con_type);
            };
            std::string target = std::string(req.target());
        
            // Обработать запрос request и отправить ответ, используя send
            if (req.method_string() == "GET") {
                                    
                if (target.substr(0,4) != "/api") {
                    FileHandler (target, text_response_with_type, send);
                    return;
                }
            } 
            if (target.substr(0,4) == "/api") {
                auto handle = [self = shared_from_this(), send,
                               req = std::forward<decltype(req)>(req)] {
                    try {
                        // Этот assert не выстрелит, так как лямбда-функция будет выполняться внутри strand
                        assert(self->api_strand_.running_in_this_thread());
                        self->ApiHandler (req, send); 
                        return;
                    } catch (...) {
                        std::cout << "Strand Error"<< std::endl;
                        
                    }
                };
                net::dispatch(api_strand_, handle);
                return;
                   
            }

            application.MakeBadRequestError (req, send);        
}



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






}  // namespace http_handler
