#pragma once

#include <string>
#include <vector>
#include <optional>
#include "author.h"
#include "../util/tagged_uuid.h"
#include "../util/types.h"

namespace domain {

namespace detail {
struct BookTag {};
}  // namespace detail

using BookId = util::TaggedUUID<detail::BookTag>;

class Book {
    public:
    Book(BookId id, std::string author_id, std::string title, std::optional<int> year)
        : id_(std::move(id))
        , author_id_(std::move(author_id))
        , title_ (std::move(title))
        , year_(year)
    {}
    Book(std::string_view id, std::string_view author_id, std::string_view title, std::optional<int> year)
        : id_(domain::BookId{util::detail::UUIDFromString(id)})
        , author_id_(std::move(author_id))
        , title_ (std::move(title))
        , year_(year)
    {}
    const BookId& GetId() const noexcept {
        return id_;
    }

    const std::string& GetTitle() const noexcept {
        return title_;
    }
    const std::optional<int>& GetYear() const noexcept {
        return year_;
    }
    const std::string& GetAuthorId() const noexcept {
        return author_id_;
    }

    private:
        BookId id_;
        std::string author_id_;
        std::string title_;
        std::optional<int> year_ = std::nullopt;
};

class BooksRepository {
public:
    virtual std::string Save(const Book& book) = 0;
    virtual std::vector <domain::Book> ShowBooks([[maybe_unused]]const std::string& author_id) const = 0;
    virtual void SaveTags(const std::string& book_id, std::vector <std::string>&& tags) = 0;
    virtual std::vector <ui::detail::BookInfo> FindBooksInfoByTitle (std::string&& title) const = 0;
    virtual ui::detail::BookInfo GetInfoBookId (std::string&& book_id) const = 0;
    virtual void UpdateBook (domain::Book&& book) = 0;
protected:
    ~BooksRepository() = default;
};

}  // namespace domain
