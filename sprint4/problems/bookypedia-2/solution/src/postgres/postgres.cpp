#include "postgres.h"

#include <pqxx/zview.hxx>

namespace postgres {

using namespace std::literals;
using pqxx::operator"" _zv;

AuthorRepositoryImpl& UnitOfWorkImpl::GetAuthors() {
    return authors_;
}
BookRepositoryImpl& UnitOfWorkImpl::GetBooks() {
    return books_;
}

void UnitOfWorkImpl::StartTransaction () {
     work_ = std::make_shared<pqxx::work> (connection_);
     authors_.SetPtrTranz(work_);
     books_.SetPtrTranz(work_);
    // pqxx::work work{connection_};
    // pqxx::zview query_text = "START TRANSACTION;"_zv;
    // work.exec (query_text);
    // work.commit();
}

void UnitOfWorkImpl::EndTransaction() {
    work_->commit();
    // pqxx::work work{connection_};
    // pqxx::zview query_text = "COMMIT;"_zv;
    // work.exec (query_text);
    // work.commit();
}

void UnitOfWorkImpl::RollbackTransaction(){
    work_->abort();
        // pqxx::work work{connection_};
        // pqxx::zview query_text = "ROLLBACK;"_zv;
        // work.exec (query_text);
        // work.commit();
}

void AuthorRepositoryImpl::SetPtrTranz (std::shared_ptr<pqxx::work>& work) {
    work_ = work;
}

void BookRepositoryImpl::SetPtrTranz (std::shared_ptr<pqxx::work>& work) {
    work_ = work;
}

std::vector <domain::Author> AuthorRepositoryImpl::ShowAuthors([[maybe_unused]]const std::string& author_name)  {
    //pqxx::read_transaction r(connection_);
    std::vector <domain::Author> result;
    std::string query_text{};
    if (author_name.empty()) {
        query_text = "SELECT id, name FROM authors ORDER BY name ASC;";  
    } else {
        query_text = "SELECT id, name FROM authors WHERE name='" + author_name + "' ORDER BY name ASC;";
    }
    for (auto [id, name] : work_->query<std::string_view, std::string_view>(query_text)) { 
            domain::Author au {util::TaggedUUID<domain::detail::AuthorTag>::FromString(std::string{id}), std::string{name}};
            result.push_back (std::move(au));
    }
    return result;
}

void AuthorRepositoryImpl::DeleteAuthor(const std::string& author_id) {
    //pqxx::work work{connection_};
    pqxx::result result_id;
    result_id = work_->exec_params(
            R"(
                DELETE FROM authors WHERE id=$1;
            )"_zv,
            author_id);
        
    //work.commit(); 
}

void AuthorRepositoryImpl::Update(std::string&& author_id, std::string&& new_name) {
    //pqxx::work work{connection_};
    pqxx::result result_id;
    result_id = work_->exec_params(
            R"(
                UPDATE authors SET name=$1 WHERE id=$2;
            )"_zv,
            new_name, author_id);
        
    //work.commit(); 
}


std::string AuthorRepositoryImpl::Save(const domain::Author& author) {
    // Пока каждое обращение к репозиторию выполняется внутри отдельной транзакции
    // В будущих уроках вы узнаете про паттерн Unit of Work, при помощи которого сможете несколько
    // запросов выполнить в рамках одной транзакции.
    // Вы также может самостоятельно почитать информацию про этот паттерн и применить его здесь.
   // pqxx::work work{connection_};
//    if (!work_) {
//         work_ = std::make_unique<pqxx::work>(connection_);
//    }
    //pqxx::work* work2_ = new pqxx::work {connection_};
    auto result_id = work_->exec_params(
        R"(
            INSERT INTO authors (id, name) VALUES ($1, $2)
            ON CONFLICT (id) DO UPDATE SET name=$2 
            RETURNING id;
        )"_zv,
        author.GetId().ToString(), author.GetName());
    //work2_->commit();
    pqxx::row row = *(result_id.begin());
    std::string res_id = row.at("id").c_str();
    
    return std::move(res_id);
}

UnitOfWorkImpl::UnitOfWorkImpl (pqxx::connection&& connection)
                        : connection_(std::move(connection))
                        , authors_(connection_)
                        , books_(connection_)
    {
        StartTransaction ();
        //pqxx::work work{connection_};
        work_->exec(R"(
                CREATE TABLE IF NOT EXISTS authors (
                    id UUID CONSTRAINT author_id_constraint PRIMARY KEY,
                    name varchar(100) UNIQUE NOT NULL
                );
            )"_zv);
        work_->exec(R"(
                CREATE TABLE IF NOT EXISTS books (
                    id UUID PRIMARY KEY,
                    author_id UUID, 
                    title varchar(100) NOT NULL,
                    publication_year integer,
                    FOREIGN KEY (author_id) REFERENCES authors (id) ON DELETE CASCADE
                );
            )"_zv);
        work_->exec(R"(
            CREATE TABLE IF NOT EXISTS book_tags (
                book_id UUID NOT NULL, 
                tag varchar(30) NOT NULL,
                FOREIGN KEY (book_id) REFERENCES books (id) ON DELETE CASCADE
            );
        )"_zv);
        
        //work.commit(); 
        EndTransaction();
    }

void BookRepositoryImpl::UpdateBook (domain::Book&& book) {
    //pqxx::work work{connection_};
    pqxx::result result_id;
    if (book.GetYear().has_value()) {
        result_id = work_->exec_params(
                R"(
                    UPDATE books SET title=$1, publication_year=$2 WHERE id=$3;
                )"_zv,
                book.GetTitle(), book.GetYear().value(), book.GetId().ToString());
    } else {
        result_id = work_->exec_params(
                R"(
                    UPDATE books SET title=$1 WHERE id=$3;
                )"_zv,
                book.GetTitle(), book.GetId().ToString());
    }
        
    //work.commit(); 
}

