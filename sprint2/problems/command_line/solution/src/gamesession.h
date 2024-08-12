#pragma once
#include <cstdint>
#include <memory>
//#include "tagged.h"
#include "players.h"
#include "model.h"

using namespace model;

class GameSession;

class Players {
    

    public:
        std::shared_ptr<Player> AddPlayer (uint64_t dog_id, std::string& map_id);
        std::shared_ptr<Player> FindByDogIDAndMapID (uint64_t dog_id, Map::Id& map_id) const;
       
    private:
        
        std::unordered_map <std::pair<uint64_t, std::string>, 
                            std::shared_ptr<Player>,
                            HashPair<uint64_t, std::string>
                            > map_players_;  // ключ id dog и название карты
};

class GameSession {
    public:
        std::shared_ptr <Dog> GetDog (uint64_t id_dog) const;
        const std::unordered_map <uint64_t, std::shared_ptr <Dog>>& GetAllDogs () const {return dogs_;} 
        std::unordered_map <uint64_t, std::shared_ptr <Dog>>& GetAllDogs () {return dogs_;} 
        uint64_t AddDog (const std::string& name);
        Map* GetMap () {return gamemap_ptr;}
        void SetMap ( Map* map) {gamemap_ptr = map;}
    private:
        std::unordered_map <uint64_t, std::shared_ptr <Dog>> dogs_{};
        uint64_t dogs_id = 0;
        Map* gamemap_ptr = nullptr;
        
};