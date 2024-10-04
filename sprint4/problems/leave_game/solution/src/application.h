#pragma once

#include "game.h"
#include <functional>
#include "serialize.h"

namespace app {

namespace beast = boost::beast;
namespace http = beast::http;
namespace json = boost::json;
namespace sys = boost::system;
namespace fs = std::filesystem;
namespace net = boost::asio;

using StringResponse = http::response<http::string_body>;
using StringRequest = http::request<http::string_body>;
using namespace std::literals;

struct ContentType {
    ContentType() = delete;
    constexpr static std::string_view TEXT_HTML = "text/html"sv;
    constexpr static std::string_view JSON = "application/json"sv;
    constexpr static std::string_view PLAIN_TEXT = "text/plain"sv;
};



class Application {
    public:
        Application(Game& game): game_(game) {}

        template <typename Request, typename Send>
        void AllMaps(Request&& req, Send&& send);

        template <typename Request, typename Send>
        void ThisMap(Request&& req, Send&& send); 

        template <typename Request, typename Send>
        void State(Request&& req, Send&& send);

        template <typename Request, typename Send>
        void Action(Request&& req, Send&& send);

        template <typename Request, typename Send>
        void Tick(Request&& req, Send&& send);

        template <typename Request, typename Send>
        void Players(Request&& req, Send&& send);

        template <typename Request, typename Send>
        void Join(Request&& req, Send&& send);

        template <typename Request, typename Send>
        void MakeBadRequestError (Request&& req, Send&& send);

        void OnTick(uint64_t time);

        template <typename Request, typename Send>
        void Records(Request&& req, Send&& send);
    
    private:
        Game& game_;
        std::vector <ApplicationListener*> list_observer_;
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

        template <typename Request>
        StringResponse TextResponseNocache (Request&& req, http::status status, std::string_view text, std::string_view allow_methods = {});
        
        template <typename Request, typename Send>
        std::shared_ptr <Player> TryExtractToken(Request&& req, Send&& send);

        template <typename Request, typename Send>
        void MakeUnauthorizedErrorInvalidToken(Request&& req, Send&& send);

        template <typename Request, typename Send>
        void MakeUnauthorizedErrorUnknownToken(Request&& req, Send&& send);

        template <typename Request, typename Send>
        void MakeInvalidMethodError (Request&& req, Send&& send, std::string_view allow_methods);

        template <typename Request, typename Send>
        std::optional<int> FindNumTarget (Request&& req, Send&& send, const std::string& req_stroke, const std::string& key);
};

template <typename Request, typename Send>
std::optional<int> Application::FindNumTarget (Request&& req, Send&& send, const std::string& req_stroke, const std::string& key)  {
    auto pos = req_stroke.find(key);
    int start = -1;
    std::string buff{};
    if (pos  != std::string::npos) {
        int i = 0;
        while ((pos + key.size() + i) < req_stroke.size() &&  std::isdigit(req_stroke.at(pos + key.size() + i))) {
            buff.push_back (req_stroke.at(pos + key.size() + i));
            ++i;
        }
        try {
            start = std::stoi (buff);
            return start;
        }
        catch (...) {
            MakeBadRequestError (req, send);
            return INT_MIN;
        }
       
    } 
    return std::nullopt;
    
}

template <typename Request, typename Send>
void Application::Records(Request&& req, Send&& send) {
    if (req.method_string() != "GET" ) {  
        MakeInvalidMethodError (req, send, "GET");
        return;
    }
    std::optional<int> start;
    std::optional<int> max_items;

    if (req.target().size() > 21) {
 
        auto params = req.target().substr (21);
        std::string key {"start="};
        start = FindNumTarget (req, send, params, key);
        if (start == INT_MIN) return;
        key = "maxItems="s;
        max_items = FindNumTarget (req, send, params, key);
        if (max_items == INT_MIN) return;
        if (max_items.has_value() && max_items > 100) {
            MakeBadRequestError (req, send);
            return;
        }
    }
    std::vector<Record> vect;
    {
        auto tranz = game_.GetUseCases()->ShowRecordsTranz();
        vect = tranz->GetRecords(start, max_items);
        tranz->Commit();
    }
    json::array arr;
    
    for (auto& rec : vect) {
        boost::json::object obj;
        obj["name"] = std::move(rec.name);
        obj["score"] = std::move(rec.score);
        obj["playTime"] = std::move(rec.play_time);
        arr.push_back(std::move(obj));
    }
    send(TextResponseNocache(req, http::status::ok, json::serialize(std::move(arr))));
    return;
}

void Application::OnTick(uint64_t time) {
    auto iter = list_observer_.begin();
    while (iter != list_observer_.end()) {
      (*iter)->OnTick(time);
      ++iter;
    }
}

template <typename Request, typename Send>
void Application::MakeInvalidMethodError (Request&& req, Send&& send, std::string_view allow_methods) {
    
    using namespace std::literals;
    json::object obj;
    obj["code"] = "invalidMethod";
    std::string mess {"Only "s};
    mess += std::string{allow_methods} + " method is expected"s;
    obj["message"] = std::move(mess);
    
    send(TextResponseNocache(req, http::status::method_not_allowed, json::serialize(obj), allow_methods));
}

template <typename Request, typename Send>
void Application::AllMaps(Request&& req, Send&& send) {
    
    if (req.target() == "/api/v1/maps") {
        json::array arr;
        for (const auto& gamemap : game_.GetMaps()) {
            boost::json::object obj;
            obj[boost::json::string {"id"}] = boost::json::string {*gamemap.GetId()};
            obj[boost::json::string {"name"}] = gamemap.GetName();
            arr.push_back(obj);
        }
        std::string res = json::serialize(arr);
       
        send(TextResponseNocache(req, http::status::ok, res));
        return;
    }
}

template <typename Request>
StringResponse Application::TextResponseNocache (Request&& req, http::status status, std::string_view text, std::string_view allow_methods) {
    bool nocache = true;
    bool keep_alive = false;
    return MakeStringResponse(status, text, req.version(), keep_alive, ContentType::JSON, nocache, allow_methods);
}

template <typename Request, typename Send>
void Application::ThisMap(Request&& req, Send&& send) {
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
        std::string temp {json::serialize(val)};
        
        send(TextResponseNocache(req, http::status::ok, std::move(temp)));
        return;
    }
    else {
        boost::json::object val;
        val["code"] = "mapNotFound";
        val["message"] = "Map not found";
        send(TextResponseNocache(req, http::status::not_found, json::serialize(val)));
        return;
    }
}

