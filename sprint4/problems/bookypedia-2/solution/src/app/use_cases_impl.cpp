#include "use_cases_impl.h"

// #include "../domain/author.h"

namespace app {
// using namespace domain;

// void UseCasesImpl::AddAuthor(const std::string& name) {
//     unity_.SaveAuthor({AuthorId::New(), name});
// }

// std::vector<domain::Author> UseCasesImpl::ShowAuthors() {
//     std::vector<domain::Author> result;
//         for (auto author : unity_.ShowAuthors()) {
//         result.emplace_back (author.GetId(), author.GetName());
//     }
//     return result;
// }

// void UseCasesImpl::AddBook (ui::detail::AddBookParams&& param) {
//     Book book {
//             BookId::New()
//         , param.author_id
//         , param.title
//         , param.publication_year
//     };
//     unity_.Save(std::move(book));
// }   

// std::vector<domain::Book> UseCasesImpl::ShowBooks ([[maybe_unused]]const std::string& author_id) const {
//     return unity_.ShowBooks(author_id);
// }

std::vector<domain::Author> UseCasesImpl::ShowAuthors() {
    return unit_impl_.GetAuthors().ShowAuthors({});
}

std::vector<domain::Book> UseCasesImpl::ShowBooks ([[maybe_unused]]const std::string& author_id) const {
    return unit_impl_.GetBooks ().ShowBooks(author_id);
}

std::unique_ptr<UnitOfWork> UseCasesImpl::MakeBookTransaction () {
    return  std::move((CreateUnitOfWork (TYPE::ADDBOOK)));
}

std::unique_ptr<UnitOfWork> UseCasesImpl::DeleteAuthorTransaction() {
    return  std::move((CreateUnitOfWork (TYPE::DELETEAUTHOR)));
}

std::unique_ptr<UnitOfWork> UseCasesImpl::AddAuthorTransaction() {
    return  std::move((CreateUnitOfWork (TYPE::ADDAUTHOR)));
}

std::unique_ptr<UnitOfWork> UseCasesImpl::CreateUnitOfWork (TYPE type) {
    if (type == TYPE::ADDBOOK) {
        return std::make_unique<UnitOfWorkAddBook> (unit_impl_);
    }
    else if (type == TYPE::DELETEAUTHOR){
        return std::make_unique<UnitOfWorkDeleteAuthor> (unit_impl_);
    }
    else if (type == TYPE::ADDAUTHOR){
        return std::make_unique<UnitOfWorkAddAuthor> (unit_impl_);
    }
    return nullptr;
}


}  // namespace app
