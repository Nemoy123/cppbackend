#include <catch2/catch_test_macros.hpp>
#include <boost/json.hpp>

#include "../src/json_map_loader.h"

using namespace boost::json;
using namespace json_map_loader;

using Roads = std::vector<Road>;

bool TestLootPositionOnRoad (const Loot& loot, const Roads& roads) {

    for (const auto& road : roads) {
        if (road.IsHorizontal()) {
            int start_x = std::min (road.GetStart().x, road.GetEnd().x);
            int end_x = std::max (road.GetStart().x, road.GetEnd().x);
            if (loot.y != road.GetStart().y) { continue; }
            else if (loot.x >= start_x && loot.x <= end_x) { return true;}
        } else {
            int start_y = std::min (road.GetStart().y, road.GetEnd().y);
            int end_y = std::max (road.GetStart().y, road.GetEnd().y);
            if (loot.x != road.GetStart().x) { continue; }
            else if(loot.y >= start_y && loot.y <= end_y) {return true;}
        }
    }
    return false;
}

SCENARIO("LOOT appearing on MAP") { 
    GIVEN("standart map1") { 
        std::string stroke = R"({"dogSpeed":4.0,"id":"map1","name":"Map 1","lootTypes":[{"name":"key","file":"assets/key.obj","type":"obj","rotation":90,"color":"#338844","scale":0.03},{"name":"wallet","file":"assets/wallet.obj","type":"obj","rotation":0,"color":"#883344","scale":0.01}],"roads":[{"x0":0,"y0":0,"x1":40},{"x0":40,"y0":0,"y1":30},{"x0":40,"y0":30,"x1":0},{"x0":0,"y0":0,"y1":30}],"buildings":[{"x":5,"y":5,"w":30,"h":20}],"offices":[{"id":"o0","x":40,"y":30,"offsetX":5,"offsetY":0}]})";
        auto val = boost::json::parse(stroke);
        auto map_value = MapConstruction(val);
        REQUIRE(map_value.has_value());
        Map* map = &(map_value.value());
        GameSession gs;
        gs.SetMap (map);
        REQUIRE(gs.GetLootCount() == 0);
        int test_count = 1000;
        gs.AddLoot(test_count);
        REQUIRE(gs.GetLootCount() == test_count);
         false;
        for (const auto& loot : gs.GetLootList()) {
            bool test_pos = TestLootPositionOnRoad (loot, gs.GetMap()->GetRoads());
            REQUIRE(test_pos == true);
        }
        
        


    }
}