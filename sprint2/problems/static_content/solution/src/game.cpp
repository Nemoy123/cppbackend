#include "game.h"
#include <algorithm>

using namespace model;

// void ChangeFunc (const bool axis, const bool direction, const Map::RoadMap* first_container, 
//                 const Map::RoadMap* second_container, std::shared_ptr<Dog>& dog_ptr, const double time) {
//             //direction true - вправо или вниз двигаемся, в сторону увеличения.
//             // false - влево или вверх в сторону уменьшения

//             //найти дороги относительно направления (налево и направо горизонт)
//             //if (*direction == "R" || *direction == "L") { 
//                // const Map::RoadMap* hroad_ptr = &(session->GetMap()->GetHorizontalRoads());
//                  //найти начальные координаты пса и направление
//                 double* axis_d = nullptr;
//                 double* axis_test = nullptr;
//                 double* axis_speed = nullptr;
//                 if (axis) {
//                     axis_d = &dog_ptr->GetPosition().x;
//                     axis_speed = &dog_ptr->GetSpeed().x;
//                     axis_test = &dog_ptr->GetPosition().y;
//                 } else {
//                     axis_d = &dog_ptr->GetPosition().y;
//                     axis_speed = &dog_ptr->GetSpeed().y;
//                     axis_test = &dog_ptr->GetPosition().x;
//                 }
//                 // double x = dog_ptr->GetPosition().x;
//                 // double y = dog_ptr->GetPosition().y;
//                 //double test = y-0.4;
//                 const double error_rate = 0.000001;
//                 auto pos_min = first_container->lower_bound ((*axis_test - (0.4 + error_rate)));
                

//                 if (pos_min != first_container->end() && (pos_min->first - *axis_test < (0.4 + error_rate) )) {  // lower_bound нашел значение (мы на гориз дороге)
//                         //if (pos_max == std::next(pos_min, 1)) { // если один элемент
//                     for (const auto& [start, end] : pos_min->second) { // смотрим в векторе отрезок гориз подходящий нам 
//                         if (*axis_d >= (start - (0.4 + error_rate)) && *axis_d <= (end + (0.4 + error_rate)) ) {
//                             if (direction == true) {
//                                 if ((*axis_d + time * (*axis_speed)) > (end + 0.4)) {
//                                     *axis_d = (end + 0.4);
//                                     *axis_speed = 0;
//                                 } else {
//                                     *axis_d = (*axis_d + time * (*axis_speed));
//                                 }
//                                 //dog_ptr->GetPosition().x = std::min ((end + 0.4),(dog_ptr->GetPosition().x + time * dog_ptr->GetSpeed().x));
//                             } else {
//                                 if ((*axis_d + time * (*axis_speed)) < (start - 0.4)) { // 30.4 + 10*-4 = 
//                                     *axis_d = (start - 0.4);
//                                     *axis_speed = 0;
//                                 } else {
//                                     *axis_d = (*axis_d + time * (*axis_speed));
//                                 }

