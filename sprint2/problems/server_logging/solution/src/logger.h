#pragma once

#include <boost/log/trivial.hpp>    // для BOOST_LOG_TRIVIAL
#include <boost/log/core.hpp>        // для logging::core
#include <boost/log/expressions.hpp> // для выражения, задающего фильтр 
#include <boost/log/utility/setup/common_attributes.hpp> //таймстемп
#include <boost/log/utility/setup/console.hpp> // изменить параметры логирования
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/beast/http.hpp>
#include <boost/log/attributes/timer.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"

#include "http_server.h"
#include <chrono>

namespace json = boost::json;
namespace logging = boost::log;
namespace keywords = boost::log::keywords;
namespace http = boost::beast::http;
using namespace std::literals;

using Request = http::request<http::string_body>;
template <typename Body, typename Fields>
using Response = http::response<http::string_body>;
using Dur = std::chrono::system_clock::duration;

BOOST_LOG_ATTRIBUTE_KEYWORD (additional_data, "AdditionalData", json::value)
BOOST_LOG_ATTRIBUTE_KEYWORD (timestamp, "TimeStamp", boost::posix_time::ptime)

// Шаблон Декоратор

template<class SomeRequestHandler>
class LoggingRequestHandler {
     
     static void LogRequest(const Request& r, std::string& ip);
     
     static void LogResponse(auto& time, auto code, auto conttype, std::string& ip);
     static void LogResponse(auto& dur_time, auto& func);
public:


    
    void operator () (auto&& req, auto&& resp) { 
        std::string ip{};
        LogRequest(req,ip);
        std::chrono::system_clock::time_point start_timer = std::chrono::system_clock::now();

        auto inter_send = [&](auto&& func) {
                
                
                // std::chrono::system_clock::time_point end_timer = std::chrono::system_clock::now();
                //     auto dur_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_timer - start_timer);
    
                if constexpr (std::is_same_v<http::response<http::string_body>, std::decay_t<decltype(func)>>) {
                    
                    LogResponse(start_timer, func.result_int(), func.at(http::field::content_type), ip);
                    resp(std::move(func));
                }
                else if constexpr (std::is_same_v<http::response <http::file_body>, std::decay_t<decltype(func)>>) {
                    
                    LogResponse(start_timer, func.result_int(), func.at(http::field::content_type), ip);
                    resp(std::move(func));
                }
                    
        };

        decorated_(std::move(req), std::move(inter_send));
    }

    LoggingRequestHandler (SomeRequestHandler& deco) : decorated_(deco) {}
    
private:
    SomeRequestHandler& decorated_;
    
};


void Formatter(logging::record_view const& rec, logging::formatting_ostream& strm) {
    
    auto ts = *rec[timestamp];
    auto obj = logging::extract<json::value>("AdditionalData", rec);
    //auto jsonobj = *obj;
    //jsonobj.as_object()["timestamp"] = to_iso_extended_string(ts);
    
    strm <<"{";
    strm << "\"timestamp\":" << json::serialize(to_iso_extended_string(ts)) << ",";
    if (obj->as_object().contains("data")) {
        strm << "\"data\":" << json::serialize(obj->at("data")) << ",";
    }
    strm << "\"message\":" << json::serialize(obj->at("message"));
    strm <<"}";

} 


template<class SomeRequestHandler>
void LoggingRequestHandler<SomeRequestHandler>::LogRequest(const Request& r, std::string& ip) {
    logging::add_common_attributes();
    json::object total;
    total ["message"] = "request received"s;

    json::value custom_data {std::move(total)};
    ip = r["ip"];
    total["ip"] = ip;
    total["URI"] = r.target();
    total["method"] = r.method_string();
    custom_data.as_object()["data"] = std::move(total);
    
    // logging::add_console_log( 
    //     std::cout,
    //     keywords::format = &Formatter,
    //     keywords::auto_flush = true
    // ); 
    
    BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, custom_data);

}

template<class SomeRequestHandler>
void LoggingRequestHandler<SomeRequestHandler>::LogResponse(auto& time, auto code, auto conttype, std::string& ip) { 
    std::chrono::system_clock::time_point end_timer = std::chrono::system_clock::now();
    auto dur_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_timer - time);
    json::object total;
    total ["message"] = "response sent"s;

    json::value custom_data {std::move(total)};

    // tempor test = r;
    total["ip"] = ip;
    total["response_time"] = dur_time.count();
    total["code"] = code;
    total["content_type"] = conttype;
    custom_data.as_object()["data"] = std::move(total);
    
    BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, custom_data);
}
//{"timestamp":"2022-09-17T01:04:59.563314","data":{"ip":"127.0.0.1","response_time":4,"code":200,"content_type":"image/svg+xml"},"message":"response sent"}

void StartServer (const unsigned short& port, const std::string& adr) {
    logging::add_common_attributes();
    json::object total;
    total ["message"] = "server started"s;
    json::value custom_data {std::move(total)};

    json::object date;
    date["port"] = port;
    date["address"] = adr;
    custom_data.as_object()["data"] = std::move(date);

    logging::add_console_log( 
        std::cout,
        keywords::format = &Formatter,
        keywords::auto_flush = true
    );
    BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, custom_data);
}

void StopServer (const bool code, const std::optional<std::exception>& exc) {
    
    json::object total;
    total ["message"] = "server exited"s;
    json::value custom_data {std::move(total)};

    json::object date;
    date["code"] = (int)code;
    if (exc.has_value()) {
        date["exception"] = exc.value().what();
    }
    custom_data.as_object()["data"] = std::move(date);

    BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, custom_data);
}

void LogNetError (const std::string&& where, const int val, const std::string&& msg) {
    
    json::object total;
    total ["message"] = "error"s;
    json::value custom_data {std::move(total)};

    json::object date;
    date["code"] = std::to_string(val);
    date["text"] = msg;
    date["where"] = where;
    custom_data.as_object()["data"] = std::move(date);

    BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, custom_data);
}

void LogInfoMessage (const std::string&& msg) {

   json::object total;
    total ["message"] = msg;
    json::value custom_data {std::move(total)};
    BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, custom_data);

}

