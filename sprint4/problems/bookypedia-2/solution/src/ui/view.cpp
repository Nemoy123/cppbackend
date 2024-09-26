#include "view.h"

#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/trim_all.hpp>
#include <cassert>
#include <iostream>
#include "../app/use_cases.h"
#include "../menu/menu.h"
#include <limits>
#include <set>


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
        std::osyncstream(out) << book.title << " by "<< book.author << ", " << book.publication_year.value();
    } else {
        std::osyncstream(out) << book.title<< " by "<< book.author<< ", " << "None";
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

void PrintDetailInfo (std::ostream& out, const ui::detail::BookInfo& info){
    
    out << "Title: " << info.title << std::endl;
    out << "Author: " << info.author << std::endl;
    if (info.publication_year.has_value()) {
        out << "Publication year: " << std::to_string(info.publication_year.value()) << std::endl;
    } else {
        out << "Publication year: None" << std::endl;
    }
    if (info.tags.size() > 0) {
        out << "Tags: ";
        bool begin = true;
        for (auto& tag : info.tags){
            if (!begin) {out << ", ";}
            out << tag;
            begin = false;
        }  
        out << std::endl;
    }

}

View::View(menu::Menu& menu, app::UseCases& use_cases, std::istream& input, std::ostream& output)
    : menu_(menu)
    , use_cases_(use_cases)
    , input_(input)
    , output_(output) {
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
    menu_.AddAction(  
        "DeleteAuthor"s, "name"s, "Delete author and his books"s, std::bind(&View::DeleteAuthor, this, ph::_1)
    );
    menu_.AddAction(  
        "EditAuthor"s, "name"s, "Edit author"s, std::bind(&View::EditAuthor, this, ph::_1)
    );
    menu_.AddAction(  
        "ShowBook"s, "title"s, "Show book's information"s, std::bind(&View::ShowBookInfo, this, ph::_1)
    );
    menu_.AddAction(  
        "EditBook"s, "title"s, "Edit book's information"s, std::bind(&View::EditBook, this, ph::_1)
    );
    menu_.AddAction(  
        "DeleteBook"s, "title"s, "Delete book"s, std::bind(&View::DeleteBook, this, ph::_1)
    );
}

bool View::DeleteBook(std::istream& cmd_input) { 
    try {
        std::unique_ptr<UnitOfWork> transaction {use_cases_.MakeBookTransaction()};
        std::string title;
        std::getline(cmd_input, title);
        if (!title.empty()) {
            boost::algorithm::trim(title);
            auto vect = transaction->FindBooks (std::move(title));
            if (vect.size() == 1) {
                transaction->DeleteBook(std::move(vect.at(0).id));
                transaction->Commit();
                return true;
            }
            else if (vect.size() < 1) {
                //std::osyncstream(output_) << "Failed to delete book"s << std::endl;
                return true;
            }
            else if (vect.size() > 1) {
                auto id = Select(vect);
                if (id.has_value()) {
                    transaction->DeleteBook(std::move(id.value()));
                    transaction->Commit();
                    return true;
                } else {
                    //std::osyncstream(output_) << "Failed to delete book"s << std::endl;
                    return true;
                }
            }
        } else {
            auto vect_book = transaction->ShowAllBooks();
            std::vector<ui::detail::BookInfo> new_vect;
            for (auto book:vect_book) {
                new_vect.push_back(transaction->GetInfoBookId ( book.GetId().ToString()));
                new_vect.back().id = book.GetId().ToString();
            }
            auto id = Select(new_vect);
            if (id.has_value()) {
                transaction->DeleteBook(std::move(id.value()));
                transaction->Commit();
                return true;
            }
            else {
                //std::osyncstream(output_) << "Failed to delete book"s << std::endl;
                return true;
            }
        }


    }
     catch (const std::exception& ex) {
        std::osyncstream(output_) << "Failed to delete book"s << std::endl;
        //std::osyncstream(output_) << ex.what() << std::endl;
    }
    return true;


}

bool View::EditAuthor(std::istream& cmd_input) {
    try {
        std::unique_ptr<UnitOfWork> transaction {use_cases_.AddAuthorTransaction()};
        std::string name;
        std::string author_id;
        std::getline(cmd_input, name);
        
        if (name.empty()) {
           
            auto author_opt = SelectAuthor(transaction);
            if (not author_opt.has_value()) {
                std::osyncstream(output_) << "Failed to edit author"s << std::endl;
                return true;
            }
            author_id = author_opt.value();
        } else {
            boost::algorithm::trim(name);
            author_id = transaction->FindAuthor (name);
            if (author_id.empty()) {
                std::osyncstream(output_) << "Failed to edit author"s << std::endl;
                return true;
            }
        }
        
        std::string new_name{};
        std::osyncstream(output_) << "Enter new name:"s << std::endl;
        std::getline(input_, new_name);
        // for (int ch; (ch = std::getchar()) != EOF ;) {
        //     if (std::isprint(ch))
        //         new_name.push_back (static_cast<char>(ch));
        //     if (ch == '\n') 
        //         break;
        // }
        boost::algorithm::trim(new_name);
        transaction->EditAuthor(std::move(author_id), std::move(new_name));
        transaction->Commit();
    } catch (const std::exception&) {
        std::osyncstream(output_) << "Failed to edit author"s << std::endl;
        
    }
    return true;
}

bool View::AddAuthor(std::istream& cmd_input) {
    try {
        std::unique_ptr<UnitOfWork> transaction {use_cases_.AddAuthorTransaction()};
        std::string name;
        std::getline(cmd_input, name);
        boost::algorithm::trim(name);
        if (name.empty()) {
            std::osyncstream(output_) << "Failed to add author"s << std::endl;
            return true;
        }
        transaction->AddAuthor(std::move(name));
        transaction->Commit();
    } catch (const std::exception& ex) {
        std::osyncstream(output_) << "Failed to add author"s << std::endl;
        //std::osyncstream(output_) << ex.what() << std::endl;
    }
    return true;
}

bool View::DeleteAuthor(std::istream& cmd_input) {
    
    try {
        std::unique_ptr<UnitOfWork> transaction{use_cases_.DeleteAuthorTransaction()};
        std::string name;
        std::getline(cmd_input, name);
        //boost::algorithm::trim_all(name);
        std::string author_id_uuid;
        if (name.empty()) {
            
            auto author_id = SelectAuthor(transaction);
            if (not author_id.has_value()) {
                std::osyncstream(output_) << "Failed to delete author"s << std::endl;
                transaction->Rollback();
                return true;
            }
            
            author_id_uuid = author_id.value();
                        
        } else {
            boost::algorithm::trim(name);
            author_id_uuid = transaction->FindAuthor(name);
            if (author_id_uuid.empty()) {
                std::osyncstream(output_) << "Failed to delete author"s << std::endl;
                transaction->Rollback();
                return true;
            }
        }
        transaction->DeleteAuthor (author_id_uuid);
        transaction->Commit();
    } catch (const std::exception& ex) {
        std::osyncstream(output_) << "Failed to delete author"s << std::endl;
        //std::osyncstream(output_) << ex.what()  << std::endl;
    }
    return true;
}

std::vector <std::string> View::AddTags () {
    std::set <std::string> result;
    std::string tag{};
    std::string buffer{};
    //cmd_input.clear();
    //cmd_input.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    // for (int ch; (ch = std::getchar()) != EOF ;) {
    //     if (std::isprint(ch))
    //     buffer.push_back (static_cast<char>(ch));
    //     if (ch == '\n') 
    //         break;
    // }
    std::getline (input_, buffer);
    for (char& ch : buffer) {
        if (ch == 44) {
            boost::algorithm::trim_all(tag);
            if (!tag.empty()) {
                result.insert(std::move(tag));
                tag.clear();
            }
        }
        else if (std::isprint(ch)) {
            tag.push_back (static_cast<char>(ch));
        }        
    }

    boost::algorithm::trim_all(tag);
    if (!tag.empty()) {
        result.insert(std::move(tag));
    }
                      
    return {result.begin(), result.end()};
}

bool View::AddBook(std::istream& cmd_input){
    
    try {
        std::unique_ptr<UnitOfWork> transaction {use_cases_.MakeBookTransaction()};
        auto params = GetBookParams(cmd_input, transaction);
        
        
        if (params.has_value() && !params.value().author_id.empty() && !params.value().title.empty()) {
            std::osyncstream(output_) << "Enter tags (comma separated):"sv << std::endl;
            auto tags = AddTags ();
            params.value().tags =  std::move(tags);
            transaction->AddBook (std::move(params.value()));
            transaction->Commit();
            // UnitOfWorkAddBook объект разрушится, завершив транзакцию
        } else {
            if (params.has_value() && params.value().stop_reading == false) {
                std::osyncstream(output_) << "Enter tags (comma separated):"sv << std::endl;
                auto tags = AddTags ();
            }
            std::osyncstream(output_) << "Failed to add book"s << std::endl;
            transaction->Rollback();
        }
    } catch (const std::exception& ex) {
        std::osyncstream(output_) << "Failed to add book"s << std::endl;
        //std::osyncstream(output_) << ex.what() << std::endl;
    }
    return true;

    
}

bool View::ShowBookInfo (std::istream& cmd_input){
    
    try {
        std::unique_ptr<UnitOfWork> transaction{use_cases_.MakeBookTransaction()};
        
        std::string title;
        std::vector <ui::detail::BookInfo> books_info;
        std::string book_id{};
        std::getline(cmd_input, title);
        
        if (title.empty()) {
            auto book_opt = SelectAllBooks(transaction);
            if (not book_opt.has_value()) {
                return false;
            }
            book_id = book_opt.value();
        } else {
            boost::algorithm::trim(title);
            books_info = transaction->FindBooks(std::move(title));
            if (books_info.empty()) {
                return true;
            }
        }
        
        if(books_info.size() == 1) {
            PrintDetailInfo (output_, transaction->GetInfoBookId ( std::move(books_info[0].id) ));
            transaction->Commit();
            return true;
        }
        else if (books_info.size() > 1) {
            auto book_id_opt = Select (books_info);
            if (book_id_opt.has_value()) {
                book_id = std::move(book_id_opt.value());
            }
        }
        
        if ( !book_id.empty() ) {
            PrintDetailInfo (output_, transaction->GetInfoBookId ( std::move(book_id) ));

        }
        transaction->Commit();
    }
    catch (const std::exception& ex) {
        //std::osyncstream(output_) << "Failed to add book"s << std::endl;
        return true;
    }
    return true;

    
}

bool View::ShowAuthors() {
    std::unique_ptr<UnitOfWork> transaction = use_cases_.AddAuthorTransaction();
    PrintVector(output_, GetAuthors(transaction));
    transaction->Commit();
    return true;
}

bool View::ShowBooks() {
    std::unique_ptr<UnitOfWork> transaction = use_cases_.MakeBookTransaction();
    PrintVector(output_, GetBooks(transaction));
    transaction->Commit();
    return true;
}

bool View::ShowAuthorBooks() {
    
    try {
        std::unique_ptr<UnitOfWork> transaction = use_cases_.AddAuthorTransaction();
        if (auto author_id = SelectAuthor(transaction)) {
            PrintVector(output_, GetAuthorBooks(*author_id));
        }
    } catch (const std::exception& ex) {
        //throw std::runtime_error("Failed to Show Books");
         std::osyncstream(output_) << "Failed to Show Books"s << std::endl;
        //output_ << ex.what() << std::endl;
    }
    return true;
}

std::optional<detail::AddBookParams> View::GetBookParams(std::istream& cmd_input, std::unique_ptr<UnitOfWork>& transaction) {
    
    detail::AddBookParams params;
    std::string temp;
    std::getline(cmd_input, temp);
    boost::algorithm::trim(temp);
    if (!temp.empty() && !std::isdigit(temp.at(0))) {
        params.title = temp;
    } else {
        std::string year{};
        int i = 0;
        for (char& ch : temp) {
            if (std::isdigit(ch)) {
                year.push_back (ch);
                ++i;
            } else {
                params.publication_year = std::stoi(year);
                break;
            } 
        }
        std::string title = temp.substr(i);
        boost::algorithm::trim(title);
        params.title = std::move(title);
    }
    
    std::osyncstream(output_) << "Enter author name or empty line to select from list:"sv << std::endl;
    std::string name{};
    std::getline (input_, name);
    
   
    if (name.empty()) { 
        auto author_id = SelectAuthor(transaction);
        if (not author_id.has_value()) {
            params.author_id = {};
        }
        else {
            params.author_id = author_id.value();
        }
        return params;
    } else {
        boost::algorithm::trim(name);
        if (name.empty()) {
            params.author_id = {};
            return params;
        }
    }
     
    auto check_author = transaction->FindAuthor(name);
    if (check_author.empty()) {
        std::osyncstream(output_) << "No author found. Do you want to add "sv;
        std::osyncstream(output_) << name;
        std::osyncstream(output_) <<" (y/n)?"sv << std::endl;
                   
        std::string ans{};
        while (ans.empty()) {
            std::getline (input_, ans);
        }
         
        if (ans == "y" || ans == "Y"){
            params.author_id = transaction->AddAuthor(std::move(name));
        }
        else {
            params.stop_reading = true;
            params.author_id ={};
        }
        //return params;
        
 
    } else {
        params.author_id = check_author;
    }
    return params;
}

std::optional<std::string> View::Select (const std::vector<ui::detail::BookInfo>& vect) const {
    PrintVector(output_, vect);
    std::osyncstream(output_) << "Enter book # or empty line to cancel" << std::endl;
    std::string str;
    if (!std::getline(input_, str) || str.empty()) {
        return std::nullopt;
    }
    int book_idx;
    try {
        book_idx = std::stoi(str);
    } catch (std::exception const&) {
        throw std::runtime_error("Invalid book num");
    }

    --book_idx;
    if (book_idx < 0 or book_idx >= vect.size()) {
        throw std::runtime_error("Invalid book num");
    }

    return vect[book_idx].id;
}

std::optional<std::string> View::SelectAllBooks( std::unique_ptr<UnitOfWork>& tran) {
    auto all_books = GetBooks(tran);
    return Select (all_books);
}

std::optional<std::string> View::SelectAuthor(std::unique_ptr<UnitOfWork>& transaction){
    std::osyncstream(output_) << "Select author:" << std::endl;
    auto authors = GetAuthors(transaction);
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

std::vector<detail::AuthorInfo> View::GetAuthors(std::unique_ptr<UnitOfWork>& transaction) const {
    //std::unique_ptr<UnitOfWork> transaction {use_cases_.AddAuthorTransaction()};
    std::vector<detail::AuthorInfo> dst_autors;
    auto autors = transaction->ShowAuthors({});
    dst_autors.reserve(autors.size());
    for (const auto& au : autors) {
        dst_autors.emplace_back ((au.GetId()).ToString(), au.GetName());
    }
    //transaction->Commit();
    return dst_autors;
}

std::vector<detail::BookInfo> View::GetBooks(std::unique_ptr<UnitOfWork>& tran) const {
    std::vector<detail::BookInfo> books;
    
    for (auto& book: tran->ShowAllBooks()) {
        books.emplace_back (book.GetTitle(), book.GetAuthorId(), book.GetYear());
        books.back().id = book.GetId().ToString();
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

std::pair <std::optional<Book>, std::vector<std::string>> View::EnterNewBook (const ui::detail::BookInfo& book) {
    std::osyncstream(output_) << "Enter new title or empty line to use the current one (" 
                                << book.title
                                <<"): "s << std::endl;
            std::string new_title{};
            std::getline (input_, new_title);
            //boost::algorithm::trim(new_title);
            
            std::string year_num = book.publication_year.has_value() ? std::to_string(book.publication_year.value()) : ""s;
            std::osyncstream(output_) << "Enter publication year or empty line to use the current one (" 
                                << year_num << "): "sv << std::endl;
            std::string buffer_num{};
            std::getline(input_, buffer_num);
            
            boost::algorithm::trim(buffer_num);
            int new_year = -1;
            if (!buffer_num.empty()) {
                try {
                    new_year = std::stoi (buffer_num);
                } catch (const std::exception& ex) {
                    new_year = -1;
                }
            }
            std::string tag_stroke{};
            bool start = true;
            for (auto& one_tag : book.tags) {
                if (!start) tag_stroke += ", ";
                tag_stroke += one_tag;
                start = false;
            }
            std::osyncstream(output_)   << "Enter tags (current tags: "
                                        << tag_stroke 
                                        << "): "
                                        << std::endl;
            tag_stroke.clear();
            auto tags = AddTags ();

            domain::Book new_book {
                  BookId{util::detail::UUIDFromString(book.id)}
                , book.author_id
                , new_title.empty() ? book.title : new_title
                , new_year < 0 ? book.publication_year : new_year
            };
            return {new_book, tags};
}

bool View::EditBook(std::istream& cmd_input){
    
    try {
        std::unique_ptr<UnitOfWork> transaction = use_cases_.MakeBookTransaction();
        std::string title{};
        std::string book_id{};
        std::vector <ui::detail::BookInfo> books_info{};
        std::getline(cmd_input, title);
        boost::algorithm::trim(title);
        if (title.empty()) {
            auto book_opt = SelectAllBooks(transaction);
            if (not book_opt.has_value()) {
                std::osyncstream(output_) << "Book not found"s << std::endl;
                return true;
            }
            book_id = book_opt.value();
        } else {
           
            books_info = transaction->FindBooks(std::move(title));
            if(books_info.size() == 0) { 
                std::osyncstream(output_) << "Book not found"s << std::endl;
                return true;
            }
        }

        
        if(books_info.size() == 1) { 
            book_id = books_info.at(0).id;
        }
        else if (books_info.size() > 1) { 
            auto book_id_opt = Select (books_info);
            if (book_id_opt.has_value()) {
                book_id = std::move(book_id_opt.value());
            } else {
                std::osyncstream(output_) << "Book not found"s << std::endl;
                return true;
            }
        }
        if ( !book_id.empty() ) { 
            auto book = transaction->GetInfoBookId (std::string{book_id});
            book.id = std::move(book_id);

            auto [new_book_opt, new_tag] = EnterNewBook(book);

            if (new_book_opt.has_value()) {
                transaction->UpdateBookInfo (std::move(new_book_opt.value()));
                transaction->DeleteTagsByBook (book.id);
                transaction->SaveTags(book.id, std::move(new_tag));
                transaction->Commit();
            }
        }

        
    } catch (const std::exception& ex) {
        std::osyncstream(output_) << "Book not found"s << std::endl;
        //std::osyncstream(output_) << ex.what() << std::endl;
    }
    return true;

    
}

}  // namespace ui
