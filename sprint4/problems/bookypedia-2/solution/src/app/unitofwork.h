#pragma once
#include "../postgres/postgres.h"
#include <pqxx/pqxx>
#include <pqxx/connection>
#include <pqxx/transaction>

#include "../domain/author.h"
#include "../domain/book.h"
#include "../util/types.h"
#include <atomic>

using namespace postgres;
using namespace domain;
using namespace ui;
// using namespace detail;

enum class TYPE {
    ADDBOOK,
    DELETEAUTHOR,
    ADDAUTHOR
};

class UnitOfWork {
    public:
        UnitOfWork (){}
        virtual void Commit() = 0;
        virtual std::string AddAuthor (std::string&& name) = 0;
        virtual std::string AddBook (ui::detail::AddBookParams&& param) = 0;
        virtual std::string FindAuthor (const std::string& name)  = 0;
        virtual void EditAuthor (std::string&& author_id, std::string&& new_name) = 0;
        virtual void DeleteAuthor (const std::string& author_UUID) = 0;
        virtual std::vector<ui::detail::BookInfo> FindBooks (std::string&& title) = 0;
        virtual ui::detail::BookInfo GetInfoBookId (std::string&& book_id) const = 0;
        virtual void UpdateBookInfo (Book&& book) = 0;
        virtual std::vector <domain::Author> ShowAuthors([[maybe_unused]]const std::string& author_name) = 0;
        virtual ~UnitOfWork() = default;
        virtual void Rollback ()  = 0;
        virtual void DeleteBook (std::string&& book_id) = 0;
        virtual std::vector <Book> ShowAllBooks() const  = 0;
        virtual void SaveTags(const std::string& book_id, std::vector <std::string>&& tags) = 0;
        virtual void DeleteTagsByBook (const std::string& book_id) = 0;
};

class UnitOfWorkAddAuthor : public UnitOfWork {
    public:
        UnitOfWorkAddAuthor (UnitOfWorkImpl& imp);
        
        void Commit() override;
        std::string AddAuthor (std::string&& name) override;
        void EditAuthor (std::string&& author_id, std::string&& new_name) override;
        std::string FindAuthor (const std::string& name) override;
        std::vector <domain::Author> ShowAuthors([[maybe_unused]]const std::string& author_name) override;
        ~UnitOfWorkAddAuthor () override;
        void Rollback ()override;
 
        
    private:
        void UpdateBookInfo (Book&& book) override {}
        std::string AddBook (ui::detail::AddBookParams&& param) override{return {};}
        std::vector<ui::detail::BookInfo> FindBooks (std::string&& title) override {return {};};
        ui::detail::BookInfo GetInfoBookId (std::string&& book_id) const override {return {};};
        void DeleteAuthor (const std::string& author_UUID) override {}
        void DeleteBook (std::string&& book_id) {}
        std::vector <Book> ShowAllBooks() const override {return {};}
        void SaveTags(const std::string& book_id, std::vector <std::string>&& tags) override {}
        void DeleteTagsByBook (const std::string& book_id) override {}
        //void CheckCommit ();
        UnitOfWorkImpl& imp_;
        bool commit;
};

class UnitOfWorkAddBook : public UnitOfWork {
    public:
        UnitOfWorkAddBook (UnitOfWorkImpl& imp);
        
        void Commit() override;
        std::vector <Book> ShowAllBooks() const override;
        std::string AddBook (ui::detail::AddBookParams&& param) override;
        std::string AddAuthor (std::string&& name) override;
        void EditAuthor (std::string&& author_id, std::string&& new_name) override {}
        std::string FindAuthor (const std::string& name) override;
        void DeleteAuthor (const std::string& author_UUID) override {}
        std::vector <domain::Author> ShowAuthors([[maybe_unused]]const std::string& author_name) override;
        std::vector<ui::detail::BookInfo> FindBooks (std::string&& title) override;
        ui::detail::BookInfo GetInfoBookId (std::string&& book_id) const override;
        void UpdateBookInfo (Book&& book) override;
        void DeleteBook (std::string&& book_id) override;
        void Rollback () override;
        ~UnitOfWorkAddBook () override;
        void SaveTags(const std::string& book_id, std::vector <std::string>&& tags) override;
        void DeleteTagsByBook (const std::string& book_id) override;
    private:
        //pqxx::connection& connection_;
        UnitOfWorkImpl& imp_;
         bool commit;
};

class UnitOfWorkDeleteAuthor : public UnitOfWork {
    public:
        UnitOfWorkDeleteAuthor (UnitOfWorkImpl& imp);
                
        void Commit() override;
        std::string FindAuthor (const std::string& name) override;
        void DeleteAuthor (const std::string& author_UUID) override;
        std::vector <domain::Author> ShowAuthors([[maybe_unused]]const std::string& author_name) override;  

        void Rollback ()override;
        ~UnitOfWorkDeleteAuthor () override;
    private:
        std::string AddBook (ui::detail::AddBookParams&& param) override {return {};}
        void EditAuthor (std::string&& author_id, std::string&& new_name) override {}
        std::vector<ui::detail::BookInfo> FindBooks (std::string&& title) override {return {};}
        ui::detail::BookInfo GetInfoBookId (std::string&& book_id) const override {return {};}; 
        void UpdateBookInfo (Book&& book) override {}  
         std::string AddAuthor (std::string&& name) override {return {};}
        void DeleteBook (std::string&& book_id) {}
        std::vector <Book> ShowAllBooks() const  override {return {};}
        void SaveTags(const std::string& book_id, std::vector <std::string>&& tags) override {}
        void DeleteTagsByBook (const std::string& book_id) override {}
        UnitOfWorkImpl& imp_;
         bool commit;
};



class UnitOfWorkFactory {
    public:
        UnitOfWorkFactory (){}
        virtual std::unique_ptr<UnitOfWork> CreateUnitOfWork (TYPE type) = 0;
    private:
        // pqxx::connection& connection_;
    protected:
    ~UnitOfWorkFactory() = default;
};