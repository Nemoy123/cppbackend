#include "model.h"

#include <stdexcept>
#include <algorithm>

namespace model {
using namespace std::literals;

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

}  // namespace model
