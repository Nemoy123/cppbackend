#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include "model.h"
#include "gamesession.h"
#include "players.h"
#include <boost/asio/strand.hpp>
#include <boost/asio/io_context.hpp>


using namespace model;
using namespace std::literals;
namespace net = boost::asio;
using Strand = net::strand<net::io_context::executor_type>;

class Game : public std::enable_shared_from_this<Game> {
    using MapIdHasher = util::TaggedHasher<Map::Id>;
    using MapIdToIndex = std::unordered_map<Map::Id, std::size_t, MapIdHasher>;
public:
    Game(Strand& strand): time_strand_(strand) {}
    using Maps = std::vector<Map>;

    void AddMap(Map&& map);
    std::shared_ptr<GameSession> AddSession ( Map* map);
    std::shared_ptr<GameSession> GetSession (const Map::Id& id) const {return game_sessions_.at(id);}
    detail::Token AddPlayer (std::string& user_name, std::string& map_name);
    std::shared_ptr<Player> FindPlayer (std::string& token) const;
    Map::Id FindPlayerMap (const std::shared_ptr <Player>& player) const;
    const Maps& GetMaps() const noexcept;
    const std::shared_ptr <Players>& GetPlayersPtr () const;
    
    const Map* FindMap(const Map::Id& id) const noexcept;
    std::vector<std::string> FindPlayersOnMap (const Map::Id& id, const bool names = true) const;

    void TimeUpdate (const uint64_t time);
    void SetDefaultSpeed (double speed);
    const double GetDefaultSpeed () const;
    void SetRandomize(bool random) {randomize_dog_start_ = random;}
    const std::unordered_map <Map::Id, std::shared_ptr <GameSession>, MapIdHasher>& GetAllSessions () const {return game_sessions_;}
    void SetDefaultBagCapacity (uint num);
    int GetDefaultBagCapacity () const;

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
 
};
