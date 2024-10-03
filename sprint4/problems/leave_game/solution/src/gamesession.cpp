#include "gamesession.h"


const std::unordered_map <uint64_t, std::shared_ptr <Dog>>& GameSession::GetAllDogs () const {
        return dogs_;
} 

std::unordered_map <uint64_t, std::shared_ptr <Dog>>& GameSession::GetAllDogs () {
    return dogs_;
} 

std::map <util::UUIDType, std::unique_ptr<Loot>>& GameSession::GetLootList () {
    return gamemap_ptr_->GetLootList();
}

const std::map <util::UUIDType, std::unique_ptr<Loot>>& GameSession::GetLootList () const  {
    return gamemap_ptr_->GetLootList();
}

std::string Dog::GetIdString() const {
    return std::to_string(id_);
}

uint64_t Dog::GetId() const {
    return id_;
}

const std::string& Dog::GetName() const {
    return name_;
}

uint64_t GameSession::AddDog (const std::string& name) {
            auto new_dog = std::make_shared <Dog>(name);
            auto id = new_dog->GetId();
            dogs_[id] = std::move (new_dog);
            return id;
}

void GameSession::AddLoot(int num) {
    gamemap_ptr_->AddLoot(std::min (num, static_cast<int>(GetAllDogs ().size())));
}

std::shared_ptr <Dog> GameSession::GetDog (uint64_t id_dog) const {
            return dogs_.contains(id_dog) ? dogs_.at(id_dog) : nullptr;
}

std::shared_ptr<Player> Players::AddPlayer (const uint64_t dog_id, const std::string& map_id) {
    map_players_[std::pair{dog_id, map_id}] = std::make_shared<Player>();
    return map_players_[std::pair{dog_id, map_id}];
}

std::shared_ptr<Player> Players::FindByDogIDAndMapID (uint64_t dog_id, Map::Id& map_id) const {
    return map_players_.at(std::pair{dog_id, *map_id});
}

const Players::PMap& Players::GetMapPlayers () const {
    return map_players_;
} 
Players::PMap& Players::GetMapPlayers () {
    return map_players_;
} 


CollisionPrepare GameSession::GetCollisionPrepare() const {
    CollisionPrepare result;
    for (const auto& office : GetMap ()->GetOffices()) {
        // первые Item это офис
        double office_width = 0.5;
        double posx = static_cast<double> (office.GetPosition().x);
        double posy = static_cast<double> (office.GetPosition().y);
        collision_detector::Item item {{posx, posy}, office_width/2};
        item.type = Type::BASE;
        result.items.push_back(std::move(item));
    }
    for (const auto& loot : GetLootList()) {
        double item_width = 0.5;
        collision_detector::Item item {{loot.second->x, loot.second->y}, (item_width/2), loot.first};
        result.items.push_back(std::move(item));
    }
    for (const auto& [id, dog_ptr] : GetAllDogs()) { 
        collision_detector::Gatherer dog {}; 
        double dog_width = 0.6;
        dog.width = (dog_width/2);
        dog.start_pos = geom::Point2D{dog_ptr->GetPosition().x, dog_ptr->GetPosition().y};
        dog.id = id;
        result.dogs.push_back(std::move(dog));
    }
    return result;
}

bool GameSession::operator==(const GameSession& other) const {
    if (*gamemap_ptr_ != *other.gamemap_ptr_ 
        || dogs_.size() != other.dogs_.size()) 
    {
            return false;
    }
    
    for (const auto& [id, ptr_dog]: dogs_) {
        if (!other.dogs_.contains(id)) return false;
        if (*ptr_dog != *(other.dogs_.at(id))) return false;
    }
    return true;
}

bool Players::operator== (const Players& other) const {
    for (const auto& [key, val]: map_players_) {
        if (!other.map_players_.contains(key)) {
            return false;
        }
        if (*val != *(other.map_players_.at(key))) {
            return false;
        }
    }
    return true;
}

void GameSession::HandlerCollision (CollisionPrepare& col) {
    if (col.Empty()) return;
    int max_capacity = gamemap_ptr_->GetBagCapacity ();
    
    for (auto& dog : col.dogs) {
        auto dog_ptr = GetDog(dog.id);
        dog.end_pos = geom::Point2D{dog_ptr->GetPosition().x, dog_ptr->GetPosition().y};
    }
    CollisionClass provider (col.items, col.dogs);
    auto events = FindGatherEvents (&provider);

    for (const auto& event : events) {
        int id_dog = col.dogs.at(event.gatherer_id).id;
        auto dog_ptr = GetDog(id_dog);
        if (col.items.at(event.item_id).type == Type::ITEM) {
            util::UUIDType uuid = provider.GetItem(event.item_id).id;
            if (GetLootList()[uuid] != nullptr) {
                if (dog_ptr->GetLootBag().size() < max_capacity) {
                    dog_ptr->AddLoot (std::move(GetLootList()[uuid]));
                    GetLootList().erase(uuid);
                }
            }
        }
        else if (col.items.at(event.item_id).type == Type::BASE){
            for (const auto& it : dog_ptr->GetLootBag()) {
                auto* map_loot = &gamemap_ptr_->GetLootDescription();
                auto iter_begin = map_loot->begin();
                auto iter_desc = std::next(iter_begin, it->type);
                if (iter_desc->second.contains ("value")) {
                    dog_ptr->GetScore() += std::get<int> (iter_desc->second.at ("value"));
                }
            }
            dog_ptr->GetLootBag().clear();
        }

    }
    // //сборка мусора
    // for (auto i = GetLootList().size()-1; i != -1; --i) {
    //     if (GetLootList()[i] == nullptr) {
    //         GetLootList().erase (GetLootList().begin()+i);
    //     }
    // }
  
}

bool GameSession::DeleteDog (uint64_t id_dog) {
    auto dog = dogs_.find (id_dog);
    if (dog != dogs_.end()) {
        dogs_.erase(dog);
        return true;
    }
    return false;
}

typename Players::PMap::iterator Players::FindByDogID (uint64_t dog_id)  {
    
    for (auto it = map_players_.begin(); it != map_players_.end(); it = std::next(it)) { 
        if (it->first.first == dog_id) {
            return it;
        }
    }
    return map_players_.end();
}