template <typename Request, typename Send>
void Application::State(Request&& req, Send&& send) {
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
        temp[json::string_view{"score"}] = ptr_dog->GetScore();
        json::array bag;
        for (const auto& loot : ptr_dog->GetLootBag()) {
            if (loot != nullptr) {
                json::object loot_item;
                //loot_item[json::string_view{std::to_string(loot->id)}]=loot->type;
                loot_item[json::string_view{util::UUIDToString(loot->id)}]=loot->type;
                bag.push_back (std::move(loot_item));
            }
        }
        temp[json::string_view{"bag"}] = std::move(bag);
        id_players[id] = std::move(temp);
    }
    json::object finish;
    json::value s = "players";
    finish[s.as_string()] = std::move(id_players);
    auto session_ptr = game_.GetSession(map_id);
    json::object loot_obj;
    for (auto i = 0; i < session_ptr->GetLootList().size(); ++i){
        auto iter = std::next(session_ptr->GetLootList().begin(),i);
        if (iter->second != nullptr) {
            json::object loot_date;
            loot_date["type"] = value_from( iter->second->type );
            loot_date["pos"] = boost::json::array {iter->second->x, iter->second->y};

            loot_obj[std::to_string(i)] = loot_date;
        }
    }
    finish["lostObjects"] = std::move(loot_obj);
        
    std::string temp2 = serialize(finish);
    send(TextResponseNocache(req, http::status::ok, temp2));
}

template <typename Request, typename Send>
std::shared_ptr <Player> Application::TryExtractToken(Request&& req, Send&& send) { 
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
void Application::MakeUnauthorizedErrorInvalidToken(Request&& req, Send&& send) {
    json::object obj;
    obj["code"] = "invalidToken";
    obj["message"] = "Authorization header is missing";
    send(TextResponseNocache(req, http::status::unauthorized, json::serialize(obj)));
}

template <typename Request, typename Send>
void Application::MakeUnauthorizedErrorUnknownToken(Request&& req, Send&& send) {
    json::object obj;
    obj["code"] = "unknownToken";
    obj["message"] = "Player token has not been found";
    send(TextResponseNocache(req, http::status::unauthorized, json::serialize(obj)));
}

template <typename Request, typename Send>
void Application::Action(Request&& req, Send&& send) {
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

        send(TextResponseNocache(req, http::status::ok, serialize(finish))); 

    } else {
        MakeBadRequestError (req, send);
        return;
    }
}

template <typename Request, typename Send>
void Application::MakeBadRequestError (Request&& req, Send&& send) {
    json::object obj;
    obj["code"] = "invalidArgument";
    obj["message"] = "Invalid content type";
    send(TextResponseNocache(req, http::status::bad_request, json::serialize(std::move(obj)))); 
}

template <typename Request, typename Send>
void Application::Tick(Request&& req, Send&& send) {
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
    if (serialization::Serialize::getInstance().GetMode() == serialization::Serialize::Mode::BY_TIME) {
        serialization::Serialize::getInstance().OnTick(time);
    }
    OnTick(time);            
    game_.TimeUpdate (time);
    
    json::object finish;
    
    send(TextResponseNocache(req, http::status::ok, serialize(finish))); 

}

template <typename Request, typename Send>
void Application::Players(Request&& req, Send&& send) {
    
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
    
    //send(text_response_nocache(http::status::ok, json::serialize(obj)));
    send(TextResponseNocache(req, http::status::ok, json::serialize(obj)));
    return; 
}

template <typename Request, typename Send>
void Application::Join(Request&& req, Send&& send) {
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
        //send(text_response_nocache(http::status::bad_request, json::serialize(obj)));
        send(TextResponseNocache(req, http::status::bad_request, json::serialize(obj)));
        return;
    }
    if (user_name == "") {

        json::object obj;
        obj["code"] = "invalidArgument";
        obj["message"] = "Invalid name";
        //send(text_response_nocache(http::status::bad_request, json::serialize(obj)));
        send(TextResponseNocache(req, http::status::bad_request, json::serialize(obj)));
        return;
    }
    if (game_.FindMap(Map::Id{mapid}) == nullptr) {
        json::object obj;
        obj["code"] = "mapNotFound";
        obj["message"] = "Map not found";
        //send(text_response_nocache(http::status::not_found, json::serialize(obj)));
        send(TextResponseNocache(req, http::status::not_found, json::serialize(obj)));
        return;

    }
    detail::Token new_player = game_.AddPlayer (user_name, mapid);

    game_.GetSession (Map::Id{mapid})->AddLoot (1);

    boost::json::object obj;
    obj["authToken"] = new_player.token_string_;
    obj["playerId"] = new_player.id_dog_;
    //send(text_response_nocache(http::status::ok, json::serialize(obj)));
    send(TextResponseNocache(req, http::status::ok, json::serialize(obj)));
    return;        
}

} // end namespace