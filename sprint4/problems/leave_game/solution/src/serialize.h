#pragma once
#include <chrono>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/unique_ptr.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/base_object.hpp>
#include <filesystem>

#include "game.h"
#include "geom.h"
#include "template.h"
#include "loot_generator.h"

using namespace std::chrono;



class ApplicationListener {
    public:
    virtual ~ApplicationListener(){};
    virtual void OnTick() = 0;
    virtual void OnTick(uint64_t time) = 0;
};


namespace serialization {

using InputArchive = boost::archive::text_iarchive;
using OutputArchive = boost::archive::text_oarchive;
using namespace loot_gen;

template <typename Archive>
void serialize(Archive& ar, Size& sz, [[maybe_unused]] const unsigned version) {
    ar& sz.width;
    ar& sz.height;
    
}

template <typename Archive>
void serialize(Archive& ar, Rectangle& pl, [[maybe_unused]] const unsigned version) {
    ar& pl.position;
    ar& pl.size;
}

class TokenRepr {
    public:
        TokenRepr(){}
        explicit TokenRepr (const detail::Token& token)
                : id_dog_(token.id_dog_)
                , token_string_(token.token_string_)
                {}

        template <typename Archive>
        void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
            ar& id_dog_;
            ar& token_string_;
            
        }
        [[nodiscard]] Token Restore() const {
            return Token {id_dog_, token_string_};
        }
    
    private:
        uint64_t id_dog_{};
        std::string token_string_{};
};

class LootRepr {

    public:
        LootRepr(){}
        explicit LootRepr (const Loot& loot)
                : type(loot.type)
                , x (loot.x)
                , y (loot.y)
                , id (util::UUIDToString(loot.id))
                {}

        template <typename Archive>
        void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
            ar& type;
            ar& x;
            ar& y;
            ar& id;
            
        }
        [[nodiscard]] Loot Restore() const {
            return Loot {type,x,y, util::UUIDFromString(id)};
        }
    
    private:
        int type = 0;
        double x = 0.0;
        double y = 0.0;
        std::string id {};
};


class RoadRepr {
    public:
        RoadRepr(){}
        explicit RoadRepr (const model::Road& road)
                : start_x(road.GetStart().x)
                , start_y(road.GetStart().y)
                , end_x(road.GetEnd().x)
                , end_y(road.GetEnd().y)
                {}

        template <typename Archive>
        void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
            ar& start_x;
            ar& start_y;
            ar& end_x;
            ar& end_y;
            
        }
        [[nodiscard]] Road Restore() const {
            return Road {Point{start_x, start_y}, Point{end_x,end_y}};
        }
    
    private:
        int start_x;
        int start_y;
        int end_x;
        int end_y;
};

class OfficeRepr {
    public:
        OfficeRepr(){}
        explicit OfficeRepr (const Office& office)
                : id_(*office.GetId())
                , position_x_(office.GetPosition().x)
                , position_y_(office.GetPosition().y)
                , offset_x_(office.GetOffset().dx)
                , offset_y_(office.GetOffset().dy)
                {}

        template <typename Archive>
        void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
            ar& id_;
            ar& position_x_;
            ar& position_y_;
            ar& offset_x_;
            ar& offset_y_;
            
        }
        [[nodiscard]] Office Restore() const {
            return Office {Office::Id{id_}, Point{position_x_, position_y_}
                                , Offset{offset_x_, offset_y_}};
        }
    
    private:
        std::string id_;
        int position_x_;
        int position_y_;
        int offset_x_;
        int offset_y_;
};

class BuildingRepr {
    public:
        BuildingRepr(){}
        explicit BuildingRepr (const Building& building)
                : position_x(building.GetBounds().position.x)
                , position_y(building.GetBounds().position.y)
                , size_w (building.GetBounds().size.width)
                , size_h (building.GetBounds().size.height)
                {}

        template <typename Archive>
        void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
            ar& position_x;
            ar& position_y;
            ar& size_w;
            ar& size_h;
        }

        [[nodiscard]] Building Restore() const {
            Rectangle rec = {Point {position_x, position_y}
                            , Size{size_w, size_h} };
            return Building {rec};
        }
    
    private:
        int position_x;
        int position_y;
        int size_w;
        int size_h;
};

