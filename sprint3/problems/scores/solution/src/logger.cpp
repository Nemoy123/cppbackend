#include "logger.h"

namespace logger {

void Formatter(logging::record_view const& rec, logging::formatting_ostream& strm) {
    
    auto ts = *rec[timestamp];
    auto obj = logging::extract<json::value>("AdditionalData", rec);
       
    strm <<"{";
    strm << "\"timestamp\":" << json::serialize(to_iso_extended_string(ts)) << ",";
    if (obj->as_object().contains("data")) {
        strm << "\"data\":" << json::serialize(obj->at("data")) << ",";
    }
    strm << "\"message\":" << json::serialize(obj->at("message"));
    strm <<"}";

} 

void StopServer (const bool code, const std::optional<std::exception>& exc) {
    
    json::object total;
    total ["message"] = "server exited"s;
    json::value custom_data {std::move(total)};

    json::object date;
    date["code"] = static_cast<int>(code);
    if (exc.has_value()) {
        date["exception"] = exc.value().what();
    }
    custom_data.as_object()["data"] = std::move(date);

    BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, custom_data);
}

void StartServer (const uint16_t port, const std::string& adr) {
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

void LogNetError (std::string&& where,int val, std::string&& msg) {
    
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



} // end namespace