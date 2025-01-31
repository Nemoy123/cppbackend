#include "gamesession.h"


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
        double item_width = 0;
        collision_detector::Item item {{loot->x, loot->y}, (item_width/2)};
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
            if (GetLootList()[event.item_id] != nullptr) {
                if (dog_ptr->GetLootBag().size() < max_capacity) {
                    dog_ptr->AddLoot (std::move(GetLootList()[event.item_id]));
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
    //сборка мусора
    for (auto i = GetLootList().size()-1; i != -1; --i) {
        if (GetLootList()[i] == nullptr) {
            GetLootList().erase (GetLootList().begin()+i);
        }
    }
  
}

