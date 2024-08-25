#pragma once
#include "http_server.h"
#include "model.h"

namespace http_handler {
namespace beast = boost::beast;
namespace http = beast::http;
namespace json = boost::json;
// Запрос, тело которого представлено в виде строки
using StringRequest = http::request<http::string_body>;
// Ответ, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;
using namespace std::literals;

struct ContentType {
    ContentType() = delete;
    constexpr static std::string_view TEXT_HTML = "text/html"sv;
    constexpr static std::string_view JSON = "application/json"sv;
};

class RequestHandler {
public:
    explicit RequestHandler(model::Game& game)
        : game_{game} {
    }

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;


    // Создаёт StringResponse с заданными параметрами
    StringResponse MakeStringResponse(http::status status, std::string_view body, unsigned http_version,
                                    bool keep_alive,
                                    std::string_view content_type = ContentType::JSON) {
        StringResponse response(status, http_version);
        response.set(http::field::content_type, std::string{content_type});
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
            std::string target = std::string(req.target());
            
            if (target == "/api/v1/maps") {
                json::array arr;
                for (const auto& gamemap : game_.GetMaps()) {
                    boost::json::object obj;
                    
                    obj[boost::json::string {"id"}] = boost::json::string {*gamemap.GetId()};
                    obj[boost::json::string {"name"}] = gamemap.GetName();
                    
                    //obj[*gamemap.GetId()] = gamemap.GetName();
                    arr.push_back(obj);
                }
                std::string res = json::serialize(arr);
                //send = text_response(http::status::ok, res );
                //boost::beast::bind_front_handler(send, text_response(http::status::ok, res));
                send(text_response(http::status::ok, res));

            }
            else if (target.starts_with ("/api/v1/maps/") && target.size() > 13) {
                std::string map_name {target.substr(13)};
                //auto iter_map = game_.GetMaps().find(map_name);
                const auto ptr_map  = (game_.FindMap(model::Map::Id{map_name}));
                if (ptr_map != nullptr) {
                    boost::json::object val;
                    val["id"] = *(ptr_map->GetId());
                    val["name"] = ptr_map->GetName();
                    val["roads"] = json_loader::GetJsonRoads(*ptr_map);
                    val["buildings"] = json_loader::GetJsonBuildings(*ptr_map);
                    val["offices"] = json_loader::GetJsonOffices(*ptr_map);
                    //boost::beast::bind_front_handler(send, text_response(http::status::ok, json::serialize(val)));
                    send(text_response(http::status::ok, json::serialize(val)));
                }
                else {
                    boost::json::object val;
                    val["code"] = "mapNotFound";
                    val["message"] = "Map not found";
                    //boost::beast::bind_front_handler(send, text_response(http::status::not_found, json::serialize(val)));
                    send(text_response(http::status::not_found, json::serialize(val)));
                }

            }
            else {
                boost::json::object val;
                val["code"] = "badRequest";
                val["message"] = "Bad request";
                //boost::beast::bind_front_handler(send, text_response(http::status::bad_request, json::serialize(val)));
                send(text_response(http::status::bad_request, json::serialize(val)));
            }
        }

        
    }

private:
    model::Game& game_;
};

}  // namespace http_handler