//                                 //dog_ptr->GetPosition().x = std::min ((start - 0.4),(dog_ptr->GetPosition().x - time * dog_ptr->GetSpeed().x));
//                             }
//                             return;
//                         }
//                     }
//                 }
//                // else { // lower_bound не нашел
//                     //const Map::RoadMap* vroad_ptr = &(session->GetMap()->GetVerticalRoads());
//                     pos_min = second_container->lower_bound ((*axis_test - (0.4 + error_rate)));
//                     //if (pos_min == second_container->end()) {throw ("Move Error");}
//                     if (pos_min != second_container->end() && (pos_min->first - *axis_test < (0.4 + error_rate) )) {
//                         if (direction == true) {
//                             if ((*axis_d + time * (*axis_speed)) > (pos_min->first + 0.4)) 
//                             {
//                                 *axis_d = (pos_min->first + 0.4);
//                                 *axis_speed = 0;
//                             } else {
//                                 *axis_d = (*axis_d + time * (*axis_speed));
//                             }
//                         //dog_ptr->GetPosition().x = std::min ((pos_min->first + 0.4),(dog_ptr->GetPosition().x + time * dog_ptr->GetSpeed().x));
//                         } else {
//                             if ((*axis_d - time * (*axis_speed)) > (pos_min->first - 0.4)) {
//                                 *axis_d = (pos_min->first - 0.4);
//                                 *axis_speed = 0;
//                             } else {
//                                 *axis_d = (*axis_d - time * (*axis_speed));
//                             }
//                             //dog_ptr->GetPosition().x = std::min ((pos_min->first - 0.4),(dog_ptr->GetPosition().x - time * dog_ptr->GetSpeed().x));
//                         }
//                         return;
//                     }
//                     //если мы здесь - двигаемся поперек дороги максимум на 0.4 от оси
//                     if (axis) { // ось горизонт двигаемся вниз D растет
//                         double* y_coord = &dog_ptr->GetPosition().y;
//                         pos_min = first_container->lower_bound ((*y_coord - (0.4 + error_rate)));
//                         if (direction) {
//                             if ((pos_min->first - *y_coord < (0.4 + error_rate) )) {
//                                 if ((*axis_d + time * (*axis_speed)) > (pos_min->first + 0.4)) {
//                                    *axis_d = (pos_min->first + 0.4);
//                                    *axis_speed = 0;
//                                 }
//                                 else {
//                                     *axis_d = (*axis_d + time * (*axis_speed));
//                                 }
//                                 return;
//                             }
//                         } else {
//                              if ((pos_min->first - *y_coord < (0.4 + error_rate) )) { 
//                                 if ((*axis_d + time * (*axis_speed)) < (pos_min->first - 0.4)) { 
//                                     *axis_d = (pos_min->first - 0.4);
//                                     *axis_speed = 0;
//                                 }
//                                 else {
//                                     *axis_d = (*axis_d + time * (*axis_speed));
//                                 }
//                                 return;
//                              }   
//                         }
//                     } else { // вертикальная ось двигаемся в горизонт
//                         double* x_coord = &dog_ptr->GetPosition().x;


//                     }
// }