std::string BookRepositoryImpl::Save(const domain::Book& book) {
   // pqxx::work work{connection_};
    pqxx::result result_id;
    if (book.GetYear().has_value()) {
        result_id = work_->exec_params(
            R"(
                INSERT INTO books (id, author_id, title, publication_year) VALUES ($1, $2, $3, $4)
                ON CONFLICT (id) DO UPDATE SET author_id=$2, title=$3, publication_year=$4
                RETURNING id;
            )"_zv,
            book.GetId().ToString(), book.GetAuthorId(), book.GetTitle(), book.GetYear().value());
    } else {
        result_id = work_->exec_params(
            R"(
                INSERT INTO books (id, author_id, title) VALUES ($1, $2, $3)
                ON CONFLICT (id) DO UPDATE SET author_id=$2, title=$3
                RETURNING id;
            )"_zv,
            book.GetId().ToString(), book.GetAuthorId(), book.GetTitle());
    }
    
   // work.commit();
    if (result_id.empty()) {
        return {};
    }
    auto [res_id] = result_id.begin().as<std::string>();
    return std::move(res_id);
}

void BookRepositoryImpl::SaveTags(const std::string& book_id, std::vector <std::string>&& tags) {
    for (auto& tag : tags) {
       //pqxx::work work{connection_};
        pqxx::result result_id;
        
        result_id = work_->exec_params(
                R"(
                    INSERT INTO book_tags (book_id, tag) VALUES ($1, $2);
                )"_zv,
                book_id, std::move(tag) );
          
        //work.commit(); 
    } 
}

std::vector <domain::Book> BookRepositoryImpl::ShowBooks([[maybe_unused]]const std::string& author_id) const {
    //pqxx::read_transaction r(connection_);
    std::vector <domain::Book> result;
    pqxx::zview query_text;
    pqxx::result res;
    
    if (!author_id.empty()) {
        query_text = R"(SELECT books.id, authors.name, books.title, books.publication_year FROM books, authors  
        WHERE books.author_id=$1 AND authors.id=$1 ORDER BY books.title ASC, authors.name ASC, books.publication_year ASC;)"_zv;
        res = work_->exec_params(query_text,author_id);
    } else {
        query_text = R"(SELECT books.id, authors.name, books.title, books.publication_year
         FROM books, authors WHERE books.author_id=authors.id ORDER BY books.title ASC, authors.name ASC, books.publication_year ASC;)"_zv;
        res = work_->exec(query_text);
    }
    
    res.for_each ([&result](std::string_view id, std::string_view name, std::string_view title, std::optional<int> year){
        if (year.has_value()) {
            result.emplace_back (id, name, title, year.value());
        } else {
            result.emplace_back (id, name, title, std::nullopt);
        }
        
    });

    return result;
}


std::vector <ui::detail::BookInfo> BookRepositoryImpl::FindBooksInfoByTitle (std::string&& title) const {
    //pqxx::read_transaction r(connection_);
    std::vector <ui::detail::BookInfo> result;
    pqxx::zview query_text;
    pqxx::result res;
    query_text = R"(SELECT books.id, authors.name, books.publication_year FROM books, authors  
        WHERE books.title=$1 AND authors.id=books.author_id ORDER BY authors.name ASC, books.publication_year ASC;)"_zv;
        res = work_->exec_params(query_text,std::string{title});

    res.for_each ([&result, &title](std::string_view book_id, std::string_view author_name, std::optional<int> year){
        ui::detail::BookInfo info;
        info.id = book_id;
        info.author = author_name;
        info.publication_year = year;
        info.title = std::string{title};
        result.push_back (std::move(info));
    }); 
    return result;   
}

ui::detail::BookInfo BookRepositoryImpl::GetInfoBookId (std::string&& book_id) const {
    //pqxx::read_transaction r(connection_);
    ui::detail::BookInfo result;
    pqxx::zview query_text;
    pqxx::result res;
    query_text = R"(SELECT title, authors.name, books.publication_year, authors.id FROM books, authors  
        WHERE books.id=$1 AND authors.id=books.author_id;)"_zv;
        res = work_->exec_params(query_text,std::string{book_id});
    
    auto [title, author, year, author_id] = res.begin().as<std::string_view, std::string_view, std::optional<int>, std::string_view>();
    result.author = author;
    result.title = title;
    result.publication_year = year;
    result.author_id = author_id;
    query_text = R"(SELECT tag FROM book_tags WHERE book_id=$1 ORDER BY tag ASC;)"_zv; //
        res = work_->exec_params(query_text,std::move(book_id));
    res.for_each ([&res, &result](std::string_view tag){
        result.tags.push_back(std::string{tag});
    });     

    return result;

}

void BookRepositoryImpl::DeleteTagsByBook (const std::string& book_id) {
    //pqxx::work work{connection_};
    pqxx::result result_id;
    result_id = work_->exec_params(
            R"(
                DELETE FROM book_tags WHERE book_id=$1;
            )"_zv,
            book_id);
        
    //work.commit(); 
}

void BookRepositoryImpl::DeleteBook(const domain::Book& book) {
    //pqxx::work work{connection_};
    pqxx::result result_id;
    DeleteTagsByBook (book.GetId().ToString());

    result_id = work_->exec_params(
            R"(
                DELETE FROM books WHERE id=$1;
            )"_zv,
            book.GetId().ToString());
        
   // work.commit(); 
}

}  // namespace postgres