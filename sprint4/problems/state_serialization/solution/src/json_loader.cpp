#include "json_loader.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include <boost/json/src.hpp>

namespace json_loader {
    namespace json = boost::json;
    using namespace std::literals;
    using namespace model;
    using namespace json_map_loader;


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

    return LoadLootGenerator(res);
}

std::optional<LootGenerator> LoadLootGenerator (const std::string& res) {
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

Game LoadGame(Strand& strand, const std::string& res) {

    Game game (strand);
    
    try {
        auto value = json::parse(res);
        if (!value.is_object()) { throw ParseError("Input Error JSON bad format");}

        double default_map_speed = -1;

        if (value.as_object().find ("defaultDogSpeed") != value.as_object().end()) {
            default_map_speed = value.as_object().at ("defaultDogSpeed").as_double();
        } else {
            default_map_speed = 1;
        }

        if (value.as_object().find ("defaultBagCapacity") != value.as_object().end()) {
            game.SetDefaultBagCapacity( value.as_object().at ("defaultBagCapacity").as_uint64());
        } 
        

        if (value.as_object().find ("maps") == value.as_object().end()) {throw ParseError("Input Error JSON no maps");} 
        //далее вектор мапов value.as_object().at ("maps")
        if (!value.as_object().at ("maps").is_array()) {throw ParseError("Input Error JSON - no array after map");}
        auto& vect_maps = value.as_object().at ("maps").as_array();
        for (const auto& gamemap : vect_maps) {
            
            auto map = MapConstruction(gamemap);
            if (map) {
                if (map.value().GetSpeed() == -1) {map.value().SetSpeed ( game.GetDefaultSpeed() );}
                if (map.value().GetBagCapacity() < 0) {map.value().SetBagCapacity(game.GetDefaultBagCapacity());}
                game.AddMap(std::move(map.value()));
            } else {
                throw ParseError("Input Error JSON");
            }
        } 
    }
    catch (...) {
        throw ParseError ("Error opening path");
    }
    
    return game;
}

Game LoadGame(Strand& strand, const std::filesystem::path& json_path) {
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

    return LoadGame (strand, res);
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

