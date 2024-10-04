#include "base.h"
#include "../src/tagged.h"
#include <iostream>
#include "../src/logger.h"


Base::Base(int thread_num, const std::string& url)
    : pool_(thread_num, [&url](){ 
                                
                                try {
                                   //pqxx::connection con {url};
                                   return std::make_shared<pqxx::connection>(url);
                                } catch(const std::exception& ex) {
                                    std::cout << "connection sql not open" << std::endl;
                                    std::cout << "ENV " << url << std::endl;
                                    logger::LogInfoMessage ("connection sql not open");
                                }
                                return std::shared_ptr<pqxx::connection> (nullptr);
                                }
            ) 
            
{
    CreateTables ();
}

void Base::CreateTables () {
    auto connection = pool_.GetConnection();
    auto work = std::make_shared<pqxx::work> (*connection);

    
    pqxx::result res = work->exec(
        R"(
            CREATE TABLE IF NOT EXISTS retired_players (
                id UUID PRIMARY KEY,
                name varchar(100) UNIQUE NOT NULL,
                score integer,
                play_time_ms integer
            );
        )"_zv);
    
    work->exec(R"(
         CREATE INDEX IF NOT EXISTS score_idx ON retired_players (score DESC, play_time_ms, name); 
     )"_zv); 
     work->commit();

    connection.~ConnectionWrapper();
    
}




// template <typename WorkTr>
std::string Base::SaveRecord(pqxx::work& work, Record&& info) {
    std::lock_guard lock{mutex_};
    auto result_id = work.exec_params(
        R"(
            INSERT INTO retired_players (id, name, score, play_time_ms) VALUES ($1, $2, $3, $4)
            ON CONFLICT (id) DO UPDATE SET name=$2, score=$3, play_time_ms=$4
            RETURNING id;
        )"_zv,
        util::UUIDToString(info.id), info.name, info.score, info.play_time);

    pqxx::row row = *(result_id.begin());
    std::string res_id = row.at("id").c_str();
    
    return std::move(res_id);
}