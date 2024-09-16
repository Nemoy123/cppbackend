#pragma once
#include <pqxx/pqxx>
#include <boost/json.hpp>

using namespace boost::json;
using namespace std::literals;
// libpqxx использует zero-terminated символьные литералы вроде "abc"_zv;
using pqxx::operator"" _zv;

constexpr pqxx::zview tag_add_book = "add_book"_zv;

class Library {
    
public:
    using PMAP = std::unordered_map <std::string, std::string>;
    struct Answ {
        std::string action{};
        PMAP map{};
    };
    Library(std::string&& argv, std::istream& in, std::ostream& out)
            : con_(std::move(argv))
            , in_(in)
            , out_(out)
            {
                Init();
            }

    
    void Run();

private:
    pqxx::connection con_;
    std::istream& in_;
    std::ostream& out_;
    
    
    Answ ParseJson (std::string&& stroke);
    void AddBook(PMAP&& map);
    void AllBooks();
    void Init();
};





