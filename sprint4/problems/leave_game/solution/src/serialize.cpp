#include "serialize.h"
#include <fstream>

namespace serialization {

GameRepresentation::GameRepresentation(Strand strand, const Game& game)
     : players (*game.GetPlayersPtr())
     , players_tokens_ (*game.GetAllPLayersTokensPtr())
     , time_strand_(strand)
     , defaultDogSpeed (game.GetDefaultSpeed())
     , randomize_dog_start_(game.GetRandomize())
     , defaultBagCapacity (game.GetDefaultBagCapacity())
     , dogRetirementTime_(game.GetRetireTime().count())
{
    maps_.reserve (game.GetMaps().size());
    for (const auto& map_in : game.GetMaps()) {
        auto map_rep = MapRepresentation (map_in);
        maps_.push_back(std::move(map_rep));
    }
    for (const auto& [key, session] : game.GetAllSessions()){
        game_sessions_.push_back(GameSessionRepresentation{*session});
    }   

}

[[nodiscard]] Game GameRepresentation::Restore() const {
    Game game {time_strand_};
    
    for (const auto& map_rep : maps_) {
        game.AddMap (map_rep.Restore());
    }
    
    std::unordered_map <Map::Id, std::shared_ptr <GameSession>, MapIdHasher> sess;
    for (const auto& ses_rep: game_sessions_) {
        const auto gamemap_id = ses_rep.GetId();
        sess[gamemap_id] = std::make_shared<GameSession> (ses_rep.Restore());
        sess[gamemap_id]->SetMap(game.FindMap(gamemap_id));
        //sess[gamemap_id]->SetLootCount(game.FindMap(gamemap_id)->GetLootList().size());
    }
    game.SetAllSessions(std::move(sess));
    game.SetPtrPlayersToken (players_tokens_.Restore());
    game.SetPlayers(std::make_shared<Players> (players.Restore()));
    // using PMap = std::unordered_map <std::pair<uint64_t, std::string>, 
    //                     std::shared_ptr<Player>,
    //                     HashPair<uint64_t, std::string>>;

    for (auto& [keys, ptr] : game.GetPlayersPtr()->GetMapPlayers()) {
        std::shared_ptr <Dog> dog_ptr = game.GetAllSessions().at(Map::Id{keys.second})->GetDog(keys.first);
        for (auto& [token, pl_ptr] : game.GetAllPLayersTokensPtr()->GetMapTokens()) {
            //using TMap = std::unordered_map <detail::Token, std::shared_ptr<Player>, Hash>;
            if (token.id_dog_ == dog_ptr->GetId()) {
                ptr = pl_ptr;
            }
        }
    }
    
    game.SetDefaultSpeed(defaultDogSpeed);
    game.SetRandomize (randomize_dog_start_);
    game.SetDefaultBagCapacity (defaultBagCapacity);
    game.SetRetireTime(std::chrono::milliseconds{dogRetirementTime_});

    return game;
}

MapRepresentation::MapRepresentation (const model::Map& gamemap)
                : id_(*gamemap.GetId())
                , name_(gamemap.GetName())
                , dogSpeed (gamemap.GetSpeed())
                , bagCapacity(gamemap.GetBagCapacity())
{
        roads_.reserve(gamemap.GetRoads().size());
        for (const auto& road: gamemap.GetRoads()) {
            roads_.push_back (RoadRepr{road});
        }
        buildings_.reserve(gamemap.GetBuildings().size());
        for (const auto& buil: gamemap.GetBuildings()) {
            buildings_.push_back (BuildingRepr{buil});
        }
        offices_.reserve(gamemap.GetOffices().size());
        for (const auto& off: gamemap.GetOffices()) {
            offices_.push_back (OfficeRepr{off});
        }
        loot_description_.reserve(gamemap.GetLootDescription().size());
        for (const auto& [name_map, loot_type]: gamemap.GetLootDescription()) {
            loot_description_.push_back (std::pair{name_map, LootTypeRepr{loot_type}});
        }
        //loot_list_.reserve(gamemap.GetLootList().size());
        for (const auto& loot: gamemap.GetLootList()) {
            loot_list_.insert({util::UUIDToString (loot.first),LootRepr{(*loot.second)}});
        }

}

[[nodiscard]] Map MapRepresentation::Restore() const {
    Map gamemap {Map::Id{id_}, name_};
    for (const auto& road : roads_) {
        gamemap.AddRoad(road.Restore());
    }
    for (const auto& building : buildings_) {
        gamemap.AddBuilding(building.Restore());
    }
    for (const auto& office : offices_) {
        gamemap.AddOffice(office.Restore());
    }
    gamemap.SetSpeed(dogSpeed);
    gamemap.SetBagCapacity(bagCapacity);
    std::map <util::UUIDType, std::unique_ptr<Loot>> maploot;
    for (const auto& loot: loot_list_) {
        auto loot_new = std::make_unique<Loot> (loot.second.Restore());
        maploot[util::UUIDFromString(loot.first)] = (std::move(loot_new));
    }
    gamemap.GetLootList() = std::move(maploot);
    std::map <std::string, Map::LootType > map_loot_desc;
    for (const auto& [map_name, loot_type]: loot_description_) {
        map_loot_desc[map_name] = loot_type.Restore();
    }
    gamemap.GetLootDescription() = std::move(map_loot_desc);
    return gamemap;
}

void Serialize::Init (Strand& strand, std::filesystem::path& path, uint64_t saving_period, Mode& mode, LootGenerator* loot_gen) {
    strand_ = &strand;
    saving_period_ = saving_period;
    path_ = path;
    mode_ = mode;
    loot_gen_ = loot_gen;
}
Game* Serialize::GetGamePtr () {
    return game_;
}

void Serialize::SetLootGenPTR(LootGenerator& loot_gen) {
    loot_gen_ = &loot_gen;
}

void Serialize::SaveAll() {
    using std::fstream;
    std::lock_guard st (mut);
    if (GetGamePtr() == nullptr || loot_gen_ == nullptr) {return;}
    std::fstream fs;
    std::filesystem::path path_temp {path_.parent_path()};
    path_temp /= "temp_save";
    fs.open(path_temp, std::ios::out);
    if (!fs) { throw ("Error opening saving file");}
    OutputArchive output_archive_{fs};
    GameRepresentation repr{*strand_, *game_};
    LootGeneratorRepresentation lg {*loot_gen_};
    output_archive_ << repr;
    output_archive_ << lg;
    fs.close();
    std::error_code ec;
    std::filesystem::rename(path_temp, path_, ec);
    if (ec) {
        throw (ec.message());
    }
}

const uint64_t Serialize::GetSerializePeriod () const {
    return saving_period_;
} 

std::optional<std::pair<Game, LootGenerator>> Serialize::Restore() {
    using std::fstream;
    std::ifstream fs;
    fs.open(path_, fstream::in);
    
    if (fs) {
        InputArchive input_archive_{fs};
        GameRepresentation game_restored (*strand_);
        LootGeneratorRepresentation lg;
        try {
            input_archive_ >> game_restored;
            input_archive_ >> lg;
            fs.close();
            Game result {game_restored.Restore()};
            LootGenerator res_lg {lg.Restore()};
            return std::pair{std::move(result), std::move(res_lg)};
        } catch (...) {
            return std::nullopt;
        }
    }
    return std::nullopt;
}

void Serialize::SetGamePtr (Game& game) {
    game_ = &game;
}

void Serialize::OnTick(uint64_t time) {

    time_since_last_save_ += time;

    if (time_since_last_save_ > saving_period_) {
        serialization::Serialize::getInstance().SaveAll();
        time_since_last_save_ = 0;
    }

}

void Serialize::OnTick() { 
   
    auto now = steady_clock::now();
    auto per = std::chrono::milliseconds(saving_period_);
    auto sum = now - real_time_last_save_;
    
    if (sum > per) {
        serialization::Serialize::getInstance().SaveAll();
        real_time_last_save_ = steady_clock::now();
    }     
}

} // end namespace