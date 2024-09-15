#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include "tagged.h"
#include <map>
#include <variant>
#include <memory>


namespace model {

using Dimension = int;
using Coord = Dimension;

struct Point {
    Coord x, y;
    bool operator== (const Point& other) const {
        return x == other.x && y == other.y;
    }
};

struct Size {
    Dimension width, height;
    bool operator== (const Size& other) const {
        return width == other.width && height == other.height;
    }
};

struct Rectangle {
    Point position;
    Size size;
    bool operator== (const Rectangle& other) const {
        return position == other.position && size == other.size;
    }
};

struct Offset {
    Dimension dx, dy;
    bool operator== (const Offset& other) const {
        return dx == other.dx && dy == other.dy;
    }
};

class Road {
    public:

    struct HorizontalTag {
        explicit HorizontalTag() = default;
    };

    struct VerticalTag {
        explicit VerticalTag() = default;
    };


    constexpr static HorizontalTag HORIZONTAL{};
    constexpr static VerticalTag VERTICAL{};

    Road(HorizontalTag, Point start, Coord end_x) noexcept
        : start_{start}
        , end_{end_x, start.y} {
    }

    Road(VerticalTag, Point start, Coord end_y) noexcept
        : start_{start}
        , end_{start.x, end_y} {
    }
    Road (const Point& start, const Point& end)
        : start_(start)
        , end_(end)
        {}
    bool IsHorizontal() const noexcept {
        return start_.y == end_.y;
    }

    bool IsVertical() const noexcept {
        return start_.x == end_.x;
    }

    Point GetStart() const noexcept {
        return start_;
    }

    Point GetEnd() const noexcept {
        return end_;
    }
    bool operator==(const Road& other) const {
        return start_ == other.start_ && end_ == other.end_;
    }

private:
    Point start_;
    Point end_;
};

class Building {
public:
    explicit Building(Rectangle bounds) noexcept
        : bounds_{bounds} {
    }

    const Rectangle& GetBounds() const noexcept {
        return bounds_;
    }
    bool operator==(const Building& other) const {
        return bounds_ == other.bounds_;
    }
private:
    Rectangle bounds_;
};

class Office {
public:
    using Id = util::Tagged<std::string, Office>;

    Office(Id id, Point position, Offset offset) noexcept
        : id_{std::move(id)}
        , position_{position}
        , offset_{offset} {
    }

    const Id& GetId() const noexcept {
        return id_;
    }

    Point GetPosition() const noexcept {
        return position_;
    }

    Offset GetOffset() const noexcept {
        return offset_;
    }
    bool operator==(const Office& other) const {
        return id_ == other.id_ && position_ == other.position_&& offset_ == other.offset_;
    }
private:
    Id id_;
    Point position_;
    Offset offset_;
};

struct Loot {
    int type = 0;
    double x = 0.0;
    double y = 0.0;
    int id = -1;
    bool operator==(const Loot& other) const {
        return type == other.type && x == other.x && y == other.y && id == other.id;
    }
};

class Map {
public:
    
    using Id = util::Tagged<std::string, Map>;
    using Roads = std::vector<Road>;
    using Buildings = std::vector<Building>;
    using Offices = std::vector<Office>;
    using RoadMap = std::map <double, std::vector <std::pair <double, double>>>;
    using LootType = std::map<std::string,std::variant<std::monostate, std::string, double, int>>;
    using LootDescription = std::map <std::string, LootType >;
    
    

    Map(Id id, std::string name) noexcept
        : id_(std::move(id))
        , name_(std::move(name)) {
    }

    const Id& GetId() const noexcept {
        return id_;
    }

    const std::string& GetName() const noexcept {
        return name_;
    }

    const Buildings& GetBuildings() const noexcept {
        return buildings_;
    }

    const Roads& GetRoads() const noexcept {
        return roads_;
    }

    const Offices& GetOffices() const noexcept {
        return offices_;
    }

    void AddRoad(const Road& road);
    void AddType (const LootType& type);
    const RoadMap& GetHorizontalRoads () const {return horiz_roads_;}
    const RoadMap& GetVerticalRoads () const {return vert_roads_;}

    void AddBuilding(const Building& building) {
        buildings_.emplace_back(building);
    }

    void AddOffice(Office office);

    void SetSpeed (double speed);
    const double GetSpeed () const;
    const LootDescription& GetLootDescription() const;
    LootDescription& GetLootDescription();
    const std::vector <std::unique_ptr<Loot>>& GetLootList () const;
    std::vector <std::unique_ptr<Loot>>& GetLootList ();
    void SetBagCapacity (int num) {bagCapacity = num;}
    int GetBagCapacity () const {return bagCapacity;}
    void AddLoot(int num);
    bool operator==(const Map& other) const;

private:
    using OfficeIdToIndex = std::unordered_map<Office::Id, std::size_t, util::TaggedHasher<Office::Id>>;
    
    Id id_;
    std::string name_;
    Roads roads_;
    Buildings buildings_;

    OfficeIdToIndex warehouse_id_to_index_;
    Offices offices_;
    double dogSpeed = -1;

    RoadMap  horiz_roads_;  // ключ  высота, значение вектор отрезков горизонтальной дороги
    RoadMap  vert_roads_;   // ключ горизонт, значение вектор отрезков вертикальной дороги

    LootDescription loot_description_; // ключ имя лута, мапа - описание 

    std::vector <std::unique_ptr<Loot>> loot_list_;
    int bagCapacity = -1;
};











}  // namespace model
