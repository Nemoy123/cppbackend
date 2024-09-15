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
#include <boost/json.hpp>

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


namespace logger {


// Шаблон Декоратор

template<class SomeRequestHandler>
class LoggingRequestHandler {

    static void LogRequest(const Request& r, std::string& ip);
    static void LogResponse(auto& time, auto code, auto conttype, std::string& ip);
    static void LogResponse(auto& dur_time, auto& func);
public:
   
    void operator () (auto&& req, auto&& resp);
    explicit LoggingRequestHandler (SomeRequestHandler& deco) : decorated_(deco) {}
    
private:
    SomeRequestHandler& decorated_;
    inline static std::mutex m_;
};

void Formatter(logging::record_view const& rec, logging::formatting_ostream& strm); 
void StartServer (const uint16_t port, const std::string& adr);
void StopServer (const bool code, const std::optional<std::exception>& exc);
void LogNetError (std::string&& where, int val, std::string&& msg);
void LogInfoMessage (const std::string&& msg);

template<class SomeRequestHandler>
void LoggingRequestHandler<SomeRequestHandler>::operator () (auto&& req, auto&& resp) {
        

        std::string ip{};
        LogRequest(req,ip);
        std::chrono::system_clock::time_point start_timer = std::chrono::system_clock::now();

        auto inter_send = [&](auto&& func) {
                                
                if constexpr (std::is_same_v<http::response<http::string_body>, std::decay_t<decltype(func)>>) {
                     std::lock_guard rty (m_);
                    LogResponse(start_timer, func.result_int(), func.at(http::field::content_type), ip);
                    resp(std::move(func));
                }
                else if constexpr (std::is_same_v<http::response <http::file_body>, std::decay_t<decltype(func)>>) {
                    std::lock_guard rty (m_); 
                    LogResponse(start_timer, func.result_int(), func.at(http::field::content_type), ip);
                    resp(std::move(func));
                }
                    
        };

        decorated_(std::move(req), std::move(inter_send));

}


template<class SomeRequestHandler>
void LoggingRequestHandler<SomeRequestHandler>::LogRequest(const Request& r, std::string& ip) {
    std::lock_guard rty (m_);
    logging::add_common_attributes();
    json::object total;
    total ["message"] = "request received"s;

    json::value custom_data {std::move(total)};
    ip = r["ip"];
    total["ip"] = ip;
    total["URI"] = r.target();
    total["method"] = r.method_string();
    custom_data.as_object()["data"] = std::move(total);
      
    BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, custom_data);

}

template<class SomeRequestHandler>
void LoggingRequestHandler<SomeRequestHandler>::LogResponse(auto& time, auto code, auto conttype, std::string& ip) { 
        std::chrono::system_clock::time_point end_timer = std::chrono::system_clock::now();
    auto dur_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_timer - time);
    json::object total;
    total ["message"] = "response sent"s;

    json::value custom_data {std::move(total)};

    total["ip"] = ip;
    total["response_time"] = dur_time.count();
    total["code"] = code;
    total["content_type"] = conttype;
    custom_data.as_object()["data"] = std::move(total);
    
    BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, custom_data);
}


} // end namespace

