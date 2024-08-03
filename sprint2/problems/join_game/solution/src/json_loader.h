#pragma once

#include <filesystem>
#include <boost/json.hpp>
//#include "model.h"
#include "game.h"

namespace json_loader {

Game LoadGame(const std::filesystem::path& json_path);
boost::json::array GetJsonRoads (const model::Map& gamemap);
boost::json::array GetJsonBuildings (const model::Map& gamemap);
boost::json::array GetJsonOffices (const model::Map& gamemap);

}  // namespace json_loader
