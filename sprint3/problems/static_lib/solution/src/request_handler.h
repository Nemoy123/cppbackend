#pragma once

#include <filesystem>

#include "http_server.h"

#include "game.h"
#include "json_loader.h"
#include <sstream>

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
    

    std::string TypeIdentity (const std::string_view in) const;

    std::optional<std::string> EncodeURL (std::string_view in) const;

    bool IsSubPath(const fs::path& p) const;

    template <typename Request, typename Send>
    void MakeUnauthorizedErrorUnknownToken(Request&& req, Send&& send);

    template <typename Request, typename Send>
    void MakeUnauthorizedErrorInvalidToken(Request&& req, Send&& send);

    template <typename Request, typename Send>
    void MakeInvalidMethodError (Request&& req, Send&& send, std::string_view allow_methods);

    template <typename Request, typename Send>
    std::shared_ptr <Player> TryExtractToken(Request&& req, Send&& send);

    template <typename Request, typename Send>
    void MakeBadRequestError (Request&& req, Send&& send);

    template <typename Request, typename Send>
    void ChangeMove (Request&& req, Send&& send);
    
};

template <typename Request, typename Send>
void RequestHandler::MakeBadRequestError (Request&& req, Send&& send) {
    bool nocache = true;
    bool keep_alive = false;
    const auto text_response_nocache= [&](http::status status, std::string_view text, std::string_view allow_methods = {}) {
            return MakeStringResponse(status, text, req.version(), keep_alive, ContentType::JSON, nocache, allow_methods);
    };
    json::object obj;
    obj["code"] = "invalidArgument";
    obj["message"] = "Invalid content type";
    
    send(text_response_nocache(http::status::bad_request, json::serialize(std::move(obj)) ));
}

template <typename Request, typename Send>
void RequestHandler::MakeInvalidMethodError (Request&& req, Send&& send, std::string_view allow_methods) {
    bool nocache = true;
    bool keep_alive = false;
    const auto text_response_nocache= [&](http::status status, std::string_view text, std::string_view allow_methods = {}) {
            return MakeStringResponse(status, text, req.version(), keep_alive, ContentType::JSON, nocache, allow_methods);
    };
    using namespace std::literals;
    json::object obj;
    obj["code"] = "invalidMethod";
    std::string mess {"Only "s};
    mess += std::string{allow_methods} + " method is expected"s;
    obj["message"] = std::move(mess);
    send(text_response_nocache(http::status::method_not_allowed, json::serialize(obj), allow_methods));
}

template <typename Request, typename Send>
std::shared_ptr <Player> RequestHandler::TryExtractToken(Request&& req, Send&& send) { 
    auto tok = req.find(http::field::authorization);
    if (tok == req.end()) { 
        MakeUnauthorizedErrorInvalidToken(req, send);
        return nullptr;
    }
    std::string token = req.at(http::field::authorization);
    if (!token.starts_with ("Bearer ") || token.size() != 39) {
        MakeUnauthorizedErrorInvalidToken(req, send);
        return nullptr;
    }
    token = token.substr(7);
    auto ptr_player = game_.FindPlayer(token);
    if (ptr_player == nullptr) {
        MakeUnauthorizedErrorUnknownToken(req, send);
        return nullptr;
    }
    return ptr_player;
}

template <typename Request, typename Send>
void RequestHandler::MakeUnauthorizedErrorUnknownToken(Request&& req, Send&& send) {
    bool nocache = true;
    bool keep_alive = false;
    const auto text_response_nocache= [&](http::status status, std::string_view text, std::string_view allow_methods = {}) {
            return MakeStringResponse(status, text, req.version(), keep_alive, ContentType::JSON, nocache, allow_methods);
    };
    json::object obj;
    obj["code"] = "unknownToken";
    obj["message"] = "Player token has not been found";
    send(text_response_nocache(http::status::unauthorized, json::serialize(obj)));  
}

