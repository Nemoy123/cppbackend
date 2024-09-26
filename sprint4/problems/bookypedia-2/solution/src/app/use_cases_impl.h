#pragma once
#include "../domain/author_fwd.h"
#include "unitofwork.h"
#include "use_cases.h"
#include <vector>

namespace app {



class UseCasesImpl : public UseCases, public UnitOfWorkFactory {
public:
    explicit UseCasesImpl(postgres::UnitOfWorkImpl& unit_impl)
        : unit_impl_ (unit_impl)
        {}

    // void AddAuthor(const std::string& name) override;
    std::vector<domain::Author> ShowAuthors() override;
    //std::unique_ptr<UnitOfWork>& AddBook ();
    std::unique_ptr<UnitOfWork> MakeBookTransaction() override;
    std::unique_ptr<UnitOfWork> DeleteAuthorTransaction() override;
    //std::unique_ptr<UnitOfWork>& GetPtr() override {return unit_of_work_;}
    std::vector<domain::Book> ShowBooks ([[maybe_unused]]const std::string& author_id) const override;
    std::unique_ptr<UnitOfWork> CreateUnitOfWork (TYPE type) override;
    std::unique_ptr<UnitOfWork> AddAuthorTransaction() override;
private:
   //domain::AuthorRepository& authors_;
   //domain::BooksRepository& books_;
   postgres::UnitOfWorkImpl& unit_impl_;
   //std::unique_ptr<UnitOfWork> unit_of_work_ = nullptr;

};



}  // namespace app
