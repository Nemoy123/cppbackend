#pragma once

#include <string>
#include <vector>
#include "../domain/author.h"
#include "../domain/book.h"
#include "../ui/view.h"
#include "unitofwork.h"

namespace app {

//class AddBookParams;

class UseCases {
public:
    
    virtual std::vector<domain::Author> ShowAuthors() = 0;
    virtual std::vector<domain::Book> ShowBooks ([[maybe_unused]]const std::string& author_id) const = 0;
    virtual std::unique_ptr<UnitOfWork> MakeBookTransaction() = 0;
    virtual std::unique_ptr<UnitOfWork> DeleteAuthorTransaction() = 0;
    virtual std::unique_ptr<UnitOfWork> AddAuthorTransaction() = 0;
protected:
    ~UseCases() = default;
};

}  // namespace app
