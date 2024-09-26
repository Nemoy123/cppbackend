#pragma once
#include <string>
#include <vector>
#include "../util/tagged_uuid.h"

namespace domain {

namespace detail {
struct AuthorTag {};
}  // namespace detail

using AuthorId = util::TaggedUUID<detail::AuthorTag>;

class Author {
public:
    Author(AuthorId id, std::string name)
        : id_(std::move(id))
        , name_(std::move(name)) {
    }

    const AuthorId& GetId() const noexcept {
        return id_;
    }

    const std::string& GetName() const noexcept {
        return name_;
    }

private:
    AuthorId id_;
    std::string name_;
};

class AuthorRepository {
public:
    virtual std::string Save(const Author& author) = 0;
    virtual std::vector <domain::Author> ShowAuthors([[maybe_unused]]const std::string& author_name) = 0;
    // virtual void Commit () = 0;
    // virtual void Abort () = 0;
    virtual ~AuthorRepository() = default; 
// protected:
//     ~AuthorRepository() = default;
};

}  // namespace domain
