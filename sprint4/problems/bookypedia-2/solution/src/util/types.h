#pragma once
#include <optional>
#include <string>
#include <vector>


namespace ui {
namespace detail {

struct AddBookParams {
    std::string title;
    std::string author_id;
    std::optional<int> publication_year = std::nullopt;
    std::vector <std::string> tags{};
    bool stop_reading = false;
};

struct AuthorInfo {
    std::string id{};
    std::string name{};
};

struct BookInfo {
    BookInfo(){}
    explicit BookInfo (std::string tit, std::string author, std::optional<int> opt)
        : title(tit)
        , author (author)
        , publication_year(opt){}
    std::string title;
    std::string author;
    std::string author_id;
    std::optional<int> publication_year = std::nullopt;
    std::string id {};
    std::vector <std::string> tags{};
};

} //end namespace detail
} //end namespace ui
