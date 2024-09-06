#include "json_map_loader.h"


namespace json_map_loader {

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

std::optional <Map> MapConstruction (const boost::json::value& val) {
    if (!val.is_object()) {return std::nullopt;}
        std::string id{};
        std::string name{};
        json::array roads {};
        json::array buildings {};
        json::array offices {};
        json::array loot_types {};
        double map_speed = -1;   

        try{
            for (const auto& [key, val] : val.as_object()) {
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
            
            map.SetSpeed (map_speed);
            return map;
        } 
        catch (...) {
            return std::nullopt;
        }
        
}

} // конец namespace