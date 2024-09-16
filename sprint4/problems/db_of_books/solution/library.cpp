#include "library.h"
#include <iostream>

void Library::Init() {
    try {
    // Создаём транзакцию. Это понятие будет разобрано в следующих уроках.
        // Транзакция нужна, чтобы выполнять запросы.
        pqxx::work w(con_);

        // Используя транзакцию создадим таблицу в выбранной базе данных:
        w.exec(
            "CREATE TABLE IF NOT EXISTS books ( \
                          id SERIAL PRIMARY KEY \
                        , title varchar(100) NOT NULL \
                        , author varchar(100) NOT NULL \
                        , year integer NOT NULL \
                        , ISBN char(123) UNIQUE \
                        );"_zv);

        // Применяем все изменения
        w.commit();
        //constexpr pqxx::zview tag_add_book = "add_book"_zv;
        con_.prepare(tag_add_book, "INSERT INTO books (title, author, year, ISBN) VALUES ($1, $2, $3, CASE WHEN $4='' THEN NULL ELSE $4 END)"_zv);

    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return;
        
    }
}

Library::Answ Library::ParseJson (std::string&& stroke) {
    Answ result;
    PMAP res_map{};
    try {
        value val{ parse (stroke) };
        //value val_map = val.as_object().at("payload");
        for (auto& [key, value]: val.as_object().at("payload").as_object()) {
            if (key == "year") {
                res_map[key] = std::to_string(value_to<int>(value));
            } else if(key == "ISBN" && !value.is_string()) {
                continue;
            }
             else {
                res_map[key] = value.as_string();
            }
        }
        //PMAP res_map = value_to <PMAP> (val_map);
        // if (!result.contains("action") || ) {
        //         throw ("Bad JSON request");
        // }
        result.action = val.as_object().at("action").as_string();
        result.map = std::move(res_map);
    } catch (const std::exception& ex) {    
        out_ << ex.what() << std::endl;
    }
    
    return result;
}

void Library::AddBook(PMAP&& map) {
    //constexpr pqxx::zview tag_add_book = "add_book"_zv;
    // con_.prepare(tag_add_book, "INSERT INTO books (title, author, year, ISBN) VALUES ($1, $2, $3, CASE WHEN $4='' THEN NULL ELSE $4 END)"_zv);
    try{
        pqxx::work w(con_);
        pqxx::result result;
        if (map.contains("ISBN")) {
            result = w.exec_prepared(tag_add_book, map.at("title"), map.at("author"), map.at("year"), map.at("ISBN"));
        } else {
            result = w.exec_prepared(tag_add_book, map.at("title"), map.at("author"), map.at("year"), "");
        }
        w.commit();
        
        if (result.affected_rows() == 1) {
            object answer ( {{ "result", true }} );
            out_ << serialize (answer ) << std::endl;
        } 
    } catch (...) {
        object answer ( {{ "result", false }} );
        out_ << serialize (answer ) << std::endl;
    }
    
    
}

void Library::AllBooks() {
    array arr;
    
    pqxx::read_transaction r(con_);
    auto query_text = "SELECT id, title, author, year, ISBN FROM books ORDER BY year DESC, title ASC, author ASC, ISBN ASC"_zv;
    for (auto [id, title, author, year, ISBN] : r.query<int, std::string_view, std::string_view, int, std::optional<std::string>>(query_text)) {
        object answer;
        if (ISBN.has_value())  {
            answer = object ({{"id", id},{"title", title},{"author", author},{"year",year},{"ISBN", ISBN.value()}});
        } else {
            value jv2( nullptr );
            answer = object ({{"id", id},{"title", title},{"author", author},{"year",year},{"ISBN", jv2}});
        }
        
        arr.push_back (std::move(answer));
    }
    out_ << serialize (arr) << std::endl;
}

void Library::Run() {
    std::string stroke;
    while (getline(in_, stroke)) {
        auto ans = ParseJson (std::move(stroke));
        if (ans.action == "add_book"s ) {
            AddBook(std::move(ans.map));
        }
        else if (ans.action == "all_books"s ) {
            AllBooks();
        }
        else if (ans.action == "exit"s ) {
            //out_ << serialize (object ( {} )) << std::endl;
            break;
        }
    }
}