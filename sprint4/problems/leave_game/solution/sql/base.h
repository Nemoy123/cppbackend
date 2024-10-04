#pragma once
#include "pqxx/pqxx"
#include "pqxx/connection"
#include "pqxx/transaction"
#include "pqxx/zview.hxx"

#include "connectionpool.h"
#include <string>
#include "types.h"
#include "../src/logger.h"


using pqxx::operator"" _zv;
using namespace sql;
using namespace std::literals;



class Base {
    public:
        Base(int thread_num, const std::string& url);

        template <typename Work>
        std::vector<Record> ShowRecords(Work& work, std::optional<int> start, std::optional<int> maxitems) const;

        ConnectionPool::ConnectionWrapper GetConnection() {return pool_.GetConnection();}
        
        //template <typename Work>
        std::string SaveRecord(pqxx::work& work, Record&& info);

    private:
        ConnectionPool pool_;
        void CreateTables ();
        std::mutex mutex_;
};


template <typename Work>
std::vector<Record> Base::ShowRecords(Work& work, std::optional<int> start, std::optional<int> maxitems) const {
    std::vector<Record> result;
    try {
        std::string query_text = "SELECT id, name, score, play_time_ms FROM retired_players ORDER BY score DESC, play_time_ms ASC, name ASC";  
        if (maxitems.has_value()) {
            query_text += " LIMIT " + std::to_string(maxitems.value());
        } else {
            query_text += " LIMIT 100";
        }
        if (start.has_value ()) {
            query_text += " OFFSET " + std::to_string(start.value());
        } else {
            query_text += " OFFSET 0";
        }
        query_text += ";";
        auto resp = work.template query<std::string_view, std::string_view, int, size_t>(query_text);
            for (auto [id, name,score, play_time] : resp) { 
                result.emplace_back (util::UUIDFromString(id), std::string{name}, score, play_time);
            }
    } catch (const std::exception& ex) {
        logger::LogInfoMessage (ex.what());
    }
    return result;
}


