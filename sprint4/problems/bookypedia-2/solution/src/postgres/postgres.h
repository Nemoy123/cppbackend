#pragma once
#include <pqxx/pqxx>
#include <pqxx/connection>
#include <pqxx/transaction>

#include "../domain/author.h"
#include "../domain/book.h"
#include "../util/types.h"

namespace postgres {

class AuthorRepositoryImpl : public domain::AuthorRepository {
public:
    explicit AuthorRepositoryImpl(pqxx::connection& connection)
        //: connection_{connection} {
    {}

    std::string Save(const domain::Author& author) override;
    std::vector <domain::Author> ShowAuthors([[maybe_unused]]const std::string& author_name) override;
    void DeleteAuthor(const std::string& author_id);
    void Update(std::string&& author_id, std::string&& new_name);
    void SetPtrTranz (std::shared_ptr<pqxx::work>& work);
    ~AuthorRepositoryImpl() override{}
private:
    //pqxx::connection& connection_;
    //pqxx::work work_;
    std::shared_ptr<pqxx::work> work_ = nullptr;
    //bool active_transaction = false;
    
};

class BookRepositoryImpl : public domain::BooksRepository {
public:
    explicit BookRepositoryImpl(pqxx::connection& connection)
        //: connection_{connection} {
    {}

    std::string Save(const domain::Book& book) override;
    std::vector <domain::Book> ShowBooks([[maybe_unused]]const std::string& author_id) const override;
    void SaveTags(const std::string& book_id, std::vector <std::string>&& tags) override;
    void DeleteTagsByBook (const std::string& book_id);
    void DeleteBook(const domain::Book& book);
    std::vector <ui::detail::BookInfo> FindBooksInfoByTitle (std::string&& title) const override;
    ui::detail::BookInfo GetInfoBookId (std::string&& book_id) const override;
    void UpdateBook (domain::Book&& book) override;
    void SetPtrTranz (std::shared_ptr<pqxx::work>& work);
    

private:
    
    std::shared_ptr<pqxx::work> work_ = nullptr;
    
};

class UnitOfWorkImpl {
    public:
        UnitOfWorkImpl (pqxx::connection&& connection);
        AuthorRepositoryImpl& GetAuthors();
        BookRepositoryImpl& GetBooks();

        ~UnitOfWorkImpl() {
            RollbackTransaction();
        }
        void StartTransaction ();
        void EndTransaction();
        void RollbackTransaction();
    private:
        pqxx::connection connection_;
        AuthorRepositoryImpl authors_;
        BookRepositoryImpl books_;
        std::shared_ptr<pqxx::work> work_ = nullptr;
        
        

};



}  // namespace postgres