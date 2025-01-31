#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include "model.h"
#include "gamesession.h"
#include "players.h"
#include <boost/asio/strand.hpp>
#include <boost/asio/io_context.hpp>
#include "../sql/base.h"
#include "../sql/usecases.h"

using namespace model;
using namespace std::literals;
namespace net = boost::asio;
using Strand = net::strand<net::io_context::executor_type>;

class Game : public std::enable_shared_from_this<Game> {
    using MapIdHasher = util::TaggedHasher<Map::Id>;
    using MapIdToIndex = std::unordered_map<Map::Id, std::size_t, MapIdHasher>;
public:
   
    Game(Strand& strand, Base& base)
                    : time_strand_(strand)
    {
        SetBase(base);
        // base_ = &base;
        // usecases_ = std::make_shared<UseCasesImpl> (*base_);
    }
    Game(const Strand& strand): time_strand_(strand) {}

    using Maps = std::vector<Map>;

    void AddMap(Map&& map);
    std::shared_ptr<GameSession> AddSession ( Map* map);
    std::shared_ptr<GameSession> GetSession (const Map::Id& id) const;
    detail::Token AddPlayer (std::string& user_name, std::string& map_name);
    std::shared_ptr<Player> FindPlayer (std::string& token) const;
    Map::Id FindPlayerMap (const std::shared_ptr <Player>& player) const;
    const Maps& GetMaps() const noexcept;
    const std::shared_ptr <Players>& GetPlayersPtr () const;
    std::shared_ptr <Players>& GetPlayersPtr ();
    Map* FindMap(const Map::Id& id) noexcept;
    const Map* FindMap(const Map::Id& id) const noexcept;
    std::vector<std::string> FindPlayersOnMap (const Map::Id& id, const bool names = true) const;
    void TimeUpdate (const uint64_t time);
    void SetDefaultSpeed (double speed);
    const double GetDefaultSpeed () const;
    void SetRandomize(bool random);
    const std::unordered_map <Map::Id, std::shared_ptr <GameSession>, MapIdHasher>& GetAllSessions () const;
    void SetDefaultBagCapacity (uint num);
    int GetDefaultBagCapacity () const;
    PlayersTokens* GetAllPLayersTokensPtr () const;
    void SetPtrPlayersToken (const PlayersTokens& pl_tok);
    void SetAllSessions (std::unordered_map <Map::Id, std::shared_ptr <GameSession>, MapIdHasher>&& sess);
    void SetPlayers (std::shared_ptr <Players> pl);
    Strand GetStrand() const;
    bool GetRandomize() const;
    std::shared_ptr<UseCasesImpl> GetUseCases() {return usecases_;}
    const std::shared_ptr<UseCasesImpl> GetUseCases() const {return usecases_;}
    Base* GetBase() {return base_;}
    const Base* GetBase() const {return base_;}
    void SetRetireTime (std::chrono::milliseconds time);
    const std::chrono::milliseconds& GetRetireTime () const;
    void Retire(std::shared_ptr <Dog> dog);
    void SetBase(Base& base);
private:
    
    
    std::vector<Map> maps_;
    MapIdToIndex map_id_to_index_;
    std::unordered_map <Map::Id, std::shared_ptr <GameSession>, MapIdHasher> game_sessions_; // ключ имя карты
    std::shared_ptr <Players> players = std::make_shared<Players> (); // все игроки
    std::unique_ptr <PlayersTokens> players_tokens_ = std::make_unique<PlayersTokens> (); // токены
    Strand time_strand_;
    double defaultDogSpeed = 1.0;
    bool randomize_dog_start_ = false;
    uint defaultBagCapacity = 3;
    Base* base_ = nullptr;
    std::shared_ptr<UseCasesImpl> usecases_ = nullptr;
    std::chrono::milliseconds dogRetirementTime_ = 60s;
    
 
};

