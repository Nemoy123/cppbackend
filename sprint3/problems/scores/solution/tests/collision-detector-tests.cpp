#define _USE_MATH_DEFINES
#include <catch2/catch_test_macros.hpp>
#include "../src/collision_detector.h"
#include <random>

// Напишите здесь тесты для функции collision_detector::FindGatherEvents

using namespace geom;
using namespace collision_detector;
using namespace std::literals;

const double epsilon = 0E-10;

std::random_device rd;
std::mt19937 gen(rd());

double Random(int low, int high)
{
    std::uniform_int_distribution<> dist(low, high);
    return static_cast<double> (dist(gen));

}

class TestClass : public ItemGathererProvider {
    private:
        std::vector <Item> items_;
        std::vector <Gatherer> dogs_;
    public:
        TestClass (const std::vector <Item>& items, const std::vector <Gatherer>& dogs): items_(items), dogs_(dogs) {}
        size_t ItemsCount() const override {return items_.size();}
        Item GetItem(size_t idx) const override {
            if (idx > ItemsCount()) throw std::invalid_argument("wrong item num");
            return items_.at(idx);}
        size_t GatherersCount() const override {return dogs_.size();}
        Gatherer GetGatherer(size_t idx) const override {
            if (idx > GatherersCount()) throw std::invalid_argument("wrong item num");
            return dogs_.at(idx);}
};



SCENARIO("Collision test") { 
    using namespace std;
     vector <Item> vect_items;
     vector <Gatherer> vect_dogs;
     GIVEN("collision on X") { 
        vect_items.clear();
        vect_dogs.clear();
        Item item ({10, 0},1);
        Gatherer dog ({0,0},{20,0},1);
        vect_items.push_back (std::move(item));
        vect_dogs.push_back(std::move(dog));
        TestClass testclass (vect_items, vect_dogs);
        const vector <GatheringEvent> result = FindGatherEvents (&testclass);
        REQUIRE (result.at(0).item_id == 0);
        REQUIRE (result.at(0).gatherer_id == 0);
        REQUIRE (result.at(0).sq_distance - 100 < epsilon);
        REQUIRE (result.at(0).time < 1);
        REQUIRE (result.at(0).time > 0);
     }
     GIVEN("collision on Y") { 
        vect_items.clear();
        vect_dogs.clear();
        Item item ({0, 10},1);
        Gatherer dog ({0,0},{0,20},1);
        vect_items.push_back (std::move(item));
        vect_dogs.push_back(std::move(dog));
        TestClass testclass (vect_items, vect_dogs);
        const vector <GatheringEvent> result = FindGatherEvents (&testclass);
        REQUIRE (result.at(0).item_id == 0);
        REQUIRE (result.at(0).gatherer_id == 0);
        REQUIRE (result.at(0).sq_distance - 100 < epsilon);
        REQUIRE (result.at(0).time < 1);
        REQUIRE (result.at(0).time > 0);
     }
    GIVEN(" without collisions") { 
        vect_items.clear();
        vect_dogs.clear();
        for (auto i =0; i < 5000; ++i) {
            Item item ({Random(2,20), Random(2,20)},0.9);
            vect_items.push_back (std::move(item));
        }
        for (auto i =0; i < 5000; ++i) {
            Item item ({Random(-20,-2), Random(-20,-2)},0.9);
            vect_items.push_back (std::move(item));
        }
        Gatherer dog ({0,0},{20,0},1);
        vect_dogs.push_back(std::move(dog));
        TestClass testclass (vect_items, vect_dogs);
        const vector <GatheringEvent> result = FindGatherEvents (&testclass);
        REQUIRE (result.size() == 0);
    }
}