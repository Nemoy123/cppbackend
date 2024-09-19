#pragma once

#include <string>
#include <vector>
#include "../domain/author.h"
#include "../domain/book.h"
#include "../ui/view.h"

namespace app {

//class AddBookParams;

class UseCases {
public:
    virtual void AddAuthor(const std::string& name) = 0;
    virtual std::vector<domain::Author> ShowAuthors() = 0;
    virtual void AddBook (ui::detail::AddBookParams&& param) = 0;
    virtual std::vector<domain::Book> ShowBooks ([[maybe_unused]]const std::string& author_id) const = 0;

protected:
    ~UseCases() = default;
};

}  // namespace app
