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

// std::shared_ptr<Player> Players::AddNewPlayerToSession (std::string& dog_name, std::shared_ptr <GameSession>  session) {
//     uint64_t new_dog = session->AddDog(dog_name);
//     std::string map_stroke = *(session->GetMap()->GetId());
//     map_players_[std::pair{new_dog, map_stroke}] = std::make_shared<Player> (session->GetDog(new_dog), session);
//     return map_players_[std::pair{new_dog, map_stroke}];
// }

std::shared_ptr<Player> Players::AddPlayer (uint64_t dog_id, std::string& map_id) {
    map_players_[std::pair{dog_id, map_id}] = std::make_shared<Player>();
    return map_players_[std::pair{dog_id, map_id}];
}

std::shared_ptr<Player> Players::FindByDogIDAndMapID (uint64_t dog_id, Map::Id& map_id) const {
    return map_players_.at(std::pair{dog_id, *map_id});
}
