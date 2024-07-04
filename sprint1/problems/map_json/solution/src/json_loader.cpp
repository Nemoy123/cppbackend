#include "json_loader.h"
#include <sstream>
#include <iostream>
#include <boost/json/src.hpp>

namespace json_loader {
    namespace json = boost::json;
    using namespace std::literals;
    using namespace model;

std::vector<Road> ParseRoads (const json::array& arr) {
    std::vector<Road> result;
    result.reserve(arr.size());
    for (const auto& road : arr) {
        int x0_coordinate = 0;
        int y0_coordinate = 0;
        int x1_coordinate = 0;
        int y1_coordinate = 0;
        bool vert = false;
        for (const auto& [key, val]:road.as_object()) {
            if (key == "x0") { x0_coordinate = val.as_int64();}  
            if (key == "y0") { y0_coordinate = val.as_int64();}
            if (key == "y1") { 
                y1_coordinate = val.as_int64();
                vert = true;
            }
            if (key == "x1") { x1_coordinate = val.as_int64();}
        }
        if (vert) {
            result.emplace_back (Road (Road::VERTICAL, Point {x0_coordinate, y0_coordinate},  y1_coordinate));
        } else {
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
        for (const auto& [key, val]:building.as_object()) {
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
        for (const auto& [key, val]:off.as_object()) {
            if (key == "x") { x = val.as_int64();} 
            if (key == "y") { y = val.as_int64();}
            if (key == "offsetx") { offsetx = val.as_int64();}
            if (key == "offsety") { offsety = val.as_int64();}
            if (key == "id") { id = val.as_string();} 
        }

        result.emplace_back ( Office (Office::Id(id), Point{x, y}, Offset{offsetx, offsety}) );
    }

    return result;
}


model::Game LoadGame(const std::filesystem::path& json_path) {
    // Загрузить содержимое файла json_path, например, в виде строки
    // Распарсить строку как JSON, используя boost::json::parse
    // Загрузить модель игры из файла
    model::Game game;
    std::stringstream ssjson(json_path);
    std::cout << ssjson.str();
    auto value = json::parse(ssjson.str());
    //model::Map(Id id, std::string name)
    if (!value.is_object()) { throw ("Input Error JSON bad format");}
    if (value.as_object().find ("maps") == value.as_object().end()) {throw ("Input Error JSON no maps");} 
    //далее вектор мапов value.as_object().at ("maps")
    if (!value.as_object().at ("maps").is_array()) {throw ("Input Error JSON - no array after map");}
    auto& vect_maps = value.as_object().at ("maps").as_array();
    for (const auto& gamemap : vect_maps) {
        if (!gamemap.is_object()) {throw ("Input Error JSON");}
        std::string id = 0;
        std::string name{};
        json::array roads {};
        json::array buildings {};
        json::array offices {};

        for (const auto& [key, val] :gamemap.as_object()) {
            if (key == "id") {id = val.as_string();}
            else if (key == "name") {name = val.as_string();}
            else if (key == "roads") {
                roads = val.as_array();
                if (roads.size() < 1) {throw ("Input Error JSON - roads empty");}
            }
            else if (key == "buildings") { buildings = val.as_array();}
            else if (key == "offices") {offices = val.as_array();}
        }
        Map map (Map::Id{std::move(id)},std::move(name));
        for (const auto& road:ParseRoads(roads)) {
            map.AddRoad (road);
        }
        for (const auto& buil:ParseBuildings(buildings)) {
            map.AddBuilding(buil);
        }
        for (const auto& off:ParseOffices(offices)) {
            map.AddOffice(off);
        }

        game.AddMap(std::move(map));
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


  // "offices": [
                    //     { "id": "o0", "x": 40, "y": 30, "offsetX": 5, "offsetY": 0 }
                    // ]

}  // namespace json_loader

