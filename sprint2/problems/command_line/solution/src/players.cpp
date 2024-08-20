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