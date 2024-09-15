#include "model.h"

#include <stdexcept>
#include <algorithm>
#include <random>

namespace model {
using namespace std::literals;

const Map::LootDescription& Map::GetLootDescription() const {
    return loot_description_;
}
Map::LootDescription& Map::GetLootDescription() {
    return loot_description_;
}
const std::vector <std::unique_ptr<Loot>>& Map::GetLootList () const  {
    return loot_list_;
}
std::vector <std::unique_ptr<Loot>>& Map::GetLootList () {
    return loot_list_;
}

bool Map::operator==(const Map& other) const {
    if (       id_ == other.id_ 
            && name_ == other.name_
            && roads_==other.roads_
            && buildings_== other.buildings_
            && warehouse_id_to_index_==other.warehouse_id_to_index_
            && offices_ == other.offices_
            && dogSpeed == other.dogSpeed
            && horiz_roads_ == other.horiz_roads_
            && vert_roads_ == other.vert_roads_
            && loot_description_ == other.loot_description_
            && bagCapacity == other.bagCapacity
            && loot_list_.size() == other.loot_list_.size()
        )
        {   
            for (auto i = 0; i < loot_list_.size(); ++i) {
                if ( *(loot_list_.at(i)) != *(other.loot_list_.at(i)) ) {
                    return false;
                }
            }
            return true;
        }

    return false;
}

void Map::AddOffice(Office office) {
    if (warehouse_id_to_index_.contains(office.GetId())) {
        throw std::invalid_argument("Duplicate warehouse");
    }

    const size_t index = offices_.size();
    Office& o = offices_.emplace_back(std::move(office));
    try {
        warehouse_id_to_index_.emplace(o.GetId(), index);
    } catch (...) {
        // Удаляем офис из вектора, если не удалось вставить в unordered_map
        offices_.pop_back();
        throw;
    }
}

void Map::SetSpeed (double speed) {
    if (speed > 0) {
        dogSpeed = speed;
    }
}

void Map::AddType (const LootType& type) {
        if (type.contains("name")) {
            std::string name_type = std::get<std::string>(type.at("name"));
            loot_description_[name_type] = type;
        }
}

const double Map::GetSpeed () const {
    return dogSpeed;
}

void Map::AddRoad(const Road& road) {
        roads_.emplace_back(road);
        
        if (road.IsHorizontal()) {
            double min = std::min(road.GetStart().x, road.GetEnd().x);
            double max = std::max(road.GetStart().x, road.GetEnd().x);

            horiz_roads_[road.GetStart().y].emplace_back(std::pair{min, max });

        } else {
            double min = std::min(road.GetStart().y, road.GetEnd().y);
            double max = std::max(road.GetStart().y, road.GetEnd().y);
            
            vert_roads_[road.GetStart().x].emplace_back(std::pair{min, max });
        }
        
}

void Map::AddLoot(int num) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(0, (GetLootDescription().size())-1);
    
    for (auto i =0; i < num; ++i) {
        auto loot = std::make_unique<Loot>();
        loot->type = distr(gen);
        std::uniform_int_distribution<> road_distr(0, ((GetRoads().size())-1));
        auto* road_ptr = &(GetRoads().at(road_distr(gen)));
        
        if (road_ptr->IsHorizontal()) {
            std::uniform_int_distribution<> road_coor(std::min(road_ptr->GetStart().x, road_ptr->GetEnd().x), 
                                                      std::max(road_ptr->GetStart().x, road_ptr->GetEnd().x));
            loot->y = road_ptr->GetStart().y;
            loot->x = road_coor(gen);
        } else {
            
            std::uniform_int_distribution<> road_coor(std::min(road_ptr->GetStart().y, road_ptr->GetEnd().y), 
                                                      std::max(road_ptr->GetStart().y, road_ptr->GetEnd().y));
            loot->x = road_ptr->GetStart().x;
            loot->y = road_coor(gen);
        }
        loot_list_.push_back(std::move(loot));
        loot_list_.back()->id = static_cast<int>(loot_list_.size()-1);
    }
    
}


}  // namespace model
