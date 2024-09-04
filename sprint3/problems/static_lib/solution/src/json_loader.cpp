#include "json_loader.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include <boost/json/src.hpp>

namespace json_loader {
    namespace json = boost::json;
    using namespace std::literals;
    using namespace model;

class ParseError : public std::exception
{
public:
    explicit ParseError(const char* message) : msg(message) {}
    ParseError(ParseError const&) noexcept = default;

    ParseError& operator=(ParseError const&) noexcept = default;
    ~ParseError() override = default;

    const char* what() const noexcept override { return msg; }
private:
  const char* msg;
};

std::vector<Road> ParseRoads (const json::array& arr) {
    std::vector<Road> result;
    result.reserve(arr.size());
    for (const auto& road : arr) {
        int x0_coordinate = 0;
        int y0_coordinate = 0;
        int x1_coordinate = 0;
        int y1_coordinate = 0;

        x0_coordinate = road.as_object().at("x0").as_int64();
        y0_coordinate = road.as_object().at("y0").as_int64();
        if (road.as_object().find("y1") != road.as_object().end()) {
            y1_coordinate = road.as_object().at("y1").as_int64();
            result.emplace_back (Road (Road::VERTICAL, Point {x0_coordinate, y0_coordinate},  y1_coordinate));
        } else {
            x1_coordinate = road.as_object().at("x1").as_int64();
            result.emplace_back (Road (Road::HORIZONTAL, Point {x0_coordinate, y0_coordinate},  x1_coordinate));
        }
    }
    return result;
}

std::vector<Building> ParseBuildings (const json::array& arr) { 
    std::vector<Building> result;
    result.reserve(arr.size());
    for (const auto& building : arr) { 
        int x = 0;
        int y = 0;
        int h = 0;
        int w = 0;
        for (const auto& [key, val] : building.as_object()) {
            if (key == "x") { x = val.as_int64();} 
            if (key == "y") { y = val.as_int64();}
            if (key == "h") { h = val.as_int64();}
            if (key == "w") { w = val.as_int64();}
        }
        result.emplace_back ( Building ( Rectangle { Point{x,y}, Size{w,h}} ));
    }
    return result;
}
//
std::vector<Office> ParseOffices (const json::array& arr) {  
    std::vector<Office> result;
    result.reserve(arr.size());
    for (const auto& off : arr) {  
        std::string id {};
        int x = 0, y = 0, offsetx = 0, offsety = 0;
        for (const auto& [key, val] : off.as_object()) {
            if (key == "x") { x = val.as_int64();} 
            if (key == "y") { y = val.as_int64();}
            if (key == "offsetX") { 
                offsetx = val.as_int64();
                }
            if (key == "offsetY") { offsety = val.as_int64();}
            if (key == "id") { id = val.as_string();} 
        }

        result.emplace_back ( Office (Office::Id(id), Point{x, y}, Offset{offsetx, offsety}) );
    }

    return result;
}


std::vector<Map::LootType> ParseTypes(const json::array& arr) {
    std::vector<Map::LootType> result;
    result.reserve(arr.size());
    for (const auto& type_json : arr) {  
        Map::LootType type {};
        for (const auto& [key, val] : type_json.as_object()) {
           
            
            if (val.is_double()) {
                type[key] = val.as_double();
                
            } 
            else if(val.is_string()) {
                type[key] = std::string{val.as_string()};
                
            }
            else if(val.is_int64()) {
                type[key] = static_cast <int> (val.as_int64());
            }
        }
        result.push_back (std::move(type));
    }
    return result;
}


std::optional<LootGenerator> LoadLootGenerator (const std::filesystem::path& json_path) {
    std::fstream in(json_path);
    if (!in) { ParseError ("Error opening path"); }
    std::string temp{};
    std::string res{};
    while (std::getline(in, temp))
    {
        res += temp;
    }
    in.close(); 

    auto value = json::parse(res);
    if (!value.is_object()) { throw ParseError("Input Error JSON bad format");}
    if (value.as_object().find ("lootGeneratorConfig") != value.as_object().end()) {
        try {
            double val_seconds = value.at("lootGeneratorConfig").as_object().at("period").get_double();
            auto val_dur_seconds = std::chrono::duration<double, std::ratio<1>>(val_seconds);
            auto val_mili = std::chrono::duration_cast<std::chrono::milliseconds> (val_dur_seconds);
            loot_gen::LootGenerator::TimeInterval period = std::chrono::milliseconds(val_mili);
            double probability = value.at("lootGeneratorConfig").as_object().at("probability").get_double();
            return LootGenerator (period, probability);
        } catch (...) {
            throw ParseError("Input Error JSON LoadLootGenerator bad format");
        }
    } 
    return std::nullopt;
}

Game LoadGame(Strand& strand, const std::filesystem::path& json_path) {
    // Загрузить содержимое файла json_path, например, в виде строки
    // Распарсить строку как JSON, используя boost::json::parse
    // Загрузить модель игры из файла
    Game game (strand);
    std::string temp{};
    std::string res{};
    std::fstream in(json_path);
    if (!in) {
        ParseError ("Error opening path");
    }
    while (std::getline(in, temp))
    {
        res += temp;
    }
    in.close(); 
    try {
        auto value = json::parse(res);
        if (!value.is_object()) { throw ParseError("Input Error JSON bad format");}

        double default_map_speed = -1;

        if (value.as_object().find ("defaultDogSpeed") != value.as_object().end()) {
            default_map_speed = value.as_object().at ("defaultDogSpeed").as_double();
        } else {
            default_map_speed = 1;
        }
        game.SetDefaultSpeed(default_map_speed);

        if (value.as_object().find ("maps") == value.as_object().end()) {throw ParseError("Input Error JSON no maps");} 
        //далее вектор мапов value.as_object().at ("maps")
        if (!value.as_object().at ("maps").is_array()) {throw ParseError("Input Error JSON - no array after map");}
        auto& vect_maps = value.as_object().at ("maps").as_array();
        for (const auto& gamemap : vect_maps) {
            if (!gamemap.is_object()) {throw ParseError("Input Error JSON");}
            std::string id{};
            std::string name{};
            json::array roads {};
            json::array buildings {};
            json::array offices {};
            json::array loot_types {};
            double map_speed = -1;    
            for (const auto& [key, val] : gamemap.as_object()) {
                if (key == "id") {id = val.as_string();}
                else if (key == "name") {name = val.as_string();}
                else if (key == "roads") {
                    roads = val.as_array();
                    if (roads.size() < 1) {throw ParseError("Input Error JSON - roads empty");}
                }
                else if (key == "buildings") { buildings = val.as_array();}
                else if (key == "offices") {offices = val.as_array();}
                else if (key == "dogSpeed") {map_speed = val.as_double();}
                else if (key == "lootTypes") { loot_types = val.as_array();}
            }
            Map map (Map::Id{std::move(id)},std::move(name));
            for (const auto& road : ParseRoads(roads)) {
                map.AddRoad (road);
            }
            for (const auto& buil : ParseBuildings(buildings)) {
                map.AddBuilding(buil);
            }
            for (const auto& off : ParseOffices(offices)) {
                map.AddOffice(off);
            }
            for (const auto& type : ParseTypes(loot_types)) {
                map.AddType(type);
            }
            map_speed > -1 ? map.SetSpeed (map_speed) : map.SetSpeed ( game.GetDefaultSpeed() );
            
            game.AddMap(std::move(map));
        } 
    }
    catch (...) {
        throw ParseError ("Error opening path");
    }
    
    return game;
}


boost::json::array GetJsonRoads (const model::Map& gamemap) {
    boost::json::array val;
    for (const auto& rd : gamemap.GetRoads()) {
        boost::json::object obj;
        obj["x0"] = rd.GetStart().x;
        obj["y0"] = rd.GetStart().y;
        if (rd.IsVertical()) {
            obj["y1"] = rd.GetEnd().y;
        } else {
            obj["x1"] = rd.GetEnd().x;
        }
        val.push_back(std::move(obj));

    }
    return val;
}

boost::json::array GetJsonBuildings (const model::Map& gamemap) {
    boost::json::array val;
    for (const auto& house : gamemap.GetBuildings()) { 
        boost::json::object obj;
        obj["x"] = house.GetBounds().position.x;
        obj["y"] = house.GetBounds().position.y;
        obj["w"] = house.GetBounds().size.width;
        obj["h"] = house.GetBounds().size.height;
        val.push_back(std::move(obj));
    }
    return val;
}

boost::json::array GetJsonOffices (const model::Map& gamemap) {
        boost::json::array val;
    for (const auto& off : gamemap.GetOffices()) { 
        boost::json::object obj;
        obj["id"] = *(off.GetId());
        obj["x"] = off.GetPosition().x;
        obj["y"] = off.GetPosition().y;
        obj["offsetX"] = off.GetOffset().dx;
        obj["offsetY"] = off.GetOffset().dy;
        val.push_back(std::move(obj));
    }
    return val;
}

boost::json::array GetLootTypes (const model::Map& gamemap) {
    boost::json::array val;
    for (const auto& [key, descr] : gamemap.GetLootDescription()) { 
        boost::json::value obj = boost::json::value_from( descr );
        val.push_back(obj.as_object());
    }
    return val;
}



}  // namespace json_loader

