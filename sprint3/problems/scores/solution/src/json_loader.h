#pragma once

#include <filesystem>
#include <boost/json.hpp>
#include "game.h"
#include "loot_generator.h"
#include "json_map_loader.h"

using namespace loot_gen;

namespace json_loader {

//TODO можно запихнуть чтение json и все функции в класс, чтоб только один раз считывать конфиг

Game LoadGame(Strand& strand, const std::filesystem::path& json_path);
std::optional<LootGenerator> LoadLootGenerator (const std::filesystem::path& json_path);
boost::json::array GetJsonRoads (const model::Map& gamemap);
boost::json::array GetJsonBuildings (const model::Map& gamemap);
boost::json::array GetJsonOffices (const model::Map& gamemap);
boost::json::array GetLootTypes (const model::Map& gamemap);


}  // namespace json_loader
