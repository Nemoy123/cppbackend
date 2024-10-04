#include "game.h"
#include <algorithm>


using namespace model;

void Game::SetRetireTime (std::chrono::milliseconds time) {
    dogRetirementTime_ = time;
}

const std::chrono::milliseconds& Game::GetRetireTime () const {
    return dogRetirementTime_;
}

void Game::Retire(std::shared_ptr <Dog> dog) {
    Record info;
    info.id = NewUUID();
    info.name = dog->GetName();
    info.score = dog->GetScore();
    info.play_time = dog->GetTimePlaying().count();
    {
        auto tranz = usecases_->SaveRecord ();
        auto ans = tranz->SaveRecord (std::move(info));
        tranz->Commit();
    }
    auto map_iterator = players->FindByDogID(dog->GetId());
    players_tokens_->DeleteTokenByPlayer (map_iterator->second);
    auto sess = GetSession (model::Map::Id{map_iterator->first.second});
    uint64_t dog_id = map_iterator->first.first;
    players->GetMapPlayers().erase (map_iterator);
    if (!sess->DeleteDog (dog_id)) {
        throw ("Error Deleting Dog " + std::to_string(dog_id));
    }
    
}   

void Game::SetBase(Base& base) {
    base_ = &base;
    usecases_ = std::make_shared<UseCasesImpl> (*base_);
}

std::shared_ptr<GameSession> Game::GetSession (const Map::Id& id) const {
    return game_sessions_.at(id);
}
const std::unordered_map <Map::Id, std::shared_ptr <GameSession>, Game::MapIdHasher>& Game::GetAllSessions () const {
    return game_sessions_;
}
    
void Game::SetRandomize(bool random) {
    randomize_dog_start_ = random;
}
 void Game::SetPtrPlayersToken (const PlayersTokens& pl_tok) {
    players_tokens_ = std::make_unique<PlayersTokens>(pl_tok);
}
void Game::SetAllSessions (std::unordered_map <Map::Id, std::shared_ptr <GameSession>, MapIdHasher>&& sess) {
    game_sessions_=std::move(sess);
}
void Game::SetPlayers (std::shared_ptr <Players> pl) {
    players = pl;
}
Strand Game::GetStrand() const {
    return time_strand_;
}
bool Game::GetRandomize() const {
    return randomize_dog_start_;
}
PlayersTokens* Game::GetAllPLayersTokensPtr () const {
        return players_tokens_.get();
}

void SetWaitForDog(Game* game, std::shared_ptr <Dog> dog_ptr, 
                const uint64_t time, std::vector <std::shared_ptr <Dog>>& delete_vector) {
    dog_ptr->SetTimeWaiting(dog_ptr->GetTimeWaiting() + std::chrono::milliseconds{time});
    const std::chrono::milliseconds total_time = dog_ptr->GetTimeWaiting();
    if (total_time >= game->GetRetireTime ()) {
        //game->Retire(dog_ptr);
        delete_vector.push_back (dog_ptr);
        
    }                 
}

