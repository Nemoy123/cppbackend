#include "unitofwork.h"


std::string UnitOfWorkAddBook::AddBook (ui::detail::AddBookParams&& param) {
    Book book {
            BookId::New()
        , param.author_id
        , param.title
        , param.publication_year
    };
    std::string result = imp_.GetBooks().Save(std::move(book));
    imp_.GetBooks().SaveTags (result, std::move(param.tags));
    //Commit();
    return result;
}

std::string UnitOfWorkAddBook::AddAuthor (std::string&& name) {
    return imp_.GetAuthors().Save({AuthorId::New(), name});
    
}
void UnitOfWorkAddBook::Commit() {
    commit = false;
    imp_.EndTransaction();
    
}
UnitOfWorkAddBook::~UnitOfWorkAddBook () {
    if (commit) {
        imp_.RollbackTransaction();
    }
}

void UnitOfWorkAddBook::Rollback () {
    imp_.RollbackTransaction();
}

UnitOfWorkAddBook::UnitOfWorkAddBook (UnitOfWorkImpl& imp)
            :imp_(imp)
            , commit(true)
{
       imp_.StartTransaction();
}

void UnitOfWorkAddBook::DeleteTagsByBook (const std::string& book_id) {
    imp_.GetBooks().DeleteTagsByBook (book_id);
}

std::vector <Book> UnitOfWorkAddBook::ShowAllBooks() const {
    return imp_.GetBooks().ShowBooks ({});
}

std::vector <domain::Author> UnitOfWorkAddBook::ShowAuthors([[maybe_unused]]const std::string& author_name) {
    return imp_.GetAuthors().ShowAuthors(author_name);
}

void UnitOfWorkAddBook::DeleteBook (std::string&& book_id) {
    imp_.GetBooks().DeleteBook(Book{domain::BookId{util::detail::UUIDFromString(std::move(book_id))},{},{},0});
}

std::string UnitOfWorkAddBook::FindAuthor (const std::string& name) {
    auto res = imp_.GetAuthors().ShowAuthors(name);
    if (!res.empty()) {
        return res.at(0).GetId().ToString();
    }
    return {};
}

void UnitOfWorkAddBook::SaveTags(const std::string& book_id, std::vector <std::string>&& tags) {
    imp_.GetBooks().SaveTags (book_id, std::move(tags));
}

std::vector<ui::detail::BookInfo> UnitOfWorkAddBook::FindBooks (std::string&& title) {
    return imp_.GetBooks().FindBooksInfoByTitle(std::move(title));
}

ui::detail::BookInfo UnitOfWorkAddBook::GetInfoBookId (std::string&& book_id) const {
    return imp_.GetBooks().GetInfoBookId (std::move(book_id));
}

void UnitOfWorkAddBook::UpdateBookInfo (Book&& book) {
    return imp_.GetBooks().UpdateBook(std::move(book));
}

UnitOfWorkDeleteAuthor::UnitOfWorkDeleteAuthor (UnitOfWorkImpl& imp)
                :imp_(imp)
                 , commit(true)
{
   // commit = true;
    imp_.StartTransaction();
    
    // imp_.GetAuthors().CheckTranzOn();
    // imp_.GetBooks().CheckTranzOn();
    // imp_
}

std::vector <domain::Author> UnitOfWorkDeleteAuthor::ShowAuthors([[maybe_unused]]const std::string& author_name) {
    return imp_.GetAuthors().ShowAuthors(author_name);
}

std::string UnitOfWorkDeleteAuthor::FindAuthor (const std::string& name) {
    auto res = imp_.GetAuthors().ShowAuthors(name);
    if (!res.empty()) {
        return res.at(0).GetId().ToString();
    }
    return {};
}

void UnitOfWorkDeleteAuthor::Commit() {
    commit = false;
    imp_.EndTransaction();
    
    //imp_.GetAuthors().Commit();
}

UnitOfWorkDeleteAuthor::~UnitOfWorkDeleteAuthor () {
    if (commit) {
        imp_.RollbackTransaction();
    }
}

void UnitOfWorkDeleteAuthor::Rollback () {
    imp_.RollbackTransaction();
}

void UnitOfWorkDeleteAuthor::DeleteAuthor (const std::string& author_UUID) {
    auto vect_books = imp_.GetBooks().ShowBooks(author_UUID);
    for (const auto& book : vect_books) {
        imp_.GetBooks().DeleteTagsByBook (book.GetId().ToString());
        imp_.GetBooks().DeleteBook(book);
    }
    imp_.GetAuthors().DeleteAuthor(author_UUID);

}

void UnitOfWorkAddAuthor::Commit() {
    commit = false;
    imp_.EndTransaction();
}

UnitOfWorkAddAuthor::~UnitOfWorkAddAuthor () {
    if (commit) {
        imp_.RollbackTransaction();
    }
}

void UnitOfWorkAddAuthor::Rollback () {
    imp_.RollbackTransaction();
}

std::vector <domain::Author> UnitOfWorkAddAuthor::ShowAuthors([[maybe_unused]]const std::string& author_name) {
    return imp_.GetAuthors().ShowAuthors(author_name);
}

UnitOfWorkAddAuthor::UnitOfWorkAddAuthor (UnitOfWorkImpl& imp)
                :imp_(imp)
                ,commit(true)
    {
        //commit = true;
         imp_.StartTransaction();
    }

std::string UnitOfWorkAddAuthor::FindAuthor (const std::string& name) {
    auto res = imp_.GetAuthors().ShowAuthors(name);
    if (!res.empty()) {
        return res.at(0).GetId().ToString();
    }
    return {};
}

std::string UnitOfWorkAddAuthor::AddAuthor (std::string&& name) {
    return imp_.GetAuthors().Save({AuthorId::New(), name});
}

void UnitOfWorkAddAuthor::EditAuthor (std::string&& author_id, std::string&& new_name) {
    imp_.GetAuthors().Update(std::move(author_id), std::move(new_name));
}   