class DogRepresentation {
    public:
    DogRepresentation(){}
    explicit DogRepresentation (const Dog& dog)
                : name_(dog.GetName())
                , id_(dog.GetId())
                , posx_(dog.GetPosition().x)
                , posy_(dog.GetPosition().y)
                , speedx_(dog.GetSpeed().x)
                , speedy_(dog.GetSpeed().y)
                , dir_(dog.GetDirection())
                , score(dog.GetScore())
                , wait_time_(dog.GetTimeWaiting().count())
                , total_play_time_(dog.GetTimePlaying().count())
                {
                    loot_bag_.reserve(dog.GetLootBag().size());
                    for (const auto& loot : dog.GetLootBag()) {
                        loot_bag_.push_back (LootRepr{*loot});
                    }
                }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar& name_;
        ar& id_;
        ar& posx_;
        ar& posy_;
        ar& speedx_;
        ar& speedy_;
        ar& dir_;
        ar& loot_bag_;
        ar& score;
        ar& wait_time_;
        ar& total_play_time_;

    }
    [[nodiscard]] Dog Restore() const {
        Dog dog{name_};
        dog.SetId (id_);
        dog.SetPosition (Pos{posx_, posy_});
        dog.SetSpeed (Speed{speedx_,speedy_});
        dog.SetDirection (dir_);
        dog.SetScore (score);
        for (const auto& loot : loot_bag_) {
            auto loot_new = std::make_unique<Loot>(loot.Restore());
            dog.AddLoot (std::move(loot_new));
        }
        dog.SetTimeWaiting (std::chrono::milliseconds{wait_time_});
        dog.SetTimePlaying (std::chrono::milliseconds{total_play_time_});
        return dog;
    }
    private:
        std::string name_{};
        uint64_t id_ = 0;
        double posx_;
        double posy_;
        double speedx_;
        double speedy_;
        std::string dir_{"U"};
        std::vector <LootRepr> loot_bag_{};
        size_t score;
        uint64_t wait_time_;
        uint64_t total_play_time_;

};


class PlayerRepr {
    public:
        PlayerRepr(){}
        explicit PlayerRepr (const Player& player)
                    : dog_(DogRepresentation{*(player.GetDogPtr())})
                    {}
        
        template <typename Archive>
        void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
            ar& dog_;
        }
        [[nodiscard]] Player Restore() const {
            
            Player player;
            player.SetDog (std::make_shared<Dog>(dog_.Restore()));
            return player;
        }
    private:
        DogRepresentation dog_;
};

class PlayersRepresentation {
    public:
        PlayersRepresentation(){}
        explicit PlayersRepresentation (const Players& pl)
                {
                    all_pl.reserve(pl.GetMapPlayers().size());
                    for (const auto& [par, player] : pl.GetMapPlayers()) { 
                        all_pl.push_back(std::pair{par.first, par.second});
                    }
                }
        
        template <typename Archive>
        void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
            ar& all_pl;
        }
        [[nodiscard]] Players Restore() const {
            Players players;
            for (const auto& [dog_id, map_id] : all_pl) {
                players.AddPlayer (dog_id, map_id);
            }
            return players;
        }

    private:
        std::vector <std::pair <uint64_t, std::string> > all_pl;
          
};


class LootTypeRepr {
    public:
        LootTypeRepr(){}
        explicit LootTypeRepr (const Map::LootType& type)
            {
                for (const auto& [name, var] : type){
                    std::variant<bool, std::string, double, int> temp;
                    if (std::holds_alternative<std::monostate>(var)) {
                        temp = bool{true};
                        cont_.push_back(std::pair{name, std::move(temp)});
                    } 
                    else if (std::holds_alternative<std::string>(var)){
                        temp = std::get<std::string>(var);
                        cont_.push_back(std::pair{name, std::move(temp)});
                    }
                    else if (std::holds_alternative<double>(var)){
                        temp = std::get<double>(var);
                        cont_.push_back(std::pair{name, std::move(temp)});
                    }
                    else if (std::holds_alternative<int>(var)){
                        temp = std::get<int>(var);
                        cont_.push_back(std::pair{name, std::move(temp)});
                    }
                }
            }
        
        template <typename Archive>
        void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
            ar& cont_;
            //ar& null_monostate;
        }
        [[nodiscard]] Map::LootType Restore() const {
            Map::LootType result;
            for (const auto& [name, var] : cont_){
                if (std::holds_alternative<bool>(var)) {
                    result[name] = std::monostate{};
                } 
                else if (std::holds_alternative<std::string>(var)){
                    result[name] = std::get<std::string>(var);
                }
                else if (std::holds_alternative<double>(var)){
                    result[name] = std::get<double>(var);
                }
                else if (std::holds_alternative<int>(var)){
                    result[name] = std::get<int>(var);
                }
                else {
                    throw ("Bad variant");
                }
                
            }
            return result;
        }
    private:
        std::vector <std::pair <std::string, 
                    std::variant<bool, std::string, double, int>>> cont_;
        //bool null_monostate = false;
};

class PlayersTokensRepresentation {
    public:
        PlayersTokensRepresentation(){}
        explicit PlayersTokensRepresentation (const PlayersTokens& tokens)
            {
                for (const auto [tok, player] : tokens.GetMapTokens()) {
                    map_tokens_.push_back(std::pair{ TokenRepr{tok}, PlayerRepr{*player} });
                }
            }
        
        template <typename Archive>
        void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
            ar& map_tokens_;
        }
        [[nodiscard]] PlayersTokens Restore() const {
            using TMap = std::unordered_map <detail::Token, std::shared_ptr<Player>, Hash>;
            TMap temp;
            for (const auto& [tok, pl]: map_tokens_) {
                temp[tok.Restore()] = std::make_shared<Player> (pl.Restore());
            }
            return PlayersTokens{temp};
        }

    private:
        std::vector<std::pair<TokenRepr, PlayerRepr>> map_tokens_;   
};

