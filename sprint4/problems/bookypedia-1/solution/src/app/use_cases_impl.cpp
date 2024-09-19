#include "use_cases_impl.h"

#include "../domain/author.h"

namespace app {
using namespace domain;

void UseCasesImpl::AddAuthor(const std::string& name) {
    authors_.Save({AuthorId::New(), name});
}

std::vector<domain::Author> UseCasesImpl::ShowAuthors() {
    std::vector<domain::Author> result;
        for (auto author : authors_.ShowAuthors()) {
        result.emplace_back (author.GetId(), author.GetName());
    }
    return result;
}

void UseCasesImpl::AddBook (ui::detail::AddBookParams&& param) {
    Book book {
            BookId::New()
        , param.author_id
        , param.title
        , param.publication_year
    };
    books_.Save(std::move(book));
}   

std::vector<domain::Book> UseCasesImpl::ShowBooks ([[maybe_unused]]const std::string& author_id) const {
    return books_.ShowBooks(author_id);
}


}  // namespace app
