#pragma once

#include <unordered_map>
#include <memory>

#include "model.h"
#include "token.h"
#include "tagged.h"

using namespace detail;
using namespace model;

struct Pos {
    double x = 0;
    double y = 0;
};

struct Speed {
    double x = 0;
    double y = 0;
};



class Dog {
    public:
        static uint64_t static_id;
        explicit Dog (std::string name) : name_(name), id_(static_id++) {}
        std::string GetIdString() const ;
        uint64_t GetId() const;
        const std::string& GetName() const;
        
        Pos& GetPosition ()  {return pos_;}
        Speed& GetSpeed () {return speed_;}
        std::string& GetDirection()  {return dir_;}
        void SetDirection(std::string&& dir)  {dir_ = std::move(dir);}
        void SetSpeed(Speed&& speed);
        void SetPosition(Pos&& pos) {pos_=std::move(pos);}
        std::vector <std::unique_ptr<Loot>>& GetLootBag () {return loot_bag_;}
        void AddLoot (std::unique_ptr<Loot> loot) {loot_bag_.push_back(std::move(loot));}

    private:
        
        std::string name_{};
        uint64_t id_ = 0;
        Pos pos_;
        Speed speed_;
        std::string dir_{"U"};
        std::vector <std::unique_ptr<Loot>> loot_bag_{};
};



class Player {
    public:
        explicit Player (const std::shared_ptr<Dog>& dog) : dog_(dog){}
        Player () {}
        std::string GetDogIdString () const  {return dog_->GetIdString();}
        std::shared_ptr<Dog> GetDogPtr() {return dog_;}
        void SetDog (const std::shared_ptr<Dog>& dog) {dog_ = dog;}
    private:
        std::shared_ptr<Dog> dog_ = nullptr;        
};

template<typename T>
void
hash_combine(std::size_t &seed, T const &key) {
  std::hash<T> hasher;
  seed ^= hasher(key) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}



struct Hash {
std::size_t operator()(const Token& p) const {
    std::size_t seed1(0);
    ::hash_combine(seed1, p.id_dog_);
    ::hash_combine(seed1, p.token_string_);

    std::size_t seed2(0);
    ::hash_combine(seed2, p.id_dog_);
    ::hash_combine(seed2, p.token_string_);

    return std::min(seed1, seed2);
}
};

template<typename S, typename X>
struct HashPair {
std::size_t operator()(const std::pair<S, X>& p) const {
    std::size_t seed1(0);
    ::hash_combine(seed1, p.first);
    ::hash_combine(seed1, p.second);

    std::size_t seed2(0);
    ::hash_combine(seed2, p.first);
    ::hash_combine(seed2, p.second);

    return std::min(seed1, seed2);
}
};


class PlayersTokens {
    public:
        detail::Token GiveTokenNewPlayer (std::shared_ptr<Player> player);
        std::shared_ptr<Player> FindPlayerByToken (detail::Token& token) const;
        std::shared_ptr<Player> FindPlayerByTokenString (const std::string& token) const;
    private:
        std::unordered_map <detail::Token, std::shared_ptr<Player>, Hash> map_tokens_;
        
        std::unique_ptr<PlayerTokens> token_generator_ = std::make_unique<PlayerTokens>();
};