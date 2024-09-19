#include "postgres.h"

#include <pqxx/zview.hxx>

namespace postgres {

using namespace std::literals;
using pqxx::operator"" _zv;

std::vector <domain::Author> AuthorRepositoryImpl::ShowAuthors()  {
    pqxx::read_transaction r(connection_);
    std::vector <domain::Author> result;
    pqxx::zview query_text = "SELECT id, name FROM authors ORDER BY name ASC;"_zv;
    for (auto [id, name] : r.query<std::string_view, std::string_view>(query_text)) { 
        domain::Author au {util::TaggedUUID<domain::detail::AuthorTag>::FromString(std::string{id}), std::string{name}};
        result.push_back (std::move(au));
    }
    return result;
}

void AuthorRepositoryImpl::Save(const domain::Author& author) {
    // Пока каждое обращение к репозиторию выполняется внутри отдельной транзакции
    // В будущих уроках вы узнаете про паттерн Unit of Work, при помощи которого сможете несколько
    // запросов выполнить в рамках одной транзакции.
    // Вы также может самостоятельно почитать информацию про этот паттерн и применить его здесь.
    pqxx::work work{connection_};
    work.exec_params(
        R"(
            INSERT INTO authors (id, name) VALUES ($1, $2)
            ON CONFLICT (id) DO UPDATE SET name=$2;
        )"_zv,
        author.GetId().ToString(), author.GetName());
    work.commit();
}

Database::Database(pqxx::connection connection)
    : connection_{std::move(connection)} {
    pqxx::work work{connection_};
    work.exec(R"(
            CREATE TABLE IF NOT EXISTS authors (
                id UUID CONSTRAINT author_id_constraint PRIMARY KEY,
                name varchar(100) UNIQUE NOT NULL
            );
        )"_zv);
    work.exec(R"(
            CREATE TABLE IF NOT EXISTS books (
                id UUID PRIMARY KEY,
                author_id UUID, 
                title varchar(100) NOT NULL,
                publication_year integer,
                FOREIGN KEY (author_id) REFERENCES authors (id) ON DELETE CASCADE
            );
        )"_zv);
    
    work.commit();
}

void BookRepositoryImpl::Save(const domain::Book& book) {
    pqxx::work work{connection_};
    if (book.GetYear().has_value()) {
        work.exec_params(
            R"(
                INSERT INTO books (id, author_id, title, publication_year) VALUES ($1, $2, $3, $4)
                ON CONFLICT (id) DO UPDATE SET author_id=$2, title=$3, publication_year=$4;
            )"_zv,
            book.GetId().ToString(), book.GetAuthorId(), book.GetTitle(), book.GetYear().value());
    } else {
        work.exec_params(
            R"(
                INSERT INTO books (id, author_id, title) VALUES ($1, $2, $3)
                ON CONFLICT (id) DO UPDATE SET author_id=$2, title=$3;
            )"_zv,
            book.GetId().ToString(), book.GetAuthorId(), book.GetTitle());
    }
    
    work.commit();
}

std::vector <domain::Book> BookRepositoryImpl::ShowBooks([[maybe_unused]]const std::string& author_id) const {
    pqxx::read_transaction r(connection_);
    std::vector <domain::Book> result;
    pqxx::zview query_text;
    pqxx::result res;
    if (!author_id.empty()) {
        query_text = "SELECT id, author_id, title, publication_year FROM books WHERE author_id=$1 ORDER BY publication_year ASC, title ASC;"_zv;
        res = r.exec_params(query_text,author_id);
    } else {
        query_text = "SELECT id, author_id, title, publication_year FROM books ORDER BY title ASC;"_zv;
        res = r.exec(query_text);
    }
    
    res.for_each ([&result](std::string_view id, std::string_view name, std::string_view title, std::optional<int> year){
        if (year.has_value()) {
            result.emplace_back (id, name, title, year.value());
        } else {
            result.emplace_back (id, name, title, std::nullopt);
        }
        
    });

    // for (auto [id, name, title, year] : r.query<std::string_view, std::string_view, std::string_view, int>(query_text)) { 
    //     //domain::Book { id, name, title, year};
    //     result.emplace_back (id, name, title, year);
    // }
    return result;
}

}  // namespace postgres