void Game::TimeUpdate (const uint64_t time) { 
        
        std::unordered_map <std::shared_ptr <GameSession>, CollisionPrepare> map_collision;
        
        for (const auto& [id, session] : GetAllSessions ()) {
            auto collision = session->GetCollisionPrepare();
            
            map_collision[session] = std::move(collision);
        }
        
        double time_seconds = (static_cast<double>(time) / 1000);
        const double error_rate = 0.000001;
        const double max_road_offset = 0.4;
        std::vector <std::shared_ptr <Dog>> delete_vector;
        for (auto& [map_id, session] : game_sessions_) { 
            if (session->GetAllDogs().empty()) continue;
            for (auto& [dog_id, dog_ptr] : session->GetAllDogs()) { 
                dog_ptr->SetTimePlaying (dog_ptr->GetTimePlaying() + std::chrono::milliseconds{time});
                const std::string* direction = &dog_ptr->GetDirection();
                if (*direction == "" || (dog_ptr->GetSpeed().x == 0 && dog_ptr->GetSpeed().y == 0 )) {
                    SetWaitForDog (this, dog_ptr, time, delete_vector);
                    continue;
                }
                else if (*direction == "R" || *direction == "L")  {
                    dog_ptr->SetTimeWaiting (0ms);
                    bool is_growing_direction = (*direction == "R") ? true : false; // тернарный оператор эфективнее присваивания через if-else
                    const Map::RoadMap* first_container = &(session->GetMap()->GetHorizontalRoads());
                    const Map::RoadMap* second_container = &(session->GetMap()->GetVerticalRoads());
                    double* y_key = &dog_ptr->GetPosition().y;
                    double* x_key = &dog_ptr->GetPosition().x;
                    double* x_speed = &dog_ptr->GetSpeed().x;
                    
                    auto pos_min = first_container->lower_bound ((*y_key - (max_road_offset + error_rate)));
                    if (pos_min != first_container->end() && ((pos_min->first - *y_key) < (max_road_offset + error_rate) )) {
                        for (const auto& [start, end] : pos_min->second) { // смотрим в векторе отрезок гориз подходящий нам 
                            if (*x_key >= (start - (max_road_offset + error_rate)) && *x_key <= (end + (max_road_offset + error_rate)) ) {
                                if (is_growing_direction == true) {
                                    if ((*x_key + time_seconds * (*x_speed)) > (end + (max_road_offset + error_rate))) {
                                        *x_key = (end + max_road_offset);
                                        *x_speed = 0;
                                    } 
                                    else {
                                        *x_key = (*x_key + time_seconds * (*x_speed));
                                    }
                                } 
                                else {
                                    if ((*x_key + time_seconds * (*x_speed)) < (start - (max_road_offset + error_rate))) { 
                                        *x_key = (start - max_road_offset);
                                        *x_speed = 0;
                                    } 
                                    else {
                                        *x_key = (*x_key + time_seconds * (*x_speed));
                                    }
                                }
                                continue;
                            }
                        } 
                    }
                    else {
                        // двигаемся поперек
                        dog_ptr->SetTimeWaiting (0ms);
                        pos_min = second_container->lower_bound ((*x_key - (max_road_offset + error_rate)));
                        if (pos_min != first_container->end() && ((pos_min->first - *x_key) < (max_road_offset + error_rate) )) { 
                            if (is_growing_direction) {
                                if ((*x_key + time_seconds * (*x_speed)) > (pos_min->first + max_road_offset)) {
                                    *x_key  = (pos_min->first + max_road_offset);
                                    *x_speed = 0;
                                }
                                else {
                                    *x_key  = (*x_key  + time_seconds * (*x_speed));
                                }
                                continue;
                            }
                            else {
                                if ((*x_key + time_seconds * (*x_speed)) < (pos_min->first - max_road_offset)) {
                                    *x_key  = (pos_min->first - max_road_offset);
                                    *x_speed = 0;
                                }
                                else {
                                    *x_key  = (*x_key  + time_seconds * (*x_speed));
                                }
                                continue;   
                            }
                        }

                    }
                }
                else if (*direction == "D" || *direction == "U") {
                    dog_ptr->SetTimeWaiting (0ms);
                    bool axis = false;
                    bool is_growing_direction = (*direction == "D") ? true : false;
                    const Map::RoadMap* vert_container = &(session->GetMap()->GetVerticalRoads());
                    const Map::RoadMap* horiz_container = &(session->GetMap()->GetHorizontalRoads());
                    double* y_key = &dog_ptr->GetPosition().y;
                    double* x_key = &dog_ptr->GetPosition().x;
                    double* y_speed = &dog_ptr->GetSpeed().y;
                    auto pos_min = vert_container->lower_bound ((*x_key - (max_road_offset + error_rate)));
                    if (pos_min != vert_container->end() && ((pos_min->first - *x_key) < (max_road_offset + error_rate) )) {
                        for (const auto& [start, end] : pos_min->second) { // смотрим в векторе отрезок верктик подходящий нам 
                            if (*y_key >= (start - (max_road_offset + error_rate)) && *y_key <= (end + (max_road_offset + error_rate)) ) { 
                                if (is_growing_direction == true) {
                                    if ((*y_key + time_seconds * (*y_speed)) > (end + max_road_offset)) {
                                        *y_key = (end + max_road_offset);
                                        *y_speed = 0;
                                    } 
                                    else {
                                        *y_key = (*y_key + time_seconds * (*y_speed));
                                    }
                                } 
                                else {
                                    if ((*y_key + time_seconds * (*y_speed)) < (start - max_road_offset)) { 
                                        *y_key = (start - max_road_offset);
                                        *y_speed = 0;
                                    } 
                                    else {
                                        *y_key = (*y_key + time_seconds * (*y_speed));
                                    }
                                }
                                continue;
                            }
                        }
                    }
                    else {
                            // двигаемся поперек
                            dog_ptr->SetTimeWaiting (0ms);
                            pos_min = horiz_container->lower_bound ((*y_key - (max_road_offset + error_rate)));
                            if (pos_min != horiz_container->end() && ((pos_min->first - *y_key) < (max_road_offset + error_rate) )) { 
                                if (is_growing_direction) {    
                                    if ((*y_key + time_seconds * (*y_speed)) > (pos_min->first + max_road_offset)) {
                                        *y_key  = (pos_min->first + max_road_offset);
                                        *y_speed = 0;
                                    }
                                    else {
                                        *y_key  = (*y_key  + time_seconds * (*y_speed));
                                    }
                                    continue;
                                }
                                else {
                                    if ((*y_key + time_seconds * (*y_speed)) < (pos_min->first - max_road_offset)) {
                                        *y_key  = (pos_min->first - max_road_offset);
                                        *y_speed = 0;
                                    }
                                    else {
                                        *y_key  = (*y_key  + time_seconds * (*y_speed));
                                    }
                                    continue;  
                                }
                            }

                    }


                }
            }
        }

    for (auto& [sess, prep]: map_collision) {
        if (sess)  {
            sess->HandlerCollision (prep);
            
        }
    }

    int i = delete_vector.size()-1;
    while (!delete_vector.empty()) {
        Retire(delete_vector.at(i));
        delete_vector.pop_back();
        --i;
    }
    

}

