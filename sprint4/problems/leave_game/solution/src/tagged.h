#pragma once
#include <compare>
#include <boost/uuid/nil_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace util {

/**
 * Вспомогательный шаблонный класс "Маркированный тип".
 * С его помощью можно описать строгий тип на основе другого типа.
 * Пример:
 *
 *  struct AddressTag{}; // метка типа для строки, хранящей адрес
 *  using Address = util::Tagged<std::string, AddressTag>;
 *
 *  struct NameTag{}; // метка типа для строки, хранящей имя
 *  using Name = util::Tagged<std::string, NameTag>;
 *
 *  struct Person {
 *      Name name;
 *      Address address;
 *  };
 *
 *  Name name{"Harry Potter"s};
 *  Address address{"4 Privet Drive, Little Whinging, Surrey, England"s};
 *
 * Person p1{name, address}; // OK
 * Person p2{address, name}; // Ошибка, Address и Name - разные типы
 */
template <typename Value, typename Tag>
class Tagged {
public:
    using ValueType = Value;
    using TagType = Tag;

    explicit Tagged(Value&& v)
        : value_(std::move(v)) {
    }
    explicit Tagged(const Value& v)
        : value_(v) {
    }

    const Value& operator*() const {
        return value_;
    }

    Value& operator*() {
        return value_;
    }

    // Так в C++20 можно объявить оператор сравнения Tagged-типов
    // Будет просто вызван соответствующий оператор для поля value_
    auto operator<=>(const Tagged<Value, Tag>&) const = default;
       
private:
    Value value_;
};



// Хешер для Tagged-типа, чтобы Tagged-объекты можно было хранить в unordered-контейнерах
template <typename TaggedValue>
struct TaggedHasher {
    size_t operator()(const TaggedValue& value) const {
        // Возвращает хеш значения, хранящегося внутри value
        return std::hash<typename TaggedValue::ValueType>{}(*value);
    }
};

using UUIDType = boost::uuids::uuid;

UUIDType NewUUID();
constexpr UUIDType ZeroUUID{{0}};

std::string UUIDToString(const UUIDType& uuid);
UUIDType UUIDFromString(std::string_view str);

template <typename Tag>
class TaggedUUID : public Tagged<UUIDType, Tag> {
public:
    using Base = Tagged<UUIDType, Tag>;
    using Tagged<UUIDType, Tag>::Tagged;

    TaggedUUID()
        : Base{ZeroUUID} {
    }

    static TaggedUUID New() {
        return TaggedUUID{NewUUID()};
    }

    static TaggedUUID FromString(const std::string& uuid_as_text) {
        return TaggedUUID{UUIDFromString(uuid_as_text)};
    }


    std::string ToString() const {
        return UUIDToString(**this);
    }
};



}  // namespace util
