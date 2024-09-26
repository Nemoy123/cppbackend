#pragma once
#include <iosfwd>
#include <optional>
#include <string>
#include <vector>
#include <syncstream>
#include <memory>
#include <iostream>
#include "../app/unitofwork.h"
#include "../util/types.h"

//using namespace detail;

namespace menu {
class Menu;
}

namespace app {
class UseCases;
}

namespace ui {

class View {
public:
    View(menu::Menu& menu, app::UseCases& use_cases, std::istream& input, std::ostream& output);

private:
    bool AddAuthor(std::istream& cmd_input);
    bool EditAuthor(std::istream& cmd_input);
    bool DeleteAuthor(std::istream& cmd_input);
    bool AddBook(std::istream& cmd_input);
    bool ShowBookInfo (std::istream& cmd_input);
    bool ShowAuthors();
    bool ShowBooks();
    bool ShowAuthorBooks();
    bool EditBook(std::istream& cmd_input);
    bool DeleteBook(std::istream& cmd_input);
    std::vector <std::string> AddTags ();
    std::optional<std::string> SelectAllBooks(std::unique_ptr<UnitOfWork>& tran);
    std::optional<std::string> Select (const std::vector<ui::detail::BookInfo>& vect) const;
    std::optional<detail::AddBookParams> GetBookParams(std::istream& cmd_input, std::unique_ptr<UnitOfWork>& transaction);
    std::optional<std::string> SelectAuthor(std::unique_ptr<UnitOfWork>& transaction);
    //std::vector<detail::AuthorInfo> GetAuthors() const;
    std::vector<detail::BookInfo> GetBooks(std::unique_ptr<UnitOfWork>& tran) const;
    std::vector<detail::BookInfo> GetAuthorBooks(const std::string& author_id) const;
    std::vector<detail::AuthorInfo> GetAuthors(std::unique_ptr<UnitOfWork>& transaction) const;
    std::pair <std::optional<Book>, std::vector<std::string>> EnterNewBook (const ui::detail::BookInfo& book);

    menu::Menu& menu_;
    app::UseCases& use_cases_;
    std::istream& input_;
    
    std::ostream& output_;
    //std::osyncstream output_;
};

}  // namespace ui