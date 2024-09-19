#include "view.h"

#include <boost/algorithm/string/trim.hpp>
#include <cassert>
#include <iostream>

#include "../app/use_cases.h"
#include "../menu/menu.h"


using namespace std::literals;
namespace ph = std::placeholders;

namespace ui {
namespace detail {

std::ostream& operator<<(std::ostream& out, const AuthorInfo& author) {
    std::osyncstream(out) << author.name;
    return out;
}

std::ostream& operator<<(std::ostream& out, const BookInfo& book) {
    if (book.publication_year.has_value()) {
        std::osyncstream(out) << book.title << ", " << book.publication_year.value();
    } else {
        std::osyncstream(out) << book.title;
    }
    return out;
}


}  // namespace detail

template <typename T>
void PrintVector(std::ostream& out, const std::vector<T>& vector) {
    int i = 1;
    for (auto& value : vector) {
        std::osyncstream(out) << i++ << " " << value << std::endl;
    }
}

View::View(menu::Menu& menu, app::UseCases& use_cases, std::istream& input, std::ostream& output)
    : menu_{menu}
    , use_cases_{use_cases}
    , input_{input}
    , output_{output} {
    menu_.AddAction(  //
        "AddAuthor"s, "name"s, "Adds author"s, std::bind(&View::AddAuthor, this, ph::_1)
        // ����
        // [this](auto& cmd_input) { return AddAuthor(cmd_input); }
    );
    menu_.AddAction("AddBook"s, "<pub year> <title>"s, "Adds book"s,
                    std::bind(&View::AddBook, this, ph::_1));
    menu_.AddAction("ShowAuthors"s, {}, "Show authors"s, std::bind(&View::ShowAuthors, this));
    menu_.AddAction("ShowBooks"s, {}, "Show books"s, std::bind(&View::ShowBooks, this));
    menu_.AddAction("ShowAuthorBooks"s, {}, "Show author books"s,
                    std::bind(&View::ShowAuthorBooks, this));
}

bool View::AddAuthor(std::istream& cmd_input) {
    try {
        std::string name;
        std::getline(cmd_input, name);
        boost::algorithm::trim(name);
        if (name.empty()) {
            std::osyncstream(output_) << "Failed to add author"s << std::endl;
            return true;
        }
        use_cases_.AddAuthor(std::move(name));
    } catch (const std::exception&) {
        std::osyncstream(output_) << "Failed to add author"s << std::endl;
        
    }
    return true;
}

bool View::AddBook(std::istream& cmd_input){
    try {
        if (auto params = GetBookParams(cmd_input)) {
            use_cases_.AddBook (std::move(params.value()));
        }
    } catch (const std::exception& ex) {
        std::osyncstream(output_) << "Failed to add book"s << std::endl;
        //output_ << ex.what() << std::endl;
    }
    return true;
}

bool View::ShowAuthors() {
    PrintVector(output_, GetAuthors());
    return true;
}

bool View::ShowBooks() {
    PrintVector(output_, GetBooks());
    return true;
}

bool View::ShowAuthorBooks() {
    // TODO: handle error
    try {
        if (auto author_id = SelectAuthor()) {
            PrintVector(output_, GetAuthorBooks(*author_id));
        }
    } catch (const std::exception& ex) {
        throw std::runtime_error("Failed to Show Books");
        //output_ << ex.what() << std::endl;
    }
    return true;
}

std::optional<detail::AddBookParams> View::GetBookParams(std::istream& cmd_input) {
    detail::AddBookParams params;
    std::string temp;
    cmd_input >> temp;
    boost::algorithm::trim(temp);
    if (temp.size() < 1) {return std::nullopt;}
    try {
        params.publication_year = std::stoi(temp);
        std::getline(cmd_input, params.title);
        boost::algorithm::trim(params.title);
    } catch(const std::exception& ex) {
        params.title = temp;
    }
 
    auto author_id = SelectAuthor();
    if (not author_id.has_value())
        return std::nullopt;
    else {
        params.author_id = author_id.value();
        return params;
    }
}

std::optional<std::string> View::SelectAuthor(){
    std::osyncstream(output_) << "Select author:" << std::endl;
    auto authors = GetAuthors();
    PrintVector(output_, authors);
    std::osyncstream(output_) << "Enter author # or empty line to cancel" << std::endl;

    std::string str;
    if (!std::getline(input_, str) || str.empty()) {
        return std::nullopt;
    }

    int author_idx;
    try {
        author_idx = std::stoi(str);
    } catch (std::exception const&) {
        throw std::runtime_error("Invalid author num");
    }

    --author_idx;
    if (author_idx < 0 or author_idx >= authors.size()) {
        throw std::runtime_error("Invalid author num");
    }

    return authors[author_idx].id;
}

std::vector<detail::AuthorInfo> View::GetAuthors() const {
    std::vector<detail::AuthorInfo> dst_autors;
    auto autors = use_cases_.ShowAuthors();
    dst_autors.reserve(autors.size());
    for (const auto& au : autors) {
        dst_autors.emplace_back ((au.GetId()).ToString(), au.GetName());
    }
    return dst_autors;
}

std::vector<detail::BookInfo> View::GetBooks() const {
    std::vector<detail::BookInfo> books;
    
    for (auto& book: use_cases_.ShowBooks (std::string{})) {
        //if (book.GetYear().has_value())
        books.emplace_back (book.GetTitle(), book.GetYear());
    }
    return books;
}

std::vector<detail::BookInfo> View::GetAuthorBooks(const std::string& author_id) const {
    std::vector<detail::BookInfo> books;
    for (auto& book: use_cases_.ShowBooks (author_id)) {
        detail::BookInfo temp;
        temp.title = book.GetTitle();
        temp.publication_year = book.GetYear();
        books.emplace_back (std::move(temp));
    }
    return books;
}

}  // namespace ui
