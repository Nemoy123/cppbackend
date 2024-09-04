#include "gamesession.h"
#include <random>

std::string Dog::GetIdString() const {return std::to_string(id_);}
uint64_t Dog::GetId() const {return id_;}
const std::string& Dog::GetName() const {return name_;}

uint64_t GameSession::AddDog (const std::string& name) {
            auto new_dog = std::make_shared <Dog>(Dog(name));
            auto id = new_dog->GetId();
            dogs_[id] = std::move (new_dog);
            return id;
}

std::shared_ptr <Dog> GameSession::GetDog (uint64_t id_dog) const {
            return dogs_.contains(id_dog) ? dogs_.at(id_dog) : nullptr;
}

std::shared_ptr<Player> Players::AddPlayer (uint64_t dog_id, std::string& map_id) {
    map_players_[std::pair{dog_id, map_id}] = std::make_shared<Player>();
    return map_players_[std::pair{dog_id, map_id}];
}

std::shared_ptr<Player> Players::FindByDogIDAndMapID (uint64_t dog_id, Map::Id& map_id) const {
    return map_players_.at(std::pair{dog_id, *map_id});
}

void GameSession::AddLoot(int num) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(0, (gamemap_ptr_->GetLootDescription().size())-1);
    
    for (auto i =0; i < num; ++i) {
        
        Loot loot;
        loot.type = distr(gen);
        std::uniform_int_distribution<> road_distr(0, ((gamemap_ptr_->GetRoads().size())-1));
        auto* road_ptr = &(gamemap_ptr_->GetRoads().at(road_distr(gen)));
        
        if (road_ptr->IsHorizontal()) {
            std::uniform_int_distribution<> road_coor(std::min(road_ptr->GetStart().x, road_ptr->GetEnd().x), 
                                                      std::max(road_ptr->GetStart().x, road_ptr->GetEnd().x));
            loot.y = road_ptr->GetStart().y;
            loot.x = road_coor(gen);
        } else {
            
            std::uniform_int_distribution<> road_coor(std::min(road_ptr->GetStart().y, road_ptr->GetEnd().y), 
                                                      std::max(road_ptr->GetStart().y, road_ptr->GetEnd().y));
            loot.x = road_ptr->GetStart().x;
            loot.y = road_coor(gen);
        }
        loot_list_.push_back(std::move(loot));
    }
    
}