void Game::TimeUpdate (const uint64_t time) { 
    // auto time_handler = [self = shared_from_this(), time] {
    //     if (self == nullptr) return;
        double time_seconds = ((double)time) / 1000;
        const double error_rate = 0.000001;
        for (auto& [map_id, session] : game_sessions_) { 
            for (auto& [dog_id, dog_ptr] : session->GetAllDogs()) { 
                const std::string* direction = &dog_ptr->GetDirection();
                if (*direction == "") {continue;}
                else if (*direction == "R" || *direction == "L")  {
                   // bool axis = true;
                    bool dir_bool = (*direction == "R") ? true : false;
                    const Map::RoadMap* first_container = &(session->GetMap()->GetHorizontalRoads());
                    const Map::RoadMap* second_container = &(session->GetMap()->GetVerticalRoads());
                    double* y_key = &dog_ptr->GetPosition().y;
                    double* x_key = &dog_ptr->GetPosition().x;
                    double* x_speed = &dog_ptr->GetSpeed().x;
                    auto pos_min = first_container->lower_bound ((*y_key - (0.4 + error_rate)));
                    if (pos_min != first_container->end() && ((pos_min->first - *y_key) < (0.4 + error_rate) )) {
                        for (const auto& [start, end] : pos_min->second) { // смотрим в векторе отрезок гориз подходящий нам 
                            if (*x_key >= (start - (0.4 + error_rate)) && *x_key <= (end + (0.4 + error_rate)) ) {
                                if (dir_bool == true) {
                                    if ((*x_key + time_seconds * (*x_speed)) > (end + (0.4 + error_rate))) {
                                        *x_key = (end + 0.4);
                                        *x_speed = 0;
                                    } 
                                    else {
                                        *x_key = (*x_key + time_seconds * (*x_speed));
                                    }
                                } 
                                else {
                                    if ((*x_key + time_seconds * (*x_speed)) < (start - (0.4 + error_rate))) { // 30.4 + 10*-4 = 
                                        *x_key = (start - 0.4);
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
                        pos_min = second_container->lower_bound ((*x_key - (0.4 + error_rate)));
                        if (pos_min != first_container->end() && ((pos_min->first - *x_key) < (0.4 + error_rate) )) { 
                            if (dir_bool) {
                                if ((*x_key + time_seconds * (*x_speed)) > (pos_min->first + 0.4)) {
                                    *x_key  = (pos_min->first + 0.4);
                                    *x_speed = 0;
                                }
                                else {
                                    *x_key  = (*x_key  + time_seconds * (*x_speed));
                                }
                                continue;
                            }
                            if (!dir_bool) {
                                if ((*x_key + time_seconds * (*x_speed)) < (pos_min->first - 0.4)) {
                                    *x_key  = (pos_min->first - 0.4);
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
                    bool axis = false;
                    bool dir_bool = (*direction == "D") ? true : false;
                    const Map::RoadMap* vert_container = &(session->GetMap()->GetVerticalRoads());
                    const Map::RoadMap* horiz_container = &(session->GetMap()->GetHorizontalRoads());
                    double* y_key = &dog_ptr->GetPosition().y;
                    double* x_key = &dog_ptr->GetPosition().x;
                    double* y_speed = &dog_ptr->GetSpeed().y;
                    auto pos_min = vert_container->lower_bound ((*x_key - (0.4 + error_rate)));
                    if (pos_min != vert_container->end() && ((pos_min->first - *x_key) < (0.4 + error_rate) )) {
                        for (const auto& [start, end] : pos_min->second) { // смотрим в векторе отрезок верктик подходящий нам 
                            if (*y_key >= (start - (0.4 + error_rate)) && *y_key <= (end + (0.4 + error_rate)) ) { 
                                if (dir_bool == true) {
                                    if ((*y_key + time_seconds * (*y_speed)) > (end + 0.4)) {
                                        *y_key = (end + 0.4);
                                        *y_speed = 0;
                                    } 
                                    else {
                                        *y_key = (*y_key + time_seconds * (*y_speed));
                                    }
                                } 
                                else {
                                    if ((*y_key + time_seconds * (*y_speed)) < (start - 0.4)) { // 30.4 + 10*-4 = 
                                        *y_key = (start - 0.4);
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
                            pos_min = horiz_container->lower_bound ((*y_key - (0.4 + error_rate)));
                            if (pos_min != horiz_container->end() && ((pos_min->first - *y_key) < (0.4 + error_rate) )) { 
                                if (dir_bool) {    
                                    if ((*y_key + time_seconds * (*y_speed)) > (pos_min->first + 0.4)) {
                                        *y_key  = (pos_min->first + 0.4);
                                        *y_speed = 0;
                                    }
                                    else {
                                        *y_key  = (*y_key  + time_seconds * (*y_speed));
                                    }
                                    continue;
                                }
                                if (!dir_bool) {
                                    if ((*y_key + time_seconds * (*y_speed)) < (pos_min->first - 0.4)) {
                                        *y_key  = (pos_min->first - 0.4);
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
}



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
    playerptr->SetDog (game_sessions_.at(Map::Id {map_name})->GetDog(id_dog));  
    // получить токен
    detail::Token token_for_player = players_tokens_->GiveTokenNewPlayer (playerptr);
    Point point = game_sessions_[Map::Id {map_name}]->GetMap()->GetRoads().at(0).GetStart();
    if (!randomize_dog_start_) {
        game_sessions_[Map::Id {map_name}]->GetDog(id_dog)->SetPosition(Pos{(double)point.x, (double)point.y});
    } else {
        std::random_device rd;
        int roads_num = game_sessions_[Map::Id {map_name}]->GetMap()->GetRoads().size();
        std::uniform_int_distribution<int> dist(0, roads_num);
        const Road* road = &(game_sessions_[Map::Id {map_name}]->GetMap()->GetRoads().at(dist(rd)));
        
        
        if (road->IsHorizontal()) {
            int start_coord_x = road->GetStart().x;
            int end_coord_x = road->GetEnd ().x;
            int coord_y = road->GetStart().y;
            std::uniform_int_distribution<int> coord_func(start_coord_x, end_coord_x);
            game_sessions_[Map::Id {map_name}]->GetDog(id_dog)->SetPosition(Pos{(double)coord_func(rd), (double)coord_y});
        } else {
            int start_coord_y = road->GetStart().y;
            int end_coord_y = road->GetEnd ().y;
            int coord_x = road->GetStart().x;
            std::uniform_int_distribution<int> coord_func(start_coord_y, end_coord_y);
            game_sessions_[Map::Id {map_name}]->GetDog(id_dog)->SetPosition(Pos{(double)coord_x, (double)coord_func(rd)});
        }
        
    }
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