#pragma once

#include <cstdint>
#include <cstddef>
#include "model.h"
#include "gamesession.h"
#include "players.h"

using namespace model;
using namespace std::literals;

class Game {
public:
    using Maps = std::vector<Map>;

    void AddMap(Map&& map);
    std::shared_ptr<GameSession> AddSession (const Map* map);
    std::shared_ptr<GameSession> GetSession (const Map::Id& id) const {return game_sessions_.at(id);}
    detail::Token AddPlayer (std::string& user_name, std::string& map_name);
    std::shared_ptr<Player> FindPlayer (std::string& token) const ;
    Map::Id FindPlayerMap (const std::shared_ptr <Player>& player) const;
    const Maps& GetMaps() const noexcept ;
    const std::shared_ptr <Players>& GetPlayersPtr () const;
    //const std::unique_ptr <PlayersTokens>& GetPlayersTokensPtr () const;
    const Map* FindMap(const Map::Id& id) const noexcept ;
    std::vector<std::string> FindPlayersOnMap (const Map::Id& id, const bool names = true) const;

private:
    using MapIdHasher = util::TaggedHasher<Map::Id>;
    using MapIdToIndex = std::unordered_map<Map::Id, std::size_t, MapIdHasher>;

    std::vector<Map> maps_;
    MapIdToIndex map_id_to_index_;
    //std::unordered_map <std::string, GameSession> game_sessions_; // ключ имя карты
    std::unordered_map <Map::Id, std::shared_ptr <GameSession>, MapIdHasher> game_sessions_; // ключ имя карты
    std::shared_ptr <Players> players = std::make_shared<Players> (); // все игроки
    std::unique_ptr <PlayersTokens> players_tokens_ = std::make_unique<PlayersTokens> (); // токены
};
