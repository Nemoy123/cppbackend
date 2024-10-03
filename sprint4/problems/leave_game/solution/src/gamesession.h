#pragma once
#include <cstdint>
#include <memory>

#include "players.h"
#include "model.h"
#include "collision_detector.h"

using namespace model;
using namespace collision_detector;

class GameSession;

class Players {
    public:
        
        using PMap = std::unordered_map <std::pair<uint64_t, std::string>, 
                            std::shared_ptr<Player>,
                            HashPair<uint64_t, std::string>>;

        Players(){}
        Players(const PMap& pmap):map_players_(pmap){}
        std::shared_ptr<Player> AddPlayer (const uint64_t dog_id, const std::string& map_id);
        std::shared_ptr<Player> FindByDogIDAndMapID (uint64_t dog_id, Map::Id& map_id) const;
        const PMap& GetMapPlayers () const;
        PMap& GetMapPlayers (); 
        bool operator== (const Players& other) const;
        typename PMap::iterator FindByDogID (uint64_t dog_id);
    private:
        
        PMap map_players_;  // ключ id dog и название карты
};


struct CollisionPrepare {
    std::vector <collision_detector::Item> items{};
    std::vector <collision_detector::Gatherer> dogs{};
    bool Empty () {return items.empty() || dogs.empty();}
};

class GameSession {
    public:
        
        std::shared_ptr <Dog> GetDog (uint64_t id_dog) const;
        const std::unordered_map <uint64_t, std::shared_ptr <Dog>>& GetAllDogs () const;
        std::unordered_map <uint64_t, std::shared_ptr <Dog>>& GetAllDogs ();
        uint64_t AddDog (const std::string& name);
        Map* GetMap () {return gamemap_ptr_;}
        const Map* GetMap () const {return gamemap_ptr_;}
        void SetMap ( Map* map) {gamemap_ptr_ = map;}
        size_t GetLootCount () {return gamemap_ptr_->GetLootList().size();}
        //void SetLootCount (int num) {loot_count_ = num;}
        void AddLoot(int num);
        const std::map <util::UUIDType, std::unique_ptr<Loot>>& GetLootList () const;
        std::map <util::UUIDType, std::unique_ptr<Loot>>& GetLootList ();
        CollisionPrepare GetCollisionPrepare() const;
        void HandlerCollision (CollisionPrepare& col);
        bool DeleteDog (uint64_t id_dog);

        bool operator==(const GameSession& other) const;
    private:
        std::unordered_map <uint64_t, std::shared_ptr <Dog>> dogs_{};
        //uint64_t dogs_id_ = 0;
        Map* gamemap_ptr_ = nullptr;
        //int loot_count_ = 0;
};




class CollisionClass : public ItemGathererProvider {
    private:
        std::vector <Item> items_;
        std::vector <Gatherer> dogs_;
    public:
        CollisionClass (const std::vector <Item>& items, const std::vector <Gatherer>& dogs): items_(items), dogs_(dogs) {}
        size_t ItemsCount() const override {return items_.size();}
        Item GetItem(size_t idx) const override {
            if (idx > ItemsCount()) throw std::invalid_argument("wrong item num");
            return items_.at(idx);}
        size_t GatherersCount() const override {return dogs_.size();}
        Gatherer GetGatherer(size_t idx) const override {
            if (idx > GatherersCount()) throw std::invalid_argument("wrong item num");
            return dogs_.at(idx);}
};