#pragma once

#include <boost/json.hpp>
#include "game.h"

namespace json_map_loader {

using namespace boost::json;

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

std::vector<Road> ParseRoads (const array& arr);
std::vector<Building> ParseBuildings (const array& arr);
std::vector<Office> ParseOffices (const array& arr);
std::vector<Map::LootType> ParseTypes(const array& arr);
std::optional <Map> MapConstruction (const value& val);

}