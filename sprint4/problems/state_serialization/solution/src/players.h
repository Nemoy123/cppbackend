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
    bool operator==(const Pos& other) const {
        return x == other.x && y == other.y;
    }
};

struct Speed {
    double x = 0;
    double y = 0;
    bool operator==(const Speed& other) const {
        return x == other.x && y == other.y;
    }
};



class Dog {
    public:
        static uint64_t static_id;
        explicit Dog (std::string name) : name_(name), id_(static_id++) {}
        std::string GetIdString() const ;
        uint64_t GetId() const;
        const std::string& GetName() const;
        
        Pos& GetPosition ();
        const Pos& GetPosition () const;
        Speed& GetSpeed ();
        const Speed& GetSpeed () const;
        std::string& GetDirection();
        const std::string& GetDirection() const;
        void SetDirection(std::string&& dir);
        void SetDirection(const std::string& dir);
        void SetSpeed(Speed&& speed);
        void SetSpeed(const Speed& speed){speed_=speed;}
        void SetPosition(Pos&& pos) {pos_=std::move(pos);}
        void SetPosition(const Pos& pos) {pos_=pos;}
        std::vector <std::unique_ptr<Loot>>& GetLootBag (); 
        const std::vector <std::unique_ptr<Loot>>& GetLootBag () const;
        void AddLoot (std::unique_ptr<Loot> loot);
        size_t& GetScore () {return score;}
        const size_t& GetScore () const {return score;}
        void SetScore (int num) {score = num;}

        void SetId (uint64_t num) {id_ = num;}
        bool operator==(const Dog& other) const;
    private:
        
        std::string name_{};
        uint64_t id_ = 0;
        Pos pos_;
        Speed speed_;
        std::string dir_{"U"};
        std::vector <std::unique_ptr<Loot>> loot_bag_{};
        size_t score = 0;
};



class Player {
    public:
        explicit Player (std::shared_ptr<Dog> dog) : dog_(dog){}
        Player () {}
        std::string GetDogIdString () const  {return dog_->GetIdString();}
        std::shared_ptr<Dog> GetDogPtr() {return dog_;}
        const std::shared_ptr<Dog> GetDogPtr() const {return dog_;}
        void SetDog (const std::shared_ptr<Dog>& dog) {dog_ = dog;}
        bool operator== (const Player& other) const  {return *dog_==*other.dog_;}
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
        using TMap = std::unordered_map <detail::Token, std::shared_ptr<Player>, Hash>;
        PlayersTokens (){}
        PlayersTokens (const TMap& map_tok): map_tokens_(map_tok){}
        PlayersTokens (TMap&& map_tok): map_tokens_(std::move(map_tok)){}
        detail::Token GiveTokenNewPlayer (std::shared_ptr<Player> player);
        std::shared_ptr<Player> FindPlayerByToken (detail::Token& token) const;
        std::shared_ptr<Player> FindPlayerByTokenString (const std::string& token) const;
        TMap& GetMapTokens() {return map_tokens_;}
        const TMap& GetMapTokens() const {return map_tokens_;}
    private:
        TMap map_tokens_;
        
        std::shared_ptr<PlayerTokens> token_generator_ = std::make_shared<PlayerTokens>();
};