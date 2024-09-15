#include <catch2/catch_test_macros.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <thread>
#include <iostream>
#include "../src/game.h"
#include "../src/loot_generator.h"
#include "../src/json_loader.h"
#include "../src/serialize.h"

using namespace std::literals;
namespace net = boost::asio;
namespace sys = boost::system;
namespace fs = std::filesystem;
using namespace json_loader;
using namespace serialization;


using InputArchive = boost::archive::text_iarchive;
using OutputArchive = boost::archive::text_oarchive;

SCENARIO( "Game serialize") { 
    GIVEN("a game") { 
       try{
       const unsigned num_threads = std::thread::hardware_concurrency();
        net::io_context ioc(num_threads);
        auto api_strand = net::make_strand(ioc);
        const std::string config {R"({"defaultDogSpeed":3.0,"lootGeneratorConfig":{"period":5.0,"probability":0.5},"maps":[{"dogSpeed":4.0,"id":"map1","name":"Map 1","lootTypes":[{"name":"key","file":"assets/key.obj","type":"obj","rotation":90,"color":"#338844","scale":0.03,"value":10},{"name":"wallet","file":"assets/wallet.obj","type":"obj","rotation":0,"color":"#883344","scale":0.01,"value":30}],"roads":[{"x0":0,"y0":0,"x1":40},{"x0":40,"y0":0,"y1":30},{"x0":40,"y0":30,"x1":0},{"x0":0,"y0":0,"y1":30}],"buildings":[{"x":5,"y":5,"w":30,"h":20}],"offices":[{"id":"o0","x":40,"y":30,"offsetX":5,"offsetY":0}]}]})"};
        auto loot_gen = LoadLootGenerator (config);
        Game game = json_loader::LoadGame(api_strand, config);
        bool randomize = true;
        game.SetRandomize(randomize);
        std::string user_name {"Gerasim"};
        std::string map_name {"map1"};
        std::string user_name2 {"MuMu"};
        
        auto token_Gerasim = game.AddPlayer(user_name , map_name);
        
        auto token_MuMu = game.AddPlayer(user_name2 , map_name);
        
        //game.GetSession (Map::Id{map_name})->AddLoot (2);
        
        
        game.GetAllSessions().begin()->second->AddLoot(2);
        GameRepresentation repr{api_strand, game};
        std::stringstream strm;
        OutputArchive output_archive{strm};
        LootGeneratorRepresentation lg {loot_gen.value()};
        output_archive << repr;
        output_archive << lg;
                
        InputArchive input_archive{strm};
        strm.flush();
        GameRepresentation repr2(api_strand);
        LootGeneratorRepresentation lg2;
        input_archive >> repr2;
        input_archive >> lg2;
        const auto game_finish = repr2.Restore();
        const auto loot_gen_finish = lg2.Restore();
        auto* map1 = &game.GetMaps();
        auto* map2 = &game_finish.GetMaps();
        
        CHECK(*map1 == *map2);
        CHECK(game.GetDefaultBagCapacity() == game_finish.GetDefaultBagCapacity());
        CHECK(game.GetDefaultSpeed() == game_finish.GetDefaultSpeed());
        CHECK(game.GetRandomize() == game_finish.GetRandomize());
        auto map_first_test = game.GetAllPLayersTokensPtr()->GetMapTokens();
        auto map_finish_test = game_finish.GetAllPLayersTokensPtr()->GetMapTokens();
        REQUIRE(map_first_test.size()==map_finish_test.size());
            for (const auto& [token, dog_ptr] : map_first_test) {
                CHECK(map_finish_test.contains(token));
                CHECK(*(dog_ptr->GetDogPtr()) == *(map_finish_test.at(token)->GetDogPtr()));
            }
            
            
        REQUIRE(game.GetAllSessions().size() == game_finish.GetAllSessions().size());
        for (const auto& [map_id, sess_ptr] : game.GetAllSessions()) { 
            CHECK(game_finish.GetAllSessions().contains(map_id));
            CHECK(*(sess_ptr) == *(game_finish.GetAllSessions().at(map_id)));
        }
        
        auto p1 = *game.GetPlayersPtr();
        auto p2 = *game_finish.GetPlayersPtr();
        
        CHECK(*game.GetPlayersPtr() == *game_finish.GetPlayersPtr());
        CHECK(loot_gen_finish.GetBaseInterval() == loot_gen.value().GetBaseInterval());
        CHECK(loot_gen_finish.GetProbability() == loot_gen.value().GetProbability());
        CHECK(game_finish.FindPlayer(token_Gerasim.token_string_)->GetDogIdString() == game.FindPlayer(token_Gerasim.token_string_)->GetDogIdString());
        CHECK(*(game_finish.FindPlayer(token_Gerasim.token_string_)->GetDogPtr()) == *(game.FindPlayer(token_Gerasim.token_string_)->GetDogPtr()) );
        CHECK(game_finish.FindPlayer(token_MuMu.token_string_)->GetDogIdString() == game.FindPlayer(token_MuMu.token_string_)->GetDogIdString());
        CHECK(*(game_finish.FindPlayer(token_MuMu.token_string_)->GetDogPtr()) == *(game.FindPlayer(token_MuMu.token_string_)->GetDogPtr()) );
        }
        catch (const std::exception& ex) {
            std::cerr << "Error throw " << ex.what() << std::endl;
        }
    }
}