class MapRepresentation {
    
    public:
    MapRepresentation(){}
    explicit MapRepresentation (const model::Map& gamemap);

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar& id_;
        ar& name_;
        ar& roads_;
        ar& buildings_;
        ar& offices_;
        ar& dogSpeed;
        ar& loot_description_;
        ar& loot_list_;
        ar& bagCapacity;
    }

    [[nodiscard]] Map Restore() const;


    private:
    std::string id_;
    std::string name_;
    std::vector <RoadRepr> roads_;
    std::vector <BuildingRepr> buildings_;
    std::vector <OfficeRepr> offices_;
    double dogSpeed = -1;
    std::vector <std::pair<std::string, LootTypeRepr>> loot_description_; 
    std::map <std::string, LootRepr> loot_list_;
    int bagCapacity = -1;
};


class GameSessionRepresentation {
    public:
    GameSessionRepresentation(){}
    explicit GameSessionRepresentation(const GameSession& session)
                : gamemap_ (*(session.GetMap()->GetId()))
                {
                    dogs_.reserve (session.GetAllDogs().size());
                    for (const auto& [id, dog_ptr] : session.GetAllDogs()) {
                        dogs_.push_back (std::pair{id, DogRepresentation{*dog_ptr}});
                    }
                }
    
    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar& dogs_;
        ar& gamemap_;

    }

    const Map::Id GetId() const {return Map::Id{gamemap_};}

    [[nodiscard]] GameSession Restore() const {
        GameSession sess;
        std::unordered_map <uint64_t, std::shared_ptr <Dog>> temp;
        for (const auto& [id, dog]:dogs_)  {
           temp[id] = std::make_shared<Dog> (dog.Restore());
        }
        sess.GetAllDogs() = std::move(temp);
        return sess;
    }

    private:
        std::vector <std::pair<uint64_t, DogRepresentation>> dogs_;
        std::string gamemap_;

};


class GameRepresentation {
    using MapIdHasher = util::TaggedHasher<Map::Id>;
public:
     GameRepresentation(Strand strand): time_strand_(strand){}   

     explicit GameRepresentation(Strand strand, const Game& game);
     
    [[nodiscard]] Game Restore() const;

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar& maps_;
        ar& game_sessions_;
        ar& players;
        ar& players_tokens_;
        ar& defaultDogSpeed;
        ar& randomize_dog_start_;
        ar& defaultBagCapacity;
        ar& dogRetirementTime_;
    }

private:

    std::vector<MapRepresentation> maps_;
    std::vector <GameSessionRepresentation> game_sessions_; // ключ имя карты
    PlayersRepresentation players; // все игроки
    PlayersTokensRepresentation players_tokens_; // токены
    Strand time_strand_;
    double defaultDogSpeed = 1.0;
    bool randomize_dog_start_ = false;
    uint defaultBagCapacity = 3;
    uint dogRetirementTime_ = 60000;
};


class LootGeneratorRepresentation {
    public:
        
        LootGeneratorRepresentation(){}
        explicit LootGeneratorRepresentation (const LootGenerator& gen)
                        : base_interval_(gen.GetBaseInterval().count())
                        , probability_ (gen.GetProbability())
            {}
        
        template <typename Archive>
        void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
            ar& base_interval_;
            ar& probability_;
        }
        [[nodiscard]] LootGenerator Restore() const {
            return LootGenerator{std::chrono::milliseconds {base_interval_}, probability_};
        }

    private:
        std::uint64_t base_interval_;
        double probability_;     
};


class Serialize : public ApplicationListener {
    public:
        enum class Mode {
            NONE,
            ONLY_EXIT,
            BY_TIME,
        };
        static Serialize& getInstance() {
            static Serialize i_;
            return i_;
        }
        
        void Init (Strand& strand, std::filesystem::path& path, uint64_t saving_period, Mode& mode, LootGenerator* loot_gen_ = nullptr);
        void OnTick(uint64_t time) override;
        void OnTick() override;
        const Mode GetMode() const {return mode_;}
        void SaveAll();
        std::optional<std::pair<Game, LootGenerator>> Restore();
        void SetGamePtr (Game& game);
        void SetLootGenPTR(LootGenerator& loot_gen); 
        const uint64_t GetSerializePeriod () const;
        Game* GetGamePtr (); 
        
    private:
        Serialize() = default;
        ~Serialize() = default;
        Serialize( const Serialize&) = delete;
        Serialize operator=(const Serialize&) = delete;

        Strand* strand_;
        std::filesystem::path path_;
        uint64_t saving_period_;
        Game* game_ = nullptr; 
        uint64_t time_since_last_save_ = 0; 
        time_point<steady_clock> real_time_last_save_{steady_clock::now()};
        Mode mode_;
        std::mutex mut;
        LootGenerator* loot_gen_;
        
};



}