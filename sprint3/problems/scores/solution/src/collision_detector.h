#pragma once

#include "geom.h"

#include <algorithm>
#include <vector>
#include <optional>

namespace collision_detector {

struct CollectionResult {
    // Квадрат расстояния до точки
    double sq_distance;
    // Доля пройденного отрезка
    double proj_ratio;
    bool IsCollected(double collect_radius) const {
        return proj_ratio >= 0 && proj_ratio <= 1 && sq_distance <= collect_radius * collect_radius;
    }
};

// Движемся из точки a в точку b и пытаемся подобрать точку c
std::optional <CollectionResult> TryCollectPoint(geom::Point2D a, geom::Point2D b, geom::Point2D c);

enum class Type {
    ITEM,
    BASE
};

struct Item {
    geom::Point2D position;
    double width;
    Type type = Type::ITEM;
};

struct Gatherer {
    geom::Point2D start_pos;
    geom::Point2D end_pos;
    double width;
    int id = -1;
};

class ItemGathererProvider {
protected:
    ~ItemGathererProvider() = default;

public:
    virtual size_t ItemsCount() const = 0;
    virtual Item GetItem(size_t idx) const = 0;
    virtual size_t GatherersCount() const = 0;
    virtual Gatherer GetGatherer(size_t idx) const = 0;
};

struct GatheringEvent {
    size_t item_id;
    size_t gatherer_id;
    double sq_distance;
    double time;
};

//std::vector<GatheringEvent> FindGatherEvents(const ItemGathererProvider& provider);
std::vector<GatheringEvent> FindGatherEvents(const ItemGathererProvider* provider);

}  // namespace collision_detector