#pragma once
#include <iosfwd>
#include <optional>
#include <string>
#include <vector>
#include <syncstream>

namespace menu {
class Menu;
}

namespace app {
class UseCases;
}

namespace ui {
namespace detail {

struct AddBookParams {
    std::string title;
    std::string author_id;
    std::optional<int> publication_year = std::nullopt;
};

struct AuthorInfo {
    std::string id{};
    std::string name{};
};

struct BookInfo {
    BookInfo(){}
    explicit BookInfo (std::string tit, std::optional<int> opt): title(tit), publication_year(opt){}
    std::string title;
    std::optional<int> publication_year;
};

}  // namespace detail

class View {
public:
    View(menu::Menu& menu, app::UseCases& use_cases, std::istream& input, std::ostream& output);

private:
    bool AddAuthor(std::istream& cmd_input);
    bool AddBook(std::istream& cmd_input);
    bool ShowAuthors();
    bool ShowBooks();
    bool ShowAuthorBooks();

    std::optional<detail::AddBookParams> GetBookParams(std::istream& cmd_input);
    std::optional<std::string> SelectAuthor();
    std::vector<detail::AuthorInfo> GetAuthors() const;
    std::vector<detail::BookInfo> GetBooks() const;
    std::vector<detail::BookInfo> GetAuthorBooks(const std::string& author_id) const;

    menu::Menu& menu_;
    app::UseCases& use_cases_;
    std::istream& input_;
    
    std::ostream& output_;
    //std::osyncstream output_;
};

}  // namespace ui