template <typename Request, typename Send>
void RequestHandler::MakeUnauthorizedErrorInvalidToken(Request&& req, Send&& send) {
    bool nocache = true;
    bool keep_alive = false;
    const auto text_response_nocache= [&](http::status status, std::string_view text, std::string_view allow_methods = {}) {
            return MakeStringResponse(status, text, req.version(), keep_alive, ContentType::JSON, nocache, allow_methods);
    };
    json::object obj;
    obj["code"] = "invalidToken";
    obj["message"] = "Authorization header is missing";
    send(text_response_nocache(http::status::unauthorized, json::serialize(obj)));
}

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
    const bool nocache = true;
    const bool keep_alive = false;
    const auto text_response_nocache= [&](http::status status, std::string_view text, std::string_view allow_methods = {}) {
            return MakeStringResponse(status, text, req.version(), keep_alive, ContentType::JSON, nocache, allow_methods);
    };
    const auto text_response= [&](http::status status, std::string_view text, std::string_view allow_methods = {}) {
            return MakeStringResponse(status, text, req.version(), req.keep_alive(), ContentType::JSON, false, allow_methods);
    };
    
   
        if (req.target() == "/api/v1/maps") {
            json::array arr;
            for (const auto& gamemap : game_.GetMaps()) {
                boost::json::object obj;
                obj[boost::json::string {"id"}] = boost::json::string {*gamemap.GetId()};
                obj[boost::json::string {"name"}] = gamemap.GetName();
                arr.push_back(obj);
            }
            std::string res = json::serialize(arr);
            send(text_response_nocache(http::status::ok, res));
            return;
        }
        else if (req.target().starts_with ("/api/v1/maps/") && req.target().size() > 13) {
            if (req.method_string() != "GET" && req.method_string() != "HEAD" ) {  
                MakeInvalidMethodError (req, send, "GET, HEAD");
                return;
            }
            std::string map_name {req.target().substr(13)};
            const auto ptr_map  = (game_.FindMap(model::Map::Id{map_name}));
            if (ptr_map != nullptr) {
                boost::json::object val;
                
                val["id"] = boost::json::string {*(ptr_map->GetId())};
                val["name"] = boost::json::string {ptr_map->GetName()};
                val["roads"] = json_loader::GetJsonRoads(*ptr_map);
                val["buildings"] = json_loader::GetJsonBuildings(*ptr_map);
                val["offices"] = json_loader::GetJsonOffices(*ptr_map);
                val["lootTypes"] = json_loader::GetLootTypes (*ptr_map);
                send(text_response_nocache(http::status::ok, json::serialize(val)));
                return;
            }
            else {
                boost::json::object val;
                val["code"] = "mapNotFound";
                val["message"] = "Map not found";
                send(text_response_nocache(http::status::not_found, json::serialize(val)));
                return;
            }

        }
        
        else if (req.target().starts_with ("/api/v1/game/join") ) {
            
            if (req.method_string() != "POST") { 
                    
                MakeInvalidMethodError (req, send, "POST");
                return;
            }      

            std::string user_name {};
            std::string mapid {};
            try {
                json::value user = json::parse(req.body());
                user_name = user.as_object().at ("userName").as_string();
                mapid = user.as_object().at ("mapId").as_string();
            } catch (...) {
                    json::object obj;
                obj["code"] = "invalidArgument";
                obj["message"] = "Join game request parse error";
                send(text_response_nocache(http::status::bad_request, json::serialize(obj)));
                return;
            }
            if (user_name == "") {

                json::object obj;
                obj["code"] = "invalidArgument";
                obj["message"] = "Invalid name";
                send(text_response_nocache(http::status::bad_request, json::serialize(obj)));
                return;
            }
            if (game_.FindMap(Map::Id{mapid}) == nullptr) {
                json::object obj;
                obj["code"] = "mapNotFound";
                obj["message"] = "Map not found";
                send(text_response_nocache(http::status::not_found, json::serialize(obj)));
                return;

            }
            detail::Token new_player = game_.AddPlayer (user_name, mapid);

            game_.GetSession (Map::Id{mapid})->AddLoot (1);

            boost::json::object obj;
            obj["authToken"] = new_player.token_string_;
            obj["playerId"] = new_player.id_dog_;
            send(text_response_nocache(http::status::ok, json::serialize(obj)));
            return;           

        }

        else if (req.target() == ("/api/v1/game/players"))  {
            if (req.method_string() != "GET" && req.method_string() != "HEAD") { 
                MakeInvalidMethodError (req,send,"GET, HEAD");
                return;
            } 
            auto ptr_player = TryExtractToken(req, send);
            if (ptr_player == nullptr) {return;};
            auto map_id = game_.FindPlayerMap (ptr_player);
            json::object obj;
            size_t count = 0;
            for (const auto& name : game_.FindPlayersOnMap (map_id)) {
                obj[std::to_string(count)] = name;
                ++count;
            }
            
            send(text_response_nocache(http::status::ok, json::serialize(obj)));
            return; 
        }
        
        else if (req.target().starts_with ("/api/v1/game/state") ) { 
            if (req.method_string() != "GET" && req.method_string() != "HEAD") { 
                MakeInvalidMethodError (req,send,"GET, HEAD");
                return;
            } 
            auto ptr_player = TryExtractToken(req, send);   
            if (ptr_player == nullptr) {return;};          
            auto map_id = game_.FindPlayerMap (ptr_player);
            bool need_id_dogs = false;
            using namespace boost::json;
            json::value obj;
            json::object id_players;

            for (const auto& id : game_.FindPlayersOnMap (map_id, need_id_dogs)) {
                auto ptr_dog = game_.GetSession(map_id)->GetDog(std::stol(id));
                std::vector <double> pos_speed {ptr_dog->GetPosition().x, ptr_dog->GetPosition().y};
                std::vector <double> vect_speed {ptr_dog->GetSpeed().x, ptr_dog->GetSpeed().y};
                json::value pos_val = value_from( pos_speed );
                json::value speed_val = value_from( vect_speed );
                json::object temp;
                temp[json::string_view{"pos"}] = pos_val;
                temp[json::string_view{"speed"}] = speed_val;
                temp[json::string_view{"dir"}] = ptr_dog->GetDirection();
                id_players[id] = std::move(temp);
            }
            json::object finish;
            json::value s = "players";
            finish[s.as_string()] = std::move(id_players);
            auto session_ptr = game_.GetSession(map_id);
            json::object loot_obj;
            for (auto i = 0; i < session_ptr->GetLootList().size(); ++i){
                json::object loot_date;
                loot_date["type"] = value_from( session_ptr->GetLootList().at(i).type );
                loot_date["pos"] = boost::json::array {session_ptr->GetLootList().at(i).x, session_ptr->GetLootList().at(i).y};

                loot_obj[std::to_string(i)] = loot_date;
            }
            finish["lostObjects"] = std::move(loot_obj);
                
            std::string temp2 = serialize(finish);
            send(text_response_nocache(http::status::ok, temp2));
        }
    ///api/v1/game/player/action
        else if (req.target().starts_with ("/api/v1/game/player/action") ) { 
            if (req.method_string() != "POST") { 
                MakeInvalidMethodError (req,send,"POST");
                return;
            } 
            auto ptr_player = TryExtractToken(req, send);
            if (ptr_player == nullptr) {return;}; 

            using namespace boost::json;
            using namespace http;
            boost::system::error_code ec;

            if (req.at (field::content_type) != "application/json") {
                MakeBadRequestError (req, send);
                return;
            }
            value mess;
            mess = parse (req.body(), ec);
            if( ec ) { MakeBadRequestError (req, send); return;}
            
            if (mess.as_object().find("move") != mess.as_object().cend()) {
                    std::string direction = mess.as_object().at("move").as_string().c_str();   
                    if (direction == "") {
                        ptr_player->GetDogPtr()->SetSpeed({0,0});
                        ptr_player->GetDogPtr()->SetDirection("");
                    }  

                    const double map_speed = game_.FindMap(game_.FindPlayerMap(ptr_player))->GetSpeed();
                    
                    if (direction == "L") {
                        ptr_player->GetDogPtr()->SetSpeed({ 0 - map_speed, 0 });
                        ptr_player->GetDogPtr()->SetDirection("L");
                    }
                    else if (direction == "R") {
                        ptr_player->GetDogPtr()->SetSpeed({ map_speed, 0 });
                        ptr_player->GetDogPtr()->SetDirection("R");
                    }
                    else if (direction == "U") {
                        ptr_player->GetDogPtr()->SetSpeed({ 0, 0 - map_speed });
                        ptr_player->GetDogPtr()->SetDirection("U");
                    }
                    else if (direction == "D") {
                        ptr_player->GetDogPtr()->SetSpeed({ 0, map_speed });
                        ptr_player->GetDogPtr()->SetDirection("D");
                    }
                    else {
                        MakeBadRequestError (req, send);
                        return;
                    }
                json::object finish;

                send(text_response_nocache(http::status::ok, serialize(finish))); 

            } else {
                MakeBadRequestError (req, send);
                return;
            }

        }  
        
        
        else if (req.target().starts_with ("/api/v1/game/tick") ) { 
            using namespace boost::json;
            using namespace http;
            if (req.method_string() != "POST") { 
                MakeInvalidMethodError (req,send,"POST");
                return;
            } 
            if (req.at (field::content_type) != "application/json") { MakeBadRequestError (req, send); return;}
            boost::system::error_code ec;
            value mess = parse (req.body(), ec);
            if( ec ) { MakeBadRequestError (req, send); return;}
            if (mess.as_object().find("timeDelta") == mess.as_object().cend()) { MakeBadRequestError (req, send); return;}
            uint64_t time = 0;
            if (mess.as_object().at("timeDelta").is_int64()) {
               time = value_to <uint64_t > (mess.as_object().at("timeDelta"));
            } else {
                MakeBadRequestError (req, send); return;
            }
                       
            game_.TimeUpdate (time);
            json::object finish;
            send(text_response_nocache(http::status::ok, serialize(finish))); 
        }

        else {
            json::object obj;
            obj["code"] = "badRequest";
            obj["message"] = "Invalid content type";
            
            send(text_response_nocache(http::status::bad_request, json::serialize(std::move(obj)) ));
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
                    
            boost::json::object val;
            val["code"] = "badRequest";
            val["message"] = "Bad request";
            send(text_response(http::status::bad_request, json::serialize(val)));
                     
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
