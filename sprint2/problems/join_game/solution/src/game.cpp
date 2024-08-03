#include "game.h"
#include <algorithm>

using namespace model;

const Map* Game::FindMap(const Map::Id& id) const  noexcept {
    if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
        return &maps_.at(it->second);
    }
    return nullptr;
}

const Game::Maps& Game::GetMaps() const  noexcept {
    return maps_;
}

void Game::AddMap(Map&& map) {
    const std::size_t index = maps_.size();
    if (auto [it, inserted] = map_id_to_index_.emplace(map.GetId(), index); !inserted) {
        throw std::invalid_argument("Map with id "s + *map.GetId() + " already exists"s);
    } else {
        try {
            maps_.emplace_back(std::move(map));
        } catch (...) {
            map_id_to_index_.erase(it);
            throw;
        }
    }
}

std::shared_ptr<GameSession> Game::AddSession (const Map* map) {
    if (map == nullptr) throw std::invalid_argument ("Can't find this map");
    game_sessions_[map->GetId()] = std::make_shared<GameSession>();
    return game_sessions_[map->GetId()];
}

detail::Token Game::AddPlayer (std::string& user_name, std::string& map_name) {
    //по имени карты найти\создать сессию
    auto iter = game_sessions_.find (Map::Id {map_name});
    
    if (iter != game_sessions_.end()) { 
        
        if (game_sessions_.at(Map::Id {map_name}) == nullptr) {
            game_sessions_.erase (Map::Id {map_name});
            iter = game_sessions_.end();
        } 

    } 
    if (iter == game_sessions_.end()) {
        auto iter_maps = std::find_if (maps_.cbegin(), maps_.cend(), [&](const auto& map){
            util::Tagged<std::string, model::Map> check_name = map.GetId();
            std::string check_name_string = *check_name;
            return check_name_string == map_name;
        });
        if (iter_maps == maps_.end()) {
            //поменять на логгер потом
            throw std::invalid_argument ("Can't find this map");
        }
        std::shared_ptr<GameSession> sessionptr = AddSession(&(*iter_maps));
               
    }   
    //  создать собаку в сессии получить ид собаки
    uint64_t id_dog = game_sessions_[Map::Id {map_name}]->AddDog (user_name);
    // добавить к игрокам
    std::shared_ptr<Player> playerptr = players->AddPlayer (id_dog, map_name);
    // присвоить указатель собаке в игроке
    playerptr->SetDog (iter->second->GetDog(id_dog));  
    // получить токен
    detail::Token token_for_player = players_tokens_->GiveTokenNewPlayer (playerptr);
     
    return std::move(token_for_player);
}

const std::shared_ptr <Players>& Game::GetPlayersPtr () const {
    return players;
}

// const std::unique_ptr <PlayersTokens>& Game::GetPlayersTokensPtr () const {
//     return players_tokens_;
// }

std::shared_ptr<Player> Game::FindPlayer (std::string& token) const  {
    return players_tokens_->FindPlayerByTokenString (token);
}

std::vector<std::string> Game::FindPlayersOnMap (const Map::Id& id) const {
    if (!game_sessions_.contains(id))  return {};
    std::vector<std::string> result;
    result.reserve (game_sessions_.at (id)->GetAllDogs().size());
    for (const auto& [dog_id, dog_ptr] : game_sessions_.at (id)->GetAllDogs()) {
        result.push_back(dog_ptr->GetName());
    }
    return result;
}



Map::Id Game::FindPlayerMap (const std::shared_ptr <Player>& player) const {
    for (const auto& [map_id, ptr_session] :game_sessions_) {
        if (ptr_session->GetDog (player->GetDogPtr()->GetId()) != nullptr) {
            return map_id;
        }
    }
    return Map::Id{""};
}