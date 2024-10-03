#include "players.h"

uint64_t Dog::static_id = 0;

Token PlayersTokens::GiveTokenNewPlayer (std::shared_ptr<Player> player) {
    

    std::string token_new_player = token_generator_->GenerateToken();
    auto tst2 = detail::Token(player->GetDogPtr()->GetId(), token_new_player);
    
    map_tokens_[tst2] = player;
    return tst2;
}

std::shared_ptr<Player> PlayersTokens::FindPlayerByToken (Token& token) const {
    auto playerptr = map_tokens_.find(token);
    if (playerptr != map_tokens_.cend()) {
        return map_tokens_.at(token);
    } 
    return nullptr;
}

std::shared_ptr<Player> PlayersTokens::FindPlayerByTokenString (const std::string& token) const {
    for  (const auto& [tok, pla_ptr]: map_tokens_) {
        if (tok.token_string_ == token) return pla_ptr;
    }
    return nullptr;
}

void Dog::SetSpeed(Speed&& speed) {
    speed_ = std::move(speed);
}

bool Dog::operator==(const Dog& other) const {
    if (    name_ == other.name_ 
         && id_ == other.id_
         && pos_ == other.pos_
         && speed_ == other.speed_
         && dir_ == other.dir_
         && score == other.score
         && loot_bag_.size() == other.loot_bag_.size()
        ) {
            for (auto i = 0; i < loot_bag_.size(); ++i) {
                if (*(loot_bag_.at(i)) != *(other.loot_bag_.at(i))) return false;
            }
            return true;
        }
    return false;
}

std::vector <std::unique_ptr<Loot>>& Dog::GetLootBag () {
    return loot_bag_;
}
const std::vector <std::unique_ptr<Loot>>& Dog::GetLootBag () const {
    return loot_bag_;
}
void Dog::AddLoot (std::unique_ptr<Loot> loot) {
    loot_bag_.push_back(std::move(loot));
}

Pos& Dog::GetPosition ()  {return pos_;}
const Pos& Dog::GetPosition () const  {return pos_;}
Speed& Dog::GetSpeed () {return speed_;}
const Speed& Dog::GetSpeed () const {return speed_;}
std::string& Dog::GetDirection()  {return dir_;}
const std::string& Dog::GetDirection() const {return dir_;}
void Dog::SetDirection(std::string&& dir)  {dir_ = std::move(dir);}
void Dog::SetDirection(const std::string& dir)  {dir_ = dir;}

bool PlayersTokens::DeleteTokenByPlayer(std::shared_ptr<Player> player) {
    
    for (auto it = map_tokens_.begin(); it != map_tokens_.end(); it = std::next(it, 1)) {
        if (it->second->GetDogIdString() == player->GetDogIdString()) {
            map_tokens_.erase (it);
            return true;
        }
    }
    return false;
}

size_t& Dog::GetScore () {return score;}
const size_t& Dog::GetScore () const {return score;}
void Dog::SetScore (int num) {score = num;}


bool Dog::CheckWaiting() {
    if (dir_.empty()) return true;
    //if (wall_) return true;
    return false;
}

void Dog::SetTimeWaiting (std::chrono::milliseconds update_time) {
    wait_time_= update_time;
}

const std::chrono::milliseconds& Dog::GetTimeWaiting () const {
    return wait_time_;
}

void Dog::SetTimePlaying (std::chrono::milliseconds update_time) {
    total_play_time_ = update_time;
}
const std::chrono::milliseconds& Dog::GetTimePlaying () const {
    return total_play_time_;
}