const Map* Game::FindMap(const Map::Id& id) const  noexcept {
    if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
        return &maps_.at(it->second);
    }
    return nullptr;
}
Map* Game::FindMap(const Map::Id& id) noexcept {
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

std::shared_ptr<GameSession> Game::AddSession (Map* map) {
    if (map == nullptr) throw std::invalid_argument ("Can't find this map");
    game_sessions_[map->GetId()] = std::make_shared<GameSession>();
    game_sessions_[map->GetId()]->SetMap(map);
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
        auto iter_maps = std::find_if (maps_.begin(), maps_.end(), [&](const auto& map){
            return *(map.GetId()) == map_name;
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
    playerptr->SetDog (game_sessions_.at(Map::Id {map_name})->GetDog(id_dog));  
    // получить токен
    detail::Token token_for_player = players_tokens_->GiveTokenNewPlayer (playerptr);
    Point point = game_sessions_[Map::Id {map_name}]->GetMap()->GetRoads().at(0).GetStart();
    if (!randomize_dog_start_) {
        game_sessions_[Map::Id {map_name}]->GetDog(id_dog)->SetPosition(Pos{static_cast<double>(point.x), static_cast<double>(point.y)});
    } else {
        std::random_device rd;
        int roads_num = game_sessions_[Map::Id {map_name}]->GetMap()->GetRoads().size();
        std::uniform_int_distribution<int> dist(0, roads_num-1);
        const Road* road = &(game_sessions_[Map::Id {map_name}]->GetMap()->GetRoads().at(dist(rd)));
        
        
        if (road->IsHorizontal()) {
            int start_coord_x = road->GetStart().x;
            int end_coord_x = road->GetEnd ().x;
            int coord_y = road->GetStart().y;
            std::uniform_int_distribution<int> coord_func(start_coord_x, end_coord_x);
            game_sessions_[Map::Id {map_name}]->GetDog(id_dog)->SetPosition(Pos{static_cast<double>(coord_func(rd)), static_cast<double>(coord_y)});
        } else {
            int start_coord_y = road->GetStart().y;
            int end_coord_y = road->GetEnd ().y;
            int coord_x = road->GetStart().x;
            std::uniform_int_distribution<int> coord_func(start_coord_y, end_coord_y);
            game_sessions_[Map::Id {map_name}]->GetDog(id_dog)->SetPosition(Pos{static_cast<double>(coord_x), static_cast<double>(coord_func(rd))});
        }
        
    }
    return std::move(token_for_player);
}

const std::shared_ptr <Players>& Game::GetPlayersPtr () const {
    return players;
}

std::shared_ptr <Players>& Game::GetPlayersPtr () {
    return players;
}

std::shared_ptr<Player> Game::FindPlayer (std::string& token) const  {
    return players_tokens_->FindPlayerByTokenString (token);
}

std::vector<std::string> Game::FindPlayersOnMap (const Map::Id& id, const bool names) const {
    if (!game_sessions_.contains(id))  return {};
    std::vector<std::string> result;
    result.reserve (game_sessions_.at (id)->GetAllDogs().size());
    for (const auto& [dog_id, dog_ptr] : game_sessions_.at (id)->GetAllDogs()) {
        if (names) {result.push_back(dog_ptr->GetName());}
        else {result.push_back(std::to_string (dog_id));}
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

void Game::SetDefaultSpeed (double speed) {
    defaultDogSpeed = speed;
}

const double Game::GetDefaultSpeed () const {
    return defaultDogSpeed;
}

void Game::SetDefaultBagCapacity (uint num) {
    defaultBagCapacity = num;
}

int Game::GetDefaultBagCapacity () const {
    return defaultBagCapacity;
}