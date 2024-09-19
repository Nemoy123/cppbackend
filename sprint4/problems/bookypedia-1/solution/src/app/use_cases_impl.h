#pragma once
#include "../domain/author_fwd.h"
#include "use_cases.h"
#include <vector>

namespace app {

class UseCasesImpl : public UseCases {
public:
    explicit UseCasesImpl(domain::AuthorRepository& authors, domain::BooksRepository& books)
        : authors_{authors}
        , books_(books)
        {}

    void AddAuthor(const std::string& name) override;
    std::vector<domain::Author> ShowAuthors() override;
    void AddBook (ui::detail::AddBookParams&& param) override;
    std::vector<domain::Book> ShowBooks ([[maybe_unused]]const std::string& author_id) const override;

private:
    domain::AuthorRepository& authors_;
    domain::BooksRepository& books_;
};



}  // namespace app
